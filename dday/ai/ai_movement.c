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
--------------------------------------------------------------
The ACE Bot is a product of Steve Yeager, and is available from
the ACE Bot homepage, at http://www.axionfx.com/ace.

This program is a modification of the ACE Bot, and is therefore
in NO WAY supported by Steve Yeager.
*/

#include "../g_local.h"
#include "ai_local.h"

void change_stance(edict_t *self, int stance);
//ACE

//==========================================
// AI_CanMove
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime.  
//==========================================
qboolean AI_CanMove(edict_t *self, int direction)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);

	if(direction == BOT_MOVE_LEFT)
		angles[1] += 90;
	else if(direction == BOT_MOVE_RIGHT)
		angles[1] -= 90;
	else if(direction == BOT_MOVE_BACK)
		angles[1] -=180;


	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);

	VectorSet(offset, 36, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	VectorSet(offset, 36, 0, -100);
	G_ProjectSource (self->s.origin, offset, forward, right, end);

	tr = gi.trace( start, NULL, NULL, end, self, MASK_AISOLID );

	if(tr.fraction == 1.0 || tr.contents & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		//if(AIDevel.debugChased)	//jal: is too spammy. Temporary disabled
		//	G_PrintMsg (AIDevel.devguy, PRINT_HIGH, "%s: move blocked\n", self->bot.botStatus.netname);
		return false;
	}

	return true;// yup, can move
}


//===================
//  AI_IsStep
//  Checks the floor one step below the player. Used to detect
//  if the player is really falling or just walking down a stair.
//===================
qboolean AI_IsStep (edict_t *ent)
{
	vec3_t		point;
	trace_t		trace;
	
	//determine a point below
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - (1.6*AI_STEPSIZE);
	
	//trace to point
//	trap_Trace (&trace, ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_PLAYERSOLID);
	trace = gi.trace( ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_PLAYERSOLID);
	
	if (trace.plane.normal[2] < 0.7 && !trace.startsolid)
		return false;
	
	//found solid.
	return true;
}


//==========================================
// AI_IsLadder
// check if entity is touching in front of a ladder
//==========================================
qboolean AI_IsLadder(vec3_t origin, vec3_t v_angle, vec3_t mins, vec3_t maxs, edict_t *passent)
{
	vec3_t	spot;
	vec3_t	flatforward, zforward;
	trace_t	trace;

	AngleVectors( v_angle, zforward, NULL, NULL);

	// check for ladder
	flatforward[0] = zforward[0];
	flatforward[1] = zforward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA ( origin, 1, flatforward, spot);

//	trap_Trace(&trace, self->s.origin, self->mins, self->maxs, spot, self, MASK_AISOLID);
	trace = gi.trace( origin, mins, maxs, spot, passent, MASK_AISOLID);
	
//	if ((trace.fraction < 1) && (trace.surfFlags & SURF_LADDER))
	if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
		return true;

	return false;
}


//==========================================
// AI_CheckEyes
// Helper for ACEMV_SpecialMove. 
// Tries to turn when in front of obstacle
//==========================================
qboolean AI_CheckEyes(edict_t *self, usercmd_t *ucmd)
{
	vec3_t  forward, right;
	vec3_t  leftstart, rightstart,focalpoint;
	vec3_t  dir,offset;
	trace_t traceRight;
	trace_t traceLeft;

	// Get current angle and set up "eyes"
	VectorCopy(self->s.angles,dir);
	AngleVectors (dir, forward, right, NULL);

	if(!self->movetarget)
		VectorSet(offset,200,0,self->maxs[2]*0.5); // focalpoint
	else
		VectorSet(offset,64,0,self->maxs[2]*0.5); // wander focalpoint 
	
	G_ProjectSource (self->s.origin, offset, forward, right, focalpoint);

	VectorSet(offset, 0, 18, self->maxs[2]*0.5);
	G_ProjectSource (self->s.origin, offset, forward, right, leftstart);
	offset[1] -= 36; //VectorSet(offset, 0, -18, self->maxs[2]*0.5);
	G_ProjectSource (self->s.origin, offset, forward, right, rightstart);

//	trap_Trace(&traceRight, rightstart, NULL, NULL, focalpoint, self, MASK_AISOLID);
//	trap_Trace(&traceLeft, leftstart, NULL, NULL, focalpoint, self, MASK_AISOLID);
	traceRight = gi.trace( rightstart, NULL, NULL, focalpoint, self, MASK_AISOLID);
	traceLeft = gi.trace( leftstart, NULL, NULL, focalpoint, self, MASK_AISOLID);

	// Find the side with more open space and turn
	if(traceRight.fraction != 1 || traceLeft.fraction != 1)
	{
		if(traceRight.fraction > traceLeft.fraction)
			self->s.angles[YAW] += (1.0 - traceLeft.fraction) * 45.0;
		else
			self->s.angles[YAW] += -(1.0 - traceRight.fraction) * 45.0;
		
		ucmd->forwardmove = 400;
		return true;
	}
				
	return false;
}

