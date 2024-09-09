/*****************************************************************

 Cam Client source code - by Paul Jordan

 ..............................................................

 This file is Copyright(c) 1998, Paul Jordan, All Rights Reserved.

 ..............................................................

 All other files are Copyright(c) Id Software, Inc.

 Please see liscense.txt in the source directory for the copyright
 information regarding those files belonging to Id Software, Inc.

 ..............................................................

 Should you decide to release a modified version of the Camera, you
 MUST include the following text (minus the BEGIN and END lines) in
 the documentation for your modification, and also on all web pages
 related to your modification, should they exist.

 --- BEGIN ---

 The Client Camera is a product of Paul Jordan, and is available from
 the Quake2 Camera homepage at http://www.prismnet.com/~jordan/q2cam,
 Or as part of the Eraser Bot at http://impact.frag.com.

 This program is a modification of the Quake2 Client Camera, and is
 therefore in NO WAY supported by Paul.

 This program MUST NOT be sold in ANY form. If you have paid for
 this product, you should contact Paul Jordan immediately, via
 the Quake2 Camera Client homepage.

 --- END ---

 Adios and have fun,

 Paul Jordan

 *****************************************************************/

#include "g_local.h"
#include "camclient.h"
#include "extra.h"

enum {
	CAM_FOLLOW_MODE,
	CAM_NORMAL_MODE
};

#define DAMP_ANGLE_Y 6
#define DAMP_VALUE_XY 6
#define DAMP_VALUE_Z  3

// Knightmare- made these vars extern to fix GCC compile
sPlayerList *EntityListHead();
sPlayerList *EntityListNext(sPlayerList *pCurrent);
sPlayerList *pTempFind;

sPlayerList *EntityListHead (void);
void CameraStaticThink (edict_t *ent, usercmd_t *ucmd);

edict_t *pDeadPlayer = NULL;

int CameraCmd (edict_t *ent)
{
	double
		fTemp;

	if ( Q_stricmp(gi.argv(1), "on") == 0)
	{
// AJ - moved code to camera_on() to make it possible to call from menu
		camera_on(ent,NULL);
// end AJ
		return true;
	}
// AJ camera mode off
	if ( Q_stricmp(gi.argv(1), "off") == 0)
	{
		ent->client->bIsCamera = 0;
		PutClientInServer(ent);
		ent->svflags &= ~SVF_NOCLIENT;
	}
// end AJ
	else if (ent->classname[0] != 'c')
	{	// not already in CAM mode, so don't go any further
		return false;
	}
	else if ( Q_stricmp(gi.argv(1), "follow") == 0)
	{
		ent->client->iMode = CAM_FOLLOW_MODE;
	}
	else if ( Q_stricmp(gi.argv(1), "normal") == 0)
	{
		ent->client->iMode = CAM_NORMAL_MODE;
	}
	else if (( Q_stricmp(gi.argv(1), "min_xy") == 0) && ent->client->bIsCamera)
	{
		if ((fTemp = atof(gi.argv(2))) < 1)
		{
			gi.cprintf (ent, PRINT_HIGH, "Min X/Y delta of %f unchanged!\n",ent->client->fXYLag);
		}
		else
		{
			ent->client->fXYLag = fTemp;
			gi.cprintf (ent, PRINT_HIGH, "Min X/Y delta of %f. set.\n",ent->client->fXYLag);
		}
	}
	else if (( Q_stricmp(gi.argv(1), "min_z") == 0) && ent->client->bIsCamera)
	{
		if ((fTemp = atof(gi.argv(2))) < 1)
		{
			gi.cprintf (ent, PRINT_HIGH, "Min Z delta of %f unchanged!\n",ent->client->fZLag);
		}
		else
		{
			ent->client->fZLag = fTemp;
			gi.cprintf (ent, PRINT_HIGH, "Min Z delta of %f set.\n",ent->client->fZLag);
		}
	}
	else if (( Q_stricmp(gi.argv(1), "min_angle") == 0) && ent->client->bIsCamera)
	{
		if ((fTemp = atof(gi.argv(2))) < 1)
		{
			gi.cprintf (ent, PRINT_HIGH, "Min Yaw Angle delta of %f unchanged!\n",ent->client->fAngleLag);
		}
		else
		{
			ent->client->fAngleLag = fTemp;
			gi.cprintf (ent, PRINT_HIGH, "Min Yaw Angle delta of %f set.\n",ent->client->fAngleLag);
		}
	}

	return false;
}

