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

NoCheat LOC support by NiceAss
Edited and Fixed by Xile and FourthX
*/

#include "client.h"

#ifdef LOC_SUPPORT

typedef struct {
	vec3_t		origin;
	char		name[64];
	qboolean	used;
} loc_t;

#define MAX_LOCATIONS 768

loc_t locations[MAX_LOCATIONS];


/*
=================
CL_FreeLoc
=================
*/
int CL_FreeLoc (void)
{
	int i;

	for (i = 0; i < MAX_LOCATIONS; i++)
	{
		if (locations[i].used == false)
			return i;
	}

	// just keep overwriting the last one....
	return MAX_LOCATIONS - 1;
}


/*
=================
CL_LoadLoc
=================
*/
void CL_LoadLoc (void)
{
	char		mapname[MAX_QPATH];
	char		filename[MAX_OSPATH];
	char		*buf, *line, *nl, *cr, *tok;
	char		*token1, *token2, *token3, *token4;
	int			fSize, nLines, index;
	qboolean	isCommentLine, fpFormat = false;
//	FILE		*f;

	memset(locations, 0, sizeof(loc_t) * MAX_LOCATIONS);

	// format map pathname
	Q_strncpyz(mapname, cl.configstrings[CS_MODELS + 1] + 5, sizeof(mapname));	// skip "maps/"
	mapname[strlen(mapname) - 4] = 0;	// remove ".bsp"
	Com_sprintf (filename, sizeof(filename), "locs/%s.loc", mapname);
	
	// load file and check buffer and size
	fSize = FS_LoadFile (filename, &buf);
	if (!buf) {
		Com_DPrintf("CL_LoadLoc: Couldn't load %s\n", filename);
		return;
	}
	if (fSize < 7) {
		Com_Printf(S_COLOR_YELLOW"CL_LoadLoc: Loc file %s is too small: %d\n", filename, fSize);
		FS_FreeFile (buf);
		return;
	}

	// check if it's in floating-point format
	if ( strstr(buf, ".000") || strstr(buf, ".125") || strstr(buf, ".250") || strstr(buf, ".375")
		|| strstr(buf, ".500") || strstr(buf, ".625") || strstr(buf, ".750") || strstr(buf, ".875") ) {
	//	Com_DPrintf("CL_LoadLoc: %s is in floating-point format.\n", filename);
		fpFormat = true;
	}

	line = buf;
	nLines = 0;
	while ( *line && (line < buf + fSize) )
	{
		// overwrite new line characters with null
		nl = strchr(line, '\n');
		if (nl)
			*nl = '\0';
		// nullify the carriage return too!
		cr = strchr(line, '\r');
		if (cr)
			*cr = '\0';
		nLines++;

		// skip comments
		isCommentLine = (line[0] == ':' || line[0] == ';' || line[0] == '/');
		if (!isCommentLine)
		{
			// break the line up into 4 tokens
			token1 = line;
			token2 = strchr(token1, ' ');
			if (token2 == NULL) {
				Com_Printf(S_COLOR_YELLOW"CL_LoadLoc: Line %d is incomplete in %s\n", nLines, filename);
				continue;
			}
			*token2 = '\0';
			token2++;

			token3 = strchr(token2, ' ');
			if (token3 == NULL) {
				Com_Printf(S_COLOR_YELLOW"CL_LoadLoc: Line %d is incomplete in %s\n", nLines, filename);
				continue;
			}
			*token3 = '\0';
			token3++;

			token4 = strchr(token3, ' ');
			if (token4 == NULL) {
				Com_Printf(S_COLOR_YELLOW"CL_LoadLoc: Line %d is incomplete in %s\n", nLines, filename);
				continue;
			}
			*token4 = '\0';
			token4++;

			// floating-point format has a ':' between coords and label
			if (fpFormat) {
				tok = token4;
				token4 = strchr(tok, ' ');
				if (token4 == NULL) {
					Com_Printf(S_COLOR_YELLOW"CL_LoadLoc: Line %d is incomplete in %s\n", nLines, filename);
					continue;
				}
				*token4 = '\0';
				token4++;
			}

		//	Com_DPrintf("%s %s %s %s\n", token1, token2, token3, token4);

			// copy the data to the struct
			index = CL_FreeLoc();
			if (fpFormat) {
				locations[index].origin[0] = atof(token1);
				locations[index].origin[1] = atof(token2);
				locations[index].origin[2] = atof(token3);
			}
			else
			{
				locations[index].origin[0] = atof(token1) * 0.125f;
				locations[index].origin[1] = atof(token2) * 0.125f;
				locations[index].origin[2] = atof(token3) * 0.125f;
			}
			Q_strncpyz(locations[index].name, token4, sizeof(locations[index].name));
			locations[index].used = true;

		//	Com_DPrintf("%.3f %.3f %.3f\n", locations[index].origin[0], locations[index].origin[1], locations[index].origin[2]);
		}

		if (!nl) break;

		line = nl + 1;
	}

	FS_FreeFile (buf);

	Com_Printf("CL_LoadLoc: Loaded %d locations from %s.\n", nLines, filename);

/*	Com_sprintf (filename, sizeof(filename), "%s/locs/%s.loc", FS_Savegamedir(), mapname);	// was FS_Gamedir()
	if (!(f = fopen(filename, "r"))) {
		Com_DPrintf("CL_LoadLoc: Couldn't load locs/%s.loc\n", mapname);
		return;
	}

	while (!feof(f))
	{
		char *token1, *token2, *token3, *token4;
		char line[128], *nl;
		int index;

		// read a line
		fgets(line, sizeof(line), f);

		// skip comments
		if (line[0] == ':' || line[0] == ';' || line[0] == '/')
			continue;

		// overwrite new line characters with null
		nl = strchr(line, '\n');
		if (nl)
			*nl = '\0';

		// break the line up into 4 tokens
		token1 = line;

		token2 = strchr(token1, ' ');
		if (token2 == NULL)
			continue;
		*token2 = '\0';
		token2++;

		token3 = strchr(token2, ' ');
		if (token3 == NULL)
			continue;
		*token3 = '\0';
		token3++;

		token4 = strchr(token3, ' ');
		if (token4 == NULL)
			continue;
		*token4 = '\0';
		token4++;

		// copy the data to the structure
		index = CL_FreeLoc();
		locations[index].origin[0] = atof(token1) * 0.125f;
		locations[index].origin[1] = atof(token2) * 0.125f;
		locations[index].origin[2] = atof(token3) * 0.125f;
		Q_strncpyz(locations[index].name, token4, sizeof(locations[index].name));
		locations[index].used = true;
	}
	fclose(f);

	Com_Printf("Loaded location data from locs/%s.loc.\n", mapname);
*/
}


