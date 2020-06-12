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
// cl_fx.c -- particle effects parsing and management

#include "client.h"
#include "particles.h"

void CL_LogoutEffect (vec3_t org, int type);
void CL_ItemRespawnParticles (vec3_t org);
void thinkDecalAlpha (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time);
void vectoanglerolled (vec3_t value1, float angleyaw, vec3_t angles);

//Knightmare- Psychospaz's smoke
void CL_GunSmokeEffect (vec3_t org, vec3_t dir);

static vec3_t avelocities [NUMVERTEXNORMALS];

extern	struct model_s	*cl_mod_smoke;
extern	struct model_s	*cl_mod_flash;

//here i convert old 256 color to RGB
const byte default_pal[768] =
{
0,0,0,15,15,15,31,31,31,47,47,47,63,63,63,75,75,75,91,91,91,107,107,107,123,123,123,139,139,139,155,155,155,171,171,171,187,187,187,203,203,203,219,219,219,235,235,235,99,75,35,91,67,31,83,63,31,79,59,27,71,55,27,63,47,
23,59,43,23,51,39,19,47,35,19,43,31,19,39,27,15,35,23,15,27,19,11,23,15,11,19,15,7,15,11,7,95,95,111,91,91,103,91,83,95,87,79,91,83,75,83,79,71,75,71,63,67,63,59,59,59,55,55,51,47,47,47,43,43,39,
39,39,35,35,35,27,27,27,23,23,23,19,19,19,143,119,83,123,99,67,115,91,59,103,79,47,207,151,75,167,123,59,139,103,47,111,83,39,235,159,39,203,139,35,175,119,31,147,99,27,119,79,23,91,59,15,63,39,11,35,23,7,167,59,43,
159,47,35,151,43,27,139,39,19,127,31,15,115,23,11,103,23,7,87,19,0,75,15,0,67,15,0,59,15,0,51,11,0,43,11,0,35,11,0,27,7,0,19,7,0,123,95,75,115,87,67,107,83,63,103,79,59,95,71,55,87,67,51,83,63,
47,75,55,43,67,51,39,63,47,35,55,39,27,47,35,23,39,27,19,31,23,15,23,15,11,15,11,7,111,59,23,95,55,23,83,47,23,67,43,23,55,35,19,39,27,15,27,19,11,15,11,7,179,91,79,191,123,111,203,155,147,215,187,183,203,
215,223,179,199,211,159,183,195,135,167,183,115,151,167,91,135,155,71,119,139,47,103,127,23,83,111,19,75,103,15,67,91,11,63,83,7,55,75,7,47,63,7,39,51,0,31,43,0,23,31,0,15,19,0,7,11,0,0,0,139,87,87,131,79,79,
123,71,71,115,67,67,107,59,59,99,51,51,91,47,47,87,43,43,75,35,35,63,31,31,51,27,27,43,19,19,31,15,15,19,11,11,11,7,7,0,0,0,151,159,123,143,151,115,135,139,107,127,131,99,119,123,95,115,115,87,107,107,79,99,99,
71,91,91,67,79,79,59,67,67,51,55,55,43,47,47,35,35,35,27,23,23,19,15,15,11,159,75,63,147,67,55,139,59,47,127,55,39,119,47,35,107,43,27,99,35,23,87,31,19,79,27,15,67,23,11,55,19,11,43,15,7,31,11,7,23,
7,0,11,0,0,0,0,0,119,123,207,111,115,195,103,107,183,99,99,167,91,91,155,83,87,143,75,79,127,71,71,115,63,63,103,55,55,87,47,47,75,39,39,63,35,31,47,27,23,35,19,15,23,11,7,7,155,171,123,143,159,111,135,151,99,
123,139,87,115,131,75,103,119,67,95,111,59,87,103,51,75,91,39,63,79,27,55,67,19,47,59,11,35,47,7,27,35,0,19,23,0,11,15,0,0,255,0,35,231,15,63,211,27,83,187,39,95,167,47,95,143,51,95,123,51,255,255,255,255,255,
211,255,255,167,255,255,127,255,255,83,255,255,39,255,235,31,255,215,23,255,191,15,255,171,7,255,147,0,239,127,0,227,107,0,211,87,0,199,71,0,183,59,0,171,43,0,155,31,0,143,23,0,127,15,0,115,7,0,95,0,0,71,0,0,47,
0,0,27,0,0,239,0,0,55,55,255,255,0,0,0,0,255,43,43,35,27,27,23,19,19,15,235,151,127,195,115,83,159,87,51,123,63,27,235,211,199,199,171,155,167,139,119,135,107,87,159,91,83
};

//this initializes all particle images - mods play with this...
void SetParticleImages (void)
{
	R_SetParticlePicture(particle_generic,		"gfx/particles/basic.tga");
	R_SetParticlePicture(particle_freongeneric,		"gfx/particles/freonbasic.tga");
	R_SetParticlePicture(particle_smoke,		"gfx/particles/smoke.tga");
	R_SetParticlePicture(particle_blood,		"gfx/particles/blood.tga");
	R_SetParticlePicture(particle_blooddrop,	"gfx/particles/blood_drop.tga");
	R_SetParticlePicture(particle_blooddrip,	"gfx/particles/blood_drip.tga");
	R_SetParticlePicture(particle_redblood,		"gfx/particles/blood_red.tga");
	R_SetParticlePicture(particle_bubble,		"gfx/particles/bubble.tga");
	R_SetParticlePicture(particle_blaster,		"gfx/particles/blaster.tga");
	R_SetParticlePicture(particle_beam,			"gfx/particles/beam.tga");
	R_SetParticlePicture(particle_beam2,		"gfx/particles/beam2.tga"); // only used for railgun
	R_SetParticlePicture(particle_lightning,	"gfx/particles/lightning.tga");
	R_SetParticlePicture(particle_lensflare,	"gfx/particles/lensflare.tga");
	R_SetParticlePicture(particle_lightflare,	"gfx/particles/lightflare.jpg");
	R_SetParticlePicture(particle_inferno,		"gfx/particles/inferno.tga");
	R_SetParticlePicture(particle_shield,		"gfx/particles/shield.jpg");
	//animations
	//rocket explosion
	R_SetParticlePicture(particle_rflash,		"gfx/particles/r_flash.tga");
	R_SetParticlePicture(particle_rexplosion1,	"gfx/particles/r_explod_1.tga");
	R_SetParticlePicture(particle_rexplosion2,	"gfx/particles/r_explod_2.tga");
	R_SetParticlePicture(particle_rexplosion3,	"gfx/particles/r_explod_3.tga");
	R_SetParticlePicture(particle_rexplosion4,	"gfx/particles/r_explod_4.tga");
	R_SetParticlePicture(particle_rexplosion5,	"gfx/particles/r_explod_5.tga");
	R_SetParticlePicture(particle_rexplosion6,	"gfx/particles/r_explod_6.tga");
	R_SetParticlePicture(particle_rexplosion7,	"gfx/particles/r_explod_7.tga");
	//disruptor explosion		
	R_SetParticlePicture(particle_dexplosion1,	"gfx/particles/d_explod_1.tga");
	R_SetParticlePicture(particle_dexplosion2,	"gfx/particles/d_explod_2.tga");
	R_SetParticlePicture(particle_dexplosion3,	"gfx/particles/d_explod_3.tga");

	R_SetParticlePicture(particle_bulletmark,	"gfx/decals/bulletmark.tga");
	R_SetParticlePicture(particle_shadow,		"gfx/decals/shadow.tga");
	R_SetParticlePicture(particle_burnmark,		"gfx/decals/burnmark.tga");
	R_SetParticlePicture(particle_blooddecal1,	"gfx/decals/blood_1.tga");
	R_SetParticlePicture(particle_blooddecal2,	"gfx/decals/blood_2.tga");
	R_SetParticlePicture(particle_blooddecal3,	"gfx/decals/blood_3.tga");
	R_SetParticlePicture(particle_blooddecal4,	"gfx/decals/blood_4.tga");
	R_SetParticlePicture(particle_blooddecal5,	"gfx/decals/blood_5.tga");
	R_SetParticlePicture(particle_footprint,	"gfx/decals/footprint.tga");
	R_SetParticlePicture(particle_glass,		"gfx/particles/glass.tga");
	R_SetParticlePicture(particle_glass2,		"gfx/particles/glass2.tga");

	R_SetParticlePicture(particle_music1,		"gfx/particles/music1.tga");
	R_SetParticlePicture(particle_music2,		"gfx/particles/music2.tga");
	R_SetParticlePicture(particle_music3,		"gfx/particles/music3.tga");

	R_SetParticlePicture(particle_heart,		"gfx/particles/heart.tga");

	R_SetParticlePicture(particle_shred0,		"models/monsters/drone/shred0.png");
	R_SetParticlePicture(particle_shred1,		"models/monsters/drone/shred1.png");
	R_SetParticlePicture(particle_shred2,		"models/monsters/drone/shred2.png");
}

int particleBlood(void)
{
	return particle_blooddecal1 + rand()%5;
}


void pRotateThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time);

int	color8red (int color8)
{ 
	return (default_pal[color8*3+0]);
}
int	color8green (int color8)
{
	return (default_pal[color8*3+1]);;
}
int	color8blue (int color8)
{
	return (default_pal[color8*3+2]);;
}

/*
==============================================================

PARTICLE MANAGEMENT

==============================================================
*/

float newParticleTime ()
{
	float lerpedTime;

	lerpedTime = cl.time;

	return lerpedTime;
}


cparticle_t	*active_particles, *free_particles;
cparticle_t	particles[MAX_PARTICLES];
int			cl_numparticles = MAX_PARTICLES;

#ifdef DECALS
//=================================================
decalpolys_t	*active_decals, *free_decals;
decalpolys_t	decalfrags[MAX_DECAL_FRAGS];
int				cl_numdecalfrags = MAX_DECAL_FRAGS;

/*
===============
CleanDecalPolys
Cleans up the active_particles linked list
===============
*/
void CleanDecalPolys (void)
{
	decalpolys_t		*d, *next;
	decalpolys_t		*active = NULL, *tail = NULL;

	for (d=active_decals; d; d=next)
	{
		next = d->next;
		if (d->clearflag) {
			d->clearflag = false;
			d->numpolys = 0;
			d->nextpoly = NULL;
			d->node = NULL; // vis node
			d->next = free_decals;
			free_decals = d;
			continue;
		}
		d->next = NULL;
		if (!tail)
			active = tail = d;
		else {
			tail->next = d;
			tail = d;
		}
	}
	active_decals = active;
}

/*
===============
ClearDecalPoly
Recursively flags a decal poly chain for cleaning
===============
*/
void ClearDecalPoly (decalpolys_t *decal)
{
	if (!decal)
		return;
	if (decal->nextpoly)
		ClearDecalPoly(decal->nextpoly);
	decal->clearflag = true; // tell cleaning loop to clean this up
}

/*
===============
CL_ClearAllDecalPolys
Clears all decal polys
===============
*/
void CL_ClearAllDecalPolys (void)
{
	int		i;
	free_decals = &decalfrags[0];
	active_decals = NULL;

	for (i=0 ;i < cl_numdecalfrags ; i++) {
		decalfrags[i].next = &decalfrags[i+1];
		decalfrags[i].clearflag = false;
		decalfrags[i].numpolys = 0;
		decalfrags[i].nextpoly = NULL;
		decalfrags[i].node = NULL; // vis node
	}
	decalfrags[cl_numdecalfrags-1].next = NULL;
}
/*
===============
NumFreeDecalPolys
Retuns number of available decalpoly_t fields
===============
*/
int NumFreeDecalPolys (void)
{
	int count = 0;
	decalpolys_t *d = NULL;
	for (d = free_decals; d; d = d->next)
		count++;
	return count;
}
/*
===============
NewDecalPoly
Retuns first free decal poly
===============
*/
decalpolys_t *NewDecalPoly(void)
{
	decalpolys_t *d = NULL;
	if (!free_decals)
		return NULL;

	d = free_decals;
	free_decals = d->next;
	d->next = active_decals;
	active_decals = d;
	return d;
}

