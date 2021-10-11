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

// Vic did core of this - THANK YOU KING VIC!!! - psychospaz

#include "r_local.h"
#include "vlights.h"

#define VLIGHT_CLAMP_MIN	50.0f
#define VLIGHT_CLAMP_MAX	256.0f

#define VLIGHT_GRIDSIZE_X	256
#define VLIGHT_GRIDSIZE_Y	256

vec3_t	vlightgrid[VLIGHT_GRIDSIZE_X][VLIGHT_GRIDSIZE_Y];


void VLight_InitAnormTable (void)
{
	int x, y;
	float angle;
	float sp, sy, cp, cy;
	
	for ( x = 0; x < VLIGHT_GRIDSIZE_X; x++ )
	{
		angle = (x * 360 / VLIGHT_GRIDSIZE_X) * ( M_PI / 180.0f );
		sy = sin(angle);
		cy = cos(angle);

		for ( y = 0; y < VLIGHT_GRIDSIZE_Y; y++ )
		{
			angle = (y * 360 / VLIGHT_GRIDSIZE_X) * ( M_PI / 180.0f );
			sp = sin(angle);
			cp = cos(angle);
			
			vlightgrid[x][y][0] = cp*cy;
			vlightgrid[x][y][1] = cp*sy;
			vlightgrid[x][y][2] = -sp;
		}
	}
}

float VLight_GetLightValue ( vec3_t normal, vec3_t dir, float apitch, float ayaw, qboolean dlight )
{
	int pitchofs, yawofs;
	float angle1, angle2, light;

	if ( normal[1] == 0 && normal[0] == 0 )
	{
		angle2 = 0;
		if ( normal[2] > 0 )
			angle1 = 90;
		else
			angle1 = 270;
	}
	else
	{
		float forward;

		angle2 = atan2( normal[1], normal[0] ) * ( 180.0f / M_PI );
		if (angle2 < 0)
			angle2 += 360;
		
		forward = sqrt ( normal[0]*normal[0] + normal[1]*normal[1] );
		angle1 = atan2( normal[2], forward ) * ( 180.0f / M_PI );
		if (angle1 < 0)
			angle1 += 360;
	}
	
	pitchofs = ( angle1 + apitch ) * VLIGHT_GRIDSIZE_X / 360;
	yawofs = ( angle2 + ayaw ) * VLIGHT_GRIDSIZE_Y / 360;

	while (pitchofs > (VLIGHT_GRIDSIZE_X-1))
		pitchofs -= VLIGHT_GRIDSIZE_X;
	while (pitchofs < 0)
		pitchofs += VLIGHT_GRIDSIZE_X;

	while (yawofs > (VLIGHT_GRIDSIZE_Y-1))
		yawofs -= VLIGHT_GRIDSIZE_Y;
	while (yawofs < 0)
		yawofs += VLIGHT_GRIDSIZE_Y;
	
	if (dlight)
	{

		light = DotProduct( vlightgrid[pitchofs][yawofs], dir );
		
		if (light>1)
			light=1;

		if (light<0)
			light=0;

		return light;
	}
	else
	{
		light = ( DotProduct( vlightgrid[pitchofs][yawofs], dir ) + 2.0f ) * 63.5f;
		
		if (light > VLIGHT_CLAMP_MAX)
			light = VLIGHT_CLAMP_MAX;
		if (light < VLIGHT_CLAMP_MIN)
			light = VLIGHT_CLAMP_MIN;
		
		return light * ( 1.0f / 256.0f );
	}
	
}

float VLight_LerpLight ( int index1, int index2, float ilerp, vec3_t dir, vec3_t angles, qboolean dlight )
{
	vec3_t normal;

	normal[0] = r_avertexnormals[index1][0] + ( r_avertexnormals[index2][0] - r_avertexnormals[index1][0] ) * ilerp;
	normal[1] = r_avertexnormals[index1][1] + ( r_avertexnormals[index2][1] - r_avertexnormals[index1][1] ) * ilerp;
	normal[2] = r_avertexnormals[index1][2] + ( r_avertexnormals[index2][2] - r_avertexnormals[index1][2] ) * ilerp;
	VectorNormalize ( normal );

	return VLight_GetLightValue( normal, dir, angles[PITCH], angles[YAW], dlight );
}

void VLight_Init (void)
{
	VLight_InitAnormTable ();
}
