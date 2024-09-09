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

*/

#include "../g_local.h"
#include "ai_local.h"

//ACE

qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
qboolean BOT_DMclass_FindEnemy(edict_t *self);
void BOT_DMclass_CombatMovement( edict_t *self, usercmd_t *ucmd );
void BOT_DMclass_Wander(edict_t *self, usercmd_t *ucmd);
void BOT_DMclass_Move(edict_t *self, usercmd_t *ucmd);

//==========================================
// M_default_Move
// movement following paths code
//==========================================
//void M_default_Move( edict_t *self, usercmd_t *ucmd )
//{
//	BOT_DMclass_Move( self, ucmd );
//}
void M_default_Move(edict_t *self, usercmd_t *ucmd)
{
	int current_node_flags = 0;
	int next_node_flags = 0;
	int	current_link_type = 0;
//	int i;

	current_node_flags = nodes[self->ai->current_node].flags;
	next_node_flags = nodes[self->ai->next_node].flags;
	if( AI_PlinkExists( self->ai->current_node, self->ai->next_node ))
	{
		current_link_type = AI_PlinkMoveType( self->ai->current_node, self->ai->next_node );
		//Com_Printf("%s\n", AI_LinkString( current_link_type ));
	}

	// Falling off ledge
	if(!self->groundentity && !self->is_step && !self->is_swim )
	{
		AI_ChangeAngle(self);
		if (current_link_type == LINK_JUMPPAD ) {
			ucmd->forwardmove = 100;
		} else if( current_link_type == LINK_JUMP ) {
			self->velocity[0] = self->ai->move_vector[0] * 280;
			self->velocity[1] = self->ai->move_vector[1] * 280;
		} else {
			self->velocity[0] = self->ai->move_vector[0] * 160;
			self->velocity[1] = self->ai->move_vector[1] * 160;
		}
		return;
	}


	// swimming
	if( self->is_swim )
	{
		// We need to be pointed up/down
		AI_ChangeAngle(self);

		//if( !(trap_PointContents(nodes[self->ai->next_node].origin) & MASK_WATER) ) // Exit water
		if( !(gi.pointcontents(nodes[self->ai->next_node].origin) & MASK_WATER) ) // Exit water
			ucmd->upmove = 400;
		
		ucmd->forwardmove = 300;
		return;
	}

	// Check to see if stuck, and if so try to free us
	if(VectorCompare(self->s.old_origin, self->s.origin))
	{
		// Keep a random factor just in case....
		if( random() > 0.1 && AI_SpecialMove(self, ucmd) ) //jumps, crouches, turns...
			return;

		self->s.angles[YAW] += random() * 180 - 90;

		AI_ChangeAngle(self);

		ucmd->forwardmove = 400;

		return;
	}

	AI_ChangeAngle(self);

	// Otherwise move as fast as we can... 
	ucmd->forwardmove = 400;
}

//==========================================
// M_default_CombatMovement
// movement while in state combat
//==========================================
void M_default_CombatMovement( edict_t *self, usercmd_t *ucmd )
{
	BOT_DMclass_CombatMovement( self, ucmd );
}

//==========================================
// M_default_Wander
// movement when couldn't find a goal
//==========================================
void M_default_Wander( edict_t *self, usercmd_t *ucmd )
{
	BOT_DMclass_Wander( self, ucmd );
}

//==========================================
// M_default_FindEnemy
//==========================================
qboolean M_default_FindEnemy(edict_t *self)
{
	return BOT_DMclass_FindEnemy(self);
}

//==========================================
// M_default_ChooseWeapon
// Choose weapon based on range & weights
//==========================================
void M_default_ChooseWeapon(edict_t *self)
{
}

//==========================================
// M_default_pain
//==========================================
void M_default_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}


//==========================================
// M_default_DeadFrame
// AI passes through here instead of M_default_RunFrame
// when self->deadflag is set up. The default monster
// explodes and frees itself, so it's not happening now.
//==========================================
void M_default_DeadFrame (edict_t *self)
{
}

//==========================================
// M_default_die
//==========================================
void M_default_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//if(AIDevel.debugMode && bot_debugmonster->value)
	Com_Printf("AI_monster: Die\n");


	//throw gibs
	//G_Sound (self, CHAN_BODY, trap_SoundIndex ("sound/misc/udeath.wav"), 1, ATTN_NORM);
	//ThrowSmallPileOfGibs ( self, 8, damage );
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	
	if (self->item)
	{
		Drop_Item (self, self->item);
		self->item = NULL;
	}

	AI_EnemyRemoved( self );

	//explode
	barrel_delay (self, inflictor, attacker, damage, point);
}

