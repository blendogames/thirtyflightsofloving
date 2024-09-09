/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_observer.c,v $
 *   $Revision: 1.20 $
 *   $Date: 2002/07/25 08:28:43 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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
#include "p_menus.h"
void Cmd_Objectives (edict_t *ent);
void M_ChooseMOS(edict_t *ent);
void Killed(edict_t * targ , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point ); 
//this file is for all the stuff that relates to observer mode, particularly during the begingg
//of the game when players are joining teams and setting mos.


//faf:  returns a count for # of players on team.  using this to fix
//		bugs with auto-join team and to fix reporting team counts incorrectly
int PlayerCountForTeam (int team_number, qboolean countbots)
{
	int i;
	int playercount = 0;
    edict_t *check_ent;

        
	for (i = 1; i <= maxclients->value; i++)
    {
         check_ent = g_edicts + i;
         if (!check_ent->inuse)
			 continue;
		 if (!check_ent->client ||
			 !check_ent->client->resp.team_on)
			 continue;

		 if (countbots == false && check_ent->ai)
			 continue;

		 if (check_ent->client->resp.team_on->index == team_number)
			 playercount++;

	}
	return playercount;
                              
}



// clients will use these

void SwitchToObserver(edict_t *ent)
{

// start as observer 
	ent->movetype = MOVETYPE_NOCLIP; 
	ent->solid = SOLID_NOT; 
	ent->svflags |= SVF_NOCLIENT; 
	ent->client->ps.gunindex = 0; 
	//ent->client->pers.weapon=NULL;
	gi.linkentity (ent); 
	ent->client->limbo_mode=true;
//	ent->client->deathfade = 0;


	if (!ent->client->display_info && ent->client->layout_type != SHOW_CAMPAIGN)
	{
		if(team_list[0])
			MainMenu(ent);
	}
}





//faf    parachutes :P
void Chute_Think(edict_t *ent)
{
	if (level.intermissiontime || ent->owner->deadflag)
	{
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + .1;
		return;
	}


	//done with it
	if (ent->s.frame == 10)
	{
		if (ent->owner->client)
			ent->owner->client->landed = true;

		ent->movetype = MOVETYPE_NONE;
		ent->gravity = 0;
		ent->nextthink = level.time + 2;
		ent->think = G_FreeEdict;
		return;
	}

	ent->nextthink = level.time + .1;
//	gi.dprintf("%f\n",ent->owner->client->jump_stamina);
	//we've touched the ground
	if (ent->owner->client && 
		ent->owner->groundentity || ent->owner->velocity[2] > 0 ||
		ent->owner->client->jump_stamina < 80 ||
		ent->s.frame > 5)
//		ent->owner->client->ps.pmove.gravity == sv_gravity->value)//landed
	{
		if (ent->s.frame < 10)  //start parachute falling
		{
			ent->owner->client->ps.pmove.gravity = sv_gravity->value;
			//ent->owner->client->landed = true;
			ent->s.sound = 0;
			ent->s.frame++;
			if (ent->owner->velocity[2] <= 0 ||  // keeps chute from floating when landing
				ent->s.frame < 3)
				ent->s.origin[2] = ent->owner->s.origin[2];
			return;
		}
	}
	else //not on ground
		if (ent->owner->velocity[2] > 0) //just jumped
	{
//			ent->s.frame = 0;
		ent->s.origin[0] = ent->owner->s.origin[0];
		ent->s.origin[1] = ent->owner->s.origin[1];
		ent->gravity = 0;

		ent->s.sound = gi.soundindex("faf/flag.wav");
		ent->owner->client->landed = false;
		ent->owner->client->ps.pmove.gravity = .25 * (sv_gravity->value) ; //parchute factor

	}
	else
	{
		ent->s.frame = 0;
		VectorCopy (ent->owner->s.origin, ent->s.origin);
		ent->gravity = .25;
		ent->s.sound = gi.soundindex("faf/flag.wav");
		ent->movetype = MOVETYPE_TOSS;
		ent->owner->client->landed = false;

	}

	if (ent->velocity[2] <-500)
		ent->velocity [2] = -500;
/*	
	if (ent->s.frame < 10 && 
		!ent->owner->groundentity)
	{
		ent->s.frame = 0;// restart the chute
		VectorCopy (ent->owner->s.origin, ent->s.origin);
		ent->movetype = MOVETYPE_TOSS;
		ent->s.sound = gi.soundindex("faf/flag.wav");

	} */
}


	


void Spawn_Chute(edict_t *ent)
{
	//faf
	vec3_t	start;
	vec3_t	end;
	vec3_t world_up, down;

	trace_t	tr;

	edict_t *chute;


	ent->client->landed = true;

	VectorCopy(ent->s.origin, start);
	VectorSet(world_up, 0, 0, 1);
	VectorMA(start, 8192, world_up, end);
		
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.surface && !(tr.surface->flags & SURF_SKY))  //under a roof
	{
		return;
	}

	VectorSet(down, 0, 0, -1);
	VectorMA(start, 100, down, end);
		
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.fraction < 1.0 )  //not high off ground
	{
		return;
	}

	ent->client->landed = false;


	chute = G_Spawn();
	chute->movetype = MOVETYPE_TOSS;
	chute->solid = SOLID_TRIGGER;
	chute->gravity = .25;
	chute->s.modelindex = gi.modelindex ("models/objects/chute/tris.md2");
	chute->s.sound = gi.soundindex("faf/flag.wav");

	chute->think = Chute_Think;
	chute->nextthink = level.time + .1;
	chute->owner = ent;
	
	chute->clipmask = MASK_SHOT;
	
	VectorClear (chute->mins);
	VectorClear (chute->maxs);
	
	chute->classname = "chute";

	VectorCopy (ent->s.origin, chute->s.origin);


}




