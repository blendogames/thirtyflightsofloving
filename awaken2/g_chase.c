// g_chase.c

//CW: This uses modified chase-cam code that was kindly sent in by 
//    Doug "Raven" Buckley; it was part of the Match Mod for Q2.

#include "g_local.h"

static char chase_modenames[][30] = {
	"FreeCam",
	"ChaseCam",
	"FloatCam",
	"EyeCam",
//CW++
	"RearCam"
//CW--
};

void SwitchModeChaseCam(edict_t *ent)
{

//	If chase cam is off, turn it on.

	if (!ent->client->chase_target)
	{
		ToggleChaseCam(ent, NULL);
		return;
	}

//	If we are in the last chasecam mode, turn it off.

	if (ent->client->chase_mode == CHASE_LASTMODE)
	{
		ToggleChaseCam(ent, NULL);
		return;
	}

//	Switch modes.

	gi_cprintf(ent, PRINT_HIGH, "Now using %s.\n", chase_modenames[++ent->client->chase_mode]);		//CW
}

void ToggleChaseCam(edict_t *ent, pmenu_t *p)
{
	edict_t	*e;
	int		i;

//	If it's on, turn if off.

	if (ent->client->chase_target)
	{
		gi_cprintf(ent, PRINT_HIGH, "ChaseCam deactivated.\n");										//CW
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

//	If it's off, find a new chase target and track it.
	
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		e = g_edicts + i;

		if (e->inuse && (e->solid != SOLID_NOT) && !e->isabot)										//CW
		{
			ent->client->chase_mode = CHASE_FIRSTMODE;
			ent->client->chase_target = e;
			ent->client->update_chase = true;
			PMenu_Close(ent);
			gi_cprintf(ent, PRINT_HIGH, "ChaseCam activated (using %s mode).\n", chase_modenames[ent->client->chase_mode]);	//CW
			return;
		}
	}

	gi_cprintf(ent, PRINT_HIGH, "ChaseCam - no target to chase!\n");								//CW
}

void UpdateChaseCam(edict_t *ent)
{
	edict_t	*targ;
	edict_t *old;
	vec3_t	org;
	vec3_t	ownerv;
	vec3_t	goal;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	oldgoal;
	vec3_t	angles;
	trace_t	trace;
	int		i;
	int		viewdist;

//	If our chase target is gone, move to the next, or stop chasing if there are
//	no more valid targets.

	if (!ent->client->chase_target->inuse || ent->client->chase_target->client->spectator)			//CW
	{
		old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old)
		{
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	targ = ent->client->chase_target;
	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(ent->s.origin, oldgoal);
	ownerv[2] += targ->viewheight;

//	If in freecam mode, use that angle (SUMFUKA).

	if (ent->client->chase_mode == CHASE_FREECAM)
	{
		for (i = 0; i < 3; i++)
			angles[i] = ent->client->resp.cmd_angles[i] + SHORT2ANGLE(ent->client->ps.pmove.delta_angles[i]);
	}
	else
		VectorCopy(targ->client->ps.viewangles, angles);

	if (angles[PITCH] > 56.0)
		angles[PITCH] = 56.0;

	AngleVectors(angles, forward, right, NULL);
	VectorNormalize(forward);

//	View at different distances (default -60) (SUMFUKA).

	viewdist = -60;	

//	Different distances...

	if (ent->client->chase_mode == CHASE_FLOATCAM)
		viewdist = -200;
	else if (ent->client->chase_mode == CHASE_EYECAM)
		viewdist = 20;

//CW++
	else if (ent->client->chase_mode == CHASE_BUTTCAM)
		viewdist = -30;
//CW--

	VectorMA(ownerv, viewdist, forward, org);
	if (org[2] < targ->s.origin[2] + -viewdist*0.6666667)											//CW
		org[2] = targ->s.origin[2] + -viewdist*0.6666667;

//	Jump animation lifts.

	if (!targ->groundentity)
		org[2] += 16.0;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, org, targ, MASK_SOLID);
	VectorCopy(trace.endpos, goal);
	VectorMA(goal, 2.0, forward, goal);

//	Pad for floors and ceilings.

	VectorCopy(goal, org);
	org[2] += 6.0;
	trace = gi.trace(goal, vec3_origin, vec3_origin, org, targ, MASK_SOLID);
	if (trace.fraction < 1)
	{
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6.0;
	}

	VectorCopy(goal, org);
	org[2] -= 6.0;
	trace = gi.trace(goal, vec3_origin, vec3_origin, org, targ, MASK_SOLID);
	if (trace.fraction < 1)
	{
		VectorCopy(trace.endpos, goal);
		goal[2] += 6.0;
	}

//	Using a free-move chasecam mode? (SUMFUKA).

	if (ent->client->chase_mode == CHASE_FREECAM)
		ent->client->ps.pmove.pm_type = PM_SPECTATOR;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);

