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
// cl_newfx.c -- MORE entity effects parsing and management

#include "client.h"
#include "particles.h"

extern cparticle_t	*active_particles, *free_particles;
extern cparticle_t	particles[MAX_PARTICLES];
extern int			cl_numparticles;

extern void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up);

//Knightmare- Psychospaz's enhanced particles
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
			qboolean thinknext);
//end Knightmare

/*
======
vectoangles2 - this is duplicated in the game DLL, but I need it here.
======
*/
void vectoangles2 (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
	// PMM - fixed to correct for pitch of 0
		if (value1[0])
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

//=============
//=============
void CL_Flashlight (int ent, vec3_t pos)
{
	cdlight_t	*dl;

	dl = CL_AllocDlight (ent);
	VectorCopy (pos,  dl->origin);
	dl->radius = 400;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[0] = 1;
	dl->color[1] = 1;
	dl->color[2] = 1;
}

/*
======
CL_ColorFlash - flash of light
======
*/
void CL_ColorFlash (vec3_t pos, int ent, int intensity, float r, float g, float b)
{
	cdlight_t	*dl;

	dl = CL_AllocDlight (ent);
	VectorCopy (pos,  dl->origin);
	dl->radius = intensity;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
}


/*
======
CL_DebugTrail
======
*/
void CL_DebugTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	float		dec;
	vec3_t		right, up;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	MakeNormalVectors (vec, right, up);

	dec = 2;
	VectorScale (vec, dec, vec);
	VectorCopy (start, move);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			50,	50,	255,
			0,	0,	0,
			1,		-0.75,
			GL_SRC_ALPHA, GL_ONE,
			7.5,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_ForceWall
===============
*/
void CL_ForceWall (vec3_t start, vec3_t end, int color8)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorScale (vec, 4, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= 4;
		
		if (frand() > 0.3)
			setupParticle (
				0,	0,	0,
				move[0] + crand()*3,	move[1] + crand()*3,	move[2] + crand()*3,
				0,	0,	-40 - (crand()*10),
				0,		0,		0,
				color[0]+5,	color[1]+5,	color[2]+5,
				0,	0,	0,
				1,		-1.0 / (3.0+frand()*0.5),
				GL_SRC_ALPHA, GL_ONE,
				5,			0,			
				particle_generic,
				0,
				NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_BubbleTrail2 (lets you control the # of bubbles by setting the distance between the spawns)

===============
*/
void CL_BubbleTrail2 (vec3_t start, vec3_t end, int dist)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;
	float		dec, size;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = dist;
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
			0.75,	-0.5 / (1 + frand() * 0.2),
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			size,		1,			
			particle_bubble,
			PART_TRANS|PART_SHADED,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_HeatbeamParticles
===============
*/
void CL_HeatbeamParticles (vec3_t start, vec3_t forward)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	vec3_t		right, up;
	int			i;
	float		c, s;
	vec3_t		dir;
	float		ltime;
	float		step, rstep;
	float		start_pt;
	float		rot;
	float		variance;
	float		size;
	int			maxsteps;
	vec3_t		end;

	VectorMA (start, 4096, forward, end);

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// FIXME - pmm - these might end up using old values?
	//MakeNormalVectors (vec, right, up);
	VectorCopy (cl.v_right, right);
	VectorCopy (cl.v_up, up);

	if (cl_3dcam->value) {
		ltime = (float) cl.time/250.0;
		step = 96;
		maxsteps = 10;
		variance = 1.2;
		size = 2;
	}
	else {
		ltime = (float) cl.time/1000.0;
		step = 32;
		maxsteps = 7;
		variance = 0.5;
		size = 1;
	}

	// this just misaligns rings with beam
	//VectorMA (move, -0.5, right, move);
	//VectorMA (move, -0.5, up, move);

	//ltime = (float) cl.time/1000.0;
	start_pt = fmod(ltime*96.0,step);
	VectorMA (move, start_pt, vec, move);

	VectorScale (vec, step, vec);

	//Com_Printf ("%f\n", ltime);
	rstep = M_PI/10.0*min(cl_particle_scale->value, 2);
	for (i=start_pt; i<len; i+=step)
	{
		if (i>step*maxsteps) // don't bother after the nth ring
			break;

		for (rot = 0; rot < M_PI*2; rot += rstep)
		{
		//	variance = 0.5;
			c = cos(rot)*variance;
			s = sin(rot)*variance;
			
			// trim it so it looks like it's starting at the origin
			if (i < 10)
			{
				VectorScale (right, c*(i/10.0), dir);
				VectorMA (dir, s*(i/10.0), up, dir);
			}
			else
			{
				VectorScale (right, c, dir);
				VectorMA (dir, s, up, dir);
			}

			setupParticle (
				0,	0,	0,
				move[0]+dir[0]*2,	move[1]+dir[1]*2,	move[2]+dir[2]*2, //Knightmare- decreased radius
				0,	0,	0,
				0,		0,		0,
				200+rand()*50,	200+rand()*25,	rand()*50,
				0,	0,	0,
				0.25,	-1000.0, // decreased alpha
				GL_SRC_ALPHA, GL_ONE,
				size,		1,		// shrunk size
				particle_blaster,
				0,
				NULL,0);
		}
		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_ParticleSteamEffect

Puffs with velocity along direction, with some randomness thrown in
===============
*/
void CL_ParticleSteamEffect (vec3_t org, vec3_t dir, int red, int green, int blue,
							 int reddelta, int greendelta, int bluedelta, int count, int magnitude)
{
	int			i;
	cparticle_t	*p;
	float		d;
	vec3_t		r, u;
	//vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	//vectoangles2 (dir, angle_dir);
	//AngleVectors (angle_dir, f, r, u);

	MakeNormalVectors (dir, r, u);

	for (i=0 ; i<count ; i++)
	{
		p = setupParticle (
			0,	0,	0,
			org[0]+magnitude*0.1*crand(),	org[1]+magnitude*0.1*crand(),	org[2]+magnitude*0.1*crand(),
			0,	0,	0,
			0,		0,		0,
			red,	green,	blue,
			reddelta,	greendelta,	bluedelta,
			0.5,		-1.0 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			4,			-2,			
			particle_smoke,
			0,
			NULL,0);

		if (!p)
			return;

		VectorScale (dir, magnitude, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, r, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, u, p->vel);
	}
}


/*
===============
CL_ParticleSteamEffect2

Puffs with velocity along direction, with some randomness thrown in
===============
*/
void CL_ParticleSteamEffect2 (cl_sustain_t *self)
//vec3_t org, vec3_t dir, int color, int count, int magnitude)
{
	int			i;
	cparticle_t	*p;
	float		d;
	vec3_t		r, u;
	vec3_t		dir;
	int			color8 = self->color + (rand()&7);
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	//vectoangles2 (dir, angle_dir);
	//AngleVectors (angle_dir, f, r, u);

	VectorCopy (self->dir, dir);
	MakeNormalVectors (dir, r, u);

	for (i=0; i<self->count; i++)
	{
		p = setupParticle (
			0,		0,		0, //angle
			self->org[0] + self->magnitude*0.1*crand(),
			self->org[1] + self->magnitude*0.1*crand(),
			self->org[2] + self->magnitude*0.1*crand(), //orgs
			0,		0,		0,//vel
			0,		0,		0,//accel
			//color[0],	color[1],	color[2],
			255,255,255,
			-50,		-50,		-50,
			1.0,		-1.0 / (0.9 + frand()*0.3),//alpha,alphavel
			GL_SRC_ALPHA, GL_ONE,
			8,			32,	//size, sizevel.		
			particle_smoke,
			PART_TRANS|PART_SHADED|PART_GRAVITY,
			NULL,0);

		if (!p)
			return;

		VectorScale (dir, self->magnitude, p->vel);
		d = crand()*self->magnitude/3;
		VectorMA (p->vel, d, r, p->vel);
		d = crand()*self->magnitude/3;
		VectorMA (p->vel, d, u, p->vel);
	}
	self->nextthink += self->thinkinterval;
}


/*
===============
CL_TrackerTrail
===============
*/
void CL_TrackerTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	vec3_t		forward,right,up,angle_dir;
	float		len;
	cparticle_t	*p;
	int			dec;
	float		dist;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorCopy(vec, forward);
	vectoangles2 (forward, angle_dir);
	AngleVectors (angle_dir, forward, right, up);

	dec = 3*max(cl_particle_scale->value/2, 1);
	VectorScale (vec, 3*max(cl_particle_scale->value/2, 1), vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		p = setupParticle (
			0,	0,	0,
			0,	0,	0,
			0,	0,	5,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			1.0,	-2.0,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			2,		0,			
			particle_generic,
			PART_TRANS,
			NULL,0);

		if (!p)
			return;

		dist = DotProduct(move, forward);
		VectorMA(move, 8 * cos(dist), up, p->org);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_TrackerShell
===============
*/
void CL_Tracker_Shell(vec3_t origin)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;

	for(i=0; i < (300/cl_particle_scale->value); i++)
	{
		p = setupParticle (
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			1.0,	-2.0,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			1,		0,	//Knightmare- changed size		
			particle_generic,
			PART_TRANS,
			NULL,0);

		if (!p)
			return;

		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorMA(origin, 40, dir, p->org);
	}
}


/*
======================
CL_MonsterPlasma_Shell
======================
*/
void CL_MonsterPlasma_Shell(vec3_t origin)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;

	for(i=0; i<40; i++)
	{
		p = setupParticle (
			0,		0,		0,
			0,		0,		0,
			0,		0,		0,
			0,		0,		0,
			220,	140,	50, //Knightmare- this was all black
			0,		0,		0,
			1.0,	INSTANT_PARTICLE,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			2,		0,			
			particle_generic,
			PART_TRANS|PART_INSTANT,
			NULL,0);

		if (!p)
			return;

		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(origin, 10, dir, p->org);
	}
}


/*
===============
CL_Widowbeamout
===============
*/
void CL_Widowbeamout (cl_sustain_t *self)
{
	vec3_t			dir;
	int				i;
	static int colortable0[6] = {125,	255,	185,	125,	185,	255};
	static int colortable1[6] = {185,	125,	255,	255,	125,	185};
	static int colortable2[6] = {255,	185,	125,	185,	255,	125};
	cparticle_t		*p;
	float			ratio;
	int				index;

	ratio = 1.0 - (((float)self->endtime - (float)cl.time)/2100.0);

	for(i=0; i<300; i++)
	{
		index = rand()&5;
		p = setupParticle (
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			colortable0[index],	colortable1[index],	colortable2[index],
			0,	0,	0,
			1.0,	INSTANT_PARTICLE,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			2,		0,			
			particle_generic,
			PART_TRANS|PART_INSTANT,
			NULL,0);

		if (!p)
			return;

		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(self->org, (45.0 * ratio), dir, p->org);
	}
}


/*
============
CL_Nukeblast
============
*/
void CL_Nukeblast (cl_sustain_t *self)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;
	static int colortable0[4] = {185,	155,	125,	95};
	static int colortable1[4] = {185,	155,	125,	95};
	static int colortable2[4] = {255,	255,	255,	255};
	float			ratio, size;
	int				index;

	ratio = 1.0 - (((float)self->endtime - (float)cl.time)/1000.0);
	size = ratio*ratio;

	for(i=0; i<(700/cl_particle_scale->value); i++)
	{
		index = rand()&3;
		p = setupParticle (
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			colortable0[index],	colortable1[index],	colortable2[index],
			0,	0,	0,
			1-size,	INSTANT_PARTICLE,
			GL_SRC_ALPHA, GL_ONE,
			10*(0.5+ratio*0.5),	-1,			
			particle_generic,
			PART_INSTANT,
			NULL,0);

		if (!p)
			return;


		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorScale(dir, -1, p->angle);
		VectorMA(self->org, 200.0*size, dir, p->org); //was 100
		VectorMA(vec3_origin, 400.0*size, dir, p->vel); //was 100

	}
}


/*
==============
CL_WidowSplash
==============
*/
void CL_WidowSplash (vec3_t org)
{
	static int colortable[4] = {2*8,13*8,21*8,18*8};
	int			i;
	cparticle_t	*p;
	vec3_t		dir;

	for (i=0; i<256; i++)
	{
		p = setupParticle (
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			rand()&255,	rand()&255,	rand()&255,
			0,	0,	0,
			1.0,		-0.8 / (0.5 + frand()*0.3),
			GL_SRC_ALPHA, GL_ONE,
			3,			0,			
			particle_generic,
			0,
			NULL,0);

		if (!p)
			return;

		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorMA(org, 45.0, dir, p->org);
		VectorMA(vec3_origin, 40.0, dir, p->vel);
	}
}


/*
==================
CL_Tracker_Explode
==================
*/
void CL_Tracker_Explode (vec3_t	origin)
{
	vec3_t			dir, backdir;
	int				i;
	cparticle_t		*p;

	for (i=0; i<(300/cl_particle_scale->value); i++)
	{
		p = setupParticle (
			0,		0,		0,
			0,		0,		0,
			0,		0,		0,
			0,		0,		-10, //was 20
			0,		0,		0,
			0,		0,		0,
			1.0,	-0.5,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			2,		0,			
			particle_generic,
			PART_TRANS,
			NULL,0);

		if (!p)
			return;

		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorScale(dir, -1, backdir);

		VectorCopy (origin, p->org); //Knightmare- start at center, not edge
	//	VectorMA(origin, 64, dir, p->org); 
		VectorScale(dir, (crand()*128), p->vel); //was backdir, 64
	}
	
}


/*
===============
CL_TagTrail
===============
*/
void CL_TagTrail (vec3_t start, vec3_t end, int color8)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len >= 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand()*16,	move[1] + crand()*16,	move[2] + crand()*16,
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		20,
			color[0],	color[1],	color[2],
			0,	0,	0,
			1.0,		-1.0 / (0.8+frand()*0.2),
			GL_SRC_ALPHA, GL_ONE,
			1.5,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
==========================
CL_ColorExplosionParticles
==========================
*/
void CL_ColorExplosionParticles (vec3_t org, int color8, int run)
{
	int			i;
	vec3_t color = {color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<128 ; i++)
	{
		setupParticle (
			0,	0,	0,
			org[0] + ((rand()%32)-16),	org[1] + ((rand()%32)-16),	org[2] + ((rand()%32)-16),
			(rand()%256)-128,	(rand()%256)-128,	(rand()%256)-128,
			0,		0,		20,
			color[0] + (rand() % run),	color[1] + (rand() % run),	color[2] + (rand() % run),
			0,	0,	0,
			1.0,		-0.4 / (0.6 + frand()*0.2),
			GL_SRC_ALPHA, GL_ONE,
			2,			1,			
			particle_generic,
			0,
			NULL,0);
	}
}


/*
=======================
CL_ParticleSmokeEffect - like the steam effect, but unaffected by gravity
=======================
*/
void pRotateThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	angle[2] =	angle[0] + *time*angle[1] + *time**time*angle[2];
	p->thinknext=true;
}

void CL_ParticleSmokeEffect (vec3_t org, vec3_t dir, float size)
{
	float alpha = fabs(crand())*0.25 + 0.750;

	setupParticle (
		crand()*180, crand()*100, 0,
		org[0],	org[1],	org[2],
		dir[0],	dir[1],	dir[2],
		0,		0,		10,
		255,	255,	255,
		0,	0,	0,
		alpha,		-1.0,
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
		size,			5,			
		particle_smoke,
		PART_TRANS|PART_SHADED|PART_OVERBRIGHT,
		pRotateThink,true);
}


/*
===============
CL_ElectricParticles

new sparks for Rogue turrets
===============
*/
void pElectricSparksThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
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

void CL_ElectricParticles (vec3_t org, vec3_t dir, int count)
{
	int			i, j;
	vec3_t		start;
	float d;

	for (i = 0; i < 40; i++)
	{
		d = rand()&31;

		for (j = 0; j < 3; j++)
			start[j] = org[j] + ((rand()&2));
			//start[j] = org[j] + ((rand()&7)-4) + d*dir[j];

		setupParticle (
			0,			0,			0,
			start[0],	start[1],	start[2],
			crand()*20,	crand()*20,	crand()*20,
			0,			0,			-PARTICLE_GRAVITY,
			25,			100,		255,
			50,			50,			50,
			1,		-4, //alpha
			GL_SRC_ALPHA, GL_ONE,
			1,		-3,		//size	
			particle_blaster,
			PART_GRAVITY|PART_SPARK,
			pElectricSparksThink, true);
	}
}


//Knightmare- removed for Psychospaz's enhanced particle code
#if 0
/*
===============
CL_SmokeTrail
===============
*/
void CL_SmokeTrail (vec3_t start, vec3_t end, int colorStart, int colorRun, int spacing)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorScale (vec, spacing, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= spacing;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = cl.time;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (1+frand()*0.5);
		p->color = colorStart + (rand() % colorRun);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand()*3;
			p->accel[j] = 0;
		}
		p->vel[2] = 20 + crand()*5;

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_FlameEffects
===============
*/
void CL_FlameEffects (centity_t *ent, vec3_t origin)
{
	int			n, count;
	int			j;
	cparticle_t	*p;

	count = rand() & 0xF;

	for(n=0;n<count;n++)
	{
		if (!free_particles)
			return;
			
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		
		VectorClear (p->accel);
		p->time = cl.time;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (1+frand()*0.2);
		p->color = 226 + (rand() % 4);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = origin[j] + crand()*5;
			p->vel[j] = crand()*5;
		}
		p->vel[2] = crand() * -10;
		p->accel[2] = -PARTICLE_GRAVITY;
	}

	count = rand() & 0x7;

	for(n=0;n<count;n++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = cl.time;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (1+frand()*0.5);
		p->color = 0 + (rand() % 4);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = origin[j] + crand()*3;
		}
		p->vel[2] = 20 + crand()*5;
	}

}