// AJ


// AJ - cut+pasted this function from above code
void camera_on (edict_t *ent, pmenu_t *menu)
{
	// clear entity values
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_NO;
	ent->movetype = MOVETYPE_FLY;
	ent->viewheight = 0;
	ent->classname = "camera";
	ent->mass = 0;
	ent->solid = SOLID_NOT;
	ent->deadflag = DEAD_NO;
	ent->clipmask = MASK_ALL;
	ent->model = "";
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = FL_FLY;// | FL_GODMODE;
	ent->client->bIsCamera = 1;

	ent->client->ps.fov = 90;
	ent->client->iMode = CAM_NORMAL_MODE;
	ent->client->bWatchingTheDead = FALSE;
	ent->client->fXYLag = DAMP_VALUE_XY;
	ent->client->fZLag = DAMP_VALUE_Z;
	ent->client->fAngleLag = DAMP_ANGLE_Y;

	// Ridah, clear the view
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	memset(ent->client->ps.stats, 0, sizeof(ent->client->ps.stats));

	CTFPlayerResetGrapple(ent);
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
	// Ridah, done.

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = 0;//ent - g_edicts - 1;
	ent->s.modelindex = 0;//255;		// will use the skin specified model
	ent->s.modelindex2 = 0;//255;		// custom gun model
	ent->s.frame = 0;

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = 0;
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
	VectorCopy (ent->s.angles, ent->client->v_angle);

	ent->client->showscores = false;
	ent->client->showinventory = false;
	ent->client->pers.hand = CENTER_HANDED;
	ent->client->ps.stats[STAT_HELPICON] = 0;
}
// end AJ


void PlayerDied (edict_t *pPlayer)
{
	if ( pPlayer->client )
	{
		pDeadPlayer = pPlayer;
	}
}

qboolean IsVisible (edict_t *pPlayer1, edict_t *pPlayer2)
{
	vec3_t  vLength;
	int		 distance;
	trace_t trace;

	// Ridah, added this so se check for looking through non-transparent water
	if (!gi.inPVS(pPlayer1->s.origin, pPlayer2->s.origin))
		return FALSE;
	// done.

	trace = gi.trace (pPlayer1->s.origin, vec3_origin, vec3_origin, pPlayer2->s.origin, pPlayer1, MASK_SOLID);

	vLength[0] = pPlayer1->s.origin[0] - pPlayer2->s.origin[0];
  	vLength[1] = pPlayer1->s.origin[1] - pPlayer2->s.origin[1];
	vLength[2] = pPlayer1->s.origin[2] - pPlayer2->s.origin[2];
	distance = VectorLength(vLength);

	if ( (distance < MAX_VISIBLE_RANGE) && (trace.fraction == 1.0) )
	{
		return TRUE;
	}
	return FALSE;
}
//============================================================================
//============================================================================
int NumPlayersVisible (edict_t *pViewer)
{
	int			iCount = 0;
	sPlayerList	*pTarget;

	for ( pTarget = EntityListHead(); pTarget != NULL; pTarget=pTarget->pNext)
	{
//		if ((!pTarget->pEntity->deadflag) &&
		if (!pTarget->pEntity->client->bIsCamera)
		{
			if (IsVisible(pTarget->pEntity, pViewer))
			{
				iCount++;

				// Ridah, added this so we follow the flag carrier if visible
				if (	(pTarget->pEntity->s.effects & (EF_FLAG1 | EF_FLAG2))
					&&	(!pViewer->client->pTarget || !(pViewer->client->pTarget->s.effects & (EF_FLAG1 | EF_FLAG2))))
				{	// view this person instead
					pViewer->client->pTarget = pTarget->pEntity;
				}
				// Ridah, done.
			}
		}
	}
	return iCount;
}