//==========================================
// M_default_BloquedTimeout
//==========================================
void M_default_BloquedTimeout( edict_t *self )
{
if (!level.intermissiontime)
	M_default_die (self, self, self, 10000, self->s.origin);
}

//==========================================
// M_default_CheckShot
// Checks if shot is blocked or if too far to shoot
//==========================================
qboolean M_default_CheckShot(edict_t *self, vec3_t	point)
{
	trace_t tr;

	//bloqued, don't shoot
	tr = gi.trace( self->s.origin, vec3_origin, vec3_origin, point, self, MASK_AISOLID);
	//trap_Trace( &tr, self->s.origin, vec3_origin, vec3_origin, point, self, MASK_AISOLID);
	if (tr.fraction < 0.3) //just enough to prevent self damage (by now)
		return false;

	return true;
}

//==========================================
// M_default_FireWeapon
// Fire if needed
//==========================================
void M_default_FireWeapon (edict_t *self)
{
	float	firedelay;
	vec3_t  target;
	vec3_t  angles;


	if (!self->enemy)
		return;
	
	// Aim
	VectorCopy(self->enemy->s.origin,target);


	// modify attack angles based on accuracy (mess this up to make the bot's aim not so deadly)
	target[0] += (random()-0.5) * ((MAX_BOT_SKILL - self->ai->pers.skillLevel) *2);
	target[1] += (random()-0.5) * ((MAX_BOT_SKILL - self->ai->pers.skillLevel) *2);

	// Set direction
	VectorSubtract (target, self->s.origin, self->ai->move_vector);
	vectoangles (self->ai->move_vector, angles);
	VectorCopy(angles,self->s.angles);


	// Set the attack 
	firedelay = random()*(MAX_BOT_SKILL*1.8);
	if (firedelay > (MAX_BOT_SKILL - self->ai->pers.skillLevel) && M_default_CheckShot(self, target))
	{
		vec3_t	start, forward, right;
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(15,15,0), forward, right, start);
		//monster_fire_bullet (self, start, forward, 4, 4, MZ2_INFANTRY_MACHINEGUN_2);
		monster_fire_bullet (self, start, forward, 4, 4, rand(), rand(), MZ2_INFANTRY_MACHINEGUN_2);
	}	

	//if(AIDevel.debugMode && bot_debugmonster->integer)
	//	G_PrintMsg (NULL, PRINT_HIGH, "monster: attacking\n");
}


//==========================================
// M_default_WeightPlayers
// weight players based on game state
//==========================================
void M_default_WeightPlayers(edict_t *self)
{
	int i;

	//clear
//	memset(self->ai->status.playersWeights, 0, sizeof (self->ai->status.playersWeights));

	for(i=0;i<num_AIEnemies;i++)
	{
		if( AIEnemies[i] == NULL)
			continue;

		if( AIEnemies[i] == self)
			continue;

		if( !strcmp(AIEnemies[i]->classname, "monster") ) {
			self->ai->status.playersWeights[i] = 0.0f;
			continue;
		}

		//ignore spectators and dead players
		if( AIEnemies[i]->svflags & SVF_NOCLIENT || AIEnemies[i]->deadflag) {
			self->ai->status.playersWeights[i] = 0.0f;
			continue;
		}

		//every player has some value
		self->ai->status.playersWeights[i] = 0.3;
	}
}


//==========================================
// M_default_WeightInventory
// monsters can't pick up items, so zero all
//==========================================
void M_default_WeightInventory(edict_t *self)
{
	//reset with persistant values
	memcpy(self->ai->status.inventoryWeights, self->ai->pers.inventoryWeights, sizeof(self->ai->pers.inventoryWeights));
}

//==========================================
// M_default_UpdateStatus
// Set up status for ai-> Happens before M_default_RunFrame
//==========================================
void M_default_UpdateStatus( edict_t *self )
{
	self->enemy = NULL;
	self->movetarget = NULL;

	//set up AI status for the upcoming AI_frame
	M_default_WeightInventory(self);
	M_default_WeightPlayers(self);
}