/*
===============
clipDecal
===============
*/
void clipDecal (cparticle_t *part, float radius, float orient, vec3_t origin, vec3_t dir)
{
	vec3_t	axis[3], verts[MAX_DECAL_VERTS];
	int		numfragments, j, i;
	markFragment_t *fr, fragments[MAX_FRAGMENTS_PER_DECAL];
	
	// invalid decal
	if ( radius <= 0 || VectorCompare (dir, vec3_origin) ) 
		return;

	// calculate orientation matrix
	VectorNormalize2 ( dir, axis[0] );
	PerpendicularVector ( axis[1], axis[0] );
	RotatePointAroundVector ( axis[2], axis[0], axis[1], orient );
	CrossProduct ( axis[0], axis[2], axis[1] );

	numfragments = R_MarkFragments (origin, axis, radius, MAX_DECAL_VERTS, verts, 
		MAX_FRAGMENTS_PER_DECAL, fragments);

	if (!numfragments)
		return;
	if (numfragments > NumFreeDecalPolys()) // not enough decalpolys free
		return;
	
	VectorScale ( axis[1], 0.5f / radius, axis[1] );
	VectorScale ( axis[2], 0.5f / radius, axis[2] );

	part->decalnum = numfragments;
	for ( i = 0, fr = fragments; i < numfragments; i++, fr++ )
	{
		decalpolys_t *decal = NewDecalPoly();
		vec3_t v;

		if (!decal)
			return;
		decal->nextpoly = part->decal;
		part->decal = decal;
		//Com_Printf("Number of verts in fragment: %i\n", fr->numPoints);
		decal->node = fr->node; // vis node

		for ( j = 0; j < fr->numPoints && j < MAX_VERTS_PER_FRAGMENT; j++ )
		{
			VectorCopy ( verts[fr->firstPoint+j], decal->polys[j] );
			VectorSubtract ( decal->polys[j], origin, v );
			decal->coords[j][0] = DotProduct ( v, axis[1] ) + 0.5f;
			decal->coords[j][1] = DotProduct ( v, axis[2] ) + 0.5f;
			decal->numpolys = fr->numPoints;
		}
	}
}
#endif

/*
===============
setupParticle
===============
*/
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
			void (*think)(cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time),
			qboolean thinknext)
{
	int j;
	cparticle_t	*p = NULL;

	if (!free_particles)
		return NULL;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->time = cl.time;

	p->angle[0]=angle0;
	p->angle[1]=angle1;
	p->angle[2]=angle2;

	p->org[0]=org0;
	p->org[1]=org1;
	p->org[2]=org2;
	p->oldorg[0]=org0;
	p->oldorg[1]=org1;
	p->oldorg[2]=org2;

	p->vel[0]=vel0;
	p->vel[1]=vel1;
	p->vel[2]=vel2;

	p->accel[0]=accel0;
	p->accel[1]=accel1;
	p->accel[2]=accel2;

	p->color[0]=color0;
	p->color[1]=color1;
	p->color[2]=color2;

	p->colorvel[0]=colorvel0;
	p->colorvel[1]=colorvel1;
	p->colorvel[2]=colorvel2;

	p->blendfunc_src = blendfunc_src;
	p->blendfunc_dst = blendfunc_dst;

	p->alpha=alpha;
	p->alphavel=alphavel;
	p->size=size;
	p->sizevel=sizevel;

	p->image=image;
	p->flags=flags;

	p->src_ent=0;
	p->dst_ent=0;

	if (think)
		p->think = think;
	else
		p->think = NULL;
	p->thinknext=thinknext;

	for (j=0;j<P_LIGHTS_MAX;j++)
	{
		cplight_t *plight = &p->lights[j];
		plight->isactive = false;
		plight->light = 0;
		plight->lightvel = 0;
		plight->lightcol[0] = 0;
		plight->lightcol[1] = 0;
		plight->lightcol[2] = 0;
	}
#ifdef DECALS
	p->decalnum = 0;
	p->decal = NULL;

	if (flags & PART_DECAL)
	{
		vec3_t dir;
		AngleVectors (p->angle, dir, NULL, NULL);
		VectorNegate(dir, dir);
		clipDecal(p, p->size, -p->angle[2], p->org, dir);

		if (!p->decalnum) // kill on viewframe
			p->alpha = 0;
	}
#endif
	return p;
}


/*
===============
addParticleLight
===============
*/
void addParticleLight (cparticle_t *p,
				  float light, float lightvel,
				  float lcol0, float lcol1, float lcol2)
{
	int i;

	for (i=0; i<P_LIGHTS_MAX; i++)
	{
		cplight_t *plight = &p->lights[i];
		if (!plight->isactive)
		{
			plight->isactive = true;
			plight->light = light;
			plight->lightvel = lightvel;
			plight->lightcol[0] = lcol0;
			plight->lightcol[1] = lcol1;
			plight->lightcol[2] = lcol2;
			return;
		}
	}
}


/*
===============
CL_ClearParticles
===============
*/
void CL_ClearParticles (void)
{
	int		i;
	
	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i < cl_numparticles ; i++) {
		particles[i].next = &particles[i+1];
#ifdef DECALS
		particles[i].decalnum = 0; // Knightmare added
		particles[i].decal = NULL; // Knightmare added
#endif
	}
	particles[cl_numparticles-1].next = NULL;
}


/*
===============
GENERIC PARTICLE THINKING ROUTINES
===============
*/
//#define SplashSize		10
#define	STOP_EPSILON	0.1

void calcPartVelocity(cparticle_t *p, float scale, float *time, vec3_t velocity)
{
	float time1 = *time;
	float time2 = time1*time1;

	velocity[0] = scale * (p->vel[0]*time1 + (p->accel[0])*time2);
	velocity[1] = scale * (p->vel[1]*time1 + (p->accel[1])*time2);

	if (p->flags & PART_GRAVITY)
		velocity[2] = scale * (p->vel[2]*time1 + (p->accel[2]-(PARTICLE_GRAVITY))*time2);
	else
		velocity[2] = scale * (p->vel[2]*time1 + (p->accel[2])*time2);
}

void ClipVelocity (vec3_t in, vec3_t normal, vec3_t out)
{
	float	backoff, change;
	int		i;
	
	backoff = VectorLength(in)*0.25 + DotProduct (in, normal) * 3.0f;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
}

void pBounceThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	float clipsize;
	trace_t tr;
	vec3_t velocity;

	clipsize = *size*0.5;
	if (clipsize<0.25) clipsize = 0.25;
	tr = CL_BrushTrace (p->oldorg, org, clipsize, MASK_SOLID); // was 1
	
	if (tr.fraction < 1)
	{
		calcPartVelocity(p, 1, time, velocity);
		ClipVelocity(velocity, tr.plane.normal, p->vel);

		VectorCopy(vec3_origin, p->accel);
		VectorCopy(tr.endpos, p->org);
		VectorCopy(p->org, org);
		VectorCopy(p->org, p->oldorg);

		p->alpha = *alpha;
		p->size = *size;

		p->start = p->time = newParticleTime();

		if (p->flags&PART_GRAVITY && VectorLength(p->vel)<2)
			p->flags &= ~PART_GRAVITY;
	}

	p->thinknext = true;
}

/*
===============
CL_LightningBeam
===============
*/
/*void CL_LightningBeam(vec3_t start, vec3_t end, int srcEnt, int dstEnt, float size)
{
	cparticle_t *list;
	cparticle_t *p=NULL;
	
	for (list=active_particles ; list ; list=list->next)
		if (list->src_ent == srcEnt && list->dst_ent == dstEnt )
		{
			p=list;
			p->time = cl.time;
			VectorCopy(start, p->angle);
			VectorCopy(end, p->org);

			if (p->link)
			{
				p->link->time = cl.time;
				VectorCopy(start, p->link->angle);
				VectorCopy(end, p->link->org);
			}
			else
			{
				p->link = setupParticle (
					start[0],	start[1],	start[2],
					end[0],		end[1],		end[2],
					0,	0,	0,
					0,		0,		0,
					150,	150,	255,
					0,	0,	0,
					1,		-1.0,
					size,		0,			
					particle_beam,
					PART_BEAM,
					NULL,0);
				
			}
			break;
		}

	if (p)
		return;

	p = setupParticle (
		start[0],	start[1],	start[2],
		end[0],		end[1],		end[2],
		0,	0,	0,
		0,		0,		0,
		115,	115,	255,
		0,	0,	0,
		1,		-1.0,
		size,		0,			
		particle_lightning,
		PART_LIGHTNING,
		NULL,0);

	p->src_ent=srcEnt;
	p->dst_ent=dstEnt;

	p->link = setupParticle (
		start[0],	start[1],	start[2],
		end[0],		end[1],		end[2],
		0,	0,	0,
		0,		0,		0,
		150,	150,	255,
		0,	0,	0,
		1,		-1.0,
		size,		0,			
		particle_beam,
		PART_BEAM,
		NULL,0);
}*/


void CL_LightningBeam (vec3_t start, vec3_t end, int srcEnt, int dstEnt, float size)
{
	cparticle_t *list;
	cparticle_t *p=NULL;

	for (list=active_particles ; list ; list=list->next)
		if (list->src_ent == srcEnt && list->dst_ent == dstEnt && list->image == particle_lightning)
		{
			p=list;
			/*p->start =*/ p->time = cl.time;
			VectorCopy(start, p->angle);
			VectorCopy(end, p->org);

			return;
		}

	p = setupParticle (
		start[0],	start[1],	start[2],
		end[0],		end[1],		end[2],
		0,	0,	0,
		0,		0,		0,
		255,	255,	255,
		0,	0,	0,
		1,		-2,
		GL_SRC_ALPHA, GL_ONE,
		size,		0,		
		particle_lightning,
		PART_LIGHTNING,
		0, false);

	if (!p)
		return;

	p->src_ent=srcEnt;
	p->dst_ent=dstEnt;
}


/*
===============
CL_Explosion_Decal
===============
*/
#ifdef DECALS
void CL_Explosion_Decal (vec3_t org, float size)
{
	if (r_decals->value)
	{
		int i, j;
		cparticle_t *p;
		vec3_t angle[6], ang;
		trace_t	trace1, trace2;
		vec3_t end1, end2, normal, sorg, dorg;
		vec3_t	planenormals[6];

		VectorSet(angle[0], -1, 0, 0);
		VectorSet(angle[1], 1, 0, 0);
		VectorSet(angle[2], 0, 1, 0);
		VectorSet(angle[3], 0, -1, 0);
		VectorSet(angle[4], 0, 0, 1);
		VectorSet(angle[5], 0, 0, -1);

		for (i=0;i<6;i++)
		{
			VectorMA(org, -2, angle[i], sorg); // move origin 2 units back
			VectorMA(sorg, size/2, angle[i], end1);
			trace1 = CL_Trace (sorg, end1, 0, CONTENTS_SOLID);
			if (trace1.fraction < 1) // hit a surface
			{	// make sure we haven't hit this plane before
				VectorCopy(trace1.plane.normal, planenormals[i]);
				for (j=0; j<i; j++)
					if (VectorCompare(planenormals[j],planenormals[i])) continue;
				// try tracing directly to hit plane
				VectorNegate(trace1.plane.normal, normal);
				VectorMA(sorg, size/2, normal, end2);
				trace2 = CL_Trace (sorg, end2, 0, CONTENTS_SOLID);
				// if seond trace hit same plane
				if (trace2.fraction < 1 && VectorCompare(trace2.plane.normal, trace1.plane.normal))
					VectorCopy(trace2.endpos, dorg);
				else
					VectorCopy(trace1.endpos, dorg);
				//if (CM_PointContents(dorg,0) & MASK_WATER) // no scorch marks underwater
				//	continue;
				vectoanglerolled(normal, rand()%360, ang);
				p = setupParticle (
					ang[0],	ang[1],	ang[2],
					dorg[0],dorg[1],dorg[2],
					0,		0,		0,
					0,		0,		0,
					255,	255,	255,
					0,		0,		0,
					1,		-1/r_decal_life->value,
					GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
					size,		0,			
					particle_burnmark,
					PART_SHADED|PART_DECAL|PART_ALPHACOLOR,
					thinkDecalAlpha, true);
			}
			/*vectoanglerolled(angle[i], rand()%360, ang);
			p = setupParticle (
				ang[0],	ang[1],	ang[2],
				org[0],	org[1],	org[2],
				0,		0,		0,
				0,		0,		0,
				255,	255,	255,
				0,		0,		0,
				1,		-0.001,
				GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
				size,		0,			
				particle_burnmark,
				PART_SHADED|PART_DECAL|PART_ALPHACOLOR,
				thinkDecalAlpha, true);*/
		}
	}
}
#endif


