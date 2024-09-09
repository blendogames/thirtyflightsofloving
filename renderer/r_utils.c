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

// r_main.c

#include "r_local.h"

//=======================================================================

/*
=================
R_ClampValue
=================
*/
float R_ClampValue (float in, float min, float max)
{
	if (in < min)	return min;
	if (in > max)	return max;
	return in;
}


/*
=================
R_SmoothStep
=================
*/
float R_SmoothStep (float in, float side0, float side1)
{
	float	tmp, diff0, diff1;

	diff0 = in - side0;
	diff1 = side1 - side0;
	if (diff1 != 0.0f)	// prevent divide by 0
		tmp = R_ClampValue ((diff0 / diff1), 0.0f, 1.0f);
	else
		tmp = 1.0f;
	return tmp * tmp * (3.0f - 2.0f * tmp);
}


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	if (r_nocull->integer)
		return false;

	for (i=0; i<4; i++)
		if (BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


/*
=================
R_CopyString
=================
*/
char *R_CopyString (char *in)
{
	char	*out;
	size_t	outSize;
	
	outSize = strlen(in)+1;
	out = Z_Malloc (outSize);
	Q_strncpyz (out, outSize, in);
	return out;
}
