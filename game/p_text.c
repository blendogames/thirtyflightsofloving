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

#include "g_local.h"
#include "pak.h"

#define MAX_LINES 24
#define MAX_LINE_LENGTH 36 //how long a line can be.  35

text_t text[MAX_LINES];

void Text_Open(edict_t *ent)
{
	if (!ent->client)
		return;
	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->textdisplay->last_update = 0;
	Text_Update(ent);
}

void Text_Close(edict_t *ent)
{
	if(!ent->client) return;
	if(!ent->client->textdisplay) return;
	if(ent->client->textdisplay->buffer)
	{
		free(ent->client->textdisplay->buffer);
		ent->client->textdisplay->buffer = NULL;
	}
	free(ent->client->textdisplay);
	ent->client->textdisplay = NULL;
	ent->client->showscores = false;
}

void Text_BuildDisplay(texthnd_t *hnd)
{
	int i, imax, n;
	char *p1, *p2, *p3;

	for(i=0; i<hnd->page_length+2; i++)
		text[i].text = NULL;

	if(!(hnd->flags & 2))
	{
		text[hnd->page_length+1].text =   "Press E";
		if(hnd->nlines > hnd->page_length)
			text[hnd->page_length].text = "Scroll Mousewheel for next page";
	}

	p1 = hnd->buffer+hnd->start_char;
	p3 = hnd->buffer+hnd->size-1;
	if(hnd->curline > 0)
	{
		// Scan for hnd->curline'th 0 byte, point to following character
		n  = hnd->curline;
		while(p1 < p3 && n)
		{
			if(*p1==0) n--;
			p1++;
		}
	}

	i = 0;
	p2 = p1;
	text[i].text = p2;
	if(hnd->nlines > hnd->page_length)
		imax = hnd->page_length-2;
	else
		imax = hnd->page_length-1;
	while(p2 <= p3 && i < imax)
	{
		if(*p2 == 0 && p2 < p3)
		{
			i++;
			p2++;
			text[i].text = p2;
		}
		else
			p2++;
	}
}