/*
===============
CL_GenericParticleEffect
===============
*/
void CL_GenericParticleEffect (vec3_t org, vec3_t dir, int color, int count, int numcolors, int dirspread, float alphavel)
{
	int			i, j;
	cparticle_t	*p;
	float		d;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = cl.time;
		if (numcolors > 1)
			p->color = color + (rand() & numcolors);
		else
			p->color = color;

		d = rand() & dirspread;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()&7)-4) + d*dir[j];
			p->vel[j] = crand()*20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
//		VectorCopy (accel, p->accel);
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand()*alphavel);
//		p->alphavel = alphavel;
	}
}


/*
===============
CL_BlasterParticles2

Wall impact puffs (Green)
===============
*/
void CL_BlasterParticles2 (vec3_t org, vec3_t dir, unsigned int color)
{
	int			i, j;
	cparticle_t	*p;
	float		d;
	int			count;

	count = 40;
	for (i = 0; i < count; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = cl.time;
		p->color = color + (rand()&7);

		d = rand()&15;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()&7)-4) + d*dir[j];
			p->vel[j] = dir[j] * 30 + crand()*40;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand()*0.3);
	}
}

/*
===============
CL_BlasterTrail2

Green!
===============
*/
void CL_BlasterTrail2 (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = cl.time;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.3+frand()*0.2);
		p->color = 0xd0;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand();
			p->vel[j] = crand()*5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}

//Knightmare 1/3/2002- blue blaster effect
/*
===============
CL_BlasterTrail3

Blue!
===============
*/
void CL_BlasterTrail3 (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = cl.time;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.3+frand()*0.2);
		p->color = 0x71;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand();
			p->vel[j] = crand()*5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}
#endif
//end Knightmare