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


enum {
	TEXT_LEFT,
	TEXT_CENTER,
	TEXT_RIGHT
};

typedef struct texthnd_s {
	struct text_s *lines;
	int		nlines;
	int		curline;
	int		size;
	int		flags;
	int		allocated;
	int		page_length;
	int		page_width;
	float	last_update;
	char	background_image[32];
	int		start_char;
	byte	*buffer;
} texthnd_t;

typedef struct text_s {
	char *text;
} text_t;

void Text_Open(edict_t *ent);
void Text_Close(edict_t *ent);
void Text_Update(edict_t *ent);
void Text_Next(edict_t *ent);
void Text_Prev(edict_t *ent);