void Spawn_Chute_Special(edict_t *ent)
{
	//faf
	vec3_t	start;
	vec3_t	end;
	vec3_t world_up, down;

	trace_t	tr;

	edict_t *chute;

	if (ent->client->has_chute == false)
		return;


	ent->client->landed = true;

	VectorCopy(ent->s.origin, start);
	VectorSet(world_up, 0, 0, 1);
	VectorMA(start, 8192, world_up, end);
		
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

//	if ( tr.surface && !(tr.surface->flags & SURF_SKY))  //under a roof
//	{
//		return;
//	}

	VectorSet(down, 0, 0, -1);
	VectorMA(start, 100, down, end);
		
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.fraction < 1.0 )  //not high off ground
	{
		return;
	}

	ent->client->landed = false;
	ent->client->has_chute = false;


	chute = G_Spawn();
	chute->movetype = MOVETYPE_TOSS;
	chute->solid = SOLID_TRIGGER;
	chute->gravity = .25;
	chute->s.modelindex = gi.modelindex ("models/objects/chute/tris.md2");
	chute->s.sound = gi.soundindex("faf/flag.wav");

	chute->think = Chute_Think;
	chute->nextthink = level.time + .1;
	chute->owner = ent;
	
	chute->clipmask = MASK_SHOT;
	
	VectorClear (chute->mins);
	VectorClear (chute->maxs);
	
	chute->classname = "chute";

	VectorCopy (ent->s.origin, chute->s.origin);


}






//this function exits observer mode, presumably after they have chosen mos. They must have
//joined a team, if one is avaiable...

void Find_Mission_Start_Point(edict_t *ent, vec3_t origin, vec3_t angles);

void EndObserverMode(edict_t* ent) 
{ 
	vec3_t	spawn_origin, spawn_angles;

	if (!ent->client->limbo_mode) 
		return;

	if (ent->leave_limbo_time > level.time)
		return;


	if( !team_list[0] || !ent->client->resp.team_on)
	{
//		safe_cprintf(ent,PRINT_HIGH,"You must join a team first!\n");
		return;
	}

	if (!(ent->svflags & SVF_NOCLIENT)) 
		return; // not in observer mode
/*
	//if they are the first on team, make em captain
	if( ent->client->resp.team_on->units[0]==ent && deathmatch->value) 
	{
		if (ent->client->resp.mos != OFFICER)
			safe_centerprintf(ent, "You have been promoted to Officer!\n");

		ent->client->resp.team_on->officer_mos=ent->client->resp.mos;
		//store the new officer's old backup mos
		ent->client->resp.bkupmos=ent->client->resp.mos;
		ent->client->resp.mos=OFFICER;
		DoEndOM(ent);
	}
	*/

	DoEndOM(ent);

	//ok put the player where he's supposed to be
	ent->client->spawntime = level.time;
	Find_Mission_Start_Point(ent, spawn_origin, spawn_angles);

	// unlink to make sure it can't possibly interfere with KillBox	
	gi.unlinkentity (ent);
	
	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);

	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_LAND; // pbowens: changed from PMF_TIME_TELEPORT, no particles 
	ent->client->limbo_mode=false;

//	VectorClear (ent->s.angles);
//	VectorClear (ent->client->ps.viewangles);
//	VectorClear (ent->client->v_angle);

	gi.linkentity (ent);

	ent->client->resp.AlreadySpawned=true;
	//safe_bprintf (PRINT_HIGH, "%s has entered the battle.\n", ent->client->pers.netname); 
	WeighPlayer(ent);
//moved up	ent->client->spawntime = level.time;

/*move to spawn_chute
if (ent->client->resp.mos == SPECIAL)
		ent->client->landed = false;
	else
		ent->client->landed = true;
*/
ent->client->landed = true;
	//faf
/*requires model*/
	if (ent->client->resp.mos == SPECIAL)
	{
		ent->client->has_chute = true;

		Spawn_Chute(ent);
	}


	Remove_Nearby_Sandbags(ent);



}

void sandbag_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Remove_Nearby_Sandbags(edict_t *ent)
{
	vec3_t vdist;
	float tdist;
	int i;
	edict_t *e;


	for (i=0 ; i<game.maxentities ; i++)
	{
		e = &g_edicts[i];

		if (!e->inuse)
			continue;
		if (!e->classnameb)
			continue;
		if (e->classnameb != SANDBAGS)
			continue;


		VectorSubtract (e->s.origin, ent->s.origin, vdist);

		tdist = VectorLength (vdist);
		
		if (tdist > 64) 
			continue;


		//gi.dprintf("djksflsdfjklsdfjkl\n");

		sandbag_die (e, ent, ent, 99999999, e->s.origin);

	}
}


