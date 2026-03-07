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

// cl_event.c -- entity events: footsteps, etc.
// moved from cl_fx.c

#include "client.h"

void CL_ItemRespawnParticles (vec3_t org);
void CL_TeleportParticles (vec3_t org);

#define MAX_TEX_SURF 2048 // was 256
struct texsurf_s
{
	int		step_id;
	char	tex[32];
};
typedef struct texsurf_s texsurf_t;
texsurf_t tex_surf[MAX_TEX_SURF];
int	num_texsurfs;


/*
===============
ReadTextureSurfaceAssignments
Reads in defintions for footsteps based on texture name.
===============
*/
#if 1
qboolean buf_gets (char *dest, int destsize, char **f)
{
	char *old = *f;
	*f = strchr (old, '\n');
	if (!*f)
	{	// no more new lines
		*f = old + strlen(old);
		if (!strlen(*f))
			return false;	// end of file, nothing else to grab
	}
	(*f)++; // advance past EOL
	strncpy (dest, old, min(destsize-1, (int)(*f-old-1)) );
	return true;
}

void ReadTextureSurfaceAssignments (void)
{
	char	filename[MAX_OSPATH];
	char	*footstep_data;
	char	*parsedata;
	char	line[80];

	num_texsurfs = 0;

	Com_sprintf (filename, sizeof(filename), "texsurfs.txt");
	FS_LoadFile (filename, (void **)&footstep_data);
	parsedata = footstep_data;
	if (!footstep_data) return;
	while (buf_gets(line, sizeof(line), &parsedata) && num_texsurfs < MAX_TEX_SURF)
	{
	//	sscanf(line, "%d %s", &tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex);
		if (sscanf(line, "%d %s", &tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex) == EOF) {
			Com_Printf (S_COLOR_YELLOW"ReadTextureSurfaceAssignments: invalid footstep assignment '%s'.\n", line);
		}
	//	Com_Printf("%d %s\n", tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex);
		num_texsurfs++;
	}
	FS_FreeFile (footstep_data);
}
#else
void ReadTextureSurfaceAssignments (void)
{
	char	filename[MAX_OSPATH];
	FILE	*f;
	char	line[80];

	num_texsurfs = 0;

	Com_sprintf (filename, sizeof(filename), "texsurfs.txt");
	FS_FOpenFile (filename, &f);
	if (!f) return;
	while (fgets(line, sizeof(line), f) && num_texsurfs < MAX_TEX_SURF)
	{
	//	sscanf(line, "%d %s", &tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex);
		if (sscanf(line, "%d %s", &tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex) == EOF) {
			Com_Printf ("ReadTextureSurfaceAssignments: invalid footstep assignment '%s'.\n", line);
		}
	//	Com_Printf("%d %s\n", tex_surf[num_texsurfs].step_id, tex_surf[num_texsurfs].tex);
		num_texsurfs++;
	}
	fclose(f);
}
#endif


/*
===============
CL_FootSteps
Plays appropriate footstep sound depending on surface flags of the ground surface.
Since this is a replacement for plain Jane EV_FOOTSTEP, we already know
the player is definitely on the ground when this is called.
===============
*/
void CL_FootSteps (entity_state_t *ent, qboolean loud, qboolean recursed)
{
	trace_t	tr;
	vec3_t	end;
	int		r;
	int		surface;
	struct	sfx_s	*stepsound = NULL;
	float	volume = 0.5;

	r = (rand()&3);

	VectorCopy(ent->origin,end);
	end[2] -= 64;
	tr = CL_PMSurfaceTrace (ent->number, ent->origin,NULL,NULL,end,MASK_SOLID | MASK_WATER);
	if (!tr.surface)
		return;
	surface = tr.surface->flags & SURF_STEPMASK;
	switch (surface)
	{
	case SURF_METAL:
		stepsound = clMedia.sfx_metal_footsteps[r];
		break;
	case SURF_DIRT:
		stepsound = clMedia.sfx_dirt_footsteps[r];
		break;
	case SURF_VENT:
		stepsound = clMedia.sfx_vent_footsteps[r];
		break;
	case SURF_GRATE:
		stepsound = clMedia.sfx_grate_footsteps[r];
		break;
	case SURF_TILE:
		stepsound = clMedia.sfx_tile_footsteps[r];
		break;
	case SURF_GRASS:
		stepsound = clMedia.sfx_grass_footsteps[r];
		break;
	case SURF_SNOW:
		stepsound = clMedia.sfx_snow_footsteps[r];
		break;
	case SURF_CARPET:
		stepsound = clMedia.sfx_carpet_footsteps[r];
		break;
	case SURF_FORCE:
		stepsound = clMedia.sfx_force_footsteps[r];
		break;
	case SURF_GRAVEL:
		stepsound = clMedia.sfx_gravel_footsteps[r];
		break;
	case SURF_ICE:
		stepsound = clMedia.sfx_ice_footsteps[r];
		break;
	case SURF_SAND:
		stepsound = clMedia.sfx_sand_footsteps[r];
		break;
	case SURF_WOOD:
		stepsound = clMedia.sfx_wood_footsteps[r];
		break;
	case SURF_STANDARD:
		stepsound = clMedia.sfx_footsteps[r];
		volume = 1.0;
		break;
	default:
	//	if (cl_footstep_override->value && num_texsurfs && !recursed)
		if (cl_footstep_override->integer && num_texsurfs && !recursed)
		{
			int	i;
			for (i=0; i<num_texsurfs; i++)
				if (strstr(tr.surface->name,tex_surf[i].tex) && tex_surf[i].step_id > 0)
				{
					tr.surface->flags &= ~SURF_STEPMASK;
					tr.surface->flags |= (SURF_METAL << (tex_surf[i].step_id - 1));
					CL_FootSteps (ent, loud, true); // start over
					return;
				}
		}
	//	tr.surface->flags &= ~SURF_STEPMASK;
	//	tr.surface->flags |= SURF_STANDARD;
	//	CL_FootSteps (ent, loud, true); // start over
	//	return;
		stepsound = clMedia.sfx_footsteps[r];
		volume = 1.0;
		break;
	}

	if (loud)
	{
		if (volume == 1.0)
			S_StartSound (NULL, ent->number, CHAN_AUTO, stepsound, 1.0, ATTN_NORM, 0);
		else
			volume = 1.0;
	}
	S_StartSound (NULL, ent->number, CHAN_BODY, stepsound, volume, ATTN_NORM, 0);
}
//end Knightmare

