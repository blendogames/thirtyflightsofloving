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

#include "client.h"


/*
===================
CL_CheckPredictionError
===================
*/
void CL_CheckPredictionError (void)
{
	int		frame;
	int		delta[3];
	int		i;
	int		len;

//	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	if (!cl_predict->integer || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
		return;

	// calculate the last usercmd_t we sent that the server has processed
	frame = cls.netchan.incoming_acknowledged;
	frame &= (CMD_BACKUP-1);

	// compare what the server returned with what we had predicted it to be
	VectorSubtract (cl.frame.playerstate.pmove.origin, cl.predicted_origins[frame], delta);

	// save the prediction error for interpolation
	len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
	if (len > 640)	// 80 world units
	{	// a teleport or something
		VectorClear (cl.prediction_error);
	}
	else
	{
	//	if (cl_showmiss->value && (delta[0] || delta[1] || delta[2]) )
		if (cl_showmiss->integer && (delta[0] || delta[1] || delta[2]) )
			Com_Printf ("prediction miss on %i: %i\n", cl.frame.serverframe, 
			delta[0] + delta[1] + delta[2]);

		VectorCopy (cl.frame.playerstate.pmove.origin, cl.predicted_origins[frame]);

		// save for error itnerpolation
		for (i=0 ; i<3 ; i++)
			cl.prediction_error[i] = delta[i]*0.125;
	}
}


/*
====================
CL_ClipMoveToEntities
====================
*/
void CL_ClipMoveToEntities (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, trace_t *tr)
{
	int			i, x, zd, zu;
	trace_t		trace;
	int			headnode;
	float		*angles;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	vec3_t		bmins, bmaxs;

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;

		if (ent->number == cl.playernum+1)
			continue;

		if (ent->solid == 31)
		{	// special value for bmodel
			cmodel = cl.model_clip[ent->modelindex];
			if (!cmodel)
				continue;
			headnode = cmodel->headnode;
			angles = ent->angles;
		}
		else
		{	// encoded bbox
			x = 8*(ent->solid & 31);
			zd = 8*((ent->solid>>5) & 31);
			zu = 8*((ent->solid>>10) & 63) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			headnode = CM_HeadnodeForBox (bmins, bmaxs);
			angles = vec3_origin;	// boxes don't rotate
		}

		if (tr->allsolid)
			return;

		trace = CM_TransformedBoxTrace (start, end,
			mins, maxs, headnode,  MASK_PLAYERSOLID,
			ent->origin, angles);

		if (trace.allsolid || trace.startsolid ||
		trace.fraction < tr->fraction)
		{
			trace.ent = (struct edict_s *)ent;
		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
			tr->startsolid = true;
	}
}

/*
====================
CL_ClipMoveToEntities2
Similar to above, but uses entnum as reference.
====================
*/
void CL_ClipMoveToEntities2 (int entnum, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, trace_t *tr)
{
	int			i, x, zd, zu;
	trace_t		trace;
	int			headnode;
	float		*angles;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	vec3_t		bmins, bmaxs;

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;

		// Don't clip against the passed entity number
		if (ent->number == entnum)
			continue;

		if (ent->solid == 31)
		{	// special value for bmodel
			cmodel = cl.model_clip[ent->modelindex];
			if (!cmodel)
				continue;
			headnode = cmodel->headnode;
			angles = ent->angles;
		}
		else
		{	// encoded bbox
			x = 8*(ent->solid & 31);
			zd = 8*((ent->solid>>5) & 31);
			zu = 8*((ent->solid>>10) & 63) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			headnode = CM_HeadnodeForBox (bmins, bmaxs);
			angles = vec3_origin;	// boxes don't rotate
		}

		if (tr->allsolid)
			return;

		trace = CM_TransformedBoxTrace (start, end,
			mins, maxs, headnode,  MASK_PLAYERSOLID,
			ent->origin, angles);

		if (trace.allsolid || trace.startsolid ||
		trace.fraction < tr->fraction)
		{
			trace.ent = (struct edict_s *)ent;
		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
			tr->startsolid = true;
	}
}

