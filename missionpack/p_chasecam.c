#include "g_local.h"

void ChasecamTrack (edict_t *ent);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void ChasecamStart (edict_t *ent)
{
	edict_t      *chasecam;
        
	ent->client->chasetoggle = 1;
	ent->client->chaseactive = 1;
	ent->client->ps.gunindex = 0;
        
	chasecam = G_Spawn ();
	chasecam->owner = ent;
	chasecam->solid = SOLID_NOT;
	chasecam->movetype = MOVETYPE_FLYMISSILE;

	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	
	ent->svflags |= SVF_NOCLIENT;
	ent->s.renderfx |= RF_NOSHADOW;

	VectorCopy (ent->s.angles, chasecam->s.angles);
	VectorClear (chasecam->mins);
	VectorClear (chasecam->maxs);
	VectorCopy (ent->s.origin, chasecam->s.origin);
        
	chasecam->classname = "chasecam";
	chasecam->nextthink = level.time + 0.100;
	chasecam->prethink = ChasecamTrack;

	ent->client->chasecam = chasecam;
	ent->client->oldplayer = G_Spawn();
	//Knightmare- mark as oldplayer and pointer to client entity
	ent->client->oldplayer->svflags |= SVF_OLDPLAYER;
	ent->client->oldplayer->to = ent;

	// Lazarus reflections
	// remove reflection of real player, if any
	DeleteReflection (ent, -1);
}


void ChasecamRemove (edict_t *ent) 
{
	/* Stop the chasecam from moving */
	VectorClear (ent->client->chasecam->velocity);
    ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	// if (ent->owner->health) ?? SKid
	ent->s.modelindex = ent->client->oldplayer->s.modelindex;
	ent->s.modelindex2= ent->client->oldplayer->s.modelindex2;
	ent->s.modelindex3= ent->client->oldplayer->s.modelindex3;

	ent->client->ps.pmove.pm_type &= ~PMF_NO_PREDICTION; 
	
	if(!ent->client->resp.spectator)
	{
		ent->svflags &= ~SVF_NOCLIENT;
		ent->s.renderfx &= ~RF_NOSHADOW;
	}

    gi.TagFree(ent->client->oldplayer->client);
	G_FreeEdict (ent->client->oldplayer);
	G_FreeEdict (ent->client->chasecam); 
	ent->client->chase_target = NULL;
    ent->client->chasetoggle = 0;
	ent->client->chaseactive = 0;
}