void M_Default_SetUpAnimMoveFlags( edict_t *ent, usercmd_t *ucmd )
{
/*
	int			xyspeedcheck;
	vec3_t		velocity;
	pmanim_t	*pmAnim;
	pmAnim = &ent->pmAnim;
	
	//Update anim movement flags for monsters with ppms.
	if(ent->svflags & SVF_FAKECLIENT)
		return;
	
	//----------------------------------------
	//splitmodels jal[start]
	VectorSubtract( ent->s.origin, ent->s.old_origin, velocity );
	xyspeedcheck = sqrt(velocity[0]*velocity[0] + velocity[1]*velocity[1]);
	
	pmAnim->anim_moveflags = 0;//start from 0
	
	if (ucmd->forwardmove < -1)
		pmAnim->anim_moveflags |= ANIMMOVE_BACK;
	else if (ucmd->forwardmove > 1)
		pmAnim->anim_moveflags |= ANIMMOVE_FRONT;
	
	if (ucmd->sidemove < -1)
		pmAnim->anim_moveflags |= ANIMMOVE_LEFT;
	else if (ucmd->sidemove > 1)
		pmAnim->anim_moveflags |= ANIMMOVE_RIGHT;
	
	if (xyspeedcheck > 160)
		pmAnim->anim_moveflags |= ANIMMOVE_RUN;
	else if (xyspeedcheck)
		pmAnim->anim_moveflags |= ANIMMOVE_WALK;
	
	//if (client->ps.pmove.pm_flags & PMF_DUCKED)
	//	client->pmAnim.anim_moveflags |= ANIMMOVE_DUCK;
*/
}



//==========================================
// AI_GravityBoxStep
// move the box one step for walk movetype:
//
// This is the LINK determination function
// quickly plugged in here as a very simple
// demonstration. It quite does the job, BUT
// gravity isn't handled (the monster is 
// instantly dropped to floor).
// By fixing dropping to floor it isn't that bad
//==========================================
int M_default_GravityBoxStep( vec3_t origin, float scale, vec3_t movedir, vec3_t neworigin, vec3_t mins, vec3_t maxs, int solidmask, edict_t *passent )
{
	trace_t	trace;
	vec3_t	v1, v2, forward, /*up,*/ angles/*, movedir*/;
	int		movemask = 0;
	int		eternalfall = 0;
	float	/*xzdist,*/ xzscale;
	float	/*ydist,*/ yscale;
//	float	dist;

	//trap_Trace( &trace, origin, mins, maxs, origin, passent, solidmask );
	trace = gi.trace( origin, mins, maxs, origin, passent, solidmask );
	if( trace.startsolid )
		return LINK_INVALID;
	
//	VectorSubtract( destvec, origin, movedir);
	VectorNormalize( movedir );
	vectoangles( movedir, angles );
	
	xzscale = scale;
	yscale = scale;
/*
	//remaining distance in planes
	if( scale < 1 )
		scale = 1;

	dist = AI_Distance( origin, destvec );
	if( scale > dist )
		scale = dist;
	
	xzscale = scale;
	xzdist = AI_Distance( tv(origin[0], origin[1], destvec[2]), destvec );
	if( xzscale > xzdist )
		xzscale = xzdist;
	
	yscale = scale;
	ydist = AI_Distance( tv(0,0,origin[2]), tv(0,0,destvec[2]) );
	if( yscale > ydist )
		yscale = ydist;
	
	
	//float move step
	if( trap_PointContents( origin ) & MASK_WATER )
	{
		angles[ROLL] = 0;
		AngleVectors( angles, forward, NULL, up );

		
		VectorMA( origin, scale, movedir, neworigin );
		trap_Trace( &trace, origin, mins, maxs, neworigin, passent, solidmask);
		if( trace.startsolid || trace.fraction < 1.0 ) 
			VectorCopy( origin, neworigin );	//update if valid
		
		if( VectorCompare(origin, neworigin) )
			return LINK_INVALID;
		
		if( trap_PointContents( neworigin ) & MASK_WATER )
			return LINK_WATER;
		
		//jal: Actually GravityBox can't leave water.
		//return INVALID and WATERJUMP, so it can validate the rest outside
		return (LINK_INVALID|LINK_WATERJUMP);
	}
*/
	//gravity step

	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AngleVectors( angles, forward, NULL, NULL );
	VectorNormalize( forward );

	// try moving forward
	VectorMA( origin, xzscale, forward, neworigin );
	//trap_Trace( &trace, origin, mins, maxs, neworigin, passent, solidmask );
	trace = gi.trace( origin, mins, maxs, neworigin, passent, solidmask );
	if( trace.fraction == 1.0 ) //moved
	{	
		movemask |= LINK_MOVE;
		goto droptofloor;

	} else {	//bloqued, try stepping up

		VectorCopy( origin, v1 );
		VectorMA( v1, xzscale, forward, v2 );
		for( ; v1[2] < origin[2] + AI_JUMPABLE_HEIGHT; v1[2] += scale, v2[2] += scale )
		{
			//trap_Trace( &trace, v1, mins, maxs, v2, passent, solidmask );
			trace = gi.trace( v1, mins, maxs, v2, passent, solidmask );
			if( !trace.startsolid && trace.fraction == 1.0 )
			{
				VectorCopy( v2, neworigin );
				if( origin[2] + AI_STEPSIZE > v2[2] )
					movemask |= LINK_STAIRS;
				else
					movemask |= LINK_JUMP;

				goto droptofloor;
			}
		}

/*		//still failed, try slide move
		VectorMA( origin, xzscale, forward, neworigin );
		trap_Trace( &trace, origin, mins, maxs, neworigin, passent, solidmask );
		if( trace.plane.normal[2] < 0.5 && trace.plane.normal[2] >= -0.4 )
		{
			VectorCopy( trace.endpos, neworigin );
			VectorCopy( trace.plane.normal, v1 );
			v1[2] = 0;
			VectorNormalize( v1 );
			VectorMA( neworigin, xzscale, v1, neworigin );
			
			//if new position is closer to destiny, might be valid
			if( AI_Distance( origin, destvec ) > AI_Distance( neworigin, destvec ) )
			{
				trap_Trace( &trace, trace.endpos, mins, maxs, neworigin, passent, solidmask );
				if( !trace.startsolid && trace.fraction == 1.0 )
					goto droptofloor;
			}
		}*/

		VectorCopy( origin, neworigin );
		return LINK_INVALID;
	}

droptofloor:

	while(eternalfall < 20000000) 
	{
		//if( trap_PointContents(neworigin) & MASK_WATER ) {
		if( gi.pointcontents(neworigin) & MASK_WATER ) {

			if( origin[2] > neworigin[2] + AI_JUMPABLE_HEIGHT )
				movemask |= LINK_FALL;
			else if ( origin[2] > neworigin[2] + AI_STEPSIZE )
				movemask |= LINK_STAIRS;

			return movemask;
		}

		//trap_Trace( &trace, neworigin, mins, maxs, tv(neworigin[0], neworigin[1], (neworigin[2] - AI_STEPSIZE)), passent, solidmask );
		trace = gi.trace( neworigin, mins, maxs, tv(neworigin[0], neworigin[1], (neworigin[2] - AI_STEPSIZE)), passent, solidmask );
		if( trace.startsolid )
		{
			return LINK_INVALID;
		}

		VectorCopy( trace.endpos, neworigin );
		if( trace.fraction < 1.0 )
		{
			if( origin[2] > neworigin[2] + AI_JUMPABLE_HEIGHT )
				movemask |= LINK_FALL;
			else if ( origin[2] > neworigin[2] + AI_STEPSIZE )
				movemask |= LINK_STAIRS;

			if( VectorCompare( origin, neworigin ) )
				return LINK_INVALID;

			return movemask;
		}

		eternalfall++;
	}

	//G_Error ("ETERNAL FALL\n");
	return 0;
}

