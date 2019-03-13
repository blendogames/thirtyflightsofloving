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
==========================
stringLen
==========================
*/
int stringLen (const char *string)
{
	return strlen(string) - stringLengthExtra(string);
}


/*
==========================
stringLengthExtra
==========================
*/
int stringLengthExtra (const char *string)
{
	unsigned i, j;
	char modifier;
	float len = strlen( string );

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
	unsigned i;
	int len;
	char character;
	char *newstring = "";

	len = strlen (string);

	for ( i = 0; i < len; i++ )
	{
		character = string[i];

		if (character&128) character &= ~128;
		if (character == '^' && i < len)
		{
			i++;
			continue;
		}
		character = string[i];

		va("%s%c", newstring, character);
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