/*
===============
CL_Explosion_Particle

Explosion effect
===============
*/
void pExplosionThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	if (*alpha>.85)
		*image = particle_rexplosion1;
	else if (*alpha>.7)
		*image = particle_rexplosion2;
	else if (*alpha>.5)
		*image = particle_rexplosion3;
	else if (*alpha>.4)
		*image = particle_rexplosion4;
	else if (*alpha>.25)
		*image = particle_rexplosion5;
	else if (*alpha>.1)
		*image = particle_rexplosion6;
	else 
		*image = particle_rexplosion7;

	*alpha *= 3.0;

	if (*alpha > 1.0)
		*alpha = 1;

	p->thinknext = true;
}

void pExplosionBubbleThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{

	if (CM_PointContents(org,0) & MASK_WATER)
		p->thinknext = true;
	else
	{
		p->think = NULL;
		p->alpha = 0;
	}
}

void CL_Explosion_Particle (vec3_t org, float size, qboolean rocket)
{
	cparticle_t *p;

	p = setupParticle (
		0,		0,		0,
		org[0],	org[1],	org[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		1,		(rocket)? -2 : -1.5,
		GL_SRC_ALPHA, GL_ONE,
		//GL_ONE, GL_ONE,
		(size!=0)?size:(150-(!rocket)?75:0),	0,			
		particle_rexplosion1,
		PART_DEPTHHACK_SHORT,
		pExplosionThink, true);
	
	if (p)
	{	// smooth color blend :D
	/*	addParticleLight (p, 225, 0, 1, 0, 0);
		addParticleLight (p, 250, 0, 1, 0.3, 0);
		addParticleLight (p, 275, 0, 1, 0.6, 0);
		addParticleLight (p, 300, 0, 1, 1, 0);*/
		// use just one, 4 lights kills the framerate
		addParticleLight (p, 300, 0, 1, 0.514, 0);
	}
}

/*
===============
CL_Explosion_FlashParticle

Explosion fash
===============
*/
void CL_Explosion_FlashParticle (vec3_t org, float size, qboolean large)
{
	if (large)
	{
		setupParticle (
			0,		0,		0,
			org[0],	org[1],	org[2],
			0,		0,		0,
			0,		0,		0,
			255,	175,	100,
			0,		0,		0,
			1,		-1.75,
			GL_SRC_ALPHA, GL_ONE,
			//GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			(size!=0)?size:50,	-10,			
			//100-(!rocket)?50:0,	-10,			
			particle_rflash,
			PART_DEPTHHACK_SHORT,
			NULL,0);
	}
	else
	{
		setupParticle (
			0,		0,		0,
			org[0],	org[1],	org[2],
			0,		0,		0,
			0,		0,		0,
			255,	175,	100,
			0,		0,		0,
			1,		-1.75,
			GL_SRC_ALPHA, GL_ONE,
			(size!=0)?size:50,	-10,	
			//100-(!rocket)?50:0,	-10,			
			particle_blaster,
			0,
			NULL,0);
	}
}


/*
===============
CL_Explosion_Sparks
===============
*/
void pExplosionSparksThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.25*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.25*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_Explosion_Sparks (vec3_t org, int size)
{
	int	i;

	for (i=0; i < (64/cl_particle_scale->value); i++) // was 256
	{
		setupParticle (
			0,	0,	0,
			org[0] + ((rand()%size)-16),	org[1] + ((rand()%size)-16),	org[2] + ((rand()%size)-16),
			(rand()%150)-75,	(rand()%150)-75,	(rand()%150)-75,
			0,		0,		0,
			255,	100,	25,
			0,	0,	0,
			1,		-0.8 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			6,		-9,		// was 4, -6
			particle_blaster,
			PART_GRAVITY|PART_SPARK,
			pExplosionSparksThink, true);
	}
}


/*
===============

Blood effects

===============
*/
void pBloodThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time);
void CL_BloodPuff (vec3_t org, vec3_t dir, int count);

#define MAXBLEEDSIZE 5
#define TIMEBLOODGROW 2.5f
#define BLOOD_DECAL_CHANCE 0.5F

void pBloodDecalThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{	// This REALLY slows things down
	/*if (*time<TIMEBLOODGROW)
	{
		vec3_t dir;

		*size *= sqrt(0.5 + 0.5*(*time/TIMEBLOODGROW));

		AngleVectors (angle, dir, NULL, NULL);
		VectorNegate(dir, dir);
		clipDecal(p, *size, angle[2], org, dir);
	}*/

	//now calc alpha
	thinkDecalAlpha (p, org, angle, alpha, size, image, time);
}

void pBloodDropThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	float length;
	vec3_t len;

	VectorSubtract(p->angle, org, len);
	{
		calcPartVelocity(p, 0.2, time, angle);

		length = VectorNormalize(angle);
		if (length>MAXBLEEDSIZE) length = MAXBLEEDSIZE;
		VectorScale(angle, -length, angle);
	}

	//now to trace for impact...
	pBloodThink (p, org, angle, alpha, size, image, time);
}

void pBloodPuffThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	angle[2] =	angle[0] + *time*angle[1] + *time**time*angle[2];

	//now to trace for impact...
	pBloodThink (p, org, angle, alpha, size, image, time);
}

/*
===============
pBloodThink
===============
*/
void pBloodThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	trace_t	trace = CL_Trace (p->oldorg, org, 0, CONTENTS_SOLID); // was 0.1
	qboolean became_decal = false;

	if (trace.fraction < 1.0) // delete and stain...
	{
#ifdef DECALS
		if (r_decals->value && (p->flags & PART_LEAVEMARK)
			&& !VectorCompare(trace.plane.normal, vec3_origin)
			&& !(CM_PointContents(p->oldorg,0) & MASK_WATER)) // no blood splatters underwater...
		{
			vec3_t	normal, dir;
			int		i;
			qboolean greenblood = false;
			qboolean timedout = false;
			if (p->color[1] > 0 && p->color[2] > 0)
				greenblood = true;
			// time cutoff for gib trails
			if (p->flags & PART_GRAVITY && !(p->flags & PART_DIRECTION))
			{	// gekk gibs go flyin faster...
				if ((greenblood) && (cl.time - p->time)*0.001 > 1.0F)
					timedout = true;
				if ((!greenblood) && (cl.time - p->time)*0.001 > 0.5F)
					timedout = true;
			}

			if (!timedout)
			{

				VectorNegate(trace.plane.normal, normal);
				vectoanglerolled(normal, rand()%360, p->angle);
				
				VectorCopy(trace.endpos, p->org);
				VectorClear(p->vel);
				VectorClear(p->accel);
				p->image = particleBlood();
				p->blendfunc_src = GL_SRC_ALPHA; //GL_ZERO
				p->blendfunc_dst = GL_ONE_MINUS_SRC_ALPHA; //GL_ONE_MINUS_SRC_COLOR
				p->flags = PART_DECAL|PART_SHADED|PART_ALPHACOLOR;
				p->alpha = *alpha;
				p->alphavel = -1/r_decal_life->value;
				if (greenblood)
					p->color[1] = 210;
				else
					for (i=0; i<3; i++)
						p->color[i] *= 0.5;
				p->start = newParticleTime();
				p->think = pBloodDecalThink;
				p->thinknext = true;
				p->size = MAXBLEEDSIZE*0.5*(random()*5.0+5);
				//p->size = *size*(random()*5.0+5);
				p->sizevel = 0;
				
				p->decalnum = 0;
				p->decal = NULL;
				AngleVectors (p->angle, dir, NULL, NULL);
				VectorNegate(dir, dir);
				clipDecal(p, p->size, -p->angle[2], p->org, dir);
				if (p->decalnum)
				{
					became_decal = true;

					//bc splat sound

					//if (rand()%2 == 1)
					//	S_StartSound (org, 0, CHAN_BODY, S_RegisterSound("misc/fhit3.wav"), frand()*0.6 + 0.4, ATTN_NORM, 0);
				}
				//else
				//	Com_Printf(S_COLOR_YELLOW"Blood decal not clipped!\n");
			}
		}
		if (!became_decal)
#endif
		{
			*alpha = 0;
			*size = 0;
			p->alpha = 0;
		}
	}
	VectorCopy(org, p->oldorg);

	p->thinknext = true;
}


/*
===============
CL_BloodSmack
===============
*/
void CL_BloodSmack (vec3_t org, vec3_t dir)
{
	cparticle_t *p;

	p = setupParticle (
		crand()*180, crand()*100, 0,
		org[0],	org[1],	org[2],
		dir[0],	dir[1],	dir[2],
		0,		0,		0,
		255,	0,		0,
		0,		0,		0,
		1.0,		-1 / (0.5 + frand()*0.3), //was -0.75
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
		10,			0,			
		particle_redblood,
		PART_SHADED|PART_OVERBRIGHT,
		pRotateThink,true);

	CL_BloodPuff(org, dir, 1);
}


/*
===============
CL_BloodBleed
===============
*/
void CL_BloodBleed (vec3_t org, vec3_t dir, int count)
{
	cparticle_t *p;
	vec3_t	pos;
	int		i;

	VectorScale(dir, 10, pos);
	for (i=0; i<count; i++)
	{
		VectorSet(pos,
			dir[0]+random()*(cl_blood->value-2)*0.01,
			dir[1]+random()*(cl_blood->value-2)*0.01,
			dir[2]+random()*(cl_blood->value-2)*0.01);
		VectorScale(pos, 10 + (cl_blood->value-2)*0.0001*random(), pos);

		p = setupParticle (
			org[0], org[1], org[2],
			org[0] + ((rand()&7)-4) + dir[0],	org[1] + ((rand()&7)-4) + dir[1],	org[2] + ((rand()&7)-4) + dir[2],
			pos[0]*(random()*3+5),	pos[1]*(random()*3+5),	pos[2]*(random()*3+5),
			0,		0,		0,
			255,	0,		0,
			0,		0,		0,
			0.7,		-0.25 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			MAXBLEEDSIZE*0.5,		0,			
			particle_blooddrip,
			PART_SHADED|PART_DIRECTION|PART_GRAVITY|PART_OVERBRIGHT,
			pBloodDropThink,true);

		if (i == 0 && random() < BLOOD_DECAL_CHANCE)
			p->flags |= PART_LEAVEMARK;
	}

}

/*
===============
CL_BloodPuff
===============
*/
void CL_BloodPuff (vec3_t org, vec3_t dir, int count)
{
	cparticle_t *p;
	int		i;
	float	d;

	for (i=0; i<count; i++)
	{
		d = rand()&31;
		p = setupParticle (
			crand()*180, crand()*100, 0,
			org[0] + ((rand()&7)-4) + d*dir[0],	org[1] + ((rand()&7)-4) + d*dir[1],	org[2] + ((rand()&7)-4) + d*dir[2],
			dir[0]*(crand()*3+5),	dir[1]*(crand()*3+5),	dir[2]*(crand()*3+5),
			0,			0,			-100,
			255,		0,			0,
			0,			0,			0,
			1.0,		-1.0,
			GL_SRC_ALPHA, GL_ONE,
			10,			0,
			particle_blood,
			PART_SHADED,
			pBloodPuffThink,true);

		if (i == 0 && random() < BLOOD_DECAL_CHANCE)
			p->flags |= PART_LEAVEMARK;
	}
}

/*
===============
CL_BloodHit
===============
*/
void CL_BloodHit (vec3_t org, vec3_t dir)
{
	if (cl_blood->value < 1) // disable blood option
		return;
	if (cl_blood->value == 2) // splat
		CL_BloodSmack(org, dir);
	else if (cl_blood->value == 3) // bleed
		CL_BloodBleed (org, dir, 6);
	else if (cl_blood->value == 4) // gore
		CL_BloodBleed (org, dir, 16);
	else // 1 = puff
		CL_BloodPuff(org, dir, 5);
}

/*
==================
CL_GreenBloodHit

green blood spray
==================
*/
void CL_GreenBloodHit (vec3_t org, vec3_t dir)
{
	cparticle_t *p;
	int		i;
	float	d;

	if (cl_blood->value < 1) // disable blood option
		return;

	for (i=0;i<5;i++)
	{
		d = rand()&31;
		p = setupParticle (
				crand()*180, crand()*100, 0,
				org[0] + ((rand()&7)-4) + d*dir[0],	org[1] + ((rand()&7)-4) + d*dir[1],	org[2] + ((rand()&7)-4) + d*dir[2],
				dir[0]*(crand()*3+5),	dir[1]*(crand()*3+5),	dir[2]*(crand()*3+5),
				0,		0,		-100,
				255,	180,	50,
				0,		0,		0,
				1,		-1.0,
				GL_SRC_ALPHA, GL_ONE,
				10,			0,			
				particle_blood,
				PART_SHADED|PART_OVERBRIGHT,
				pBloodPuffThink,true);

		if (i == 0 && random() < BLOOD_DECAL_CHANCE)
			p->flags |= PART_LEAVEMARK;
	}

}