void ChasecamTrack (edict_t *ent)
{
    /* Create tempory vectors and trace variables */
	trace_t      tr;
	vec3_t       spot1, spot2, dir;
	vec3_t       forward, right, up;
	int          dist;
	double          cap;	 //mxd. int -> double
        
	ent->nextthink = level.time + 0.100;

    /* get the CLIENT's angle, and break it down into direction vectors,
     * of forward, right, and up. VERY useful */
	AngleVectors (ent->owner->client->v_angle, forward, right, up);
        
        /* go starting at the player's origin, forward, ent->chasedist1
         * distance, and save the location in vector spot2 */

   VectorMA (ent->owner->s.origin, -ent->chasedist1, forward, spot2);
        
        /* make spot2 a bit higher, but adding 20 to the Z coordinate */

   spot2[2] += 20.000;

        /* if the client is looking down, do backwards up into the air, 0.6
         * to the ratio of looking down, so the crosshair is still roughly
         * aiming at where the player is aiming. */

   if (ent->owner->client->v_angle[0] < 0.000)
      VectorMA (spot2, (ent->owner->client->v_angle[0] * 0.2), up, spot2);

        /* if the client is looking up, do the same, but do DOWN rather than
         * up, so the camera is behind the player aiming in a similar dir */

   else if (ent->owner->client->v_angle[0] > 0.000)
      VectorMA (spot2, (ent->owner->client->v_angle[0] * 0.2), up, spot2);

        /* make the tr traceline trace from the player model's position, to spot2,
         * ignoring the player, with MASK_SHOT. These masks have been fixed
         * from the previous version. The MASK_SHOT will stop the camera from
         * getting stuck in walls, sky, etc. */

   tr = gi.trace (ent->owner->s.origin, NULL, NULL, spot2, ent->owner, MASK_SHOT);
        
        /* subtract the endpoint from the start point for length and
         * direction manipulation */

   VectorSubtract (tr.endpos, ent->owner->s.origin, spot1);

        /* in this case, length */

   ent->chasedist1 = VectorLength (spot1);
        
        /* go, starting from the end of the trace, 2 points forward (client
         * angles) and save the location in spot2 */

   VectorMA (tr.endpos, 2, forward, spot2);

        /* make spot1 the same for tempory vector modification and make spot1
         * a bit higher than spot2 */

   VectorCopy (spot2, spot1);
   spot1[2] += 32;

        /* another trace from spot2 to spot2, ignoring player, no masks */

   tr = gi.trace (spot2, NULL, NULL, spot1, ent->owner, MASK_SHOT);

        /* if we hit something, copy the trace end to spot2 and lower spot2 */

   if (tr.fraction < 1.000)
   {
        VectorCopy (tr.endpos, spot2);
        spot2[2] -= 32;
   }

        /* subtract endpos spot2 from startpos the camera origin, saving it to
         * the dir vector, and normalize dir for a direction from the camera
         * origin, to the spot2 */

   VectorSubtract (spot2, ent->s.origin, dir);
   VectorNormalize (dir);
        
        /* subtract the same things, but save it in spot1 for a temporary
         * length calculation */

   VectorSubtract (spot2, ent->s.origin, spot1);
   dist = VectorLength (spot1);
        
        /* another traceline */

   tr = gi.trace (ent->s.origin, NULL, NULL, spot2, ent->owner, MASK_SHOT);
        
        /* if we DON'T hit anyting, do some freaky stuff <G> */

   if (tr.fraction == 1.000)
   {

           /* Make the angles of the chasecam, the same as the player, so
            * we are always behind the player. (angles) */

           VectorCopy (ent->owner->s.angles, ent->s.angles);
        
           /* calculate the percentages of the distances, and make sure we're
            * not going too far, or too short, in relation to our panning
            * speed of the chasecam entity */

      cap = (dist * 0.400);

           /* if we're going too fast, make us top speed */

      if (cap > 5.200)
      {
           ent->velocity[0] = ((dir[0] * dist) * 5.2);
           ent->velocity[1] = ((dir[1] * dist) * 5.2);
           ent->velocity[2] = ((dir[2] * dist) * 5.2);
      }
      else
      {

              /* if we're NOT going top speed, but we're going faster than
               * 1, relative to the total, make us as fast as we're going */

         if ( (cap > 1.000) )
         {
            ent->velocity[0] = ((dir[0] * dist) * cap);
            ent->velocity[1] = ((dir[1] * dist) * cap);
            ent->velocity[2] = ((dir[2] * dist) * cap);

         }
         else
         {

              /* if we're not going faster than one, don't accelerate our
               * speed at all, make us go slow to our destination */

            ent->velocity[0] = (dir[0] * dist);
            ent->velocity[1] = (dir[1] * dist);
            ent->velocity[2] = (dir[2] * dist);

         }

      }
                
           /* subtract endpos;player position, from chasecam position to get
            * a length to determine whether we should accelerate faster from
            * the player or not */

      VectorSubtract (ent->owner->s.origin, ent->s.origin, spot1);

      if (VectorLength(spot1) < 20)
      {
         ent->velocity[0] *= 2; 
         ent->velocity[1] *= 2; 
         ent->velocity[2] *= 2; 

      }
        
   }

        /* if we DID hit something in the tr.fraction call ages back, then
         * make the spot2 we created, the position for the chasecamera. */

   else
      VectorCopy (spot2, ent->s.origin);

        /* If the distance is less than 90, then we haven't reached the
         * furthest point. If we HAVEN'T reached the furthest point, keep
         * going backwards. This was a fix for the "shaking". The camera was
         * getting forced backwards, only to be brought back, next think */

   
   if (ent->chasedist1 < 90.00)   
	   ent->chasedist1 += 1;

        /* if we're too far away, give us a maximum distance */

   else if (ent->chasedist1 > 90.00) 
        ent->chasedist1 = 90;
		
		/* if we haven't gone anywhere since the last think routine, and we
         * are greater than 20 points in the distance calculated, add one to
         * the second chasedistance variable

         * The "ent->movedir" is a vector which is not used in this entity, so
         * we can use this a tempory vector belonging to the chasecam, which
         * can be carried through think routines. */

   if (ent->movedir == ent->s.origin)
   {
      if (dist > 20)
         ent->chasedist2++;
   }

        /* if we've buggered up more than 3 times, there must be some mistake,
         * so restart the camera so we re-create a chasecam, destroy the old one,
         * slowly go outwards from the player, and keep thinking this routing in
         * the new camera entity */

   if (ent->chasedist2 > 3)
   {
      ChasecamStart (ent->owner);
      G_FreeEdict(ent);
      return;
   }

        /* Copy the position of the chasecam now, and stick it to the movedir
         * variable, for position checking when we rethink this function */

   VectorCopy (ent->s.origin, ent->movedir);

}


