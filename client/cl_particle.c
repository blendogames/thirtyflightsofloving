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

// cl_particle.c -- particle effect and decal management

#include "client.h"
#include "particles.h"

/*
===============
CL_SetParticleImages
This initializes all particle images - mods play with this...
===============
*/
void CL_SetParticleImages (void)
{
	R_SetParticlePicture(particle_solid,		"***whitetexture***");				// only used for sparks
	R_SetParticlePicture(particle_generic,		"gfx/particles/basic.tga");
	R_SetParticlePicture(particle_smoke,		"gfx/particles/smoke.tga");
	R_SetParticlePicture(particle_blood,		"gfx/particles/blood.tga");
	R_SetParticlePicture(particle_blooddrop,	"gfx/particles/blood_drop.tga");
	R_SetParticlePicture(particle_blooddrip,	"gfx/particles/blood_drip.tga");
	R_SetParticlePicture(particle_redblood,		"gfx/particles/blood_red.tga");
	R_SetParticlePicture(particle_bubble,		"gfx/particles/bubble.tga");
	R_SetParticlePicture(particle_blaster,		"gfx/particles/blaster.tga");
	R_SetParticlePicture(particle_blasterblob,	"gfx/particles/blaster_blob.tga");
	R_SetParticlePicture(particle_beam,			"gfx/particles/beam.tga");
	R_SetParticlePicture(particle_beam2,		"gfx/particles/beam2.tga"); // only used for railgun
	R_SetParticlePicture(particle_lightning,	"gfx/particles/lightning.tga");
	R_SetParticlePicture(particle_inferno,		"gfx/particles/inferno.tga");
//	R_SetParticlePicture(particle_lensflare,	"gfx/particles/lensflare.tga");
//	R_SetParticlePicture(particle_lightflare,	"gfx/particles/lightflare.jpg");
//	R_SetParticlePicture(particle_shield,		"gfx/particles/shield.jpg");
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
//	R_SetParticlePicture(particle_dexplosion1,	"gfx/particles/d_explod_1.tga");
//	R_SetParticlePicture(particle_dexplosion2,	"gfx/particles/d_explod_2.tga");
//	R_SetParticlePicture(particle_dexplosion3,	"gfx/particles/d_explod_3.tga");

	R_SetParticlePicture(particle_bfgmark,		"gfx/decals/bfgmark.tga");
	R_SetParticlePicture(particle_burnmark,		"gfx/decals/burnmark.tga");
	R_SetParticlePicture(particle_blooddecal1,	"gfx/decals/blood_1.tga");
	R_SetParticlePicture(particle_blooddecal2,	"gfx/decals/blood_2.tga");
	R_SetParticlePicture(particle_blooddecal3,	"gfx/decals/blood_3.tga");
	R_SetParticlePicture(particle_blooddecal4,	"gfx/decals/blood_4.tga");
	R_SetParticlePicture(particle_blooddecal5,	"gfx/decals/blood_5.tga");
	R_SetParticlePicture(particle_shadow,		"gfx/decals/shadow.tga");
	R_SetParticlePicture(particle_bulletmark,	"gfx/decals/bulletmark.tga");
	R_SetParticlePicture(particle_trackermark,	"gfx/decals/trackermark.tga");
//	R_SetParticlePicture(particle_footprint,	"gfx/decals/footprint.tga");
}


/*
===============
CL_GetRandomBloodParticle
===============
*/
int CL_GetRandomBloodParticle (void)
{
	return particle_blooddecal1 + rand()%5;
}


/*
==============================================================

PARTICLE MANAGEMENT

==============================================================
*/

cparticle_t	*active_particles, *free_particles;
cparticle_t	particles[MAX_PARTICLES];
int			cl_numparticles = MAX_PARTICLES;

decalpolys_t	*active_decals, *free_decals;
decalpolys_t	decalfrags[MAX_DECAL_FRAGS];
int				cl_numdecalfrags = MAX_DECAL_FRAGS;