edict_t *ClosestVisible (edict_t *ent)
{
	vec3_t		vDistance;
	sPlayerList	*pTarget, *pBest = NULL;
	unsigned int iCurrent, iClosest = 0xffffffff;

	for ( pTarget = EntityListHead(); pTarget != NULL; pTarget = pTarget->pNext)
	{
		if (!pTarget->pEntity->client->bIsCamera &&
			IsVisible(pTarget->pEntity, ent))
		{
			VectorSubtract(pTarget->pEntity->s.origin, ent->s.origin, vDistance);
			iCurrent = VectorLength(vDistance);
			if (iCurrent < iClosest)
			{
				pBest = pTarget;
				iClosest = iCurrent;
			}
		}
	}
	if (pBest == NULL)
	{
		return NULL;
	}
	return pBest->pEntity;
}
//============================================================================
//============================================================================
edict_t *PlayerToFollow (void)
{
	sPlayerList	*pViewer, *pBest = NULL;
	int		iPlayers, iBestCount = 0;

	for ( pViewer = EntityListHead(); pViewer != NULL; pViewer = pViewer->pNext)
	{
		iPlayers = 0;
		//
		// Don't switch to dead people
		//
		if ( (!pViewer->pEntity->deadflag) &&
			(!pViewer->pEntity->client->bIsCamera))
		{
			iPlayers = NumPlayersVisible(pViewer->pEntity);
			if (iPlayers > iBestCount)
			{
				iBestCount = iPlayers;
				pBest = pViewer;
			}
			else if ((iPlayers != 0) && (iPlayers == iBestCount) )
			{
				if (pBest->pEntity->client->resp.score <
					pViewer->pEntity->client->resp.score)
				{
					pBest = pViewer;
				}
			}
		}
	}
	if (pBest == NULL)
	{
		return NULL;
	}
	return pBest->pEntity;
}


//============================================================================
//============================================================================
void CameraAloneThink (edict_t *ent, usercmd_t *ucmd)
{
	CameraStaticThink(ent, ucmd);
}

//============================================================================
//============================================================================
void PointCamAtOrigin (edict_t *ent, vec3_t vLocation)
{
	vec3_t	vDiff, vAngles;

	VectorSubtract(vLocation,ent->s.origin,vDiff);

	vectoangles(vDiff, vAngles);

	VectorCopy (vAngles, ent->s.angles);
	VectorCopy (vAngles, ent->client->ps.viewangles);
	VectorCopy (vAngles, ent->client->v_angle);
}

//============================================================================
//============================================================================
void PointCamAtTarget (edict_t *ent)
{
	vec3_t	vDiff, vAngles;
	float	fDifference;

	VectorSubtract(ent->client->pTarget->s.origin,ent->s.origin,vDiff);

	vectoangles(vDiff, vAngles);

	ent->s.angles[0] = vAngles[0];
	ent->s.angles[2] = 0;
	fDifference = vAngles[1] - ent->s.angles[1];

	while (abs(fDifference) > 180)
	{
		if (fDifference > 0)
		{
			fDifference -= 360;
		}
		else
		{
			fDifference += 360;
		}
	}

	if (abs(fDifference) > ent->client->fAngleLag)
	{
		if (fDifference > 0)
		{
			ent->s.angles[1] += ent->client->fAngleLag;
		}
		else
		{
			ent->s.angles[1] -= ent->client->fAngleLag;
		}
	}
	else
	{
		ent->s.angles[1] = vAngles[1];
	}

	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
	VectorCopy (ent->s.angles, ent->client->v_angle);
}