void Text_Update(edict_t *ent)
{
	int align;
	int i;
	int	x0, y0;
	text_t *p;
	int x, xlast;
	char *t, *tnext;
	qboolean alt = false;
	char string[2048];
	texthnd_t *hnd;


	if (!ent->client->textdisplay) {
		gi.dprintf("warning:  ent has no text display\n");
		return;
	}

	hnd = ent->client->textdisplay;
	if(hnd->last_update + 2*FRAMETIME > level.time) return;
	hnd->last_update = level.time;


	x0 = (35 - hnd->page_width)*4;
	y0 = (22 - hnd->page_length)*4;

	if(!(hnd->flags & 2))
	{
		sprintf(string,"xv %d yv %d picn %s ",
			x0, y0, hnd->background_image);
	}
	xlast = 9999;
	for (i = 0, p = hnd->lines; i < hnd->page_length+2; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
		align = TEXT_LEFT;
		if (*t == '\\')
		{
			tnext = t;
			tnext++;
			if(*tnext == 'c')
			{
				align = TEXT_CENTER;
				t++;
				t++;
			}
			if (*tnext == 'r')
			{
				align = TEXT_RIGHT;
				t++;
				t++;
			}
		}
		if(strlen(t))
		{
			sprintf(string + strlen(string), "yv %d ", y0 + 24 + i * 8);
			if (align == TEXT_CENTER)
				x = x0 + 20 + (hnd->page_width-1-strlen(t))*4;
			else if (align == TEXT_RIGHT)
				x = x0 + 20 + (hnd->page_width-1-strlen(t))*8;
			else
				x = x0 + 20;
			if(x != xlast)
			{
				sprintf(string + strlen(string), "xv %d ",x);
				xlast = x;
			}
			if (alt) {
				sprintf(string + strlen(string), "string2 \"%s\" ", t);
			} else {
				sprintf(string + strlen(string), "string \"%s\" ", t);
			}
		}
		alt = false;
	}
//	if(strlen(string) > 1000)
//		gi.dprintf("WARNING: formatted string length (%d) > 1000\n",strlen(string));

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

void Text_Next(edict_t *ent)
{
	int	current;
	int displayed_lines;
	texthnd_t *hnd;

	if (!ent->client->textdisplay) {
		gi.dprintf("warning:  ent has no text display\n");
		return;
	}

	hnd = ent->client->textdisplay;

	displayed_lines = hnd->page_length;
	if(hnd->nlines > hnd->page_length) displayed_lines--;
	if(hnd->curline+displayed_lines+1 < hnd->nlines)
	{
		current = hnd->curline;
	//	hnd->curline = min(hnd->curline+MAX_LINES/2,hnd->nlines-displayed_lines-1);
		hnd->curline = hnd->curline+hnd->page_length-1;
		if(hnd->curline > current)
		{
			Text_BuildDisplay(hnd);
			Text_Update(ent);
		}
	}
}

void Text_Prev(edict_t *ent)
{
	texthnd_t *hnd;

	if (!ent->client->textdisplay) {
		gi.dprintf("warning:  ent has no text display\n");
		return;
	}

	hnd = ent->client->textdisplay;

	if(hnd->curline > 0)
	{
	//	hnd->curline = max(0, hnd->curline-MAX_LINES/2);
		hnd->curline = max(0, hnd->curline-hnd->page_length+1);
		Text_BuildDisplay(hnd);
		Text_Update(ent);
	}
}


void Do_Text_Display(edict_t *activator, int flags, char *message)
{
	int			/*i,*/ L;
	char		*p1, *p2, *p3;
	char		sound[64];
	texthnd_t	*hnd;
	byte		*temp_buffer;
	int			line_length;
	int			new_line_length;
	qboolean	alt, centered, right_justified;
	qboolean	linebreak;
	qboolean	do_linebreaks;

	

	hnd = malloc(sizeof(*hnd));
	// If a file, open and read it
	if(flags & 1)
	{
#ifdef KMQUAKE2_ENGINE_MOD // use new engine file loading function instead
		char	textname[128];
		int		textsize;
		byte	*readbuffer;
		
		if (!strstr (message, ".txt"))
			sprintf(textname,"text/%s.txt", message);
		else
			sprintf(textname,"text/%s", message);

		textsize = gi.LoadFile(textname, (void **)&readbuffer);
		if (textsize < 2) // file not found
		{
			gi.dprintf("File not found: %s\n",textname);
			return;
		}
		hnd->allocated = textsize + 128; // add some slop for additional control characters
		hnd->buffer = malloc(hnd->allocated);
		if(!hnd->buffer)
		{
			gi.dprintf("Memory allocation failure on target_text\n");
			Text_Close(activator);
			return;
		}
		memset(hnd->buffer,0,hnd->allocated);
		memcpy(hnd->buffer, readbuffer, textsize);
		hnd->buffer[textsize] = 0;
		gi.FreeFile(readbuffer);
#else
		cvar_t			*basedir, *gamedir;
		char			filename[256];
		char			pakfile[256];
		char			textname[128];
		int				k, num, numitems;
		qboolean		in_pak;
		FILE			*f;
		pak_header_t	pakheader;
		pak_item_t		pakitem;
		int i;
		
		basedir = gi.cvar("basedir", "", 0);
		gamedir = gi.cvar("gamedir", "", 0);
		strcpy(filename,basedir->string);
		if(strlen(gamedir->string))
		{
			strcat(filename,"\\");
			strcat(filename,gamedir->string);
		}
		// First check for existence of text file in pak0.pak -> pak9.pak
		in_pak = false;
		for(i=0; i<=9 && !in_pak; i++)
		{
			sprintf(pakfile,"%s\\pak%d.pak",filename,i);
			if (NULL != (f = fopen(pakfile, "rb")))
			{
				num=fread(&pakheader,1,sizeof(pak_header_t),f);
				if(num >= sizeof(pak_header_t))
				{
					if( pakheader.id[0] == 'P' &&
						pakheader.id[1] == 'A' &&
						pakheader.id[2] == 'C' &&
						pakheader.id[3] == 'K'     )
					{
						numitems = pakheader.dsize/sizeof(pak_item_t);
						sprintf(textname,"maps/%s",message);
						fseek(f,pakheader.dstart,SEEK_SET);
						for(k=0; k<numitems && !in_pak; k++)
						{
							fread(&pakitem,1,sizeof(pak_item_t),f);
							if(!stricmp(pakitem.name,textname))
							{
								in_pak = true;
								fseek(f,pakitem.start,SEEK_SET);
								hnd->allocated = pakitem.size + 128;  // add some slop for additional control characters
								hnd->buffer = malloc(hnd->allocated);
								if(!hnd->buffer)
								{
									fclose(f);
									gi.dprintf("Memory allocation failure on target_text\n");
									Text_Close(activator);
									return;
								}
								memset(hnd->buffer,0,hnd->allocated);
								fread(hnd->buffer,1,pakitem.size,f);
								hnd->buffer[pakitem.size] = 0;
							}
						}
					}
				}
				fclose(f);
			}
		}
		if(!in_pak)
		{
			strcat(filename,"\\maps\\");
			strcat(filename,message);
			f = fopen(filename,"rb");
			if(!f)
			{
				gi.dprintf("File not found:%s\n",filename);
				return;
			}
			fseek(f,0,SEEK_END);
			L = ftell (f);
			fseek(f,0,SEEK_SET);
			hnd->allocated = L+128;
			hnd->buffer = malloc(hnd->allocated);
			if(!hnd->buffer)
			{
				gi.dprintf("Memory allocation failure on target_text\n");
				Text_Close(activator);
				return;
			}
			memset(hnd->buffer,0,hnd->allocated);
			fread(hnd->buffer,1,L,f);
			fclose(f);
		}
#endif // KMQUAKE2_ENGINE_MOD

		if(!hnd->buffer)
		{
			gi.dprintf("Umm... how'd you get here?\n");
			Text_Close(activator);
			return;
		}
	}
	else
	{
		L = strlen(message);
		hnd->allocated = L+128;
		hnd->buffer = malloc(hnd->allocated);
		if(!hnd->buffer)
		{
			gi.dprintf("Memory allocation failure\n");
			Text_Close(activator);
			return;
		}
		memset(hnd->buffer,0,hnd->allocated);
		memcpy(hnd->buffer,message,L);
	}
	
	hnd->size = strlen(hnd->buffer) + 1;

	// Default page length:
	hnd->page_length = MAX_LINES-2;
	hnd->page_width  = MAX_LINE_LENGTH;
	strcpy(hnd->background_image,"textdisplay");
	hnd->start_char = 0;
	do_linebreaks = true;

	// If 1st line starts with $, read page length, width, and image name
	p1 = hnd->buffer;
	if(*p1 == '$')
	{
		p3 = p1;
		while((p3 < hnd->buffer+hnd->size) && (*p3 != 13))
			p3++;

		p2 = strstr(p1,"L=");
		if(p2 && (p2 < p3))
		{
			p2 += 2;
			sscanf(p2,"%d",&hnd->page_length);
			hnd->page_length += 1;
		}
		p2 = strstr(p1,"W=");
		if(p2 && (p2 < p3))
		{
			p2 += 2;
			sscanf(p2,"%d",&hnd->page_width);
		}
		p2 = strstr(p1,"I=");
		if(p2 && (p2 < p3))
		{
			p2 += 2;
			sscanf(p2,"%s",hnd->background_image);
		}
		p3++;
		if(*p3 == 10) p3++;
		hnd->start_char = p3-p1;
		do_linebreaks = false;
	}

	// Eliminate all <CR>'s so lines are delineated with <LF>'s only
	p1 = hnd->buffer+hnd->start_char;
	while(p1 < hnd->buffer+hnd->size)
	{
		if(*p1 == 13)
		{
			for(p2=p1, p3=p1+1; p2<hnd->buffer+hnd->size; p2++, p3++)
				*p2 = *p3;
			hnd->size--;
		}
		else
			p1++;
	}
	// Count number of lines and replace all line feeds with 0's
	hnd->nlines = 1;
	for(p1 = hnd->buffer+hnd->start_char; p1 < hnd->buffer+hnd->size; p1++)
	{
		if(*p1 == 10)
		{
			hnd->nlines++;
			*p1 = 0;
		}
	}
	// Line break stuff
	if(!do_linebreaks)
		goto done_linebreaks;

	line_length = 0;
	p1 = hnd->buffer+hnd->start_char;
	alt = false;
	centered = false;
	right_justified = false;
	while(p1 < hnd->buffer+hnd->size)
	{
		// Don't count control characters
		if(line_length == 0) {
			if(*p1 == '*') {
				p1++;
				alt = true;
			} else {
				alt = false;
			}
			if(*p1 == '\\') {
				p1++;
				if(*p1 == 'c') {
					p1++;
					centered = true;
					right_justified = false;
				} else if(*p1 == 'r') {
					p1++;
					centered = false;
					right_justified = true;
				} else {
					centered = false;
					right_justified = false;
				}
			} else {
				centered = false;
				right_justified = false;
			}
		}
		if((line_length == 0) && (*p1 == '\\')) p1 += 2;
		if(*p1 != 0) line_length++;
		linebreak = false;
		if(line_length > hnd->page_width)
		{
			if(*p1 == 32)
			{
				// We're at a space... good deal, just replace space with 
				// a line-break 0 and move on
				*p1 = 0;
				hnd->nlines++;
				linebreak = true;
			}
			else
			{
				// back up from current position to last space character and
				// replace with a 0 (but don't go past previous 0)
				p2 = p1;
				while(p1 > hnd->buffer+hnd->start_char && *p1 != 0)
				{
					if(*p1 == 32)
					{
						*p1 = 0;
						hnd->nlines++;
						linebreak = true;
					}
					else
						p1--;
				}
				if(!linebreak) {
					// Must be an ugly Mad Dog test trying my patience - say
					// a 40-character line with no spaces. Back up one space,
					// add a hyphen then a 0.
					hnd->size += 2;
					if(hnd->size >= hnd->allocated) {
						hnd->allocated += 128;
						temp_buffer = hnd->buffer;
						hnd->buffer = malloc(hnd->allocated);
						if(!hnd->buffer)
						{
							gi.dprintf("Memory allocation failure\n");
							Text_Close(activator);
							return;
						}
						memset(hnd->buffer,0,hnd->allocated);
						memcpy(hnd->buffer,temp_buffer,hnd->size);
						p1 = hnd->buffer + ((size_t)p2-(size_t)temp_buffer);
						p2 = p1;
						free(temp_buffer);
					}
					p1 = p2-1;
					p2 = hnd->buffer + hnd->size;
					p3 = p2 - 2;
					while(p3 >= p1) {
						*p2 = *p3;
						p2--;
						p3--;
					}
					*p1 = '-';
					p1++;
					*p1 = 0;
					hnd->nlines++;
					linebreak = true;
				}
			}
		}
		if(linebreak && alt) {
			// We broke a line and the line was green text. Insert another
			// '*' at beginning of next line
			hnd->size += 1;
			if(hnd->size > hnd->allocated) {
				hnd->allocated += 128;
				temp_buffer = hnd->buffer;
				hnd->buffer = malloc(hnd->allocated);
				if(!hnd->buffer)
				{
					gi.dprintf("Memory allocation failure\n");
					Text_Close(activator);
					return;
				}
				memset(hnd->buffer,0,hnd->allocated);
				memcpy(hnd->buffer,temp_buffer,hnd->size);
				p2 = p1;
				p1 = hnd->buffer + ((size_t)p2-(size_t)temp_buffer);
				free(temp_buffer);
			}
			p2 = hnd->buffer + hnd->size;
			p3 = p2 - 1;
			while(p3 >= p1) {
				*p2 = *p3;
				p2--;
				p3--;
			}
			p2 = p1+1;
			*p2 = '*';
		}
		if(linebreak && (centered || right_justified)) {
			// We broke a line and the line had other than left justification. Insert another
			// '\c' or '\r' at beginning of next line
			hnd->size += 2;
			if(hnd->size > hnd->allocated) {
				hnd->allocated += 128;
				temp_buffer = hnd->buffer;
				hnd->buffer = malloc(hnd->allocated);
				if(!hnd->buffer)
				{
					gi.dprintf("Memory allocation failure\n");
					Text_Close(activator);
					return;
				}
				memset(hnd->buffer,0,hnd->allocated);
				memcpy(hnd->buffer,temp_buffer,hnd->size);
				p2 = p1;
				p1 = hnd->buffer + ((size_t)p2-(size_t)temp_buffer);
				free(temp_buffer);
			}
			p2 = hnd->buffer + hnd->size;
			p3 = p2 - 2;
			while(p3 >= p1) {
				*p2 = *p3;
				p2--;
				p3--;
			}
			p2 = p1+1;
			if(alt) p2++;
			*p2 = '\\';
			p2++;
			if(centered)
				*p2 = 'c';
			else
				*p2 = 'r';
		}
		if(*p1=='\\') {
			p2 = p1+1;
			if(*p2=='n') {
				*p1 = 0;
				p3 = p2 + 1;
				while(p3 < hnd->buffer + hnd->size) {
					*p2 = *p3;
					p2++;
					p3++;
				}
				hnd->nlines++;
				linebreak = true;
				centered = false;
				right_justified = false;
				alt = false;
			}
		}
		// If we're at a 0, check to see if subsequent words will fit on this line
		if((!linebreak) && (*p1 == 0) && (p1 < hnd->buffer+hnd->size-1) && 
			(line_length < hnd->page_width) )
		{
			// Don't do this if 2 consecutive 0's are found (end of paragraph)
			// or if 1st character in next line is '*' or '\'
			p2 = p1;
			p2--;
			if(*p2 != 0)
			{
				p2++;
				p2++;
				if(*p2 != 0 && *p2 != '*' && *p2 != '\\' && p2 < hnd->buffer+hnd->size)
				{
					new_line_length = line_length+2;
					while(p2 < hnd->buffer+hnd->size && *p2 != 32 && *p2 != 0)
					{
						new_line_length++;
						p2++;
					}
					if(new_line_length <= hnd->page_width)
					{
						*p1 = 32;
						line_length++; // include the space that was a 0
						hnd->nlines--;
					}
				}
			}
		}
		if(*p1 == 0) line_length = 0;
		p1++;
	}

done_linebreaks:

	// Finally, scan for a \a code (embedded audio). If present remove that line
	// and play the sound
	p1 = hnd->buffer+hnd->start_char;
	while(p1 < hnd->buffer+hnd->size)
	{
		if((*p1 == 0) || (p1 == hnd->buffer+hnd->start_char))
		{
			if(*p1 == 0)
				p1++;
			if(*p1 == '\\')
			{
				p1++;
				if(*p1 == 'a')
				{
					strcpy(sound,p1+1);
					p1--;
					p2=p1;
					while(*p2 != 0)
						p2++;
					p2++;
					memcpy(p1,p2,hnd->buffer+hnd->size-(size_t)p2+1);
					hnd->nlines--;
					// Found one (only one is allowed)
					gi.sound (activator, CHAN_AUTO, gi.soundindex (sound), 1, ATTN_NORM, 0);
				}
			}
		}
		p1++;
	}

	hnd->curline = 0;
	hnd->flags = flags;
	Text_BuildDisplay(hnd);
	hnd->lines  = text;
	activator->client->textdisplay = hnd;
	Text_Open(activator);
	
}

void Use_Target_Text(edict_t *self, edict_t *other, edict_t *activator)
{	
	if(!activator || !activator->client) return;
	activator->client->showinventory = false;
	activator->client->showscores = false;
	activator->client->showhelp = false;
	
	Text_Close(activator);

	Do_Text_Display(activator, self->spawnflags, self->message);

}

void SP_target_text(edict_t *self)
{
	if(!self->message)
	{
		gi.dprintf("target_text with no message at %s\n",
			vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}
	self->class_id = ENTITY_TARGET_TEXT;
	self->use = Use_Target_Text;
}




#define SPAWNFLAG_ISVIDEO  4
#define SPAWNFLAG_NODROP   2

int sound_chime;


void use_commentary(edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.effects &= ~EF_ROTATE;
	self->s.renderfx &= ~RF_GLOW;

	if (!self->message && self->style <= 1)
	{
		gi.dprintf("target_commentary with no message at %s\n",
			vtos(self->s.origin));
		return;
	}

	gi.sound (self, CHAN_VOICE, sound_chime, 1, ATTN_NONE, 0);

	if (self->style == 1)
	{
		char	*modelname;
		int length;

		if (!self->target)
		{
			gi.dprintf("target_commentary with no target spawnpoint at %s\n",
				vtos(self->s.origin));
			return;
		}

		if (!self->usermodel)
			self->usermodel = level.mapname;


		length = strlen( self->message ) + strlen( self->usermodel) + strlen( self->target );
		
		modelname = gi.TagMalloc( length + 8, TAG_LEVEL);
		sprintf(modelname, "%s.roq+%s$%s", self->message, self->usermodel, self->target);
		self->map = modelname;

		use_target_changelevel(self, other, activator);
		return;
	}
	else if (self->style == 2)
	{
		use_target_tele(self, other, activator);
	}
	else 
	{

		

		//text.
		/*
		if (!strstr (self->message, ".txt"))
		{
			char *s;
			//Q_strcat(s, ".txt", maxSize);


			gi.dprintf("message1: %s\n", self->message);

			sprintf(s,"%s.txt", self->message);
			strcpy( self->message, s );

			//sprintf(self->message,"%s.txt", self->message);

			gi.dprintf("message2: %s\n", self->message);
		}*/


		

		self->spawnflags |= 1;

		Use_Target_Text(self, other, activator);
	}
}



void SP_target_commentary(edict_t *self)
{
	trace_t	tr;
	cvar_t *commentary = gi.cvar("commentary", "0", 0);

	if (commentary->value <= 0)
	{
		G_FreeEdict(self);
		return;
	}

	sound_chime = gi.soundindex ("world/chime.wav");

	self->s.modelindex = gi.modelindex ("models/objects/commentary/tris.md2");	

	self->s.renderfx |= RF_NOSHADOW;
	self->s.renderfx |= RF_MINLIGHT;
	self->solid			= SOLID_BBOX;
	self->takedamage	= DAMAGE_NO;
	self->svflags          |= SVF_DEADMONSTER;
	VectorSet(self->mins, -3, -3, -3);
	VectorSet(self->maxs, 3, 3, 3);

	self->s.renderfx |= RF_GLOW;

	self->use = Use_Target_Text;
	self->s.effects |= EF_ROTATE;

	if (!(self->spawnflags & SPAWNFLAG_NODROP))
	{
		//drop to ground.
		vec3_t target;
		VectorCopy(self->s.origin, target);
		target[2] -= 1024;
		tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);

		self->s.origin[2] = tr.endpos[2] + 44;
	}

	gi.linkentity (self);

	if (self->style >= 1 )
	{
		edict_t *frame;
		frame = G_Spawn();

		if (self->style == 2)
			frame->s.modelindex = gi.modelindex ("models/objects/commentary_arrow/tris.md2");	
		else
			frame->s.modelindex = gi.modelindex ("models/objects/commentary_camera/tris.md2");	

		frame->s.renderfx |= RF_NOSHADOW;
		frame->s.renderfx |= RF_MINLIGHT;
		frame->solid			= SOLID_NOT;
		frame->takedamage	= DAMAGE_NO;
		frame->s.effects |= EF_ROTATE;
		VectorCopy(self->s.origin, frame->s.origin);
		frame->s.origin[2] += 5;
		gi.linkentity (frame);
	}
}