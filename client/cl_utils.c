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

// cl_util.c -- misc client utility functions

#include "client.h"

//=================================================

// Here I convert old 256 color palette to RGB
const byte default_pal[768] = {
#include "q2palette.h"
};


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

//=================================================

/*
================
CL_TextColor
This sets the actual text color, can be called from anywhere
================
*/
void CL_TextColor (int colornum, int *red, int *green, int *blue)
{
	if (!red || !green || !blue) // paranoia
		return;

	switch (colornum)
	{
#ifdef NOTTHIRTYFLIGHTS
		case 1:		// red
			*red =	255;
			*green=	0;
			*blue =	0;
			break;
		case 2:		// green
			*red =	0;
			*green=	255;
			*blue =	0;
			break;
		case 3:		// yellow
			*red =	255;
			*green=	255;
			*blue =	0;
			break;
		case 4:		// blue
			*red =	0;
			*green=	0;
			*blue =	255;
			break;
#else
		case 1:		// red
			*red =	212;
			*green=	25;
			*blue =	25;
			break;
		case 2:		// green
			*red =	100;
			*green=	220;
			*blue =	100;
			break;
		case 3:		// yellow
			*red =	240;
			*green=	235;
			*blue =	90;
			break;
		case 4:		// blue
			*red =	130;
			*green=	180;
			*blue =	250;
			break;
#endif
		case 5:		// cyan
			*red =	0;
			*green=	255;
			*blue =	255;
			break;
		case 6:		//magenta
			*red =	255;
			*green=	0;
			*blue =	255;
			break;
		case 7:		// white
			*red =	255;
			*green=	255;
			*blue =	255;
			break;
		case 8:		// black
			*red =	0;
			*green=	0;
			*blue =	0;
			break;
		case 9:		// orange
			*red =	255;
			*green=	135;
			*blue =	0;
			break;
		case 0:		// gray
			*red =	155;
			*green=	155;
			*blue =	155;
			break;
		default:	// white
			*red =	255;
			*green=	255;
			*blue =	255;
			break;
	}
}


/*
================
CL_EffectColor
This sets railtrail color
================
*/
void CL_EffectColor (int colornum, int *red, int *green, int *blue)
{
	if (!red || !green || !blue) // paranoia
		return;

	switch (colornum)
	{
		case 1:		// blue
			*red =	20;
			*green=	50;
			*blue =	175;
			break;
		case 2:		// green
			*red =	20;
			*green=	255;
			*blue =	20;
			break;
		case 3:		// yellow
			*red =	255;
			*green=	255;
			*blue =	20;
			break;
		case 4:		// orange
			*red =	255;
			*green=	135;
			*blue =	20;
			break;
		case 5:		// red
			*red =	255;
			*green=	20;
			*blue =	20;
			break;
		case 6:		// cyan
			*red =	20;
			*green=	255;
			*blue =	255;
			break;
		case 7:		// indigo
			*red =	80;
			*green=	20;
			*blue =	255;
			break;
		case 8:		// viridian
			*red =	80;
			*green=	255;
			*blue =	180;
			break;
		case 9:		// violet
			*red =	160;
			*green=	20;
			*blue =	255;
			break;
		case 10:	// magenta
			*red =	255;
			*green=	20;
			*blue =	255;
			break;
		case 11:	// pink
			*red =	255;
			*green=	125;
			*blue =	175;
			break;
		case 12:	// white
			*red =	255;
			*green=	255;
			*blue =	255;
			break;
		case 13:	// silver
			*red =	195;
			*green=	195;
			*blue =	195;
			break;
		case 14:	// gray
			*red =	155;
			*green=	155;
			*blue =	155;
			break;
		case 15:	// black
			*red =	0;
			*green=	0;
			*blue =	0;
			break;
		case 0:
		default:	// blue
			*red =	20;
			*green=	50;
			*blue =	255;
			break;
	}
}