//============================================================================
//============================================================================
void RepositionAtTarget (edict_t *ent, vec3_t vOffsetPosition)
{
	vec3_t
		vDiff;
//		vAngles;
	vec3_t
		vCamPos,
		forward;//, up;

	trace_t
		trace;

	AngleVectors(ent->client->pTarget->client->v_angle, forward, NULL,NULL);
	forward[2] = 0;

	VectorNormalize(forward);

	vCamPos[0] = ent->client->pTarget->s.origin[0] +
		(vOffsetPosition[0] * forward[0]);

	vCamPos[1] = ent->client->pTarget->s.origin[1] +
		(vOffsetPosition[1] * forward[1]);

	vCamPos[2] = ent->client->pTarget->s.origin[2] +
		vOffsetPosition[2];

	trace = gi.trace( ent->client->pTarget->s.origin, NULL, NULL, vCamPos,
			ent->client->pTarget, CONTENTS_SOLID);

	// Ridah, changed this, so we are moved away from the intersection point
	if (trace.fraction < 1)
	{
		VectorSubtract(trace.endpos, ent->client->pTarget->s.origin, vDiff);
		VectorNormalize(vDiff);
		VectorMA(trace.endpos, -8, vDiff, trace.endpos);

		if (trace.plane.normal[2] > 0.8)
			trace.endpos[2] += 4;
/*
		trace.endpos[0] += (5 * forward[0]);
		trace.endpos[1] += (5 * forward[1]);
		trace.endpos[2] -= 5;
*/
	}
	// Ridah , done.

	if (abs(trace.endpos[0]-ent->s.origin[0]) > ent->client->fXYLag)
	{
		if (trace.endpos[0] > ent->s.origin[0])
		{
			ent->s.origin[0] += ent->client->fXYLag;
		}
		else
		{
			ent->s.origin[0] -= ent->client->fXYLag;
		}
	}
	else
	{
		ent->s.origin[0] = trace.endpos[0];
	}

	if (abs(trace.endpos[1]-ent->s.origin[1]) > ent->client->fXYLag)
	{
		if (trace.endpos[1] > ent->s.origin[1])
		{
			ent->s.origin[1] += ent->client->fXYLag;
		}
		else
		{
			ent->s.origin[1] -= ent->client->fXYLag;
		}
	}
	else
	{
		ent->s.origin[1] = trace.endpos[1];
	}

	if (abs(trace.endpos[2]-ent->s.origin[2]) > ent->client->fZLag)
	{
		if (trace.endpos[2] > ent->s.origin[2])
		{
			ent->s.origin[2] += ent->client->fZLag;
		}
		else
		{
			ent->s.origin[2] -= ent->client->fZLag;
		}
	}
	else
	{
		ent->s.origin[2] = trace.endpos[2];
	}

	trace = gi.trace( ent->client->pTarget->s.origin, NULL, NULL, ent->s.origin,
		ent->client->pTarget, CONTENTS_SOLID);

	// Ridah, changed this, so we are moved away from the intersection point
	if (trace.fraction < 1)
	{
		VectorSubtract(trace.endpos, ent->client->pTarget->s.origin, vDiff);
		VectorNormalize(vDiff);
		VectorMA(trace.endpos, -8, vDiff, trace.endpos);

		if (trace.plane.normal[2] > 0.8)
			trace.endpos[2] += 4;

		VectorCopy(trace.endpos, ent->s.origin);
	}
/*
	if (trace.fraction != 1)
	{

		VectorSubtract(ent->client->pTarget->s.origin,ent->s.origin,vDiff);

		vectoangles(vDiff, vAngles);

		AngleVectors(vAngles, forward, NULL,NULL);

		forward[2] = 0;

		VectorNormalize(forward);

		trace.endpos[0] += (5 * forward[0]);
		trace.endpos[1] += (5 * forward[1]);
		trace.endpos[2] -= 5;
		VectorCopy(trace.endpos, ent->s.origin);
	}
*/
	// Ridah , done.
}

