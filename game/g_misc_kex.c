/*
===========================================================================
Copyright (C) 2023 Knightmare

This file is part of Lazarus Quake 2 Mod source code.

Lazarus Quake 2 Mod source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Lazarus Quake 2 Mod source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lazarus Quake 2 Mod source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// g_misc_kex.c
// misc entities added by Kex-based Q2 Rerelease

#include "g_local.h"

/*QUAKED misc_flare (1.0 1.0 0.0) (-8 -8 -8) (8 8 8) RED GREEN BLUE LOCK_ANGLE
Creates a flare as seen in the N64 Q2 version.
RED					specifies a red outer rim
GREEN				specifies a green outer rim
BLUE				specifies a blue outer rim
LOCK_ANGLE			forces flare sprite to be vertically oriented

"image"				path to custom flare image, default is "gfx/effects/flare.tga"
"radius"			size muliplier, scales default radius of 50
"rgba"				flare RGBA color separated by spaces, e.g. "20 160 192 255"
					4th component is used for outer color if RED, GREEN,
					or BLUE spawnflags is set
"alpha"				translucency of flare, is multiplied by 0.5
"fade_start_dist"	fade start distance, should be less than fade_end_dist
"fade_end_dist"		fade end distance, should be greater than fade_start_dist
"targetname"		used to toggle flare's visibility
*/

#define SF_FLARE_RED			1
#define SF_FLARE_GREEN			2
#define SF_FLARE_BLUE			4
#define SF_FLARE_LOCK_ANGLE		8

void misc_flare_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags ^= SVF_NOCLIENT;
	gi.linkentity (self);
}

#ifndef KMQUAKE2_ENGINE_MOD
void SP_misc_flare (edict_t *self)
{
	// not supported in non-KMQ2 builds
	G_FreeEdict (self);
}
#else	// KMQUAKE2_ENGINE_MOD
void SP_misc_flare (edict_t *self)
{
	self->class_id = ENTITY_MISC_FLARE;
	self->common_name = "Flare";

	self->s.modelindex = 1;

	if (st.fade_start_dist != 0)
		self->s.modelindex2 = st.fade_start_dist;
	else
		self->s.modelindex2 = 96;

	if (st.fade_end_dist != 0)
		self->s.modelindex3 = st.fade_end_dist;
	else
		self->s.modelindex3 = 384;

	// next game API version will copy self->radius directly to self->s.scale
	if (self->radius != 0.0f)
		self->s.modelindex4 = (int)(self->radius * 50.0f);
	else
		self->s.modelindex4 = 50;

	self->s.renderfx = RF_FLARE;
	if (self->spawnflags & SF_FLARE_RED)
		self->s.renderfx |= RF_SHELL_RED;
	if (self->spawnflags & SF_FLARE_GREEN)
		self->s.renderfx |= RF_SHELL_GREEN;
	if (self->spawnflags & SF_FLARE_BLUE)
		self->s.renderfx |= RF_SHELL_BLUE;
	if (self->spawnflags & SF_FLARE_LOCK_ANGLE)
		self->s.renderfx |= RF_FLARE_LOCK_ANGLE;
	if ( st.image && (st.image[0] != 0) ) {
		self->s.renderfx |= RF_CUSTOMSKIN;
		self->s.frame = gi.imageindex(st.image);
	}
	self->s.skinnum = Com_ParseRGBAFieldPacked (st.rgba);
	if (self->alpha != 0.0f)
		self->s.alpha = self->alpha;

	self->solid = SOLID_NOT;
	VectorSet (self->mins, -32, -32, -32);
	VectorSet (self->maxs, 32, 32, 32);

	if (self->targetname)
		self->use = misc_flare_use;

//	gi.dprintf ("spawned misc_flare with radius %d, color (%d %d %d %d)\n", self->s.modelindex4,
//		self->s.skinnum & 255, (self->s.skinnum >> 8) & 255, (self->s.skinnum >> 16) & 255, (self->s.skinnum >> 24) & 255);

	gi.linkentity (self);
}
#endif	// KMQUAKE2_ENGINE_MOD
