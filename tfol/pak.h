/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

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

typedef struct
{
	char id[4]; // Should be 'PACK'
	int dstart; // Offest in the file to the directory
	int dsize;  // Size in bytes of the directory, same as num_items*64
} pak_header_t;

typedef struct
{
	char name[56]; // The name of the item, normal C string
	int start; // Offset in .pak file to start of item
	int size; // Size of item in bytes
} pak_item_t;