qboolean OpenSpot (edict_t *ent, mos_t class)
{
//	int index;
	int spots, taken, j;
	TeamS_t *team;
	edict_t *cl_ent;

	team=ent->client->resp.team_on;

	if (class_limits->value == 0) // class limits turned off
	{
		team->mos[class]->available = 99;
		return true;
	}

/*	for (taken = 0, index = 0; index < MAX_TEAM_MATES; index++)
	{
		if (!team->units[index]) 
			continue;
		
		if (team->units[index]->client->resp.mos == class) 
		{
			// if you're already that class, leave a spot for yourself
			//if (team->units[index] == ent)
			//	continue;

			 taken++;
		}
	} */

	//faf: hopefully fixes class bugs
	taken = 0;

	for (j=0 ; j < game.maxclients ; j++)
	{
		cl_ent = g_edicts + 1 + j;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->ai)
			continue;
		if (!cl_ent->client || 
			!cl_ent->client->resp.team_on ||
			!cl_ent->client->resp.team_on->mos ||
			cl_ent->client->resp.team_on->index != ent->client->resp.team_on->index)
			continue; 
			if (class ==
			cl_ent->client->resp.mos)
			taken++;
	}




	// Not-so-good way of doing things, but it gets the job done
	switch (class)
	{
	case INFANTRY:
		spots							= MAX_INFANTRY;
		team->mos[INFANTRY]->available	= MAX_INFANTRY	- taken;
		break;
	case OFFICER:
		spots							= MAX_OFFICERS;
		team->mos[OFFICER]->available	= MAX_OFFICERS	- taken;
		break;
	case L_GUNNER:
		spots							= MAX_L_GUNNER;
		team->mos[L_GUNNER]->available	= MAX_L_GUNNER	- taken;
		break;
	case H_GUNNER:
		spots							= MAX_H_GUNNER;			
		team->mos[H_GUNNER]->available	= MAX_H_GUNNER	- taken;
		break;
	case SNIPER:
		spots							= MAX_SNIPER;
		team->mos[SNIPER]->available	= MAX_SNIPER	- taken;
		break;
	case SPECIAL:
		spots							= MAX_SPECIAL;
		team->mos[SPECIAL]->available	= MAX_SPECIAL	- taken;
		break;
	case ENGINEER:
		spots							= MAX_ENGINEER;
		team->mos[ENGINEER]->available	= MAX_ENGINEER	- taken;
		break;
	case MEDIC:
		spots							= MAX_MEDIC;
		team->mos[MEDIC]->available		= MAX_MEDIC		- taken;
		break;
	case FLAMER:
		spots							= MAX_FLAMER;
		team->mos[FLAMER]->available	= MAX_FLAMER	- taken;
		break;
	default:
		spots							= 0;
		team->mos[class]->available		= 0;
		break;
	}

	if (mapclasslimits[team->index][class].limit)
	{
		spots = mapclasslimits[team->index][class].limit;
		team->mos[class]->available = spots - taken;
	}

	if (spots < 0)
		spots = 0;


/*	safe_bprintf(PRINT_HIGH, "class_stat %s: %s -- %i/%i (%i)\n",
		ent->client->pers.netname,
		team->mos[class]->name,
		taken, spots, 
		team->mos[class]->available);*/

	if (team->mos[class]->available > 0)
		return true;
	else
		return false;
}



void DoEndOM(edict_t *ent /*,qboolean notOfficer*/)
{

	/*if (!ent->client->resp.mos) {
		safe_cprintf(ent, PRINT_HIGH, "You aren't assigned to a class!\n");
		return;
	}*/

	if (!ent->client->resp.team_on) {
		safe_cprintf(ent, PRINT_HIGH, "You aren't assigned to any team!\n");
		return;
	}

	//	assign bkupmos to mos
	//	ent->client->resp.bkupmos=ent->client->resp.mos;

	// if they changed class

	if (ent->client->resp.newmos)
	{
		if (ent->client->resp.mos == NONE || ent->client->resp.mos != ent->client->resp.newmos)
		{
			if (OpenSpot(ent, ent->client->resp.newmos))
			{
				ent->client->resp.mos =  ent->client->resp.newmos;
				ent->client->resp.team_on->mos[ent->client->resp.mos]->available--;
			}
			else 
			{	
				if (ent->client->resp.mos == NONE)
				{
					safe_centerprintf(ent, "Request for class denied: Infantry\n");
					ent->client->resp.mos = INFANTRY;						
				} 
				else
					safe_centerprintf(ent, "Your new selected class already\nhas enough players. Retain your\nassignment.\n");

			}
			ent->client->resp.newmos = NONE;
		}
	}


	// reset playermodel with team's
	SyncUserInfo(ent, true); 
	
	ent->takedamage = DAMAGE_YES;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 20;//faf 22;
	ent->inuse = true;
	//ent->classname = "private";
	ent->mass = 200;
	ent->solid = SOLID_TRIGGER; //don't set this until seconds after respawn
	//ent->client->OBTime=level.time+OBDELAY;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
    ent->svflags &= ~SVF_NOCLIENT; 
	ent->wound_location=0;
	Give_Class_Weapon(ent);
	Give_Class_Ammo(ent);

	//if (ent->client->resp.mos == AIRBORNE)
	//	ent->flags |= FL_BOOTS;

	safe_cprintf(ent,PRINT_HIGH, "Your class is %s.\n", 
		ent->client->resp.team_on->mos[ent->client->resp.mos]->name);
	ent->client->limbo_mode = false;
	ent->client->resp.changeteam = false;
	ent->client->respawn_time = level.time;


	ent->client->layout_type = SHOW_OBJECTIVES_TEMP;
	ent->client->show_obj_temp_time = level.time;
	Cmd_Objectives(ent);

	ent->client->mg42_temperature = 0;


} 