/*
====================
CL_ClipMoveToBrushEntities
Similar to CL_ClipMoveToEntities,
but only checks against brush models.
====================
*/
void CL_ClipMoveToBrushEntities ( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, trace_t *tr )
{
	int			i;
	trace_t		trace;
	int			headnode;
	float		*angles;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (ent->solid != 31) // brush models only
			continue;

		// special value for bmodel
		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;
		headnode = cmodel->headnode;
		angles = ent->angles;

		if (tr->allsolid)
			return;

		trace = CM_TransformedBoxTrace (start, end,
			mins, maxs, headnode,  MASK_PLAYERSOLID,
			ent->origin, angles);

		if (trace.allsolid || trace.startsolid ||
		trace.fraction < tr->fraction)
		{
			trace.ent = (struct edict_s *)ent;
		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
			tr->startsolid = true;
	}
}


/*
====================
CL_Trace
====================
*/
trace_t CL_Trace (vec3_t start, vec3_t end, float size,  int contentmask)
{
	vec3_t maxs, mins;

	VectorSet(maxs, size, size, size);
	VectorSet(mins, -size, -size, -size);

	return CM_BoxTrace (start, end, mins, maxs, 0, contentmask);
}


/*
====================
CL_BrushTrace
Similar to CL_Trace, but also
clips against brush models.
====================
*/
trace_t CL_BrushTrace (vec3_t start, vec3_t end, float size,  int contentmask)
{
	vec3_t maxs, mins;
	trace_t	t;

	VectorSet(maxs, size, size, size);
	VectorSet(mins, -size, -size, -size);

	t = CM_BoxTrace (start, end, mins, maxs, 0, contentmask);
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *)1;

	// check all solid brush models
	CL_ClipMoveToBrushEntities (start, mins, maxs, end, &t);

	return t;
}

/*
================
CL_PMTrace
================
*/
trace_t CL_PMTrace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	trace_t	t;

	if (!mins)
		mins = vec3_origin;
	if (!maxs)
		maxs = vec3_origin;

	// check against world
	t = CM_BoxTrace (start, end, mins, maxs, 0, MASK_PLAYERSOLID);
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *)1;

	// check all other solid models
	CL_ClipMoveToEntities (start, mins, maxs, end, &t);

	return t;
}

//Knightmare added- this can check using masks, good for checking surface flags
//	also checks for bmodels
/*
================
CL_PMSurfaceTrace
================
*/
trace_t CL_PMSurfaceTrace (int playernum, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int contentmask)
{
	trace_t	t;

	if (!mins)
		mins = vec3_origin;
	if (!maxs)
		maxs = vec3_origin;

	// check against world
	t = CM_BoxTrace (start, end, mins, maxs, 0, contentmask);
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *)1;

	// check all other solid models
	CL_ClipMoveToEntities2 (playernum, start, mins, maxs, end, &t);

	return t;
}

int CL_PMpointcontents (vec3_t point)
{
	int			i;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	int			contents;

	contents = CM_PointContents (point, 0);

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (ent->solid != 31) // special value for bmodel
			continue;

		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;

		contents |= CM_TransformedPointContents (point, cmodel->headnode, ent->origin, ent->angles);
	}

	return contents;
}


// Modified version of above for ignoring bmodels by Berserker
typedef struct model_s model_t;
int CL_PMpointcontents2 (vec3_t point, model_t *ignore)
{
	int			i;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	int			contents;

	contents = CM_PointContents (point, 0);

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (ent->solid != 31) // special value for bmodel
			continue;

		if (cl.model_draw[ent->modelindex] == ignore)
			continue;

		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;

		contents |= CM_TransformedPointContents (point, cmodel->headnode, ent->origin, ent->angles);
	}

	return contents;
}