/*
===============
CL_ParticleEffect

Wall impact puffs
===============
*/
void CL_ParticleEffect (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&31;
		setupParticle (
			0,		0,		0,
			org[0] + ((rand()&7)-4) + d*dir[0],	org[1] + ((rand()&7)-4) + d*dir[1],	org[2] + ((rand()&7)-4) + d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			1.0,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}

/*
===============
CL_ParticleEffect2
===============
*/

#define colorAdd 25
void CL_ParticleEffect2 (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		setupParticle (
			0,	0,	0,
			org[0]+((rand()&7)-4)+d*dir[0],	org[1]+((rand()&7)-4)+d*dir[1],	org[2]+((rand()&7)-4)+d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0] + colorAdd,		color[1] + colorAdd,		color[2] + colorAdd,
			0,	0,	0,
			1,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}

// RAFAEL
/*
===============
CL_ParticleEffect3
===============
*/

void CL_ParticleEffect3 (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		setupParticle (
			0,	0,	0,
			org[0]+((rand()&7)-4)+d*dir[0],	org[1]+((rand()&7)-4)+d*dir[1],	org[2]+((rand()&7)-4)+d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0] + colorAdd,		color[1] + colorAdd,		color[2] + colorAdd,
			0,	0,	0,
			1,		-0.25 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			2,			-0.25,			
			particle_generic,
			PART_GRAVITY,
			NULL, false);
	}
}


/*
===============
CL_ParticleEffectSplash

Water Splashing
===============
*/
#define SplashSize 7.5
void pSplashThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;
	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
//	*size *= (float)(SplashSize/VectorLength(len)) * 0.5/((4-*size));
//	if (*size > SplashSize)
//		*size = SplashSize;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.5*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.5*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_ParticleEffectSplash (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = {color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&5;
		setupParticle (
			org[0],	org[1],	org[2],
			org[0]+d*dir[0],	org[1]+d*dir[1],	org[2]+d*dir[2],
			dir[0]*40 + crand()*10,	dir[1]*40 + crand()*10,	dir[2]*40 + crand()*10,
			0,		0,		0,
			color[0],	color[1],	color[2],
			0,	0,	0,
			1,		-0.75 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			5,			-7,			
			particle_smoke,
			PART_GRAVITY|PART_DIRECTION   /*|PART_TRANS|PART_SHADED*/,
			pSplashThink,true);
	}
}

/*
===============
CL_ParticleEffectSparks
===============
*/
void pSparksThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	//vec3_t dir;
	int i;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.25*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.25*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_ParticleEffectSparks (vec3_t org, vec3_t dir, vec3_t color, int count)
{
	int			i;
	float		d;
	cparticle_t *p;

	int spread = 40;
	int dist = 140;

	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		p = setupParticle (
			0,	0,	0,
			org[0]+((rand()&3)-2),	org[1]+((rand()&3)-2),	org[2]+((rand()&3)-2),
			crand()*spread + dir[0]*dist,			crand()*spread + dir[1]*dist,			crand()*spread + dir[2]*dist,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			0.75,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			4,			0, //Knightmare- increase size
			particle_generic,
			PART_GRAVITY|PART_SPARK,
			pSparksThink,true);
	}
	if (p) // added light effect
		addParticleLight (p, (count>8)?130:65, 0, color[0]/255, color[1]/255, color[2]/255);
}


#ifdef DECALS
/*
===============
thinkDecalAlpha
===============
*/
void thinkDecalAlpha (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	*alpha = pow(*alpha, 0.1);
	p->thinknext = true;
}

void vectoanglerolled (vec3_t value1, float angleyaw, vec3_t angles)
{
	float	forward, yaw, pitch;

	yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
	forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
	pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);

	if (pitch < 0)
		pitch += 360;

	angles[PITCH] = -pitch;
	angles[YAW] =  yaw;
	angles[ROLL] = - angleyaw;
}

/*
===============
CL_ParticleBulletDecal
===============
*/
#define DECAL_OFFSET 0.5f
void CL_ParticleBulletDecal (vec3_t org, vec3_t dir, float size)
{
	cparticle_t	*p;
	vec3_t		ang, angle, end, origin;
	trace_t		tr;

	if (!r_decals->value)
		return;

	VectorMA(org, DECAL_OFFSET, dir, origin);
	VectorMA(org, -DECAL_OFFSET, dir, end);
	tr = CL_Trace (origin, end, 0, CONTENTS_SOLID);
	//tr = CL_Trace (origin, end, 1, 1);

	if (tr.fraction == 1)
	//if (!tr.allsolid)
		return;

	VectorNegate(tr.plane.normal, angle);
	//VectorNegate(dir, angle);
	vectoanglerolled(angle, rand()%360, ang);
	VectorCopy(tr.endpos, origin);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		1,		-1/r_decal_life->value,
		GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
		size,			0,			
		particle_bulletmark,
		PART_SHADED|PART_DECAL|PART_ALPHACOLOR, // was part_saturate
		thinkDecalAlpha, true);
}


/*
===============
CL_ParticleRailDecal
===============
*/
#define RAIL_DECAL_OFFSET 2.0f
void CL_ParticleRailDecal (vec3_t org, vec3_t dir, float size, qboolean isRed)
{
	vec3_t		ang, angle, end, origin;
	trace_t		tr;

	if (!r_decals->value)
		return;

	VectorMA(org, -RAIL_DECAL_OFFSET, dir, origin);
	VectorMA(org, 2*RAIL_DECAL_OFFSET, dir, end);
	tr = CL_Trace (origin, end, 0, CONTENTS_SOLID);

	if (tr.fraction==1)
		return;
	if (VectorCompare(tr.plane.normal, vec3_origin))
		return;

	VectorNegate(tr.plane.normal, angle);
	vectoanglerolled(angle, rand()%360, ang);
	VectorCopy(tr.endpos, origin);

	setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		1,		-1/r_decal_life->value,
		GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
		size,			0,			
		particle_bulletmark,
		PART_SHADED|PART_DECAL|PART_ALPHACOLOR,
		thinkDecalAlpha, true);

	setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		(isRed)?255:cl_railred->value,	(isRed)?20:cl_railgreen->value,	(isRed)?20:cl_railblue->value,
		0,		0,		0,
		1,		-0.25,
		GL_SRC_ALPHA, GL_ONE,
		size,			0,			
		particle_generic,
		PART_DECAL,
		NULL, false);

	setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		1,		-0.25,
		GL_SRC_ALPHA, GL_ONE,
		size*0.67,		0,			
		particle_generic,
		PART_DECAL,
		NULL, false);
}


/*
===============
CL_ParticleBlasterDecal
===============
*/
void CL_ParticleBlasterDecal (vec3_t org, vec3_t dir, float size, int red, int green, int blue)
{
	cparticle_t	*p;
	vec3_t		ang, angle, end, origin;
	trace_t		tr;

	if (!r_decals->value)
		return;
 
	VectorMA(org, DECAL_OFFSET, dir, origin);
	VectorMA(org, -DECAL_OFFSET, dir, end);
	tr = CL_Trace (origin, end, 0, CONTENTS_SOLID);

	if (tr.fraction==1)
		return;
	if (VectorCompare(tr.plane.normal, vec3_origin))
		return;

	VectorNegate(tr.plane.normal, angle);
	vectoanglerolled(angle, rand()%360, ang);
	VectorCopy(tr.endpos, origin);
/*
	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		0.7,	-1/r_decal_life->value,
		GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
		size,		0,			
		particle_shadow,
		PART_SHADED|PART_DECAL,
		NULL, false);*/

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		red,	green,	blue,
		0,		0,		0,
		0.5,		-0.1,
		GL_SRC_ALPHA, GL_ONE,
		size*0.4,	0,			
		particle_freongeneric,
		PART_SHADED|PART_DECAL,
		NULL, false);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		red,	green,	blue,
		0,		0,		0,
		0.5,		-0.3,
		GL_SRC_ALPHA, GL_ONE,
		size*0.3,	0,			
		particle_freongeneric,
		PART_SHADED|PART_DECAL,
		NULL, false);
}



void CL_ParticleFreonDecal (vec3_t org, vec3_t dir, float size, int red, int green, int blue)
{
	cparticle_t	*p;
	vec3_t		ang, angle, end, origin;
	trace_t		tr;

	if (!r_decals->value)
		return;
 
	VectorMA(org, DECAL_OFFSET, dir, origin);
	VectorMA(org, -DECAL_OFFSET, dir, end);
	tr = CL_Trace (origin, end, 0, CONTENTS_SOLID);

	if (tr.fraction==1)
		return;
	if (VectorCompare(tr.plane.normal, vec3_origin))
		return;

	VectorNegate(tr.plane.normal, angle);
	vectoanglerolled(angle, rand()%360, ang);
	VectorCopy(tr.endpos, origin);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		0.7,	-1/r_decal_life->value,
		GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
		size,		0,			
		particle_shadow,
		PART_SHADED|PART_DECAL,
		NULL, false);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		red,	green,	blue,
		0,		0,		0,
		1,		-0.3,
		GL_SRC_ALPHA, GL_ONE,
		size*0.4,	0,			
		particle_generic,
		PART_SHADED|PART_DECAL,
		NULL, false);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		red,	green,	blue,
		0,		0,		0,
		1,		-0.6,
		GL_SRC_ALPHA, GL_ONE,
		size*0.3,	0,			
		particle_generic,
		PART_SHADED|PART_DECAL,
		NULL, false);
}



/*
===============
CL_ParticlePlasmaBeamDecal
===============
*/
void CL_ParticlePlasmaBeamDecal (vec3_t org, vec3_t dir, float size)
{
	cparticle_t	*p;
	vec3_t		ang, angle, end, origin;
	trace_t		tr;

	if (!r_decals->value)
		return;
 
	VectorMA(org, DECAL_OFFSET, dir, origin);
	VectorMA(org, -DECAL_OFFSET, dir, end);
	tr = CL_Trace (origin, end, 0, CONTENTS_SOLID);

	if (tr.fraction==1)
		return;
	if (VectorCompare(tr.plane.normal, vec3_origin))
		return;

	VectorNegate(tr.plane.normal, angle);
	vectoanglerolled(angle, rand()%360, ang);
	VectorCopy(tr.endpos, origin);

	p = setupParticle (
		ang[0],	ang[1],	ang[2],
		origin[0],	origin[1],	origin[2],
		0,		0,		0,
		0,		0,		0,
		255,	255,	255,
		0,		0,		0,
		0.85,	-1/r_decal_life->value,
		GL_ZERO, GL_ONE_MINUS_SRC_ALPHA,
		size,		0,			
		particle_shadow,
		PART_SHADED|PART_DECAL,
		NULL, false);
}
#endif // DECALS