void M_MOS_Join(edict_t *ent, pmenu_t *p, int choice)
{
	choice -= 6;

	PMenu_Close(ent);

	ent->client->resp.newmos = choice;

	if (ent->client->resp.AlreadySpawned && ent->client->resp.changeteam == false)
	{
		if (choice == ent->client->resp.mos) { // Already playing that class!
			safe_cprintf(ent, PRINT_HIGH, "You've already been assigned the %s class!\n",
				ent->client->resp.team_on->mos[choice]->name);
		
		} else {
			safe_cprintf(ent, PRINT_HIGH, "Requesting %s class assignment your next operation.\n",
				ent->client->resp.team_on->mos[choice]->name);
		}

		return;
	}

	if (ent->client->resp.changeteam) {
		ent->client->resp.mos = INFANTRY;
//faf		respawn(ent);
	}
//	} else
//		EndObserverMode(ent);//faf: handle this in begin client frame
//	else if (level.framenum >   ((int)(level_wait->value * 10) +  (ent->client->spawn_delay * 10))   )
//		EndObserverMode(ent);

}

/*
void SMOS_Join(edict_t *ent,int choice)
{
	//pbowens: just in case
	safe_cprintf(ent, PRINT_HIGH, "Secondary MOS/CLASS has been disabled!\n");
	return;

	if(choice!=0) choice--;
	ent->client->resp.smos=choice;
	ent->client->usr_menu_sel=NULL;
	EndObserverMode(ent);
}
*/


// There are many ways to do this.. but this way was easier on the eyes
void client_menu(edict_t *ent, int entry, char *text, int align, void *arg, void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry, int choice)) {
	ent->client->menu_cur[entry].text		= text;
	ent->client->menu_cur[entry].align		= align;
	ent->client->menu_cur[entry].arg		= arg;
	ent->client->menu_cur[entry].SelectFunc = SelectFunc;
}