qboolean M_default_movestep (edict_t *self, usercmd_t *ucmd)
{
	vec3_t	movedir;
	vec3_t	neworigin;
	int		movetype;
	float	speed = 0;

	AngleVectors( tv( 0, self->s.angles[YAW], 0), movedir, NULL, NULL);

	if( ucmd->forwardmove > 200 )
		speed = 20;
	else if( ucmd->forwardmove > 10 )
		speed = 5;
	//else if( ucmd->forwardmove < -200 )
		//speed = -20;
	//else if( ucmd->forwardmove < -10 )
		//speed = -5;
	else {
		//speed is 0
		VectorCopy( self->s.origin, self->s.old_origin );
		//relink
		gi.linkentity(self);//trap_LinkEntity (self);
		G_TouchTriggers (self);
		return true;
	}


	VectorCopy( self->s.origin, self->s.old_origin );

	//VectorCopy( self->ai->move_vector, movedir );
	VectorNormalize( movedir );

	movetype = M_default_GravityBoxStep( self->s.origin, speed, movedir, neworigin, self->mins, self->maxs, MASK_AISOLID, self );
	if( movetype & LINK_INVALID )
	{
		return false;
	} else {
		VectorCopy( neworigin, self->s.origin );

		//store velocity for dmclass move code checks
		VectorSubtract(self->s.origin, self->s.old_origin, self->velocity);

		//relink
		gi.linkentity(self);//trap_LinkEntity (self);
		G_TouchTriggers (self);
		return true;
	}

	return false;
}