//==========================================
// AI_SpecialMove
// Handle special cases of crouch/jump
// If the move is resolved here, this function returns true.
//==========================================
qboolean AI_SpecialMove(edict_t *self, usercmd_t *ucmd)
{
	vec3_t forward;
	trace_t tr;
	vec3_t	boxmins, boxmaxs, boxorigin;

	// Get current direction
	AngleVectors( tv(0, self->s.angles[YAW], 0), forward, NULL, NULL );

	//make sure we are bloqued
	VectorCopy( self->s.origin, boxorigin );
	VectorMA( boxorigin, 8, forward, boxorigin ); //move box by 8 to front
	tr = gi.trace( self->s.origin, self->mins, self->maxs, boxorigin, self, MASK_AISOLID);
	if( !tr.startsolid && tr.fraction == 1.0 ) // not bloqued
		return false;

	if( self->ai->pers.moveTypesMask & LINK_JUMP && self->groundentity &&
		self->stanceflags == STANCE_STAND && self->stanceflags == self->oldstance)
	{
		//jump box
		VectorCopy( self->s.origin, boxorigin );
		VectorCopy( self->mins, boxmins );
		VectorCopy( self->maxs, boxmaxs );
		VectorMA( boxorigin, 8, forward, boxorigin );	//move box by 8 to front
		//
		boxorigin[2] += ( boxmins[2] + AI_JUMPABLE_HEIGHT );	//put at bottom + jumpable height
		boxmaxs[2] = boxmaxs[2] - boxmins[2];	//total player box height in boxmaxs
		boxmins[2] = 0;
		if( boxmaxs[2] > AI_JUMPABLE_HEIGHT ) //the player is smaller than AI_JUMPABLE_HEIGHT
		{
			boxmaxs[2] -= AI_JUMPABLE_HEIGHT;
			tr = gi.trace( boxorigin, boxmins, boxmaxs, boxorigin, self, MASK_AISOLID);
			if( !tr.startsolid )	//can move by jumping
			{
				if (self->ai->last_jump_try < level.time -3)
				{	ucmd->forwardmove = 400;
					ucmd->upmove = 2000;				
				return true;
				}
				else
					return false;
			}
		}
	}

	if ( (self->ai->pers.moveTypesMask & LINK_CROUCH && 
		self->stanceflags == STANCE_STAND == self->oldstance)
		|| self->is_swim )
	{
		//crouch box
		VectorCopy( self->s.origin, boxorigin );
		VectorCopy( self->mins, boxmins );
		VectorCopy( self->maxs, boxmaxs );
		boxmaxs[2] = 14;	//crouched size
		VectorMA( boxorigin, 8, forward, boxorigin ); //move box by 8 to front
		//see if bloqued
		tr = gi.trace( boxorigin, boxmins, boxmaxs, boxorigin, self, MASK_AISOLID);
		if( !tr.startsolid ) // can move by crouching
		{
//			gi.dprintf("DUCK\n");
			if (self->oldstance == self->stanceflags)
				change_stance (self, STANCE_DUCK);
			self->ai->ducktime = level.time;
			ucmd->forwardmove = 400;
			//ucmd->upmove = -400;
			return true;
		}
	}

	//nothing worked, check for turning
	return AI_CheckEyes(self, ucmd);
}


//==========================================
// AI_ChangeAngle
// Make the change in angles a little more gradual, not so snappy
// Subtle, but noticeable.
// 
// Modified from the original id ChangeYaw code...
//==========================================
void AI_ChangeAngle (edict_t *ent)
{
	float	ideal_yaw;
	float   ideal_pitch;
	float	current_yaw;
	float   current_pitch;
	float	move;
	float	speed;
	vec3_t  ideal_angle;


	// Normalize the move angle first
	VectorNormalize(ent->ai->move_vector);

	current_yaw = anglemod(ent->s.angles[YAW]);
	current_pitch = anglemod(ent->s.angles[PITCH]);

	vectoangles (ent->ai->move_vector, ideal_angle);

	ideal_yaw = anglemod(ideal_angle[YAW]);
	ideal_pitch = anglemod(ideal_angle[PITCH]);


	// Yaw
	if (current_yaw != ideal_yaw)
	{
		move = ideal_yaw - current_yaw;
		speed = ent->yaw_speed;
		if (ideal_yaw > current_yaw)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		if (move > 0)
		{
			if (move > speed)
				move = speed;
		}
		else
		{
			if (move < -speed)
				move = -speed;
		}
		ent->s.angles[YAW] = anglemod (current_yaw + move);
	}


	// Pitch
	if (current_pitch != ideal_pitch)
	{
		move = ideal_pitch - current_pitch;
		speed = ent->yaw_speed;
		if (ideal_pitch > current_pitch)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		if (move > 0)
		{
			if (move > speed)
				move = speed;
		}
		else
		{
			if (move < -speed)
				move = -speed;
		}
		ent->s.angles[PITCH] = anglemod (current_pitch + move);
	}
}