void M_ChooseMOS(edict_t *ent)
{
	int		i,j;
	char	*theText = NULL;
	size_t	theTextSize;	// Knightmare added
	int		taken;
	int		maxSlots;
//	int		index;
	edict_t	*cl_ent;

	//pmenu = (ent->client->resp.team_on->index) ? menu_classes_grm : menu_classes_usa;
	//memcpy(ent->client->menu_cur, menu_classes, sizeof(pmenu_t));

	PMenu_Close(ent);

	client_menu( ent, 2, "*D-DAY: NORMANDY "  /*DEVVERSION*/,	PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 3, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 4,  NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 5, "Choose a Class",					PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 9, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 17, NULL,								PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu( ent, 18, "Main Menu",						PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );

	if (ent->client->limbo_mode && 
	   !ent->client->resp.AlreadySpawned &&
	  (!ent->client->resp.team_on || !ent->client->resp.team_on->teamname) )
		return;

	if (ent->flyingnun)
		return;

	for(i=1; i < MAX_MOS;i++) 
	{
		//faf: hopefully fixes class bugs
		taken = 0;
		maxSlots = 0;

		for (j=0 ; j < game.maxclients ; j++)
		{
			cl_ent = g_edicts + 1 + j;

			if (cl_ent->ai)
				continue; //don't count bots against class limits

			if (!cl_ent->inuse)
				continue;
			if (!cl_ent->client || 
				!cl_ent->client->resp.team_on ||
				!cl_ent->client->resp.team_on->mos ||
				cl_ent->client->resp.team_on->index != ent->client->resp.team_on->index)
				continue; 
			
//			if (cl_ent == ent && ent->client->resp.mos == INFANTRY)
if (cl_ent == ent && (!ent->client->resp.AlreadySpawned || ent->client->resp.changeteam))
continue;


			if (ent->client->resp.team_on->mos[i]->mos ==
				cl_ent->client->resp.mos)
				taken++;
		}



		// Now set the available for this class

		
		switch (ent->client->resp.team_on->mos[i]->mos) //crash
		{
		case INFANTRY:
			maxSlots = MAX_INFANTRY;
			ent->client->resp.team_on->mos[i]->available = MAX_INFANTRY - taken;
			break;
		case OFFICER:
			maxSlots = MAX_OFFICERS;
			ent->client->resp.team_on->mos[i]->available = MAX_OFFICERS - taken;
			break;
		case L_GUNNER:
			maxSlots = MAX_L_GUNNER;
			ent->client->resp.team_on->mos[i]->available = MAX_L_GUNNER - taken;
			break;
		case H_GUNNER:
			maxSlots = MAX_H_GUNNER;
			ent->client->resp.team_on->mos[i]->available = MAX_H_GUNNER - taken;
			break;
		case SNIPER:
			maxSlots = MAX_SNIPER;
			ent->client->resp.team_on->mos[i]->available = MAX_SNIPER - taken;
			break;
		case SPECIAL:
			maxSlots = MAX_SPECIAL;
			ent->client->resp.team_on->mos[i]->available = MAX_SPECIAL - taken;
			break;
		case ENGINEER:
			maxSlots = MAX_ENGINEER;
			ent->client->resp.team_on->mos[i]->available = MAX_ENGINEER - taken;
			break;
		case MEDIC:
			maxSlots = MAX_MEDIC;
			ent->client->resp.team_on->mos[i]->available = MAX_MEDIC - taken;
			break;
		case FLAMER:
			maxSlots = MAX_FLAMER;
			ent->client->resp.team_on->mos[i]->available = MAX_FLAMER - taken;
			break;
		default:
			maxSlots = 0;
			ent->client->resp.team_on->mos[i]->available = 0;
			break;
		}

		if (mapclasslimits[ent->client->resp.team_on->index][i].limit)
		{
			maxSlots = mapclasslimits[ent->client->resp.team_on->index][i].limit;
			ent->client->resp.team_on->mos[i]->available = maxSlots - taken;
		}

		if (maxSlots < 0)
			maxSlots = 0;



		// Setup text variable
		// Knightmare- make this use a stored size
	//	theText = gi.TagMalloc(sizeof("123456789012 [00/00]"), TAG_GAME);
		theTextSize = 24;
		theText = gi.TagMalloc(theTextSize, TAG_GAME);
		if (maxSlots == 0)
		{
			Q_strncpyz (theText, theTextSize, "                    ");
		}
		else
			Q_strncpyz (theText, theTextSize, va("%12s [%i/%i]", ent->client->resp.team_on->mos[i]->name, taken, maxSlots));

		ent->client->menu_cur[i+6].text  = (class_limits->value)?(char *)theText:ent->client->resp.team_on->mos[i]->name;
		
		
		
		ent->client->menu_cur[i+6].align = PMENU_ALIGN_LEFT;
		ent->client->menu_cur[i+6].arg   = NULL;
		ent->client->menu_cur[i+6].SelectFunc = M_MOS_Join;

	}

	// You can't go back and change stuff before you've spawned
	if (ent->client->resp.AlreadySpawned || !ent->client->resp.changeteam)
		client_menu(ent, 18, "Main Menu",		PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );

	client_menu(ent, 21, "*Use [ and ] to select", PMENU_ALIGN_CENTER,	NULL, NULL );

	PMenu_Open(ent, ent->client->menu_cur, 7, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
//	gi.TagFree(theText);
}


void M_Team_Join(edict_t *ent, pmenu_t *p, int choice)
{

	qboolean foundspot=false;
	int i,j,k,l,m;

	if (ent->client->menu)
		PMenu_Close(ent);

	choice -= 11;//7;



	if (choice == -2) { // auto team
		i = j = k = 0;

		/*faf
		for (k = 0; k <= MAX_TEAM_MATES; k++)
		{
			if (team_list[0]->units[k])
				i++;
			if (team_list[1]->units[k])
				j++;
		}*/
		//faf
		i = PlayerCountForTeam(0, true);
		j = PlayerCountForTeam(1, true);

		l = PlayerCountForTeam (0, false);
		m = PlayerCountForTeam (1, false);

		//faf:  if theyre already on a team subtract them from the total
		if (ent->client->resp.team_on &&
			ent->client->resp.team_on->index == 0)
			i--;

		if (ent->client->resp.team_on &&
			ent->client->resp.team_on->index == 1)
			j--;


		if (i > j) 
			choice = 1;
		else if (i < j) 
			choice = 0;

		//else if there are bots, go to the team with fewer humans
		else if  (l > m)
			choice = 1;
		else if (m > l)
			choice = 0;

		//otherwise go to losing team
		else if (team_list[0]->kills > team_list[1]->kills)//faf
			choice = 1;
		else if (team_list[1]->kills > team_list[0]->kills)//faf
			choice = 0;
		else
		{
			if (ent->client->resp.team_on)
			{
				PMenu_Close(ent);
				return;
			}
			choice = (int)(random() * 2);
		}
	}

//	if (ent->client->resp.AlreadySpawned)
//	{
		if (ent->client->resp.team_on &&
			ent->client->resp.team_on->index == team_list[choice]->index)
		{
			safe_cprintf(ent, PRINT_HIGH, "Already on team %s!\n", team_list[choice]->teamname);
			PMenu_Close(ent);
			return;
		}
//	}

/*	for(i=0;i<MAX_TEAM_MATES;i++)
	{
		if (!team_list[choice])
			continue;

		if (!team_list[choice]->units[i])
		{
			if (ent->client->resp.team_on)
			{
//faf: "total" not used	now			team_list[ent->client->resp.team_on->index]->total--;
				team_list[ent->client->resp.team_on->index]->units[ent->client->resp.unit_index] = NULL;
				ent->client->resp.unit_index = i;
			}
			ent->client->resp.team_on=team_list[choice];
//faf: not used			team_list[choice]->total++;
			team_list[choice]->units[i]=ent;
			foundspot=true;

			ent->client->resp.mos = NONE; // reset MOS
			break;
		}
	}*/




	if (ent->client->resp.AlreadySpawned) // used choose_team cmd
	{
		if (ent->health ==100)
			T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->health + 999, 0, DAMAGE_NO_PROTECTION, MOD_CHANGETEAM);
		else
			T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->health + 999, 0, DAMAGE_NO_PROTECTION, MOD_CHANGETEAM_WOUNDED);
	}
	
	if (ent->client->resp.team_on)
	{
		ent->client->resp.team_on=team_list[choice];
		safe_bprintf(PRINT_HIGH, "%s has switched to team %s.\n", ent->client->pers.netname, ent->client->resp.team_on->teamname);
	}
	else
	{
		ent->client->resp.team_on=team_list[choice];
		safe_bprintf(PRINT_HIGH, "%s has joined team %s.\n", ent->client->pers.netname, ent->client->resp.team_on->teamname);
		ent->client->pers.afk_check_time = level.framenum;
	}

	ent->client->resp.mos = NONE; // reset MOS

	ent->client->resp.mos = INFANTRY;
	ent->client->resp.changeteam = true;
	ent->client->forcespawn = level.time + .5;//faf: fixes standing corpse bug


//	stuffcmd(ent, va("play %s/shout/yes1.wav", team_list[choice]->teamid));
				

	M_ChooseMOS(ent);
	//EndObserverMode(ent);	// *RSH Copied this from SMOS_Join code to try and start the game. 

	return;


//	gi.dprintf("warning: %s got to end of M_Team_Join().\n", ent->client->pers.netname);


}