/*
===============
CL_TeleporterParticles
===============
*/
void CL_TeleporterParticles (entity_state_t *ent)
{
	int			i;

	for (i = 0; i < 8; i++)
	{
		setupParticle (
			0,	0,	0,
			ent->origin[0]-16+(rand()&31),	ent->origin[1]-16+(rand()&31),	ent->origin[2]-16+(rand()&31),
			crand()*14,		crand()*14,		80 + (rand()&7),
			0,		0,		0,
			230+crand()*25,	125+crand()*25,	25+crand()*25,
			0,		0,		0,
			1,		-0.5,
			GL_SRC_ALPHA, GL_ONE,
			2,		0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_LogoutEffect

===============
*/
void CL_LogoutEffect (vec3_t org, int type)
{
	int			i;
	vec3_t	color;

	for (i=0 ; i<500 ; i++)
	{
		if (type == MZ_LOGIN)// green
		{
			color[0] = 20;
			color[1] = 200;
			color[2] = 20;
		}
		else if (type == MZ_LOGOUT)// red
		{
			color[0] = 200;
			color[1] = 20;
			color[2] = 20;
		}
		else// yellow
		{
			color[0] = 200;
			color[1] = 200;
			color[2] = 20;
		}
		
		setupParticle (
			0,	0,	0,
			org[0] - 16 + frand()*32,	org[1] - 16 + frand()*32,	org[2] - 24 + frand()*56,
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			1,		-1.0 / (1.0 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_ItemRespawnParticles

===============
*/
void CL_ItemRespawnParticles (vec3_t org)
{
	int			i;

	for (i=0 ; i<64 ; i++)
	{
		setupParticle (
			0,	0,	0,
			org[0] + crand()*8,	org[1] + crand()*8,	org[2] + crand()*8,
			crand()*8,			crand()*8,			crand()*8,
			0,		0,		PARTICLE_GRAVITY*0.2,
			0,		150+rand()*25,		0,
			0,	0,	0,
			1,		-1.0 / (1.0 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_BigTeleportParticles
===============
*/
void CL_BigTeleportParticles (vec3_t org)
{
	int			i, index;
	float		angle, dist;
	static int colortable0[4] = {10,50,150,50};
	static int colortable1[4] = {150,150,50,10};
	static int colortable2[4] = {50,10,10,150};

	for (i=0; i<(1024/cl_particle_scale->value); i++) // was 4096
	{

		index = rand()&3;
		angle = M_PI*2*(rand()&1023)/1023.0;
		dist = rand()&31;
		setupParticle (
			0,	0,	0,
			org[0]+cos(angle)*dist,	org[1] + sin(angle)*dist,org[2] + 8 + (rand()%90),
			cos(angle)*(70+(rand()&63)),sin(angle)*(70+(rand()&63)),-100 + (rand()&31),
			-cos(angle)*100,	-sin(angle)*100,PARTICLE_GRAVITY*4,
			colortable0[index],	colortable1[index],	colortable2[index],
			0,	0,	0,
			1,		-0.1 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			5,		0.15 / (0.5 + frand()*0.3),	 // was 2, 0.05	
			particle_generic,
			0,
			NULL,0);
	}
}


/*
===============
CL_BlasterParticles

Wall impact puffs
===============
*/
#define pBlasterMaxVelocity 100
#define pBlasterMinSize 1.0
#define pBlasterMaxSize 5.0

void pBlasterThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec_t  length;
	vec3_t len;
	float clipsize = 1.0;
	VectorSubtract(p->angle, org, len);

	*size *= (float)(pBlasterMaxSize/VectorLength(len)) * 1.0/((4-*size));
	*size += *time * p->sizevel;

	if (*size > pBlasterMaxSize)
		*size = pBlasterMaxSize;
	if (*size < pBlasterMinSize)
		*size = pBlasterMinSize;

	pBounceThink (p, org, angle, alpha, &clipsize, image, time); // was size

	length = VectorNormalize(p->vel);
	if (length>pBlasterMaxVelocity)
		VectorScale(p->vel,	pBlasterMaxVelocity,	p->vel);
	else
		VectorScale(p->vel,	length, p->vel);

/*	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
	*size *= (float)(pBlasterMaxSize/VectorLength(len)) * 1.0/((4-*size));
	if (*size > pBlasterMaxSize)
		*size = pBlasterMaxSize;
	
	p->thinknext = true;*/
}

void pLobbyGlassThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec_t  length;
	vec3_t len;
	float clipsize = 1.0;
	VectorSubtract(p->angle, org, len);

	*size *= (float)(10/VectorLength(len)) * 1.0/((4-*size));
	*size += *time * p->sizevel;

	if (*size > 10)
		*size = 10;
	if (*size < 3)
		*size = 3;

	pBounceThink (p, org, angle, alpha, &clipsize, image, time); // was size

	length = VectorNormalize(p->vel);
	if (length>256)
		VectorScale(p->vel,	256,	p->vel);
	else
		VectorScale(p->vel,	length, p->vel);
}


void pFreonThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec_t  length;
	vec3_t len;
	float clipsize = 1.0;
	VectorSubtract(p->angle, org, len);

	*size *= (float)(pBlasterMaxSize/VectorLength(len)) * 1.0/((4-*size));
	*size += *time * p->sizevel;

	if (*size > pBlasterMaxSize)
		*size = pBlasterMaxSize;
	if (*size < pBlasterMinSize)
		*size = pBlasterMinSize;

	pBounceThink (p, org, angle, alpha, &clipsize, image, time); // was size

	length = VectorNormalize(p->vel);
	if (length>pBlasterMaxVelocity)
		VectorScale(p->vel,	pBlasterMaxVelocity,	p->vel);
	else
		VectorScale(p->vel,	length, p->vel);

/*	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
	*size *= (float)(pBlasterMaxSize/VectorLength(len)) * 1.0/((4-*size));
	if (*size > pBlasterMaxSize)
		*size = pBlasterMaxSize;
	
	p->thinknext = true;*/
}





void CL_LavaParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
												int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	count = 1;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + dir[0]*(1 + random()*64 ),
			org[1] + dir[1]*(1 + random()*64  ),
			org[2] + dir[2]*(1 + random()*16 )
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*75 + crand()*40)*speed,	(dir[1]*75 + crand()*40)*speed,	(dir[2]*75 + crand()*40)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-0.5 / (0.5 + frand()*0.3), //alpha
			GL_SRC_ALPHA, GL_ONE, 
			96,	-0.01,		  //size
			particle_smoke,
			PART_GRAVITY,
			NULL,true);
	}

	speed *= 1.7;

	count = 1;


	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + dir[0]*(1 + random()*48 ),
			org[1] + dir[1]*(1 + random()*48  ),
			org[2] + dir[2]*(1 + random()*16 )
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*75 + crand()*40)*speed,	(dir[1]*75 + crand()*40)*speed,	(dir[2]*75 + crand()*40)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-0.3, //alpha
			GL_SRC_ALPHA, GL_ONE, 
			14,	-5,		  //size
			particle_generic,
			PART_GRAVITY,
			NULL,true);
	}
}



void CL_HeartParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
												int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .7;
	cparticle_t *p;
	vec3_t		origin;

	count = 1;

	for (i = 0; i < count; i++)
	{
		/*VectorSet(origin,
			org[0] + dir[0]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[1] + dir[1]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[2] + dir[2]*(1 + random()*3 + pBlasterMaxSize/2.0)
			);*/

		VectorSet(origin,
			org[0] + dir[0]*(1 + random()*2),
			org[1] + dir[1]*(1 + random()*2),
			org[2] + dir[2]*(1 + random()*2)
			);

		p = setupParticle (
			crandom() * 4, crandom() * 4,crandom() * 4,
			//org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			//(dir[0]*75 + crand()*40)*speed,	(dir[1]*75 + crand()*40)*speed,	(dir[2]*75 + crand()*40)*speed,

			(dir[0]*10 + crand()*5)*speed,	(dir[1]*10 + crand()*5)*speed,	(dir[2]*10 + crand()*5)*speed,

			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			0.5,		-0.1,
			GL_SRC_ALPHA, GL_ONE,
			5,	-0.1,	
			particle_heart,
			0,
			pBlasterThink,true);
	}
}


void CL_MusicParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
												int reddelta, int greendelta, int bluedelta)
{
	int			i;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;
	float ltime;

	ltime = (float)cl.time / 1000.0;

	count = 1;

	//the little droplets.
	for (i = 0; i < count; i++)
	{
		int irand;
		int textureName;
		irand = random() * 3;

		if (irand <= 0)
			textureName = particle_music1;
		else if (irand == 1)
			textureName = particle_music2;
		else
			textureName = particle_music3;

		VectorSet(origin,
			org[0] + (-2 + random()*4 ),
			org[1] + (-2 + random()*4 ),
			org[2] + (-2 + random()*4 )			
			);

		origin[0] += sin(ltime * 1.5) * 8;
		origin[1] += cos(ltime * 1.5) * 8;


		p = setupParticle (
			0,0, 0,
			origin[0],	origin[1],	origin[2],

			(dir[0]*2 + crand()*4)*speed,
			(dir[1]*2 + crand()*4)*speed,
			dir[2]  + 8, //float up.

			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		0, //alpha
			GL_SRC_ALPHA, GL_ONE, 
			2,	-0.7,		  //size -5
			textureName,
			0,
			NULL,true);
	}
}

void CL_WaterfallParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
												int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	count = 1;

	//the wash on the top.
	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + (-32 + random()*64 ),
			org[1] + (-32 + random()*64 ),
			org[2] + (-8 + random()*16 )
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*75 + crand()*64)*speed,
			(dir[1]*75 + crand()*64)*speed,
			(dir[2]*75 + crand()*64)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			0.4,		-0.3 / (0.5 + frand()*0.3), //alpha
			GL_SRC_ALPHA, GL_ONE, 
			16,	96,		  //size
			particle_smoke,
			PART_GRAVITY,
			NULL,true);
	}

	count=2;

	//the main torrent of water.
	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + (-6 + random()*12 ),
			org[1] + (-6 + random()*12 ),
			org[2] + (-6 + random()*12 )
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*75 + crand()*8)*speed,
			(dir[1]*75 + crand()*8)*speed,
			(dir[2]*75 + crand()*128)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			0.3,		-0.1, //alpha
			GL_SRC_ALPHA, GL_ONE, 
			32,	16.0,		  //size
			particle_smoke,
			PART_GRAVITY,
			NULL,true);
	}



	//speed *= 1.7;

	speed *= (random()*2.9);

	count = 2;


	//the little droplets.
	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + (-12 + random()*24 ),
			org[1] + (-12 + random()*24 ),
			org[2] + (-12 + random()*24 )			
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],

			(dir[0]*4 + crand()*8)*speed,
			(dir[1]*4 + crand()*8)*speed,
			(dir[2]*75 + crand()*40)*speed,

			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-0.15, //alpha
			GL_SRC_ALPHA, GL_ONE, 
			1.8,	0.1,		  //size -5
			particle_generic,
			PART_GRAVITY,
			NULL,true);
	}
}



void CL_BlasterParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
												int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + dir[0]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[1] + dir[1]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[2] + dir[2]*(1 + random()*3 + pBlasterMaxSize/2.0)
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*75 + crand()*40)*speed,	(dir[1]*75 + crand()*40)*speed,	(dir[2]*75 + crand()*40)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-0.3,
			GL_SRC_ALPHA, GL_ONE,
			3.5,	-0.1,	
			particle_generic,
			PART_GRAVITY,
			pBlasterThink,true);
	
	/*	d = rand()&5;
		p = setupParticle (
			org[0],	org[1],	org[2],
			org[0]+((rand()&5)-2)+d*dir[0],	org[1]+((rand()&5)-2)+d*dir[1],	org[2]+((rand()&5)-2)+d*dir[2],
			(dir[0]*50 + crand()*20)*speed,	(dir[1]*50 + crand()*20)*speed,	(dir[2]*50 + crand()*20)*speed,
			0,			0,			0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			4,		-1.0,
			particle_generic,
			PART_GRAVITY,
			pBlasterThink,true);*/
	}
	if (p) // added light effect
		addParticleLight (p, 150, 0, ((float)red)/255, ((float)green)/255, ((float)blue)/255);
}



void CL_FreonParticles (vec3_t org, vec3_t dir, int count, int red, int green, int blue,
											int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0] + dir[0]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[1] + dir[1]*(1 + random()*3 + pBlasterMaxSize/2.0),
			org[2] + dir[2]*(1 + random()*3 + pBlasterMaxSize/2.0)
			);

		p = setupParticle (
			org[0],	org[1],	org[2],
			origin[0],	origin[1],	origin[2],
			(dir[0]*7 + crand()*4)*speed,	(dir[1]*7 + crand()*4)*speed,	(dir[2]*7 + crand()*4)*speed,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-3,
			GL_SRC_ALPHA, GL_ONE,
			0.01,	-0.5,		
			particle_smoke,
			0,
			pFreonThink,true);
	
	/*	d = rand()&5;
		p = setupParticle (
			org[0],	org[1],	org[2],
			org[0]+((rand()&5)-2)+d*dir[0],	org[1]+((rand()&5)-2)+d*dir[1],	org[2]+((rand()&5)-2)+d*dir[2],
			(dir[0]*50 + crand()*20)*speed,	(dir[1]*50 + crand()*20)*speed,	(dir[2]*50 + crand()*20)*speed,
			0,			0,			0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			4,		-1.0,
			particle_generic,
			PART_GRAVITY,
			pBlasterThink,true);*/
	}
}