//============================================================================
//============================================================================
void RepositionAtOrigin (edict_t *ent, vec3_t vOffsetPosition)
{
	vec3_t
		vCamPos;

	trace_t
		trace;

	vCamPos[0] = vOffsetPosition[0] + 40;

	vCamPos[1] = vOffsetPosition[1] + 40;

	vCamPos[2] = vOffsetPosition[2] + 30;

	trace = gi.trace( vOffsetPosition, NULL, NULL, vCamPos,
			ent->client->pTarget, CONTENTS_SOLID);

	// Ridah, added this, so we are moved away from the intersection point
	if (trace.fraction < 1)
	{
		vec3_t vDiff;

		VectorSubtract(trace.endpos, vOffsetPosition, vDiff);
		VectorNormalize(vDiff);
		VectorMA(trace.endpos, -8, vDiff, trace.endpos);

		if (trace.plane.normal[2] > 0.8)
			trace.endpos[2] += 4;
	}
	// Ridah, done.

	if (abs(trace.endpos[0]-ent->s.origin[0]) > ent->client->fXYLag)
	{
		if (trace.endpos[0] > ent->s.origin[0])
		{
			ent->s.origin[0] += ent->client->fXYLag;
		}
		else
		{
			ent->s.origin[0] -= ent->client->fXYLag;
		}
	}
	else
	{
		ent->s.origin[0] = trace.endpos[0];
	}

	if (abs(trace.endpos[1]-ent->s.origin[1]) > ent->client->fXYLag)
	{
		if (trace.endpos[1] > ent->s.origin[1])
		{
			ent->s.origin[1] += ent->client->fXYLag;
		}
		else
		{
			ent->s.origin[1] -= ent->client->fXYLag;
		}
	}
	else
	{
		ent->s.origin[1] = trace.endpos[1];
	}

	if (abs(trace.endpos[2]-ent->s.origin[2]) > ent->client->fZLag)
	{
		if (trace.endpos[2] > ent->s.origin[2])
		{
			ent->s.origin[2] += ent->client->fZLag;
		}
		else
		{
			ent->s.origin[2] -= ent->client->fZLag;
		}
	}
	else
	{
		ent->s.origin[2] = trace.endpos[2];
	}

	trace = gi.trace( vOffsetPosition, NULL, NULL, ent->s.origin,
		ent->client->pTarget, CONTENTS_SOLID);

	// Ridah, added this, so we are moved away from the intersection point
	if (trace.fraction < 1)
	{
		vec3_t vDiff;

		VectorSubtract(trace.endpos, vOffsetPosition, vDiff);
		VectorNormalize(vDiff);
		VectorMA(trace.endpos, -8, vDiff, trace.endpos);

		if (trace.plane.normal[2] > 0.8)
			trace.endpos[2] += 4;
	}
	// Ridah, done.

	if (trace.fraction != 1)
	{
		VectorCopy(trace.endpos, ent->s.origin);
	}
}
void UpdateValues (edict_t *ent)
{
	edict_t
		*pTarget;

	pTarget = ent->client->pTarget;

	if (pTarget != NULL)
	{
	//	ent->client->resp.score = pTarget->client->resp.score;
	//	ent->health = pTarget->health;
	//	ent->client->ammo_index = pTarget->client->ammo_index;
	//	ent->client->pers.inventory[ent->client->ammo_index] =
	//	pTarget->client->pers.inventory[ent->client->ammo_index];
	}
	else
	{
		ent->client->resp.score = 0;
		ent->health = 0;
	}
}


//============================================================================
//============================================================================
void CameraFollowThink (edict_t *ent, usercmd_t *ucmd)
{
	vec3_t	vCameraOffset;
// Ridah, changed this so we keep tracking the same player until a new player is selected
	if (ent->client->pTarget || (ent->client->pTarget = PlayerToFollow()) != NULL)
	{
		//
		// Just keep looking for action!
		//
		vCameraOffset[0] = -60;
		vCameraOffset[1] = -60;
		vCameraOffset[2] = 40;
		RepositionAtTarget(ent, vCameraOffset);
		PointCamAtTarget(ent);
	}
	UpdateValues (ent);
}