//==========================================
// AI_MoveToGoalEntity
// Set bot to move to it's movetarget. Short range goals
//==========================================
qboolean AI_MoveToGoalEntity(edict_t *self, usercmd_t *ucmd)
{
	if (!self->movetarget || !self->client)
		return false;



	// If a rocket or grenade is around deal with it
	// Simple, but effective (could be rewritten to be more accurate)
	if(self->movetarget->classnameb == ROCKET ||
	//   self->movetarget->classnameb,"grenade") ||
	   self->movetarget->classnameb == HGRENADE)
	{
		VectorSubtract (self->movetarget->s.origin, self->s.origin, self->ai->move_vector);
		AI_ChangeAngle(self);
//		if(AIDevel.debugChased && bot_showcombat->value)
//			G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: Oh crap a rocket!\n",self->ai->pers.netname);

		// strafe left/right
		if(rand()%1 && AI_CanMove(self, BOT_MOVE_LEFT))
				ucmd->sidemove = -400;
		else if(AI_CanMove(self, BOT_MOVE_RIGHT))
				ucmd->sidemove = 400;
		return true;

	}

	// Set bot's movement direction
	VectorSubtract (self->movetarget->s.origin, self->s.origin, self->ai->move_vector);
	AI_ChangeAngle(self);
	if(!AI_CanMove(self, BOT_MOVE_FORWARD) ) 
	{
		self->movetarget = NULL;
		ucmd->forwardmove = -100;
		return false;
	}

	//Move
	ucmd->forwardmove = 400;
	return true;
}



void AI_MoveToCampSpot(edict_t *self, usercmd_t *ucmd)
{
	vec3_t v;


	if (self->ai->camp_targ < 0 || self->ai->state != BOT_STATE_CAMP || self->enemy)
		return;



	VectorSubtract (camp_spots[self->ai->camp_targ].origin, self->s.origin, v);

	//gi.dprintf("%f \n",VectorLength (v));


	self->ideal_yaw = camp_spots[self->ai->camp_targ].angle;


	if (VectorLength (v) < 10)//50
	{
		self->client->aim = true;

		self->s.angles[YAW] = camp_spots[self->ai->camp_targ].angle;
		self->s.angles[PITCH] = 0;

		self->ai->bloqued_timeout = level.time + 10.0;




		//camping, look back and forth every once in a while
		if (self->ai->state == BOT_STATE_CAMP && self->ai->last_enemy_time < level.time - 3)
		{
			int num;
			num = ((int)self->s.origin[1] +  (level.framenum - self->client->forcespawn))%100;

			if (num <10)
				self->s.angles[YAW]+=45;
			else if (num <35 && num >25)
				self->s.angles[YAW]-=45;
			

		}


		return;

	}

	//
	

	//bot is near camp_spot, but can't get to the exact spot
	if (self->ai->actual_camp_start < level.time - 5 &&
		(!self->client->last_fire_time ||
		self->client->last_fire_time < level.time - 10))
	{
		int camp_targ_save;

		//gi.dprintf ("HELLO\n");
		camp_targ_save = self->ai->camp_targ;
		self->ai->camp_targ = -1;
		AI_PickLongRangeGoal(self);
		if (self->ai->camp_targ != camp_targ_save)
			camp_spots[camp_targ_save].owner = NULL;
		if (self->oldstance == self->stanceflags)
			change_stance(self, STANCE_STAND);
		self->client->aim = false;
		self->ai->actual_camp_start = 0;
		return;
	}




	// Set bot's movement direction
	VectorSubtract (camp_spots[self->ai->camp_targ].origin, self->s.origin, self->ai->move_vector);
	AI_ChangeAngle(self);
	if(!AI_CanMove(self, BOT_MOVE_FORWARD) ) 
	{
		self->movetarget = NULL;
		ucmd->forwardmove = -100;
		return;
	}

	//Move
	ucmd->forwardmove = 400;
	return;
}




void AI_MoveToDodgeSpot(edict_t *self, usercmd_t *ucmd)
{
	vec3_t v;

	self->client->aim = false;


	VectorSubtract (nodes[self->ai->dodge_node].origin, self->s.origin, v);

	//gi.dprintf("ewfdsfds\n");



	if (VectorLength (v) < 30)//50
	{
		//self->client->aim = true;

		//self->s.angles[YAW] = camp_spots[self->ai->camp_targ].angle;
		//self->s.angles[PITCH] = 0;
		return;

	}


	// Set bot's movement direction
	VectorSubtract (nodes[self->ai->dodge_node].origin, self->s.origin, self->ai->move_vector);
	AI_ChangeAngle(self);
	if(!AI_CanMove(self, BOT_MOVE_FORWARD) ) 
	{
		self->movetarget = NULL;
		ucmd->forwardmove = -100;
		return;
	}

	//Move
	ucmd->forwardmove = 400;
	return;
}