void CL_bloodcough(vec3_t org, vec3_t dir,  int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = 2;
	cparticle_t *p;
	vec3_t		origin;
	vec3_t r, u;

	Com_Printf ("dir1 %f %f %f\n", dir[0], dir[1], dir[2]);

	//dir[0] *= 56;
	//dir[1] *= 56;
	//dir[2] *= 56;

	Com_Printf ("dir2 %f %f %f\n", dir[0], dir[1], dir[2]);

	for (i = 0; i < 16; i++)
	{
		vec3_t velocity;

		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		velocity[0] = crand() * 2;
		velocity[1] = crand() * 2;
		velocity[2] = 1;

		VectorAdd (velocity, dir, velocity);	

		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0],	origin[1],	origin[2],

			velocity[0],	velocity[1],	velocity[2],
			//dir[0] * speed + 5, dir[1] * speed + 5, dir[2] * speed + 5,			

			0,		0,		0,
			255,		255,		255,
			0,	0,	0,
			0.4,		-0.3,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.4+crand()*0.2,	0.1+crand()*0.1,		//size
			particle_smoke,
			0,
			NULL,true);

	}
}

void CL_burpgas(vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = 6;
	cparticle_t *p;
	vec3_t		origin;

	for (i = 0; i < 8; i++)
	{
		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0],	origin[1],	origin[2],

			(dir[0]*0.2 + crand()*0.2)*speed,	(dir[1]*0.2 + crand()*0.2)*speed,	(dir[2]*2 + crand()*1)*speed,

			0,		0,		0,
			128,		255,		128,
			0,	0,	0,
			0.4,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.8+crand()*0.2,	8+crand()*0.1,		//size
			particle_smoke,
			0,
			NULL,true);
	
	}
}


void CL_shred (vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	cparticle_t *p;
	vec3_t		origin;
	float speed = 9 + random() * 4;	
	

	dir[2] += 5;

	for (i = 0; i < count; i++)
	{
		int image;
		float rsize;
		int r = random()  * 3;

		if (r == 0)
			image = particle_shred0;
		else if (r == 1)
			image = particle_shred1;
		else
			image = particle_shred2;

		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		if (random() > 0.3)
			rsize = 5  + crand()* 2;
		else
			rsize = 2 + crand() * 1;

		p = setupParticle (
			org[0] + crand() * 350,	org[1]+ crand() * 350,	org[2]+ crand() * 350,
			origin[0] + crand()* 8,	origin[1]+ crand()* 8,	origin[2]+ crand()* 8,
			(dir[0]*0.2 + crand()*2.9)*speed,	(dir[1]*0.2 + crand()*2.9)*speed,	(dir[2]*1.5 + crand()*1)*speed,
			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			1,		0,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			rsize,	random() * -1,		//size
			particle_shred0,
			PART_GRAVITY,
			NULL,0);
	}
}


void CL_lobbyglass (vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = 7;
	cparticle_t *p;
	vec3_t		origin;

	dir[2] += 5;

	for (i = 0; i < count; i++)
	{
		//(dir[2]*1 + crand()*2)*(random()*5)
		float zspeed = -96 + crandom() * 64;

		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		p = setupParticle (
			org[0] + crand() * 350,	org[1]+ crand() * 350,	org[2]+ crand() * 350,
			origin[0] + crand()*48,	origin[1]+ crand()*48,	     origin[2]+ crand()*5,
			(dir[0]*0.2 + crand()*2.9)*speed,	(dir[1]*0.2 + crand()*2.9)*speed,	zspeed,
			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			1,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			6+crand()*2,	0,		//size
			particle_glass,
			PART_GRAVITY   /*|PART_TRANS|PART_SHADED*/,
			NULL,true);
		
		p = setupParticle (
			org[0] + crand() * 350,	org[1]+ crand() * 350,	org[2]+ crand() * 350,
			origin[0] + crand()*5,	origin[1]+ crand()*5,	origin[2]+ crand()*5,
			(dir[0]*0.2 + crand()*3.9)*speed,	(dir[1]*0.2 + crand()*3.9)*speed,	zspeed,
			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			1,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			6+crand()*2,	0,		//size
			particle_glass2,
			PART_GRAVITY   /*|PART_TRANS|PART_SHADED*/,
			NULL,true);

	}
}



void CL_wineglassbreak (vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = 7;
	cparticle_t *p;
	vec3_t		origin;

	dir[2] += 5;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0] + crand()*0.3,	origin[1]+ crand()*0.3,	origin[2]+ crand()*0.3,

			(dir[0]*0.2 + crand()*2.9)*speed,	(dir[1]*0.2 + crand()*2.9)*speed,	(dir[2]*2 + crand()*1)*speed,

			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			0.8,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.9+crand()*0.7,	0,		//size
			particle_glass,
			PART_GRAVITY,
			NULL,0);
		
		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0] + crand()*0.3,	origin[1]+ crand()*0.3,	origin[2]+ crand()*0.3,

			(dir[0]*0.2 + crand()*3.9)*speed,	(dir[1]*0.2 + crand()*3.9)*speed,	(dir[2]*2 + crand()*2)*speed,

			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			0.8,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.9+crand()*0.7,	0,		//size
			particle_glass2,
			PART_GRAVITY,
			NULL,0);

	}



	//zz
}



void CL_cigsmoke (vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0],	origin[1],	origin[2],

			(dir[0]*0.2 + crand()*0.2)*speed,	(dir[1]*0.2 + crand()*0.2)*speed,	(dir[2]*2 + crand()*1)*speed,

			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			0.5,		-0.06,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.05+crand()*0.2,	0.1+crand()*0.1,		//size
			particle_smoke,
			0,
			NULL,true);
	
	}
}

void CL_cigbigsmoke (vec3_t org, vec3_t dir, int count,
				  int red, int green, int blue,
				int reddelta, int greendelta, int bluedelta)
{
	int			i;
	//float		d;
	float speed = .75;
	cparticle_t *p;
	vec3_t		origin;

	for (i = 0; i < count; i++)
	{
		VectorSet(origin,
			org[0],
			org[1],
			org[2]
			);

		p = setupParticle (
			org[0],	org[1],	org[2],

			origin[0],	origin[1],	origin[2],

			(dir[0]*0.2 + crand()*0.9)*speed,
			(dir[1]*0.2 + crand()*0.9)*speed,
			(dir[2]*6 + crand()*4)*speed,

			0,		0,		0,
			red,		green,		blue,
			0,	0,	0,
			0.7,		-0.3,//alpha
			GL_SRC_ALPHA, GL_ONE, 
			0.3+crand()*0.5,	1.5+crand()*1.5,		//size
			particle_smoke,
			0,
			NULL,true);
	
	}
}


/*
===============
CL_BlasterTrail

===============
*/
void CL_BlasterTrail (vec3_t start, vec3_t end, int red, int green, int blue,
									int reddelta, int greendelta, int bluedelta)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 4 * cl_particle_scale->value; 
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand(),	move[1] + crand(),	move[2] + crand(),
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			red,	green,	blue,
			reddelta,	greendelta,	bluedelta,
			1,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			4,			-6,
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_HyperBlasterTrail