/*
==============
CL_EntityEvent

An entity has just been parsed that has an event value

the female events are there for backwards compatability
==============
*/

void CL_EntityEvent (entity_state_t *ent)
{
	switch (ent->event)
	{
	case EV_ITEM_RESPAWN:
	//	S_StartSound (NULL, ent->number, CHAN_WEAPON, S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
		S_StartSound (NULL, ent->number, CHAN_WEAPON, clMedia.sfx_item_respawn, 1, ATTN_IDLE, 0);
		CL_ItemRespawnParticles (ent->origin);
		break;
	case EV_PLAYER_TELEPORT:
	//	S_StartSound (NULL, ent->number, CHAN_WEAPON, S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
		S_StartSound (NULL, ent->number, CHAN_WEAPON, clMedia.sfx_player_teleport, 1, ATTN_IDLE, 0);
		CL_TeleportParticles (ent->origin);
		break;
	case EV_PLAYER_TELEPORT2:
		S_StartSound (NULL, ent->number, CHAN_WEAPON, clMedia.sfx_player_teleport2[rand()%5], 1, ATTN_IDLE, 0);
		CL_TeleportParticles (ent->origin);
		break;
	case EV_FOOTSTEP:
	//	if (cl_footsteps->value)
		if (cl_footsteps->integer)
//Knightmare- Lazarus footsteps
			//S_StartSound (NULL, ent->number, CHAN_BODY, clMedia.sfx_footsteps[rand()&3], 1, ATTN_NORM, 0);
			CL_FootSteps (ent, false, false);
		break;
	case EV_LOUDSTEP:
#ifdef NOTTHIRTYFLIGHTS
	//	if (cl_footsteps->value)
		if (cl_footsteps->integer)
			CL_FootSteps (ent, true, false);
#else
		S_StartSound (NULL, ent->number, CHAN_BODY, S_RegisterSound("player/land0.wav"), 1.0, ATTN_NORM, 0);
#endif
		break;
//end Knightmare
	case EV_FALLSHORT:
	//	S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("player/land1.wav"), 1, ATTN_NORM, 0);
		S_StartSound (NULL, ent->number, CHAN_AUTO, clMedia.sfx_player_land, 1, ATTN_NORM, 0);
		break;
	case EV_FALL:
		S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("*fall2.wav"), 1, ATTN_NORM, 0);
		break;
	case EV_FALLFAR:
		S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("*fall1.wav"), 1, ATTN_NORM, 0);
		break;
// Knightmare- more Lazarus sounds
	case EV_SLOSH:
		S_StartSound (NULL, ent->number, CHAN_BODY, clMedia.sfx_slosh[rand()&3], 0.5, ATTN_NORM, 0);
		break;
	case EV_WADE:
		S_StartSound (NULL, ent->number, CHAN_BODY, clMedia.sfx_wade[rand()&3], 0.5, ATTN_NORM, 0);
		break;
	case EV_WADE_MUD:
		S_StartSound (NULL, ent->number, CHAN_BODY, clMedia.sfx_mud_wade[rand()&1], 0.5, ATTN_NORM, 0);
		break;
	case EV_CLIMB_LADDER:
		S_StartSound (NULL, ent->number, CHAN_BODY, clMedia.sfx_ladder[rand()&3], 0.5, ATTN_NORM, 0);
		break;
//end Knightmare
	}
}