/*
===============
CL_CleanDecalPolys
Cleans up the active_particles linked list
===============
*/
void CL_CleanDecalPolys (void)
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
CL_ClearDecalPoly
Recursively flags a decal poly chain for cleaning
===============
*/
void CL_ClearDecalPoly (decalpolys_t *decal)
{
	if (!decal)
		return;
	if (decal->nextpoly)
		CL_ClearDecalPoly(decal->nextpoly);
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
CL_NumFreeDecalPolys
Retuns number of available decalpoly_t fields
===============
*/
int CL_NumFreeDecalPolys (void)
{
	int count = 0;
	decalpolys_t *d = NULL;
	for (d = free_decals; d; d = d->next)
		count++;
	return count;
}


/*
===============
CL_NewDecalPoly
Retuns first free decal poly
===============
*/
decalpolys_t *CL_NewDecalPoly(void)
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
CL_ClipDecal
===============
*/
void CL_ClipDecal (cparticle_t *part, float radius, float orient, vec3_t origin, vec3_t dir)
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
	if (numfragments > CL_NumFreeDecalPolys()) // not enough decalpolys free
		return;
	
	VectorScale ( axis[1], 0.5f / radius, axis[1] );
	VectorScale ( axis[2], 0.5f / radius, axis[2] );

	part->decalnum = numfragments;
	for ( i = 0, fr = fragments; i < numfragments; i++, fr++ )
	{
		decalpolys_t *decal = CL_NewDecalPoly();
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


/*
===============
CL_NewParticleTime
===============
*/
float CL_NewParticleTime (void)
{
	float lerpedTime = cl.time;
	return lerpedTime;
}


/*
===============
CL_SetupParticle
===============
*/
cparticle_t *CL_SetupParticle (
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

	p->decalnum = 0;
	p->decal = NULL;

	if (flags & PART_DECAL)
	{
		vec3_t dir;
		AngleVectors (p->angle, dir, NULL, NULL);
		VectorNegate(dir, dir);
		CL_ClipDecal(p, p->size, -p->angle[2], p->org, dir);

		if (!p->decalnum) // kill on viewframe
			p->alpha = 0;
	}

	return p;
}


/*
===============
CL_AddParticleLight
===============
*/
void CL_AddParticleLight (cparticle_t *p,
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
		particles[i].decalnum = 0; // Knightmare added
		particles[i].decal = NULL; // Knightmare added
	}
	particles[cl_numparticles-1].next = NULL;
}


/*
======================================

GENERIC PARTICLE THINKING ROUTINES

======================================
*/
//#define SplashSize		10
#define	STOP_EPSILON	0.1

/*
===============
CL_CalcPartVelocity
===============
*/
void CL_CalcPartVelocity (cparticle_t *p, float scale, float *time, vec3_t velocity)
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

/*
===============
CL_ClipParticleVelocity
===============
*/
void CL_ClipParticleVelocity (vec3_t in, vec3_t normal, vec3_t out)
{
	float	backoff, change;
	int		i;
	
	backoff = VectorLength(in)*0.25 + DotProduct (in, normal) * 3.0f;

	for (i=0; i<3; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
}

/*
===============
CL_ParticleBounceThink
===============
*/
void CL_ParticleBounceThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	float clipsize;
	trace_t tr;
	vec3_t velocity;

	clipsize = *size*0.5;
	if (clipsize<0.25) clipsize = 0.25;
	tr = CL_BrushTrace (p->oldorg, org, clipsize, MASK_SOLID); // was 1
	
	if (tr.fraction < 1)
	{
		CL_CalcPartVelocity(p, 1, time, velocity);
		CL_ClipParticleVelocity(velocity, tr.plane.normal, p->vel);

		VectorCopy(vec3_origin, p->accel);
		VectorCopy(tr.endpos, p->org);
		VectorCopy(p->org, org);
		VectorCopy(p->org, p->oldorg);

		p->alpha = *alpha;
		p->size = *size;

		p->start = p->time = CL_NewParticleTime();

		if (p->flags&PART_GRAVITY && VectorLength(p->vel)<2)
			p->flags &= ~PART_GRAVITY;
	}

	p->thinknext = true;
}

/*
===============
CL_ParticleRotateThink
===============
*/
void CL_ParticleRotateThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	angle[2] =	angle[0] + *time*angle[1] + *time**time*angle[2];
	p->thinknext=true;
}

/*
===============
CL_DecalAlphaThink
===============
*/
void CL_DecalAlphaThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	*alpha = pow(*alpha, 0.1);
	p->thinknext = true;
}


//===================================================================

/*
===============
CL_FixParticleCvars
===============
*/
void CL_FixParticleCvars (void)
{
	// clamp to acceptable value (don't allow infinite particles)
	if (cl_particle_scale->value < 1.0f)
		Cvar_SetValue("cl_particle_scale", 1);

	// clamp to acceptable minimum length
	if (cl_rail_length->value < MIN_RAIL_LENGTH)
		Cvar_SetValue("cl_rail_length", MIN_RAIL_LENGTH);

	// clamp to acceptable minimum duration
	if (r_decal_life->value < MIN_DECAL_LIFE)
		Cvar_SetValue("r_decal_life", MIN_DECAL_LIFE);
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

	CL_FixParticleCvars (); // clamp critical effects vars to acceptable bounds

	for (p=active_particles; p; p=next)
	{
		next = p->next;
		flags = p->flags;

		// PMM - added INSTANT_PARTICLE handling for heat beam
		if (p->alphavel != INSTANT_PARTICLE)
		{
			time = (cl.time - p->time)*0.001;
			alpha = p->alpha + time*p->alphavel;
			if (flags&PART_DECAL)
			{
			//	if (decals >= r_decals->value || alpha <= 0)
				if (decals >= r_decals->integer || alpha <= 0)
				{	// faded out
					p->alpha = 0;
					p->flags = 0;
					CL_ClearDecalPoly (p->decal); // flag decal chain for cleaning
					p->decalnum = 0;
					p->decal = NULL;
					p->next = free_particles;
					free_particles = p;
					continue;
				}
			}
			else if (alpha <= 0)
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
			V_AddParticle (org, angle, color, alpha, p->blendfunc_src, p->blendfunc_dst, size, image, flags);
		
		if (p->alphavel == INSTANT_PARTICLE)
		{
			p->alphavel = 0.0;
			p->alpha = 0.0;
		}
		VectorCopy(org, p->oldorg);
	}

	active_particles = active;

	CL_CleanDecalPolys(); // clean up active_decals linked list
}


/*
==============
CL_ClearEffects
==============
*/
void CL_ClearEffects (void)
{
	CL_ClearParticles ();
	CL_ClearAllDecalPolys ();
	CL_ClearDlights ();
	CL_ClearLightStyles ();
}


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
			CL_ClipDecal(p, p->size, -p->angle[2], p->org, dir);

			if (!p->decalnum) // kill on viewframe
				p->alpha = 0;
		}
	}
}