/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictMovement (void)
{
	int			ack, current;
	int			frame;
	int			oldframe;
	usercmd_t	*cmd;
	pmove_t		pm;
	int			i;
	int			step;
	int			oldz;
#ifdef CLIENT_SPLIT_NETFRAME
	static int	last_step_frame = 0;
#endif

	if (cls.state != ca_active)
		return;

//	if (cl_paused->value)
	if (cl_paused->integer)
		return;

//	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	if (!cl_predict->integer || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{	// just set angles
		for (i=0 ; i<3 ; i++)
		{
			cl.predicted_angles[i] = cl.viewangles[i] + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[i]);
		}
		return;
	}

	ack = cls.netchan.incoming_acknowledged;
	current = cls.netchan.outgoing_sequence;

	// if we are too far out of date, just freeze
	if (current - ack >= CMD_BACKUP)
	{
	//	if (cl_showmiss->value)
		if (cl_showmiss->integer)
			Com_Printf ("exceeded CMD_BACKUP\n");
		return;	
	}

	// copy current state to pmove
	memset (&pm, 0, sizeof(pm));
	pm.trace = CL_PMTrace;
	pm.pointcontents = CL_PMpointcontents;

	pm_airaccelerate = atof(cl.configstrings[CS_AIRACCEL]);

	pm.s = cl.frame.playerstate.pmove;

//	SCR_DebugGraph (current - ack - 1, 0);

	frame = 0;

#ifdef CLIENT_SPLIT_NETFRAME
//	if (cl_async->value)
	if (cl_async->integer)
	{
		// run frames
		while (++ack <= current) // Changed '<' to '<=' cause current is our pending cmd
		{
			frame = ack & (CMD_BACKUP-1);
			cmd = &cl.cmds[frame];

			if (!cmd->msec) // Ignore 'null' usercmd entries
				continue;

			pm.cmd = *cmd;
			Pmove (&pm);

			// save for debug checking
			VectorCopy (pm.s.origin, cl.predicted_origins[frame]);
		}

		oldframe = (ack-2) & (CMD_BACKUP-1);
		oldz = cl.predicted_origins[oldframe][2];
		step = pm.s.origin[2] - oldz;

		// TODO: Add Paril's step down fix here
		if (last_step_frame != current && step > 63 && step < 160 && (pm.s.pm_flags & PMF_ON_GROUND) )
		{
			cl.predicted_step = step * 0.125;
			cl.predicted_step_time = cls.realtime - cls.netFrameTime * 500;
			last_step_frame = current;
		}
	}
	else
	{
#endif // CLIENT_SPLIT_NETFRAME
		// run frames
		while (++ack < current)
		{
			frame = ack & (CMD_BACKUP-1);
			cmd = &cl.cmds[frame];

			pm.cmd = *cmd;
			Pmove (&pm);

			// save for debug checking
			VectorCopy (pm.s.origin, cl.predicted_origins[frame]);
		}

		oldframe = (ack-2) & (CMD_BACKUP-1);
		oldz = cl.predicted_origins[oldframe][2];
		step = pm.s.origin[2] - oldz;
		if (step > 63 && step < 160 && (pm.s.pm_flags & PMF_ON_GROUND) )
		{
			cl.predicted_step = step * 0.125;
			cl.predicted_step_time = cls.realtime - cls.netFrameTime * 500;
		}
#ifdef CLIENT_SPLIT_NETFRAME
	}
#endif // CLIENT_SPLIT_NETFRAME


	// copy results out for rendering
	cl.predicted_origin[0] = pm.s.origin[0]*0.125;
	cl.predicted_origin[1] = pm.s.origin[1]*0.125;
	cl.predicted_origin[2] = pm.s.origin[2]*0.125;

	VectorCopy (pm.viewangles, cl.predicted_angles);
}