//void G_SetPModelFrame (edict_t *ent);
void M_WorldEffects (edict_t *ent);
//==========================================
// M_default_RunFrame
//
// States Machine & call client movement
//==========================================
void M_default_RunFrame( edict_t *self )
{
	usercmd_t	ucmd;
	memset( &ucmd, 0, sizeof(ucmd) );

	// Look for enemies
	if( M_default_FindEnemy(self) )
	{
		M_default_ChooseWeapon( self );
		M_default_FireWeapon( self );
		self->ai->state = BOT_STATE_ATTACK;
		self->ai->state_combat_timeout = level.time + 1.0;
	
	} else if( self->ai->state == BOT_STATE_ATTACK && 
		level.time > self->ai->state_combat_timeout)
	{
		//Jalfixme: change to: AI_SetUpStateMove(self);
		self->ai->state = BOT_STATE_MOVE;
	}

	// Execute the move, or wander
	if( self->ai->state == BOT_STATE_MOVE )
		M_default_Move( self, &ucmd );

	else if(self->ai->state == BOT_STATE_ATTACK)
		M_default_CombatMovement( self, &ucmd );

	else if ( self->ai->state == BOT_STATE_WANDER )
		M_default_Wander( self, &ucmd );


	//move a step
	if( M_default_movestep ( self, &ucmd ) )
		self->ai->bloqued_timeout = level.time + 10.0;

	M_WorldEffects (self);

	//MAKEME: NEEDS TO ADD MODEL ANIMATIONS (jal: I won't do it)
	//M_Default_SetUpAnimMoveFlags( self, &ucmd );
	//G_SetPModelFrame ( self );
	
	self->nextthink = level.time + FRAMETIME;
}


//==========================================
// M_default_InitPersistant
// Persistant after respawns.
//==========================================
void M_default_InitPersistant(edict_t *self)
{
	//set 'class' functions
	self->ai->pers.RunFrame = M_default_RunFrame;
	self->ai->pers.UpdateStatus = M_default_UpdateStatus;
	self->ai->pers.bloquedTimeout = M_default_BloquedTimeout;
	self->ai->pers.deadFrame = M_default_DeadFrame;

	//skill
	self->ai->pers.skillLevel = (int)(random()*MAX_BOT_SKILL);
	if (self->ai->pers.skillLevel > MAX_BOT_SKILL)	//fix if off-limits
		self->ai->pers.skillLevel =  MAX_BOT_SKILL;
	else if (self->ai->pers.skillLevel < 0)
		self->ai->pers.skillLevel =  0;

	//available moveTypes for this class
	self->ai->pers.moveTypesMask = (LINK_MOVE|LINK_STAIRS|LINK_WATER|LINK_WATERJUMP);

	//Persistant Inventory Weights (0 = can not pick)
	memset(self->ai->pers.inventoryWeights, 0, sizeof (self->ai->pers.inventoryWeights));
}


//==========================================
// M_default_Spawn
// 
//==========================================

void M_default_Start( edict_t *self )
{
	self->health = 30;
	self->max_health = self->health;
	self->item = FindItemByClassname("ammo_bullets");


	self->think = AI_Think;
	self->nextthink = level.time + FRAMETIME;
	self->yaw_speed = 100;
	M_default_InitPersistant(self);
	AI_ResetNavigation(self);

	//add as bot enemy
	AI_EnemyAdded( self );

	//if(AIDevel.debugMode && bot_debugmonster->integer)
		Com_Printf ("monster: Spawn\n");
}


void M_default_Spawn (void)
{
	edict_t	*ent;
	vec3_t	spawn_origin, spawn_angles;//spawn at a spawnpoint
	vec3_t	mins = {-15, -15, -24};
	vec3_t	maxs = {15, 15, 32};

	ent = G_Spawn();

	G_SpawnAI (ent); //jabot092(2)

	//spawn at a spawnpoint
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	spawn_origin[2] += 8;

	//-------------------------------------------------

	// clear entity values
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;

	ent->classname = "monster";

	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_MONSTERSOLID;
	//ent->model = "models/monsters/infantry/tris.md2";//jalfixme
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;

	ent->pain = M_default_pain;
	ent->die = M_default_die;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);
	

	ent->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");


	// clear entity state values
	ent->s.effects = 0;
	ent->s.frame = 0;

	VectorCopy (spawn_origin, ent->s.origin);
	VectorCopy (ent->s.origin, ent->s.old_origin);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;

	if (!KillBox (ent))
	{	// could't spawn in?
	}
	gi.linkentity (ent);

	//finish
	M_default_Start(ent);
}