//	Only set angles if in a fixed viewangle mode (SUMFUKA).

	if (ent->client->chase_mode != CHASE_FREECAM)
	{
		for (i = 0; i < 3; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

//CW++
		if (ent->client->chase_mode == CHASE_BUTTCAM)
		{
			ent->client->ps.viewangles[0] = -targ->client->ps.viewangles[0];
			ent->client->ps.viewangles[1] = 180.0 + targ->client->ps.viewangles[1];
			ent->client->ps.viewangles[2] = 0.0;

			ent->client->v_angle[0] = -targ->client->ps.viewangles[0];
			ent->client->v_angle[1] = 180.0 + targ->client->ps.viewangles[1];
			ent->client->v_angle[2] = 0.0;
		}
		else
		{
//CW--
			VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
			VectorCopy(targ->client->v_angle, ent->client->v_angle);
		}
	}

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);

	if ((!ent->client->showscores && !ent->client->menu && !ent->client->showinventory && !ent->client->showhelp &&
		!(level.framenum & 31)) || ent->client->update_chase)
	{
//CW++
		char s[1024];

		Com_sprintf(s, sizeof(s), "xv 0 yb -68 string2 \"Chasing %s\"", targ->client->pers.netname);
		gi.WriteByte(svc_layout);
		gi.WriteString(s);
		gi.unicast(ent, false);
//CW--
		ent->client->update_chase = false;
	}

}

void ChaseNext(edict_t *ent)
{
	edict_t	*e;
	int		i;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do
	{
		i++;
		if (i > (int)maxclients->value)
			i = 1;

		e = g_edicts + i;
		if (!e->inuse)
			continue;

		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
	edict_t	*e;
	int		i;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do
	{
		i--;
		if (i < 1)
			i = (int)maxclients->value;

		e = g_edicts + i;
		if (!e->inuse)
			continue;

		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChaseRemoveTarget(edict_t *target)
{
	edict_t	*ent;
	int		i;

//	Anyone chasing this target must no longer do so.

//CW++
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		ent = g_edicts + i;

		if (!ent->inuse)
			continue;
		if (ent->client->spectator)
			continue;
//CW--
		if (ent->client->chase_target == target)
		{
			gi_cprintf(ent, PRINT_HIGH, "ChaseCam deactivated - target lost!\n");					//CW
			ent->client->chase_target = NULL;
		}
	}

}
void GetChaseTarget(edict_t *ent)
{
	edict_t	*other;
	int		i;

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		other = g_edicts + i;

//CW++
		if (!other->inuse)
			continue;
		if (other->client->spectator)
			continue;
//CW--
		ent->client->chase_target = other;
		ent->client->update_chase = true;
		UpdateChaseCam(ent);
		return;
	}

	gi_centerprintf(ent, "No other players to chase.");												//CW
}

void ChaseHelp(edict_t *ent)
{
	gi_centerprintf(ent, "(use fire to change ChaseCam mode)\n(and [ or ] to change ChaseCam target)\n");
}