/*
=================
CL_LocIndex
=================
*/
int CL_LocIndex (vec3_t origin)
{
	vec3_t	diff; // FourthX fix
	float	minDist = -1;
	int		locIndex = -1;
	int			i;

	for (i = 0; i < MAX_LOCATIONS; i++)
	{
		float dist;

		if (!locations[i].used)
			continue;

		VectorSubtract(origin, locations[i].origin, diff);
		
        //dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]); // FourthX fix, wtf was this other guy thinking?!?
		dist = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]; // jit - small optimization
		// (sqrt unnecessary w/relative distances: if dist1 < dist2 then dist1^2 < dist2^2)

		if (dist < minDist || minDist == -1)
		{
			minDist = dist;
			locIndex = i;
		}
	}

	return locIndex;
}


/*
=================
CL_LocDelete
=================
*/
void CL_LocDelete (void)
{
	vec3_t		point;
	int			index;

	point[0] = cl.frame.playerstate.pmove.origin[0] * 0.125f;
	point[1] = cl.frame.playerstate.pmove.origin[1] * 0.125f;
	point[2] = cl.frame.playerstate.pmove.origin[2] * 0.125f;
	index = CL_LocIndex(point);

	if (index != -1)
	{
		locations[index].used = false;
        Com_Printf("Location '%s' deleted.\n", locations[index].name);                // Xile reworked.
	}  
	else
	{
		Com_Printf("Warning: No location to delete!\n");
	}
}


/*
=================
CL_LocAdd
=================
*/
void CL_LocAdd (char *name)
{
	int index = CL_FreeLoc();

	locations[index].origin[0] = cl.frame.playerstate.pmove.origin[0] * 0.125f;
	locations[index].origin[1] = cl.frame.playerstate.pmove.origin[1] * 0.125f;
	locations[index].origin[2] = cl.frame.playerstate.pmove.origin[2] * 0.125f;
	Q_strncpyz(locations[index].name, name, sizeof(locations[index].name));
	locations[index].used = true;

	Com_Printf("Location '%s' added at (%.3f %.3f %.3f). Loc #%d.\n", locations[index].name, 
		locations[index].origin[0],
		locations[index].origin[1],
		locations[index].origin[2],
		index);
}