//============================================================================
//============================================================================
void CameraNormalThink (edict_t *ent, usercmd_t *ucmd)
{
	vec3_t
		vCameraOffset;
	int
		iNumVis;

	iNumVis = NumPlayersVisible(ent);

	// Ridah, changed this so it only changes when our following target dies
//	if (pDeadPlayer && IsVisible(ent, pDeadPlayer))
	if (!ent->client->bWatchingTheDead && ent->client->pTarget && ent->client->pTarget->deadflag)
	{
		ent->client->bWatchingTheDead = TRUE;
//		ent->client->pTarget = pDeadPlayer;
		ent->last_move_time = level.time + CAMERA_DEAD_SWITCH_TIME;
		PointCamAtTarget(ent);
	}
	else if (ent->client->bWatchingTheDead)
	{
		if (ent->last_move_time < level.time)
		{
			ent->client->bWatchingTheDead = FALSE;
		}
		else
		{
			if (ent->client->pTarget->deadflag)
			{
				VectorCopy(ent->client->pTarget->s.origin,ent->client->vDeadOrigin);
			}
			PointCamAtOrigin(ent, ent->client->vDeadOrigin);
			RepositionAtOrigin(ent, ent->client->vDeadOrigin);
		}
	}
	else if ( iNumVis < 2 )
	{
		vCameraOffset[0] = -60;
		vCameraOffset[1] = -60;
		vCameraOffset[2] = 40;

		if (ent->last_move_time > level.time)
		{
			if (iNumVis > 0)
			{
				//
				// This should always be true but who knows lets check anyway.
				//
				if ((ent->client->pTarget = ClosestVisible(ent)) != NULL)
				{
					RepositionAtTarget(ent, vCameraOffset);
					PointCamAtTarget(ent);
				}
			}
			else if ((ent->client->pTarget = PlayerToFollow()) != NULL)
			{
				//
				// Look for someone new!
				//
				RepositionAtTarget(ent, vCameraOffset);
				PointCamAtTarget(ent);
				ent->last_move_time = 0;
			}
		}
		else if ((ent->client->pTarget = PlayerToFollow()) != NULL)
		{
			//
			// Just keep looking for action!
			//
			vCameraOffset[0] = -60;
			vCameraOffset[1] = -60;
			vCameraOffset[2] = 40;
			RepositionAtTarget(ent, vCameraOffset);
			PointCamAtTarget(ent);
		}
	}
	else if ((ent->last_move_time < level.time) || (ent->client->pTarget && !gi.inPVS(ent->s.origin, ent->client->pTarget->s.origin)))
	{
		if ((ent->client->pTarget = PlayerToFollow()) != NULL)
		{
			vCameraOffset[0] = -60;
			vCameraOffset[1] = -60;
			vCameraOffset[2] = 80;
			PointCamAtTarget(ent);
			RepositionAtTarget(ent, vCameraOffset);
			ent->last_move_time = level.time + CAMERA_SWITCH_TIME;
		}
	}
	else if (ent->client->pTarget != NULL)
	{
// Ridah, added this to give the camera some movement
		if (	(ent->last_move_time > (level.time + CAMERA_SWITCH_TIME - 3))
			||	(ent->last_move_time < (level.time + 5)))
		{
			RepositionAtOrigin(ent, ent->client->pTarget->s.origin);
		}
// Ridah, done.

		PointCamAtTarget(ent);
	}

	pDeadPlayer = NULL;

	if (ent->client->pTarget == NULL)
	{
		return;
	}

	UpdateValues(ent);
}

//============================================================================
//============================================================================
void CameraStaticThink (edict_t *ent, usercmd_t *ucmd)
{
	trace_t
		trace;
	vec3_t
		vEndFloor,
		vEndCeiling;

	vEndFloor[0] = ent->s.origin[0];
	vEndFloor[1] = ent->s.origin[1];
	vEndFloor[2] = ent->s.origin[2] - 40000;
	trace = gi.trace( ent->s.origin, NULL, NULL, vEndFloor, ent, CONTENTS_SOLID);

	VectorCopy ( trace.endpos, vEndFloor );

	vEndCeiling[0] = vEndFloor[0];
	vEndCeiling[1] = vEndFloor[1];
	vEndCeiling[2] = vEndFloor[2] + 175;
	trace = gi.trace( vEndFloor, NULL, NULL, vEndCeiling, ent, CONTENTS_SOLID);

	VectorCopy ( trace.endpos, ent->s.origin );

	if ( ent->last_move_time < level.time )
	{
		ent->last_move_time = level.time + 2;
		ent->s.angles[0] = 45;
		ent->s.angles[1] = 0;
		ent->s.angles[2] = 0;
		VectorCopy (ent->s.angles, ent->client->ps.viewangles);
		VectorCopy (ent->s.angles, ent->client->v_angle);
	}
}