void ChooseTeam (edict_t *ent)
{
	int		i;	//,j;
	char	*theText = NULL;
	size_t	theTextSize;	// Knightmare added
	int		max_clients;

	PMenu_Close(ent);

	if (ent->flyingnun)
	{
		safe_cprintf (ent, PRINT_HIGH, "You need to leave observer mode first.  Type \"observer\".\n");
		return;
	}

//	if (ent->client->resp.changeteam == true) {
//		safe_centerprintf(ent, "You have already changed teams once!\nYou must wait for your next assignment\n");
//		return;
//	}



	// rezmoth - must wait until end of lobby time //faf:  not
//faf	if (level.framenum < ((int)level_wait->value * 10))
//faf		return;

	// Eliminates ghost-bug
	if ((ent->client->limbo_mode || ent->deadflag) && ent->client->resp.team_on)
	{
		safe_centerprintf(ent, "You must wait for your next assignment\nto change teams!");
		return;
	}


	client_menu(ent, 4, "*D-DAY: NORMANDY "  /*DEVVERSION*/,PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 5, "*by Vipersoft",			PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 2, NULL,						PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 7, "*Choose a Team",				PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 4, NULL,						PMENU_ALIGN_CENTER, NULL, NULL );


	if (!force_auto_select->value)//faf
	{

		for(i=0;i<MAX_TEAMS;i++)
		{
			if (!team_list[i]) 
				continue;

			//faf:  password off teams
			if (i == 0)
			{
				if (Q_stricmp(allied_password->string, "") != 0)
				{
					if (Q_stricmp(allied_password->string, Info_ValueForKey (ent->client->pers.userinfo, "password")) != 0) 
					{
						if (Q_stricmp(allied_password->string, "none") != 0)
						continue;
					}
				}
			}				

			if (i == 1)
			{
				if (Q_stricmp(axis_password->string, "") != 0)
				{
					if (Q_stricmp(axis_password->string, Info_ValueForKey (ent->client->pers.userinfo, "password")) != 0) 
					{
						if (Q_stricmp(axis_password->string, "none") != 0)
						continue;
					}
				}
			}				



//			for (j=0; team_list[i]->units[j]; j++);

			max_clients = maxclients->value;
			// Make the text look good
			// Knightmare- make this use a stored size
		//	theText = gi.TagMalloc(sizeof("123456789012 [00/00]"), TAG_GAME);
			theTextSize = 24;
			theText = gi.TagMalloc(theTextSize, TAG_GAME);
			Q_strncatz (theText, theTextSize, va("%12s [%i/%i]", team_list[i]->teamname, PlayerCountForTeam(i, true), max_clients));//faf: removed "team_list[i]->total,"

			// Put it on the menu
			client_menu (ent, (i + 11), theText, PMENU_ALIGN_LEFT, NULL, M_Team_Join );
		}
	}

//	client_menu(ent, 7, NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
	if (!ent->client->resp.team_on)
	{
		if (((Q_stricmp(allied_password->string, "") == 0) ||
			(Q_stricmp(allied_password->string, "none") == 0)) && 
			((Q_stricmp(axis_password->string, "") == 0) ||
			(Q_stricmp(axis_password->string, "none") == 0)))
			client_menu (ent, 9, "Auto Select",		PMENU_ALIGN_CENTER,	NULL, M_Team_Join );		
	}

//	client_menu (ent, 9, NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu (ent, 14, "Main Menu",		PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );
//	client_menu (ent, 11, NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu (ent, 17, "*Use [ and ] to select", PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open (ent, ent->client->menu_cur , 5, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
//	gi.TagFree(theText);


} 
void M_Observe_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	Cmd_FlyingNunMode_f(ent);
}
void M_Binds_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	Cmd_DDHelp_f(ent);
	stuffcmd(ent, "toggleconsole;");
}
void M_Name_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	stuffcmd(ent, "menu_playerconfig;");
}