/*
==========================
ClampCvar
==========================
*/
float ClampCvar (float min, float max, float value)
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}


/*
==========================
stringLen
==========================
*/
int stringLen (const char *string)
{
	return (int)strlen(string) - stringLengthExtra(string);
}


/*
==========================
stringLengthExtra
==========================
*/
int stringLengthExtra (const char *string)
{
	unsigned	i, j;
	char		modifier;
//	float		len = strlen( string );
	int			len = (int)strlen( string );

	for ( i = 0, j = 0; i < len; i++ )
	{
		modifier = string[i];
		if (modifier>128)
			modifier-=128;

		if ((string[i] == '^') || (i>0 && string[i-1] == '^'))
			j++;
	}

	return j;
}


/*
==========================
unformattedString
==========================
*/
char *unformattedString (const char *string)
{
	unsigned int	i;
	size_t			len;
	char			character;
//	char			*newstring = "";
	static char		newstring[MSG_STRING_SIZE];
	char			addchar[2];

	len = strlen (string);
	newstring[0] = '\0';	// init as blank

	for ( i = 0; i < len; i++ )
	{
		character = (string[i] & ~128);

		if ( (character == '^') && (i < len) ) {	// skip formatting codes
			i++;
			continue;
		}

		if (character < 32) {	// skip unprintable chars
			continue;
		}

	//	va("%s%c", newstring, character);
		addchar[0] = character;
		addchar[1] = '\0';
		Q_strncatz (newstring, sizeof(newstring), addchar);
	}

	return newstring;
}


/*
==========================
listSize
==========================
*/
int listSize (char* list[][2])
{
	int i=0;
	while (list[i][1])
		i++;

	return i;	
}


/*
==========================
isNumeric
==========================
*/
qboolean isNumeric (char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	else return false;
}

//=================================================

/*
==========================
vectoangles
==========================
*/
void vectoangles (vec3_t value1, vec3_t angles)
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


/*
==========================
vectoangles2
==========================
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
==================
LegacyProtocol
A utility function that determines
if parsing of old protocol should be used.
==================
*/
qboolean LegacyProtocol (void)
{
//	if (dedicated->integer)	// Server always uses new protocol
//		return false;
	if ( (Com_ServerState() && cls.serverProtocol <= OLD_PROTOCOL_VERSION)
		|| (cls.serverProtocol == OLD_PROTOCOL_VERSION) )
		return true;
	return false;
}

/*
==================
R1Q2Protocol
A utility function that determines
if parsing of R1Q2 protocol should be used.
==================
*/
qboolean R1Q2Protocol (void)
{
	//if (dedicated->integer)	// Server always uses new protocol
	//	return false;
	if ( cls.serverProtocol == R1Q2_PROTOCOL_VERSION )
		return true;
	return false;
}

/*
===============
IsRunningDemo
Returns true if a demo is currently running.
===============
*/
qboolean IsRunningDemo (void)
{
	if ( cl.attractloop && !(cl.cinematictime > 0 && cls.realtime - cl.cinematictime > 1000) )
		return true;
	return false;
}

/*
===============
IsThirdPerson
Returns true if the thirdperson
cvar is set and other conditons are met.
===============
*/
qboolean IsThirdPerson (void)
{
	if ( cg_thirdperson->integer && (!IsRunningDemo() || cg_thirdperson_indemo->integer) )
		return true;
	return false;
}

/*
================
CL_EntityCmpFnc
================
*/
int CL_EntityCmpFnc (const entity_t *a, const entity_t *b)
{
	/*
	** all other models are sorted by model then skin
	*/
	if ( a->model == b->model )
	{
	//	return ( ( int ) a->skin - ( int ) b->skin );
		return ( ( size_t ) a->skin - ( size_t ) b->skin );
	}
	else
	{
	//	return ( ( int ) a->model - ( int ) b->model );
		return ( ( size_t ) a->model - ( size_t ) b->model );
	}
}