//============================================================================
//============================================================================
void CameraThink (edict_t *ent, usercmd_t *ucmd)
{
	// Ridah, added this to cycle through players
	if ((ent->client->pTarget && !ent->client->pTarget->inuse) || ((ucmd->buttons & BUTTON_ATTACK) && !(ent->client->oldbuttons & BUTTON_ATTACK)))
	{
		int count=0;
		sPlayerList *start, *trav;

		trav = EntityListHead();

		while (trav && ent->client->pTarget && ent->client->pTarget->inuse && (trav->pEntity != ent->client->pTarget))
		{
			trav = EntityListNext(trav);
		}

		start = trav;

		if (!(trav = EntityListNext(trav)))
			trav = EntityListHead();

		while (!trav->pEntity->solid)
		{
			if (!(trav = EntityListNext(trav)))
				trav = EntityListHead();
			count++;

			if (count >= maxclients->value)
			{
				break;
			}
		}

		if (count < maxclients->value)
		{
			ent->client->pTarget = trav->pEntity;
			gi.cprintf(ent, PRINT_HIGH, "Now showing %s\n", ent->client->pTarget->client->pers.netname);
		}
	}

	ent->client->oldbuttons = ucmd->buttons;
	// Ridah, done.

	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.pmove.gravity = 0;

	if (EntityListNumber() == 0)
	{
		CameraAloneThink(ent,ucmd);
	}
	else
	{
		switch (ent->client->iMode)
		{
			case CAM_FOLLOW_MODE:
				CameraFollowThink(ent,ucmd);
				break;
			case CAM_NORMAL_MODE:
			default:
				CameraNormalThink(ent,ucmd);
				break;

		}
	}
}


sPlayerList		*pEntityListHead;
unsigned long	ulCount=0;

//============================================================================
//============================================================================
void EntityListRemove (edict_t *pEntity)
{
	sPlayerList
		*pTrail, *pLead;

	pTrail = pLead = pEntityListHead;

	while ( pLead != NULL )
	{
		if (pLead->pEntity->client->bIsCamera)
		{
			//
			// Force rethink on all cameras
			//
			pLead->pEntity->last_move_time = level.time;
		}
		if (pLead->pEntity == pEntity)
		{
			if ( pLead == pTrail)
			{
				pEntityListHead = pLead->pNext;

			}
			else
			{
				pTrail->pNext = pLead->pNext;
			}

			free(pLead);
			ulCount--;
			pLead = NULL;
		}
		else
		{
			pTrail = pLead;
			pLead = pLead->pNext;
		}
	}
}

//============================================================================
//============================================================================
void EntityListAdd (edict_t *pEntity)
{
	sPlayerList *pNew;

	pNew = malloc (sizeof(sPlayerList));
	pNew->pEntity = pEntity;
	pNew->pNext = pEntityListHead;
	pEntityListHead = pNew;
	ulCount++;
}

//============================================================================
//============================================================================
unsigned long EntityListNumber (void)
{
	return ulCount;
}

//============================================================================
//============================================================================
sPlayerList *EntityListHead (void)
{
	if (pEntityListHead != NULL)
	{
		return (pEntityListHead);
	}
	else
	{
		return NULL;
	}
}

sPlayerList *EntityListNext (sPlayerList *pCurrent)
{
	return (pCurrent->pNext);
}
//============================================================================
//============================================================================
void PrintEntityList (void)
{
	sPlayerList	*pNode;

	unsigned long ultemp=0;
	gi.dprintf	("PrintEntityList\n");

	for ( pNode = pEntityListHead; pNode != NULL; pNode = pNode->pNext)
	{
		gi.dprintf("Name: %s ",pNode->pEntity->client->pers.netname);
		gi.dprintf("Class: %s\n",pNode->pEntity->classname);
		ultemp++;
	}
	gi.dprintf("Actual Count: %d List Count %d\n",ultemp,EntityListNumber());
}

void EnitityListClean (void)
{
	sPlayerList		*pNode;

	while (NULL != (pNode=EntityListHead()))
	{
	//	gi.dprintf("Name: %s\n",pNode->pEntity->client->pers.netname);
		EntityListRemove(pNode->pEntity);
	}
}
