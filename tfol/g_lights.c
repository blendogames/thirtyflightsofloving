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
//==========================================================

void Lights()
{
	if (lights->value)
	{
		// 0 normal
		gi.configstring(CS_LIGHTS+0, "m");
		// 1 FLICKER (first variety)
		gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
		// 2 SLOW STRONG PULSE
		gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
		// 3 CANDLE (first variety)
		gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
		// 4 FAST STROBE
		gi.configstring(CS_LIGHTS+4, "mamamamamama");
		// 5 GENTLE PULSE 1
		gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
		// 6 FLICKER (second variety)
		gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
		// 7 CANDLE (second variety)
		gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
		// 8 CANDLE (third variety)
		gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
		// 9 SLOW STROBE (fourth variety)
		gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
		// 10 FLUORESCENT FLICKER
		gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");
		// 11 SLOW PULSE NOT FADE TO BLACK
		gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	}
	else
	{
		// 0 normal
		gi.configstring(CS_LIGHTS+0, lightsmin->string);
		// 1 FLICKER (first variety)
		gi.configstring(CS_LIGHTS+1, lightsmin->string);
		// 2 SLOW STRONG PULSE
		gi.configstring(CS_LIGHTS+2, lightsmin->string);
		// 3 CANDLE (first variety)
		gi.configstring(CS_LIGHTS+3, lightsmin->string);
		// 4 FAST STROBE
		gi.configstring(CS_LIGHTS+4, lightsmin->string);
		// 5 GENTLE PULSE 1
		gi.configstring(CS_LIGHTS+5, lightsmin->string);
		// 6 FLICKER (second variety)
		gi.configstring(CS_LIGHTS+6, lightsmin->string);
		// 7 CANDLE (second variety)
		gi.configstring(CS_LIGHTS+7, lightsmin->string);
		// 8 CANDLE (third variety)
		gi.configstring(CS_LIGHTS+8, lightsmin->string);
		// 9 SLOW STROBE (fourth variety)
		gi.configstring(CS_LIGHTS+9, lightsmin->string);
		// 10 FLUORESCENT FLICKER
		gi.configstring(CS_LIGHTS+10,lightsmin->string);
		// 11 SLOW PULSE NOT FADE TO BLACK
		gi.configstring(CS_LIGHTS+11,lightsmin->string);
	}
}

void ToggleLights()
{
	lights->value = !lights->value;
	Lights();
}

void target_lightswitch_toggle (edict_t *self)
{
	ToggleLights();

	self->count--;
	if(!self->count) {
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
}

void use_target_lightswitch (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (!self->delay)
	{
		target_lightswitch_toggle (self);
		return;
	}

	self->think = target_lightswitch_toggle;
	self->nextthink = level.time + self->delay;
}

void SP_target_lightswitch (edict_t *self)
{
	int i;
	edict_t *e;
	char lightvalue[2];

	// ensure this is the only target_lightswitch in the map
	for(i=1, e=g_edicts+i; i<globals.num_edicts; i++, e++)
	{
		if(!e->inuse) continue;
		if(!e->classname) continue;
		if(e==self) continue;
		if(!Q_stricmp(e->classname,"target_lightswitch"))
		{
			gi.dprintf("Only one target_lightswitch per map is allowed.\n");
			G_FreeEdict(self);
			return;
		}
	}
	if(!self->message)
		lightvalue[0] = 'a';
	else
		lightvalue[0] = self->message[0];
	lightvalue[1] = 0;
	gi.cvar_forceset("lightsmin", lightvalue);
	self->use = use_target_lightswitch;
	self->svflags = SVF_NOCLIENT;
	if(self->spawnflags & 1)
	{
		self->think     = target_lightswitch_toggle;
		self->nextthink = level.time + 2*FRAMETIME;
		gi.linkentity(self);
	}
}