Hyperblaster particle glow effect
===============
*/
void CL_HyperBlasterTrail (vec3_t start, vec3_t end, int red, int green, int blue,
										int reddelta, int greendelta, int bluedelta)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;
	int			i;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	VectorMA (move, 0.5, vec, move);
	len = VectorNormalize (vec);

	dec = 1 * cl_particle_scale->value;
	VectorScale (vec, dec, vec);

	for (i = 0; i < (18/cl_particle_scale->value); i++)
	{
		len -= dec;

		setupParticle (
			0,		0,		0,
			move[0] + crand(),	move[1] + crand(),	move[2] + crand(),
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			red,		green,		blue,
			reddelta,	greendelta,	bluedelta,
			1,		-16.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			3,		-36,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_BlasterTracer 
===============
*/
void CL_BlasterTracer (vec3_t origin, vec3_t angle, int red, int green, int blue, float len, float size)
{
	int i;
	vec3_t		dir;
	
	AngleVectors (angle, dir, NULL, NULL);
	VectorScale(dir, len,dir);

	for (i=0;i<3;i++)
		setupParticle (
			dir[0],	dir[1],	dir[2],
			origin[0],	origin[1],	origin[2],
			0,	0,	0,
			0,		0,		0,
			red, green, blue,
			0,	0,	0,
			1,		INSTANT_PARTICLE,
			GL_SRC_ALPHA, GL_ONE,
			size,		0,			
			particle_generic,
			PART_DIRECTION|PART_INSTANT,
			NULL,0);
}

void CL_HyperBlasterEffect (vec3_t start, vec3_t end, vec3_t angle, int red, int green, int blue,
										int reddelta, int greendelta, int bluedelta, float len, float size)
{
	if (cl_particle_scale->value >= 2)
		CL_BlasterTracer (end, angle, red, green, blue, len, size);
	else
		CL_HyperBlasterTrail (start, end, red, green, blue, reddelta, greendelta, bluedelta);
}













void CL_FreonTrail (vec3_t start, vec3_t end, int red, int green, int blue,
										int reddelta, int greendelta, int bluedelta)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;
	int			i;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	VectorMA (move, 0.5, vec, move);
	len = VectorNormalize (vec);

	dec = 1 * cl_particle_scale->value;
	VectorScale (vec, dec, vec);

	for (i = 0; i < (18/cl_particle_scale->value); i++)
	{
		len -= dec;

		setupParticle (
			0,		0,		0,
			


			//origin
			move[0]  + crand()*0.5,	move[1]+ crand()*0.5,	move[2] + crand()*0.5,

			//velocity
			crand()*3,	crand()*3,	crand()*3,

			0,		0,		0,
			100,		210,		250,
			reddelta,	greendelta,	bluedelta,
			0.99,		-16.0 / (0.5 + frand()*0.3), //alpha
			GL_SRC_ALPHA, GL_ONE,
			0.5,		-16,			 //size
			particle_smoke,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

void CL_FreonEffect (vec3_t start, vec3_t end, vec3_t angle, int red, int green, int blue,
										int reddelta, int greendelta, int bluedelta, float len, float size)
{
	CL_FreonTrail (start, end, red, green, blue, reddelta, greendelta, bluedelta);
}




/*
===============
CL_QuadTrail

===============
*/
void CL_QuadTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand()*16,	move[1] + crand()*16,	move[2] + crand()*16,
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			0,		0,		200,
			0,	0,	0,
			1,		-1.0 / (0.8+frand()*0.2),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_FlagTrail

===============
*/
void CL_FlagTrail (vec3_t start, vec3_t end, qboolean isred, qboolean isgreen)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand()*16, move[1] + crand()*16, move[2] + crand()*16,
			crand()*5,	crand()*5, crand()*5,
			0,		0,		0,
			(isred)?255:0, (isgreen)?255:0, (!isred && !isgreen)?255:0,
			0,	0,	0,
			1,		-1.0 / (0.8+frand()*0.2),
			GL_SRC_ALPHA, GL_ONE,
			1,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_DiminishingTrail

===============
*/
void CL_DiminishingTrail (vec3_t start, vec3_t end, centity_t *old, int flags)
{
	cparticle_t	*p;
	vec3_t		move;
	vec3_t		vec;
	float		len, oldlen;
	float		dec;
	float		orgscale;
	float		velscale;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = oldlen = VectorNormalize (vec);

	dec = (flags & EF_ROCKET) ? 10 : 2;
	dec *= cl_particle_scale->value;
	VectorScale (vec, dec, vec);

	if (old->trailcount > 900)
	{
		orgscale = 4;
		velscale = 15;
	}
	else if (old->trailcount > 800)
	{
		orgscale = 2;
		velscale = 10;
	}
	else
	{
		orgscale = 1;
		velscale = 5;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;

		if (flags & EF_ROCKET)
		{
			if (CM_PointContents(move,0) & MASK_WATER)
				setupParticle (
					0,	0,	crand()*360,
					move[0],	move[1],	move[2],
					crand()*9,	crand()*9,	crand()*9+5,
					0,		0,		0,
					255,	255,	255,
					0,	0,	0,
					0.75,		-0.2 / (1 + frand() * 0.2),
					GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
					1+random()*3,	1,			
					particle_bubble,
					PART_TRANS|PART_SHADED,
					pExplosionBubbleThink,true);
			else
				setupParticle (
					crand()*180, crand()*100, 0,
					move[0],	move[1],	move[2],
					crand()*5,	crand()*5,	crand()*5,
					0,		0,		5,
					255,	255,	255,
					-50,	-50,	-50,
					1,		-0.5,
					GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
					5,			15,			
					particle_smoke,
					PART_TRANS|PART_SHADED,
					pRotateThink, true);
		}
		else
		{
			// drop less particles as it flies
			//if ((rand()&1023) < old->trailcount)
			if ((rand()&511) < old->trailcount)
			{
				if (flags & EF_GIB)
				{
						p = setupParticle (
							0,	0,	random()*360,
							move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
							crand()*velscale,	crand()*velscale,	crand()*velscale,
							0,		0,		0,
							255,	0,		0,
							0,		0,		0,
							0.8f,	-0.75 / (1+frand()*0.4),
							GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							0.01,			0,
							particle_shield,
							PART_OVERBRIGHT|PART_GRAVITY|PART_SHADED,
							pBloodThink,true);
					
					/*
					if (cl_blood->value > 1)
						p = setupParticle (
							0,	0,	random()*360,
							move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
							crand()*velscale,	crand()*velscale,	crand()*velscale,
							0,		0,		0,
							255,	0,		0,
							0,		0,		0,
							0.75,	-0.75 / (1+frand()*0.4),
							GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							3 + random()*2,			0,			
							particle_blooddrop,
							PART_OVERBRIGHT|PART_GRAVITY|PART_SHADED,
							pBloodThink,true);
							//NULL,0);
					else
						p = setupParticle (
							0,	0,	0,
							move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
							crand()*velscale,	crand()*velscale,	crand()*velscale,
							0,		0,		0,
							255,	0,		0,
							0,		0,		0,
							1,		-1.0 / (1+frand()*0.4),
							GL_SRC_ALPHA, GL_ONE,
							5,			-1,			
							particle_blood,
							PART_GRAVITY|PART_SHADED,
							pBloodThink,true);
							//NULL,0);
*/
					if (crand() < (double)0.0001F)
						p->flags |= PART_LEAVEMARK;
				}
				else if (flags & EF_GREENGIB)
				{
					p = setupParticle (
						0,	0,	0,
						move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
						crand()*velscale,	crand()*velscale,	crand()*velscale,
						0,		0,		0,
						255,	180,	50,
						0,		0,		0,
						1,		-0.5 / (1+frand()*0.4),
						GL_SRC_ALPHA, GL_ONE,
						5,			-1,			
						particle_blood,
						PART_OVERBRIGHT|PART_GRAVITY|PART_SHADED,
						pBloodThink,true);
						//NULL,0);
					if (crand() < (double)0.0001F) 
						p->flags |= PART_LEAVEMARK;

				}
				else if (flags & EF_GRENADE) // no overbrights on grenade trails
				{
					if (CM_PointContents(move,0) & MASK_WATER)
						setupParticle (
							0,	0,	crand()*360,
							move[0],	move[1],	move[2],
							crand()*9,	crand()*9,	crand()*9+5,
							0,		0,		0,
							255,	255,	255,
							0,	0,	0,
							0.75,		-0.2 / (1 + frand() * 0.2),
							GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							1+random()*3,	1,			
							particle_bubble,
							PART_TRANS|PART_SHADED,
							pExplosionBubbleThink,true);
					else
						setupParticle (
							crand()*180, crand()*50, 0,
							move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
							crand()*velscale,	crand()*velscale,	crand()*velscale,
							0,		0,		20,
							255,	255,	255,
							0,		0,		0,
							0.5,		-0.35,
							GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							5,			5,			
							particle_smoke,
							PART_TRANS|PART_SHADED,
							pRotateThink,true); //bcccc
				}
				else
				{
					setupParticle (
						crand()*180, crand()*50, 0,
						move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
						crand()*velscale,	crand()*velscale,	crand()*velscale,
						0,		0,		20,
						255,		255,		255,
						0,	0,	0,
						0.5,		-0.5,
						GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
						5,			5,			
						particle_smoke,
						PART_OVERBRIGHT|PART_TRANS|PART_SHADED,
						pRotateThink,true);
				}
			}

			old->trailcount -= 5;
			if (old->trailcount < 100)
				old->trailcount = 100;
		}

		VectorAdd (move, vec, move);
	}
}


void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up)
{
	float		d;

	// this rotate and negat guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}

/*
===============
CL_RocketTrail

===============
*/
void CL_RocketTrail (vec3_t start, vec3_t end, centity_t *old)
{
	vec3_t		move;
	vec3_t		vec;
	float		len, totallen;
	float		dec;

	// smoke
	CL_DiminishingTrail (start, end, old, EF_ROCKET);

	// fire
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	totallen = len = VectorNormalize (vec);

	dec = 1*cl_particle_scale->value;
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;

		//falling particles
		if ( (rand()&7) == 0)
		{
			setupParticle (
				0,	0,	0,
				move[0] + crand()*5,	move[1] + crand()*5,	move[2] + crand()*5,
				crand()*20,	crand()*20,	crand()*20,
				0,		0,		20,
				255,	255,	200,
				0,	-50,	0,
				1,		-1.0 / (1+frand()*0.2),
				GL_SRC_ALPHA, GL_ONE,
				2,			-2,			
				particle_blaster,
				PART_GRAVITY,
				NULL,0);
		}
		VectorAdd (move, vec, move);
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	totallen = len = VectorNormalize (vec);
	dec = 1.5*cl_particle_scale->value;
	VectorScale (vec, dec, vec);
/*	len = totallen;
	VectorCopy (start, move);
	dec = 1.5;//*cl_particle_scale->value;
	VectorScale (vec, dec, vec);*/

	while (len > 0)
	{
		len -= dec;

		//flame
		setupParticle (
			crand()*180, crand()*100, 0,
			move[0],	move[1],	move[2],
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		5,
			255,	225,	200,
			-50,	-50,	-50,
			0.75,		-3,
			GL_SRC_ALPHA, GL_ONE,
			5,			5,
			particle_inferno,
			0,
			pRotateThink, true);

		VectorAdd (move, vec, move);
	}
}


/*
===============
FartherPoint
Returns true if the first vector
is farther from the viewpoint.
===============
*/
qboolean FartherPoint (vec3_t pt1, vec3_t pt2)
{
	vec3_t		distance1, distance2;

	VectorSubtract(pt1, cl.refdef.vieworg, distance1);
	VectorSubtract(pt2, cl.refdef.vieworg, distance2);
	return (VectorLength(distance1) > VectorLength(distance2));
}


/*
===============
CL_RailTrail
===============
*/
#define DEVRAILSTEPS 2
//this is the length of each piece...
#define RAILTRAILSPACE 15

void CL_RailSprial (vec3_t start, vec3_t end, qboolean isRed)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	vec3_t		right, up;
	int			i;
	float		d, c, s;
	vec3_t		dir;

	// Draw from closest point
	if (FartherPoint(start, end)) {
		VectorCopy (end, move);
		VectorSubtract (start, end, vec);
	}
	else {
		VectorCopy (start, move);
		VectorSubtract (end, start, vec);
	}
	len = VectorNormalize (vec);
	len = min (len, cl_rail_length->value);  // cap length
	MakeNormalVectors (vec, right, up);

	VectorScale(vec, cl_rail_space->value*cl_particle_scale->value, vec);

	for (i=0; i<len; i += cl_rail_space->value*cl_particle_scale->value)
	{
		d = i * 0.1;
		c = cos(d);
		s = sin(d);

		VectorScale (right, c, dir);
		VectorMA (dir, s, up, dir);

		setupParticle (
			0,	0,	0,
			move[0] + dir[0]*3,	move[1] + dir[1]*3,	move[2] + dir[2]*3,
			dir[0]*6,	dir[1]*6,	dir[2]*6,
			0,		0,		0,
			(isRed)?255:cl_railred->value,	(isRed)?20:cl_railgreen->value,	(isRed)?20:cl_railblue->value,
			0,	0,	0,
			1,		-1.0,
			GL_SRC_ALPHA, GL_ONE,
			3,	0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

void pDevRailThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;
	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
	*size *= (float)(SplashSize/VectorLength(len)) * 0.5/((4-*size));
	if (*size > SplashSize)
		*size = SplashSize;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 3*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 3*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_DevRailTrail (vec3_t start, vec3_t end, qboolean isRed)
{
	vec3_t		move;
	vec3_t		vec, point;
	float		len;
	int			dec, i=0;

	// Draw from closest point
	if (FartherPoint(start, end)) {
		VectorCopy (end, move);
		VectorSubtract (start, end, vec);
	}
	else {
		VectorCopy (start, move);
		VectorSubtract (end, start, vec);
	}
	len = VectorNormalize (vec);
	len = min (len, cl_rail_length->value);  // cap length
	VectorCopy(vec, point);

	dec = 4;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;
		i++;

		if (i>=DEVRAILSTEPS)
		{
			for (i=3;i>0;i--)
			setupParticle (
				point[0],	point[1],	point[2],
				move[0],	move[1],	move[2],
				0,		0,		0,
				0,		0,		0,
				(isRed)?255:cl_railred->value,	(isRed)?20:cl_railgreen->value,	(isRed)?20:cl_railblue->value,
				0,		-90,	-30,
				0.75,		-.75,
				GL_SRC_ALPHA, GL_ONE,
				dec*DEVRAILSTEPS*TWOTHIRDS,	0,			
				particle_beam2,
				PART_DIRECTION,
				NULL,0);
		}

		setupParticle (
			0,	0,	0,
			move[0],	move[1],	move[2],
			crand()*10,	crand()*10,	crand()*10+20,
			0,		0,		0,
			(isRed)?255:cl_railred->value,	(isRed)?20:cl_railgreen->value,	(isRed)?20:cl_railblue->value,
			0,	0,	0,
			1,		-0.75 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			2,			-0.25,			
			0,
			PART_GRAVITY|PART_SPARK,
			pDevRailThink,true);
		
		setupParticle (
			crand()*180, crand()*100, 0,
			move[0],	move[1],	move[2],
			crand()*10,	crand()*10,	crand()*10+20,
			0,		0,		5,
			255,	255,	255,
			0,	0,	0,
			0.25,		-0.25,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			5,			10,			
			particle_smoke,
			PART_TRANS|PART_GRAVITY|PART_OVERBRIGHT,
			pRotateThink, true);

		VectorAdd (move, vec, move);
	}
}

void CL_RailTrail (vec3_t start, vec3_t end, qboolean isRed)
{
	vec3_t		move, last;
	vec3_t		vec, point;
	//vec3_t	right, up;
	int			i;
	int			beamred, beamgreen, beamblue;
	float		len;//, dec;
	qboolean	colored = (cl_railtype->value!=0);

#ifdef DECALS // railgun impact decal
	VectorSubtract (end, start, vec);
	VectorNormalize(vec);
	CL_ParticleRailDecal (end, vec, 7, isRed);
#endif

	if (cl_railtype->value == 2)
	{
		CL_DevRailTrail (start, end, isRed);
		return;
	}
	// Draw from closest point
	if (FartherPoint(start, end)) {
		VectorCopy (end, move);
		VectorSubtract (start, end, vec);
	}
	else {
		VectorCopy (start, move);
		VectorSubtract (end, start, vec);
	}
	len = VectorNormalize (vec);
	if (cl_railtype->value == 0)
		len = min (len, cl_rail_length->value);  // cap length
	VectorCopy (vec, point);
	VectorScale (vec, RAILTRAILSPACE, vec);
	//MakeNormalVectors (vec, right, up);

	if (colored) {
		if (isRed) {
			beamred = 255;
			beamgreen = beamblue = 20;
		}
		else {
			beamred = cl_railred->value;
			beamgreen = cl_railgreen->value;
			beamblue = cl_railblue->value;
		}
	}
	else
		beamred = beamgreen = beamblue = 255;

	while (len > 0)
	{
		VectorCopy (move, last);	
		VectorAdd (move, vec, move);

		len -= RAILTRAILSPACE;

		for (i=0;i<3;i++)
			setupParticle (
				last[0],	last[1],	last[2],
				move[0],	move[1],	move[2],
				0,	0,	0,
				0,	0,	0,
				beamred,	beamgreen,	beamblue,
				0,	0,	0,
				0.75,		-0.75,
				GL_SRC_ALPHA, GL_ONE,
				RAILTRAILSPACE*TWOTHIRDS,	(colored)?0:-5,			
				particle_beam2,
				PART_BEAM,
				NULL,0);
	}
	if (cl_railtype->value == 0)
		CL_RailSprial (start, end, isRed);
}


// RAFAEL
/*
===============
CL_IonripperTrail
===============
*/
void CL_IonripperTrail (vec3_t start, vec3_t ent)
{
	vec3_t	move;
	vec3_t	vec;
	vec3_t  leftdir,up;
	float	len;
	int		dec;
	int     left = 0;

	VectorCopy (start, move);
	VectorSubtract (ent, start, vec);
	len = VectorNormalize (vec);

	MakeNormalVectors (vec, leftdir, up);

	dec = 3*cl_particle_scale->value;
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			255,	75,		0,
			0,	0,	0,
			0.75,		-1.0 / (0.3 + frand() * 0.2),
			GL_SRC_ALPHA, GL_ONE,
			3,			0,			// was dec
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_BubbleTrail

===============
*/
void CL_BubbleTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;
	float		dec, size;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 32;
	VectorScale (vec, dec, vec);

	for (i=0 ; i<len ; i+=dec)
	{
		size = (frand()>0.25)? 1 : (frand()>0.5) ? 2 : (frand()>0.75) ? 3 : 4;

		setupParticle (
			0,	0,	0,
			move[0]+crand()*2,	move[1]+crand()*2,	move[2]+crand()*2,
			crand()*5,	crand()*5,	crand()*5+6,
			0,		0,		0,
			255,	255,	255,
			0,	0,	0,
			0.75,		-0.5 / (1 + frand() * 0.2),
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			size,	1,			
			particle_bubble,
			PART_TRANS|PART_SHADED,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_FlyParticles
===============
*/
#define	BEAMLENGTH			16


void CL_FlyParticles (vec3_t origin, int count)
{
	int			i;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	float		ltime;


	if (count > NUMVERTEXNORMALS)
		count = NUMVERTEXNORMALS;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cl.time / 1000.0;
	for (i=0 ; i<count ; i+=2)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		dist = sin(ltime + i)*64;

		setupParticle (
			0,	0,	0,
			origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH,origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH,
			origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			1,		-100,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			1+sin(i+ltime),	1,			
			particle_generic,
			PART_TRANS,
			NULL,0);
	}
}

/*
===============
CL_FlyEffect
===============
*/
void CL_FlyEffect (centity_t *ent, vec3_t origin)
{
	int		n;
	int		count;
	int		starttime;

	if (ent->fly_stoptime < cl.time)
	{
		starttime = cl.time;
		ent->fly_stoptime = cl.time + 60000;
	}
	else
	{
		starttime = ent->fly_stoptime - 60000;
	}

	n = cl.time - starttime;
	if (n < 20000)
		count = n * 162 / 20000.0;
	else
	{
		n = ent->fly_stoptime - cl.time;
		if (n < 20000)
			count = n * 162 / 20000.0;
		else
			count = 162;
	}

	CL_FlyParticles (origin, count);
}


/*
===============
CL_BfgParticles
===============
*/
void pBFGThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec3_t len;
	VectorSubtract(p->angle, p->org, len);
	
	*size = (float)((300/VectorLength(len))*0.75);
}

#define	BEAMLENGTH			16

void CL_BfgParticles (entity_t *ent)
{
	int			i;
	cparticle_t	*p;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64, dist2;
	vec3_t		v;
	float		ltime;
	
	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cl.time / 1000.0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		dist2 = dist;
		dist = sin(ltime + i)*64;

		p = setupParticle (
			ent->origin[0],	ent->origin[1],	ent->origin[2],
			ent->origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH,ent->origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH,
			ent->origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH,
			0,	0,		0,
			0,	0,		0,
			50,	200*dist2,	20,
			0,	0,	0,
			1,		-100,
			GL_SRC_ALPHA, GL_ONE,
			1,			1,			
			particle_generic,
			0,
			pBFGThink, true);
		
		if (!p)
			return;

		VectorSubtract (p->org, ent->origin, v);
		dist = VectorLength(v) / 90.0;
	}
}


// RAFAEL
/*
===============
CL_TrapParticles
===============
*/
void CL_TrapParticles (entity_t *ent)
{
	vec3_t		move;
	vec3_t		vec;
	vec3_t		start, end;
	float		len;
	int			dec;

	ent->origin[2]-=14;
	VectorCopy (ent->origin, start);
	VectorCopy (ent->origin, end);
	end[2]+=64;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand(),	move[1] + crand(),	move[2] + crand(),
			crand()*15,	crand()*15,	crand()*15,
			0,	0,	PARTICLE_GRAVITY,
			230+crand()*25,	125+crand()*25,	25+crand()*25,
			0,		0,		0,
			1,		-1.0 / (0.3+frand()*0.2),
			GL_SRC_ALPHA, GL_ONE,
			3,			-3,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}

	{
	int			i, j, k;
	float		vel;
	vec3_t		dir;
	vec3_t		org;

	
	ent->origin[2]+=14;
	VectorCopy (ent->origin, org);


	for (i=-2 ; i<=2 ; i+=4)
		for (j=-2 ; j<=2 ; j+=4)
			for (k=-2 ; k<=4 ; k+=4)
			{

				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				VectorNormalize (dir);						
				vel = 50 + rand()&63;

				setupParticle (
					0,	0,	0,
					org[0] + i + ((rand()&23) * crand()), org[1] + j + ((rand()&23) * crand()),	org[2] + k + ((rand()&23) * crand()),
					dir[0]*vel,	dir[1]*vel,	dir[2]*vel,
					0,		0,		0,
					230+crand()*25,	125+crand()*25,	25+crand()*25,
					0,		0,		0,
					1,		-1.0 / (0.3+frand()*0.2),
					GL_SRC_ALPHA, GL_ONE,
					1,			1,			
					particle_generic,
					PART_GRAVITY,
					NULL,0);

			}
	}
}


/*
===============
CL_BFGExplosionParticles
===============
*/
//FIXME combined with CL_ExplosionParticles
void CL_BFGExplosionParticles (vec3_t org)
{
	int			i;

	for (i=0 ; i<256 ; i++)
	{
		setupParticle (
			0,	0,	0,
			org[0] + ((rand()%32)-16), org[1] + ((rand()%32)-16),	org[2] + ((rand()%32)-16),
			(rand()%150)-75,	(rand()%150)-75,	(rand()%150)-75,
			0,	0,	0,
			50,	100+rand()*50,	0, //Knightmare- made more green
			0,	0,	0,
			1,		-0.8 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			10,			-10,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_TeleportParticles

===============
*/
void CL_TeleportParticles (vec3_t org)
{
	int			i, j, k;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<=16 ; i+=4)
		for (j=-16 ; j<=16 ; j+=4)
			for (k=-16 ; k<=32 ; k+=4)
			{
				dir[0] = j*16;
				dir[1] = i*16;
				dir[2] = k*16;

				VectorNormalize (dir);						
				vel = 150 + (rand()&63);

				setupParticle (
					0,	0,	0,
					org[0]+i+(rand()&3), org[1]+j+(rand()&3),	org[2]+k+(rand()&3),
					dir[0]*vel,	dir[1]*vel,	dir[2]*vel,
					0,		0,		0,
					200 + 55*rand(),	200 + 55*rand(),	200 + 55*rand(),
					0,		0,		0,
					1,		-1.0 / (0.3 + (rand()&7) * 0.02),
					GL_SRC_ALPHA, GL_ONE,
					1,			3,			
					particle_generic,
					PART_GRAVITY,
					NULL,0);
			}
}


/*
===============
CL_AddParticles
===============
*/

void CL_AddParticles (void)
{
	cparticle_t		*p, *next;
	float			alpha, size, light;
	float			time=0, time2;
	vec3_t			org, color, angle;
	int				i, image, flags, decals;
	cparticle_t		*active, *tail;

	active = NULL;
	tail = NULL;
	decals = 0;

	for (p=active_particles; p; p=next)
	{
		next = p->next;
		flags = p->flags;

		// PMM - added INSTANT_PARTICLE handling for heat beam
		if (p->alphavel != INSTANT_PARTICLE)
		{
			time = (cl.time - p->time)*0.001;
			alpha = p->alpha + time*p->alphavel;
#ifdef DECALS
			if (flags&PART_DECAL)
			{
				if (decals >= r_decals->value || alpha <= 0)
				{	// faded out
					p->alpha = 0;
					p->flags = 0;
					ClearDecalPoly (p->decal); // flag decal chain for cleaning
					p->decalnum = 0;
					p->decal = NULL;
					p->next = free_particles;
					free_particles = p;
					continue;
				}
			}
			else
#endif
					if (alpha <= 0)
			{	// faded out
				p->alpha = 0;
				p->flags = 0;
				p->next = free_particles;
				free_particles = p;
				continue;
			}
		}
		else
		{
			alpha = p->alpha;
		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}

		if (alpha > 1.0)
			alpha = 1;
		if (alpha < 0.0)
			alpha = 0;

		time2 = time*time;
		image = p->image;

		for (i=0;i<3;i++)
		{
			color[i] = p->color[i] + p->colorvel[i]*time;
			if (color[i]>255) color[i]=255;
			if (color[i]<0) color[i]=0;
			
			angle[i] = p->angle[i];
			org[i] = p->org[i] + p->vel[i]*time + p->accel[i]*time2;
		}

		if (p->flags&PART_GRAVITY)
			org[2]+=time2*-PARTICLE_GRAVITY;

		size = p->size + p->sizevel*time;

		for (i=0;i<P_LIGHTS_MAX;i++)
		{
			const cplight_t *plight = &p->lights[i];
			if (plight->isactive)
			{
				light = plight->light*alpha + plight->lightvel*time;
				V_AddLight (org, light, plight->lightcol[0], plight->lightcol[1], plight->lightcol[2]);
			}
		}

		if (p->thinknext && p->think)
		{
			p->thinknext=false;
			p->think(p, org, angle, &alpha, &size, &image, &time);
		}

#ifdef DECALS
		if (flags & PART_DECAL)
		{
			decalpolys_t *d;
			if (p->decalnum > 0 && p->decal)
				for (d=p->decal; d; d=d->nextpoly)
					V_AddDecal (org, angle, color, alpha, p->blendfunc_src, p->blendfunc_dst, size, image, flags, d);
			else
				V_AddDecal (org, angle, color, alpha, p->blendfunc_src, p->blendfunc_dst, size, image, flags, NULL);
			decals++;
		}
		else
#endif
			V_AddParticle (org, angle, color, alpha, p->blendfunc_src, p->blendfunc_dst, size, image, flags);
		
		if (p->alphavel == INSTANT_PARTICLE)
		{
			p->alphavel = 0.0;
			p->alpha = 0.0;
		}
		VectorCopy(org, p->oldorg);
	}

	active_particles = active;
#ifdef DECALS
	CleanDecalPolys(); // clean up active_decals linked list
#endif
}


void CL_ClearDlights (void);
void CL_ClearLightStyles (void);

/*
==============
CL_ClearEffects

==============
*/
void CL_ClearEffects (void)
{
	CL_ClearParticles ();
#ifdef DECALS
	CL_ClearAllDecalPolys ();
#endif
	CL_ClearDlights ();
	CL_ClearLightStyles ();
}

#ifdef DECALS
/*
==============
CL_UnclipDecals
Removes decal fragment pointers
and resets decal fragment data
Called during a vid_restart
==============
*/
void CL_UnclipDecals (void)
{
	cparticle_t	*p;

	//Com_Printf ("Unclipping decals\n");
	for (p=active_particles; p; p=p->next)
	{
		p->decalnum = 0;
		p->decal = NULL;
	}
	CL_ClearAllDecalPolys();
}

/*
==============
CL_ReclipDecals
Re-clips all decals
Called during a vid_restart
==============
*/
void CL_ReclipDecals (void)
{
	cparticle_t	*p;
	vec3_t		dir;

	//Com_Printf ("Reclipping decals\n");
	for (p=active_particles; p; p=p->next)
	{
		p->decalnum = 0;
		p->decal = NULL;
		if (p->flags & PART_DECAL)
		{
			AngleVectors (p->angle, dir, NULL, NULL);
			VectorNegate(dir, dir);
			clipDecal(p, p->size, -p->angle[2], p->org, dir);

			if (!p->decalnum) // kill on viewframe
				p->alpha = 0;
		}
	}
}
#endif