void MainMenu(edict_t *ent)
{
	PMenu_Close(ent);

	client_menu(ent, 4, "*D-DAY: NORMANDY " /*DEVVERSION*/, PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 5, "*by Vipersoft",	PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 2,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL ),
	client_menu(ent, 7,  "Main Menu",		PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 4,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL );

	if (!ent->flyingnun)
	{
		client_menu(ent, 9,  "Join The Game ",		PMENU_ALIGN_CENTER,	NULL, M_Team_Choose );
		
		if (ent->client && ent->client->resp.team_on)
		{	client_menu(ent, 9,  "Choose a Team ",		PMENU_ALIGN_CENTER,	NULL, M_Team_Choose );
		
			client_menu(ent, 10,  "Choose a Class",		PMENU_ALIGN_CENTER,	NULL, M_Class_Choose );
		}
		else
			client_menu(ent, 9,  "Join The Game ",		PMENU_ALIGN_CENTER,	NULL, M_Team_Choose );
	}

	if (!ent->flyingnun)
		client_menu(ent, 11,  "Observe       ",	PMENU_ALIGN_CENTER,	NULL, M_Observe_Choose );
	else
		client_menu(ent, 11,  "Stop Observing",	PMENU_ALIGN_CENTER,	NULL, M_Observe_Choose );

	client_menu(ent, 12, "Change Name   ",	PMENU_ALIGN_CENTER,	NULL, M_Name_Choose );
	client_menu(ent, 13, "Help          ",	PMENU_ALIGN_CENTER,	NULL, M_Binds_Choose );
	client_menu(ent, 14, "Credits       ",	PMENU_ALIGN_CENTER,	NULL, M_View_Credits );
//	client_menu(ent, 7,  NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu(ent, 17, "*Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open(ent, ent->client->menu_cur, 9, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}

void M_Main_Menu (edict_t *ent, pmenu_t *p, int choice) 
{ MainMenu(ent); }

void M_Team_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	ChooseTeam(ent);
}
void M_Class_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	M_ChooseMOS(ent);
}
void M_View_Credits (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Development Credits",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 4, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 5, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 5, "D-Day: Normandy",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 6, "Was Created By",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 7,  "*Vipersoft",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_Vipersoft);
//	client_menu(ent, 8,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 9, "Further",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 10, "Unofficial",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 11, "Development By...",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 12,  "*SHAEF",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_Shaef);
	client_menu(ent, 14,  "*British/Russian Team",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_GBR);
	client_menu(ent, 16,  "*Japan/USMC Team",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_JPN);
	client_menu(ent, 18,  "*Polish/Italian Team",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_ITA);
	client_menu(ent, 20,  "And many others!",		PMENU_ALIGN_CENTER,	NULL, NULL);

	client_menu(ent, 22, "Visit DdayDev.com",			PMENU_ALIGN_CENTER,	NULL, NULL );
	client_menu(ent, 24, "Main Menu",						PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );
	client_menu(ent, 25, "*Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open(ent, ent->client->menu_cur, 7, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}

void credit_sound (edict_t *ent, pmenu_t *p, int choice) 
{

	switch (choice)
	{
	case 6:
		stuffcmd(ent, "play usa/shout/follow2.wav");
		break;
	case 9:
		stuffcmd(ent, "play inland/buzz2.wav");
		break;
	case 14:
		stuffcmd(ent, "play jpn/katana/draw.wav");
		break;
	case 17:
		stuffcmd(ent, "play usa/shout/smoke1.wav");
		break;

	}
	PMenu_Close(ent);
	M_View_Credits_Vipersoft (ent, p, choice);
}

void M_View_Credits_Vipersoft (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY" /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Development Credits",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 4, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );

		client_menu(ent, 5, "Project Leader",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 6, "*Jason 'Abaris' Mohr",				PMENU_ALIGN_CENTER,	NULL, credit_sound );
//		client_menu(ent, 7, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 8, "Programming",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 9, "*Phil Bowens",						PMENU_ALIGN_CENTER,	NULL, credit_sound );
		client_menu(ent, 10, "*Species",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 11, "*Adam 'RezMoth' Sherburne",		PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 12, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 13, "Level Design",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 14, "*Peter 'Castrator' Lipman",		PMENU_ALIGN_CENTER,	NULL, credit_sound );
//		client_menu(ent, 15, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 16, "Visual Artist",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 17, "*Darwin Allen",					PMENU_ALIGN_CENTER,	NULL, credit_sound );
//		client_menu(ent, 18, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
//		client_menu(ent, XX, "Sound Engineer",					PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, XX, "*Oliver 'JumperDude' Snavely",	PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 19, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 20, "Webmistress",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 21, "*Wheaty",							PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 22, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );

	
		client_menu(ent, 25, "Back",						PMENU_ALIGN_RIGHT,	NULL, M_View_Credits );

		if (choice == 7)
			choice = 25;
	PMenu_Open(ent, ent->client->menu_cur, choice, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}

void M_View_Credits_Shaef (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Supreme Headquarters,",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 4, "*Allied Expeditionary",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 5, "*Forces",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 6, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 7, "(Tons of bug fixes and",								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 8, "gameplay enhancements)",								PMENU_ALIGN_CENTER, NULL, NULL );


		client_menu(ent, 10, "Programming",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 11, "*Fafner",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 12, "*Bill Stokes",						PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 14, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 13, "Modelling",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 14, "*Parts",						PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 17, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
//		client_menu(ent, 18, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );

	
		client_menu(ent, 25, "Back",						PMENU_ALIGN_RIGHT,	NULL, M_View_Credits );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}
void M_View_Credits_GBR (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "British and Russian",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 4, "Add-ons Dev Team",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 6, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );


		client_menu(ent, 6, "*Mjr Parts",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 7, "GBR Models/Skins",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 9, "*Bill Stokes",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 10, "Programming",								PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 12, "*Gen Pepper",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 13, "GBR Shouts",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 15, "*Karr",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 16, "RUS Shouts",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 18, "*Afrow UK",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 19, "Maps",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 21, "*Fafner",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 22, "Code/RUS weaps",						PMENU_ALIGN_CENTER,	NULL, NULL );

	
		client_menu(ent, 25, "Back",						PMENU_ALIGN_RIGHT,	NULL, M_View_Credits );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}
void M_View_Credits_JPN (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "Japan and USMC",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 4, "Add-on Dev Team",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 6, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );


		client_menu(ent, 6, "*Julhelm",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 7, "Weapon Models/Skins",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 9, "*EON_Magicman",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 10, "Player Skins",								PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 12, "*Mjr Parts",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 13, "Player Models",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 15, "*Karr",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 16, "Shouts",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 18, "*Fafner & Van Wilder",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 19, "Code",						PMENU_ALIGN_CENTER,	NULL, NULL );
	
		client_menu(ent, 25, "Back",						PMENU_ALIGN_RIGHT,	NULL, M_View_Credits );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}
void M_View_Credits_ITA (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " /*DEVVERSION*/,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );

		client_menu(ent, 3, "Polish and Italian",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 4, "Add-ons Dev Team",					PMENU_ALIGN_CENTER, NULL, NULL );

		client_menu(ent, 6, "*Wolf",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 7, "Models and POL maps",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 9, "*Fernan",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 10, "Models",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 12, "*Rab,d",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 13, "ITA Skins",								PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 15, "*Gypsyllama",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 16, "*Tanatovago",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 17, "*Mr. YOur no fun",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 18, "Maps",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 20, "*Mieker",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 21, "POL Shouts",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 23, "*Fafner",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 24, "Code",						PMENU_ALIGN_CENTER,	NULL, NULL );

		client_menu(ent, 25, "Back",						PMENU_ALIGN_RIGHT,	NULL, M_View_Credits );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}

void VoteMap (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);

	choice = (choice - 7)/2;

	//safe_bprintf (PRINT_HIGH, "%s voted for %s.\n", ent->client->pers.netname, votemaps[choice]); 
	safe_bprintf (PRINT_HIGH, "1 vote for %s.\n", votemaps[choice]); 

    mapvotes[choice]++;

	ent->client->voted = true;
	gi.WriteByte (svc_layout);
	gi.WriteString ("");
	gi.unicast (ent, true);

	level.last_vote_time = level.time;
}