void Cmd_Chasecam_Toggle (edict_t *ent)
{
	int i;
	edict_t *e;

////////////////////////////////////
// ADDED
	if((ent->client->resp.spectator == true))// || 
	   //(ent->client->resp.team == CTF_NOTEAM && ent->client->resp.player_class == NO_CLASS))
	{
		if (ent->client->chase_target) 
		{
			//ent->svflags &= ~SVF_NOCLIENT;       //added
			ent->client->chase_target = NULL;
		}
		else
		{
			for (i = 1; i <= maxclients->value; i++) 
			{
				e = g_edicts + i;
				if (e->inuse && e->solid != SOLID_NOT) 
				{
					ent->client->chase_target = e;
					ent->client->update_chase = true;
					ent->svflags |= SVF_NOCLIENT;    //added
					break;
				}
			}
		}
	}
	// Lazarus: Don't allow thirdperson when using spycam
	else if (!ent->deadflag  && !ent->client->spycam /*&& ent->client->resp.player_class*/)
	{
//		if (ent->client->chasetoggle)
		if (ent->client->chaseactive)
		{
		//	gi.cprintf (ent, PRINT_HIGH, "Leaving Third Person mode.\n");			
			ChasecamRemove (ent); 
		}
		// Knightmare- don't use server chasecam if client chasecam is on
		else if (!cl_thirdperson->value || deathmatch->value || coop->value)
		{
		//	gi.cprintf (ent, PRINT_HIGH, "Starting Third Person mode.\n");
			ChasecamStart (ent);
		}
	}
}


void CheckChasecam_Viewent (edict_t *ent)
{
	// Added by WarZone - Begin
	gclient_t       *cl;

	if (!ent->client->oldplayer->client)
	{
		cl = (gclient_t *) gi.TagMalloc(sizeof(gclient_t), TAG_LEVEL);
		ent->client->oldplayer->client = cl;
	}
// Added by WarZone - End

//	if ((ent->client->chasetoggle == 1) && (ent->client->oldplayer))	if ((ent->client->chasetoggle == 1) && (ent->client->oldplayer))
	if ((ent->client->chaseactive == 1) && (ent->client->oldplayer))
	{
		ent->client->oldplayer->s.frame = ent->s.frame;

		/* Copy the origin, the speed, and the model angle, NOT
                      * literal angle to the display entity */

		VectorCopy (ent->s.origin, ent->client->oldplayer->s.origin);
		VectorCopy (ent->velocity, ent->client->oldplayer->velocity);
		VectorCopy (ent->s.angles, ent->client->oldplayer->s.angles);
                     /* Make sure we are using the same model + skin as selected,
                      * as well as the weapon model the player model is holding.
                      * For customized deathmatch weapon displaying, you can
                      * use the modelindex2 for different weapon changing, as you
                      * can read in forthcoming tutorials */
		ent->client->oldplayer->s.modelindex = ent->s.modelindex;
		ent->client->oldplayer->s.modelindex2 = ent->s.modelindex2;
		ent->client->oldplayer->s.modelindex3 = ent->s.modelindex3;
		ent->client->oldplayer->s = ent->s;
		gi.linkentity (ent->client->oldplayer);
	}
}