/*
=================
CL_LocWrite
=================
*/
void CL_LocWrite (void)
{
	char	mapname[MAX_QPATH];
	char	filename[MAX_OSPATH];
	int		i;
	FILE	*f;

	// format map pathname
//	strncpy(mapname, cl.configstrings[CS_MODELS + 1] + 5);   // Xile; lets just keep saving em to one file mmmkay?
	Q_strncpyz(mapname, cl.configstrings[CS_MODELS + 1] + 5, sizeof(mapname));
	mapname[strlen(mapname) - 4] = 0;
	Com_sprintf (filename, sizeof(filename), "%s/locs/%s.loc", FS_Savegamedir(), mapname);	// was FS_Gamedir()

//	Sys_Mkdir("locs");
	FS_CreatePath (filename);

//	if (!(f = fopen(va("locs/%s.loc", mapname), "w")))
	if (!(f = fopen(filename, "w")))
	{
		Com_Printf(S_COLOR_YELLOW"Unable to open locs/%s.loc for writing.\n", mapname);
		return;
	}

	fprintf(f, "// This location file is generated by KMQuake2, edit at your own risk.\n");

	for (i = 0; i < MAX_LOCATIONS; i++)
	{
		if (!locations[i].used)
			continue;

		fprintf(f, "%d %d %d %s\n",
			(int)(locations[i].origin[0] * 8),
			(int)(locations[i].origin[1] * 8),
			(int)(locations[i].origin[2] * 8),
			locations[i].name);
	}

	fclose(f);

	Com_Printf("Saved location data to locs/%s.loc.\n", mapname);
}


/*
=================
CL_LocPlace
=================
*/
void CL_LocPlace (void)
{
	trace_t	tr;
	vec3_t	point, end;
//	int		there[3];
	int		index1, index2 = -1;

	point[0] = cl.frame.playerstate.pmove.origin[0] * 0.125f;
	point[1] = cl.frame.playerstate.pmove.origin[1] * 0.125f;
	point[2] = cl.frame.playerstate.pmove.origin[2] * 0.125f;
	index1 = CL_LocIndex(point);

	VectorMA(cl.predicted_origin, WORLD_SIZE, cl.v_forward, end);	// was 8192
	tr = CM_BoxTrace(cl.predicted_origin, end, vec3_origin, vec3_origin, 0, MASK_PLAYERSOLID);
//	there[0] = tr.endpos[0] * 8;
//	there[1] = tr.endpos[1] * 8;
//	there[2] = tr.endpos[2] * 8;
	index2 = CL_LocIndex(tr.endpos);

	if (index1 != -1)
		Cvar_ForceSet("loc_here", locations[index1].name);
	else
		Cvar_ForceSet("loc_here", "");

	if (index2 != -1)
		Cvar_ForceSet("loc_there", locations[index2].name);
	else
		Cvar_ForceSet("loc_there", "");
}


/*
=================
CL_AddViewLocs
=================
*/
void CL_AddViewLocs (void)
{
	vec3_t	point;
	int		i, index, num = 0;

	if (!cl_drawlocs->value)
		return;

	point[0] = cl.frame.playerstate.pmove.origin[0] * 0.125f;
	point[1] = cl.frame.playerstate.pmove.origin[1] * 0.125f;
	point[2] = cl.frame.playerstate.pmove.origin[2] * 0.125f;
	index = CL_LocIndex(point);

	for (i = 0; i < MAX_LOCATIONS; i++)
	{
		float dist;
		entity_t ent;

		if (locations[i].used == false)
			continue;

		dist = ((float)cl.frame.playerstate.pmove.origin[0] * 0.125f - locations[i].origin[0]) *
		    ((float)cl.frame.playerstate.pmove.origin[0] * 0.125f - locations[i].origin[0]) +
		    ((float)cl.frame.playerstate.pmove.origin[1] * 0.125f - locations[i].origin[1]) *
		    ((float)cl.frame.playerstate.pmove.origin[1] * 0.125f - locations[i].origin[1]) +
		    ((float)cl.frame.playerstate.pmove.origin[2] * 0.125f - locations[i].origin[2]) *
		    ((float)cl.frame.playerstate.pmove.origin[2] * 0.125f - locations[i].origin[2]);

		if (dist > 4000 * 4000)
			continue;

		memset(&ent, 0, sizeof(entity_t));
		ent.origin[0] = locations[i].origin[0];
		ent.origin[1] = locations[i].origin[1];
		ent.origin[2] = locations[i].origin[2];
		ent.skinnum = 0;
		ent.skin = NULL;
//		memset(ent.skins, 0, sizeof(ent.skins));
		ent.model = NULL;

		if (i == index)
			ent.origin[2] += sin(cl.time * 0.01f) * 10.0f;

		V_AddEntity(&ent);
		num++;
	}
}


/*
=================
CL_LocHelp_f
=================
*/
void CL_LocHelp_f (void)
{
    // Xile/jitspoe - simple help cmd for reference
	Com_Printf(
		"Loc Commands:\n"
		"-------------\n"
		"loc_add <label/description>\n"
		"loc_del\n"
		"loc_save\n"
		"cl_drawlocs\n"
		"say_team $loc_here\n"
		"say_team $loc_there\n"
		"-------------\n");
}

#endif	// LOC_SUPPORT