void MapVote(edict_t *ent)
{
//	int			randstart,
	int			i;
	char		filename[100];
	FILE		*f;
	qboolean	botmap = false;
	char		*theText = NULL;
	size_t		theTextSize;	// Knightmare added
	char		*add;

	PMenu_Close(ent);

	client_menu(ent, 4, "*VOTE FOR THE NEXT MAP! " , PMENU_ALIGN_CENTER, NULL, NULL );
	

	for (i=0; i <4; i++)
	{
		// check for bot support
	//	gi.dprintf ("x%s\n",mapstring);
		if (bots->value)
		{
		/*	Com_sprintf (filename, sizeof(filename), "dday/navigation/%s.cmp", votemaps[i]);
			f = fopen (filename, "rb"); */
			// Knightmare- use  SavegameDir() / GameDir() instead for compatibility on all platforms
			Com_sprintf (filename, sizeof(filename), "%s/navigation/%s.cmp", SavegameDir(), votemaps[i]);
			// fall back to GameDir() if not found in SavegameDir()
			f = fopen (filename, "rb");
			if ( !f ) {
				Com_sprintf (filename, sizeof(filename), "%s/navigation/%s.cmp", GameDir(), votemaps[i]);
				f = fopen (filename, "rb");
			}
			// end Knightmare

			if (f) {
				fclose (f);
				add = "*";
				botmap = true;
			}
			else
				add = " ";

		}
		else
			add = "";

			// Knightmare- make this use a stored size
		//	theText = gi.TagMalloc(sizeof("1234567890123456789012345"), TAG_GAME);
			theTextSize = 32;
			theText = gi.TagMalloc(theTextSize, TAG_GAME);
			Q_strncatz (theText, theTextSize, va("      %s %s",add,votemaps[i]));

		client_menu (ent, 7+(i*2),  theText,		PMENU_ALIGN_LEFT,	NULL, VoteMap );
	}
		

	if (bots->value){
        client_menu (ent, 18, "** = Has bot support",			PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu (ent, 21, "Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );
	}
	else
		client_menu (ent, 18, "Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );

	PMenu_Open(ent, ent->client->menu_cur, 7, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}


