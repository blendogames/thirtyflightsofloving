/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_combat.c,v $
 *   $Revision: 1.53 $
 *   $Date: 2002/07/23 22:48:26 $
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

// g_combat.c

#include "g_local.h"
#include "g_cmds.h"
#include "x_fire.h"//faf


#define LEG_DAMAGE		(height/2) - abs(targ->mins[2]) - 3
#define STOMACH_DAMAGE	(height/1.6) - abs(targ->mins[2])
#define CHEST_DAMAGE	(height/1.4) - abs(targ->mins[2])


#define WIDTH 0
#define DEPTH 1
#define HEIGHT 2

#define SPAWN_CAMP_TIME	10 //seconds



/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest;
	trace_t	trace;


// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}

	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;


	return false;
}


/*
============
Killed
============
*/
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;


	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type

		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))

		{
			level.killed_monsters++;

			if (coop->value && attacker->client)
				attacker->client->resp.score++;

			// medics won't heal monsters that they kill themselves

	//		if (strcmp(attacker->classname, "monster_medic") == 0)

	//			targ->owner = attacker;

		}
	}

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

/*	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		targ->touch = NULL;
		monster_death_use (targ);
	}*/

	// pbowens: quick hack for misplaced artys and grenades
	if (targ->client && attacker && !attacker->client && attacker->owner && attacker->owner->client)
		attacker = attacker->owner;
	if (targ->client && inflictor && inflictor != attacker && !inflictor->client && inflictor->owner && inflictor->owner->client)
		attacker = inflictor->owner;


	if(!targ->deadflag)
	{

	// pbowens: suicide gives other team kill
		if (targ->client && targ->client->resp.team_on)
		{

			if (attacker == targ || // themselves
				attacker == NULL ||
				attacker == world )// suicide/ unfotunate death
			{
				if(targ->client && targ->client->last_wound_inflictor && targ->client->last_wound_inflictor != targ)
				{
					targ->client->resp.score++;//point gets taken away somewhere, let's give it back since it's counted as a kill, not suicide
					targ->client->last_wound_inflictor->client->resp.score++;

					if (targ->ai)
						targ->client->last_wound_inflictor->client->resp.stat_bot_plus++;
					else
						targ->client->last_wound_inflictor->client->resp.stat_human_plus++;
				}

				//changed teams with full health
				if (meansOfDeath == MOD_CHANGETEAM)
				{
					//hacky shite: give back frag taken away
					targ->client->resp.score++;
				}
				else
				{
					team_list[(targ->client->resp.team_on->index+1)%2]->kills++;
					//targ->client->resp.plus_minus--;
					if (targ->client->aim)
						targ->client->resp.stat_bot_minus--;
					else
						targ->client->resp.stat_human_minus++;
				}
			}

			// pbowens: undo the previous kill count if changeteam
			if (targ->client && targ->client->resp.changeteam)
			{
				team_list[(targ->client->resp.team_on->index) ? 0 : 1]->kills--;
				//targ->client->resp.plus_minus++;
				targ->client->resp.stat_human_plus++; //assuming bots won't be changing teams
			}
		}

		if(targ->client					&&
			attacker &&
			attacker->client				&&
			targ->client->resp.team_on	&&
			attacker->client->resp.team_on )
		{

			if (attacker != targ && attacker->client->resp.team_on == targ->client->resp.team_on) {
				if (team_kill->value == 2)
					attacker->client->penalty = PENALTY_TEAM_KILL;
			}

			if (attacker->client->resp.team_on != targ->client->resp.team_on)
			{
				attacker->client->resp.team_on->kills++;
				//attacker->client->resp.plus_minus++;

				if (targ->ai)
					attacker->client->resp.stat_bot_plus++;
				else
					attacker->client->resp.stat_human_plus++;

				if (targ->client)
					//targ->client->resp.plus_minus--;
					if (attacker->ai)
						targ->client->resp.stat_bot_minus++;
					else
						targ->client->resp.stat_human_minus++;

			}
			targ->client->resp.team_on->losses++;

			if (spawn_camp_check->value)
			{
				if (!spawn_camp_time->value)
					 gi.cvar_set("spawn_camp_time", "2");

				if ((level.time - targ->client->spawntime) < spawn_camp_time->value) // how long it's been since they spawns
				{
					if (attacker->client->resp.killtime) {

						attacker->client->resp.oldkilltime = attacker->client->resp.killtime;
						attacker->client->resp.killtime = level.time;

						if ((attacker->client->resp.killtime - attacker->client->resp.oldkilltime) <= SPAWN_CAMP_TIME + ( (attacker->delay) ? attacker->delay : RI->value) )
						{
							if (targ->client->ps.pmove.pm_type != PM_DEAD)
								attacker->client->penalty= PENALTY_SPAWN_CAMP; // make sure to count respawn time
						}

					}
					else
						attacker->client->resp.killtime = level.time;
				}
			}
		}
	}

	if (targ->client)
	{
		//if (targ->client->resp.team_on  &&  targ->client->resp.mos)
		//	targ->client->resp.team_on->mos[targ->client->resp.mos]->available++;

		/*
		if (targ->client->resp.mos==OFFICER)
			targ->client->resp.bkupmos=targ->client->resp.team_on->officer_mos;
			*/

		//targ->client->resp.mos=targ->client->resp.bkupmos;
		//targ->client->limbo_mode=true;

		if (!targ->deadflag)
			targ->client->forcespawn = (level.framenum + FORCE_RESPAWN);
	}

	targ->die_time=0;
	targ->die (targ, inflictor, attacker, damage, point);
}

/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
	if (damage > 255)
		damage = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
//	gi.WriteByte (damage);
	gi.WritePosition (origin);
	gi.WriteDir (normal);
	gi.multicast (origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/



void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
		return;

	if (attacker == targ || attacker == targ->enemy)
		return;

	// if we are a good guy monster and our attacker is a player
	// or another good guy, do not get mad at them
	if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
			return;
	}

	// we now know that we are not both good guys

	// if attacker is a client, get mad at them because he's good and we're not
	if (attacker->client)
	{
		// this can only happen in coop (both new and old enemies are clients)
		// only switch if can't see the current enemy
		if (targ->enemy && targ->enemy->client)
		{
			if (visible(targ, targ->enemy))
			{
				targ->oldenemy = attacker;
				return;
			}
			targ->oldenemy = targ->enemy;
		}
		targ->enemy = attacker;
		//if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		//	FoundTarget (targ);
		return;
	}

	// it's the same base (walk/swim/fly) type and a different classname and it's not a tank
	// (they spray too much), get mad at them
	if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
		 (strcmp (targ->classname, attacker->classname) != 0) /*	&&
	 (strcmp(attacker->classname, "monster_tank") != 0) &&
		 (strcmp(attacker->classname, "monster_supertank") != 0) &&
		 (strcmp(attacker->classname, "monster_makron") != 0) &&
		 (strcmp(attacker->classname, "monster_jorg") != 0)*/ )
	{
		if (targ->enemy)
			if (targ->enemy->client)
				targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		//if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		//	FoundTarget (targ);
	}
	else
	// otherwise get mad at whoever they are mad at (help our buddy)
	{
		if (targ->enemy)
			if (targ->enemy->client)
				targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

qboolean In_Vector_Range(vec3_t point, vec3_t origin,
						 float x_range,
						 float y_range,
						 float z_range)
{
	vec3_t temp;

	VectorSubtract(point,origin, temp);

	if( (abs(temp[0])>x_range) ) return false;
	if( (abs(temp[1])>y_range) ) return false;
	if( (abs(temp[2])>z_range) ) return false;
	return true;
}

int Damage_Loc(edict_t *targ, vec3_t point, edict_t *attacker)
{
	float x_coord, y_coord, p_angle; //faf


	float min_x = targ->s.origin[0] + targ->mins[0] - 0.1;
	float max_x = targ->s.origin[0] + targ->maxs[0] + 0.1;
	float min_y = targ->s.origin[1] + targ->mins[1] - 0.1;
	float max_y = targ->s.origin[1] + targ->maxs[1] + 0.1;
	float min_z = targ->s.origin[2] + targ->mins[2] - 0.1;
	float max_z = targ->s.origin[2] + targ->maxs[2] + 0.1;

	// rezmoth - impact debug info
	//gi.dprintf("mins  (%f, %f, %f)\n", targ->mins[0], targ->mins[1], targ->mins[2]);
	//gi.dprintf("maxs  (%f, %f, %f)\n", targ->maxs[0], targ->maxs[1], targ->maxs[2]);
	//gi.dprintf("point (%f, %f, %f)\n", point[0], point[1], point[2]);
	//gi.dprintf("Pmin  (%f, %f, %f)\n", min_x, min_y, min_z);
	//gi.dprintf("Pmax  (%f, %f, %f)\n", max_x, max_y, max_z);


	if (targ->s.frame >= 283 &&
		targ->s.frame <= 294 )
		return STOMACH_WOUND;



	switch (targ->stanceflags)
	{
	case STANCE_STAND:
		if (point[2] > min_z + 41)//faf46)
		{
			//gi.dprintf("head\n");
			return HEAD_WOUND;

		}

		//faf: check for flamer hit
		else if (attacker->client &&
			targ->client &&
			targ->client->pers.weapon &&
			targ->client->pers.weapon->position == LOC_FLAME
				&& point[2] > min_z + 30 && point[2] < min_z + 46
				&& targ->client->tank_hit == false
				&& attacker->client->resp.team_on != targ->client->resp.team_on)

		{
			//messy but it works decent... detects the angle where the bullet
			//hit the bounding box, from the origin of the player
			x_coord = point[0] - targ->s.origin[0];
			y_coord = point[1] - targ->s.origin[1];
			if (x_coord > 0)
				p_angle = (atan ((y_coord)  / (x_coord))) * (360 / (2 * 3.14159));
			else if (y_coord < 0)
				p_angle = - (180 -(atan ((y_coord)  / (x_coord))) * (360 / (2 * 3.14159)));
			else
				p_angle = (atan ((y_coord)  / (x_coord))) * (360 / (2 * 3.14159)) + 180;

			//adjusts the angle according to direction player is facing
			//this is messed and goes to -220 in the back instead of -180 but it wont matter here:
			p_angle = p_angle - targ->client->v_angle[1];

//			safe_bprintf(PRINT_HIGH, "x %f, y %f \n", x_coord, y_coord);
//			safe_bprintf(PRINT_HIGH, "x %f \n", p_angle);

			if (p_angle > 90 || p_angle < -90)
			{
				safe_cprintf(targ, PRINT_HIGH, "Your tank has been shot! \n");
				return TANK_HIT;
//				safe_bprintf(PRINT_HIGH, "TANK HIT\n");
			}



//faf		} else if (point[2] > min_z + 37 && point[2] < min_z + 46) {
		} else if (point[2] > min_z + 32 && point[2] < min_z + 41) {
			//gi.dprintf("chest\n");
			return CHEST_WOUND;
//faf		} else if (point[2] > min_z + 28 && point[2] < min_z + 37) {
		} else if (point[2] > min_z + 22 && point[2] < min_z + 32) {
			// we would check for flamethrower here but the box is not rotated so the
			// player would have to be facing north for the code to work
			// for now we could just treat all fatal chest wounds as a flamethrower hit
			//gi.dprintf("stomach\n");
			return STOMACH_WOUND;
//faf		} else if (point[2] < min_z + 28) {
		} else if (point[2] < min_z + 22) {
			//gi.dprintf("leg\n");
			return LEG_WOUND;
		}
		break;
	case STANCE_DUCK:
//faf		if (point[2] > min_z + 34)
		if (point[2] > min_z + 28)
		{
			//gi.dprintf("head\n");
			return HEAD_WOUND;
//faf		} else if (point[2] > min_z + 26 && point[2] < min_z + 34) {
		} else if (point[2] > min_z + 19 && point[2] < min_z + 28) {
			//gi.dprintf("chest\n");
			return CHEST_WOUND;
//faf		} else if (point[2] > min_z + 16 && point[2] < min_z + 26) {
		} else if (point[2] > min_z + 12 && point[2] < min_z + 19) {
			//gi.dprintf("stomache\n");
			return STOMACH_WOUND;
//faf		} else if (point[2] < min_z + 18) {
		} else if (point[2] < min_z + 12) {
			//gi.dprintf("leg\n");
			return LEG_WOUND;
		}
		break;
	case STANCE_CRAWL:
		// since crawl boxes are not rotated, you would have to face north for this to work
		// for this reason, we simply treat all prone hits as stomach wounds so the damage
		// is lessened because you are being sneaky and hard to hit
		return STOMACH_WOUND;
		break;
	}

	// used to ensure that even leaks are counted for some damage
	// this prevents the hitboxes from not being hit even if traced
	//gi.dprintf("default wound\n");
	return STOMACH_WOUND;

	/*
	float height, width, depth;//, temp;

	float leg_x,	leg_y,		leg_z;
	float stom_x,	stom_y,		stom_z;
	float head_x,	head_y,		head_z;
	float chest_x,	chest_y,	chest_z;
	float near_x,	near_y,		near_z; //Wheaty: determine when a bullet 'almost' hits.

	width  = abs(targ->mins[WIDTH])  + targ->maxs[WIDTH];		//0 x
	depth  = abs(targ->mins[DEPTH])  + targ->maxs[DEPTH];		//1 y
	height = abs(targ->mins[HEIGHT]) + targ->maxs[HEIGHT];		//2 z

	//gi.dprintf("h%f:w%f:d%f\n", height, width, depth);

	near_x = width;
	near_y = depth;
	near_z = height;


	switch (targ->stanceflags)
	{
	case STANCE_STAND:
	case STANCE_DUCK:
		leg_x = (width * 0.75) / 2;						// width
		leg_y = depth / 2;								// depth
		leg_z = targ->mins[HEIGHT]   + (height * 0.4);	// height

		stom_x = (width - 2) / 2;
		stom_y = depth / 2;
		stom_z = targ->mins[HEIGHT]  + (height * 0.4);

		chest_x = width / 2;
		chest_y = depth / 2;
		chest_z = targ->mins[HEIGHT] + (height * 0.75);

		head_x = (width *.5) / 2;
		head_y = depth / 2;
		head_z = targ->mins[HEIGHT]  + height;

		break;

	case STANCE_CRAWL:
		leg_x = (width * 0.75) / 2;
		leg_y = depth / 2;
		leg_z = targ->mins[HEIGHT]	 + (height * 0.4);

		stom_x = (width - 2) / 2;
		stom_y = depth / 2;
		stom_z = targ->mins[HEIGHT]	 + (height * 0.4);

		chest_x = width / 2;
		chest_y = depth / 2;
		chest_z = targ->mins[HEIGHT] + (height * 0.75);

		head_x = (width * 0.5) / 2;
		head_y = depth / 2;
		head_z = targ->mins[HEIGHT]  + height;

		break;
	}


	if (In_Vector_Range(point, targ->s.origin, leg_x, leg_y, leg_z) )
	{ //leg damage

		//gi.dprintf("leg x%f:y%f:z%f\n", leg_x, leg_y, leg_z);
		return LEG_WOUND;
	}

	if (In_Vector_Range(point, targ->s.origin, stom_x, stom_y, stom_z) )
	{ //stomach damage

		//gi.dprintf("stom x%f:y%f:z%f\n", stom_x, stom_y, stom_z);
		return STOMACH_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin, chest_x, chest_y, chest_z) )
	{ //chest damage

		//gi.dprintf("chest x%f:y%f:z%f\n", chest_x, chest_y, chest_z);
		return CHEST_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin, head_x, head_y, head_z) )
	{ //head damage

		//gi.dprintf("head x%f:y%f:z%f\n", head_x, head_y, head_z);
		return HEAD_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin,near_x,near_y,near_z))
	{ //Wheaty: Near miss

		return NEAR_MISS;
	}
	else
		return 0;
	*/
}

#define HELMET_SAVE 90		//15% chance helmet will deflect the round
#define	BLEND_TIME 2		// How long the player is affected by damage.. (seconds)
//bcass start - 3% chance of being shot/function to do it
#define DROP_SHOT 97

//void Drop_Shot (edict_t *ent, gitem_t *item);

void Use_Weapon (edict_t *ent, gitem_t *inv);
void Drop_Shot (edict_t *ent, gitem_t *item)
{
	int		index;


	if (!item)
		return;

	index = ITEM_INDEX(item);

	//pbowens: stop firing
	ent->client->buttons &= ~BUTTON_ATTACK;
	ent->client->latched_buttons &= ~BUTTON_ATTACK;
	ent->client->weaponstate = WEAPON_READY;

	Use_Weapon (ent, FindItem("Fists"));

	// wheaty: fix for drop-shot spam bug
	if (ent->client->pers.inventory[index])
	{
		Drop_Item (ent, item);
		ent->client->pers.inventory[index] = 0;

		gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/drop.wav"), 1, ATTN_NORM, 0);
		safe_centerprintf(ent, "YOU DROPPED YOUR WEAPON!!\n");

		ent->s.modelindex2 = 0; //faf:  remove the weapon model immediately or it looks like theres 2
	}
}
//bcass end


//not using this
void Drop_Flamed (edict_t *ent)
{
	int		index;
	gitem_t *item;

	item = 	ent->client->pers.weapon;


	if (item == FindItem("Fists") ||
		item == FindItem("Flamethrower") ||
		item == FindItem("TNT") ||
		item == FindItem("Binoculars") ||
		item == FindItem("Morphine"))
		return;


	if (item && ent->client->pers.weapon->position== LOC_GRENADES)
	{
		index = ITEM_INDEX(item);

		if (ent->client->pers.inventory[index])
		{
			Drop_Item (ent, item);
			//ent->client->pers.inventory[index] = 0;

			//ent->s.modelindex2 = 0; //faf:  remove the weapon model immediately or it looks like theres 2
		}
	}
	else if (item)
	{

		index = ITEM_INDEX(item);

		//pbowens: stop firing
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		// wheaty: fix for drop-shot spam bug
		if (ent->client->pers.inventory[index])
		{
			Drop_Item (ent, item);
			ent->client->pers.inventory[index] = 0;

			ent->s.modelindex2 = 0; //faf:  remove the weapon model immediately or it looks like theres 2
		}
	}
//	ent->client->newweapon = FindItem ("fists");
	Use_Weapon (ent, FindItem("Fists"));
}




void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take,

				save,
//				asave,
//	int			psave;
				te_sparks,
				result = 0,
				height,
				randnum,

				wound_location,
				die_time;

	//faf
	int i;
		vec3_t	v;
		vec3_t  blood_orig;

		int d;
		edict_t *check_ent;

		qboolean sniper;



    qboolean saved=false;

	if (!(dflags & DAMAGE_NO_PROTECTION) && IsValidPlayer(targ) && level.time < targ->client->spawntime + invuln_spawn->value) // pbowens: invulnerability
		return;

	//faf
	if (attacker && targ &&
		targ->client && attacker->client &&
		targ != attacker &&
		!OnSameTeam (targ, attacker) &&
		mod != MOD_EXPLOSIVE)
		targ->client->last_wound_inflictor = attacker;


	//hacky
	if (attacker && attacker->ai && targ->classnameb && targ->classnameb == SANDBAGS)
		damage = 1000;






//JABOT:  if someone attacks this guy, the teammate bots go into aim mode                         ///make attacker their enemy: //needs work
	sniper = false;
	for (d = 1; d<= maxclients->value; d++)
    {
         check_ent = g_edicts + d;
         if (!check_ent->inuse)
			 continue;
		 if (!check_ent->ai)
			 continue;
		 if (check_ent == targ)
			 continue;
		 if (!check_ent->client ||
			 !check_ent->client->resp.team_on ||
			 check_ent->health < 1)
			 continue;
		  if (!inflictor || !inflictor->client)
			 continue;
		 if (!inflictor->client->resp.team_on)
			 continue;
		 if (check_ent->client->resp.team_on ==
			 inflictor->client->resp.team_on)
			 continue;
		 if (check_ent->enemy)
			 continue;
		 if (check_ent->deadflag)
			 continue;

		 if (visible (check_ent, targ))
		 {
			 if (!(check_ent->stanceflags != STANCE_STAND && check_ent->client->resp.mos == H_GUNNER))
			 {
				 check_ent->ai->last_enemy_time = level.time + 2;
				 check_ent->client->aim = true;
				 VectorCopy (vec3_origin, check_ent->ai->last_enemy_origin); //empty
			 }

			if (sniper == false && inflictor->client->pers.weapon && inflictor->client->pers.weapon->position == LOC_SNIPER)
			{
				 check_ent->ai->sniperspot = level.time;
				 sniper = true;//only want one dude to shout this
				//gi.dprintf("SNIPER\n");
			}
		 }

	}
//END JABOT










	wound_location = die_time = 0;
	height = abs(targ->mins[2]) + targ->maxs[2];

   	if (targ->client &&((mod == MOD_PISTOL) ||
						(mod == MOD_SHOTGUN) ||
						(mod == MOD_RIFLE) ||
						(mod == MOD_LMG) ||
						(mod == MOD_HMG) ||
						(mod == MOD_SUBMG) ||
						(mod == MOD_SNIPER) ||
						(mod == MOD_BAYONET) ||
						(mod == MOD_KNIFE)) )
	{

		result = Damage_Loc(targ, point, attacker);
		//gi.dprintf ("%i\n",result);
		targ->client->damage_knockback = 20;


		//faf: if flamethrower tank is hit
		if (result == TANK_HIT)
		{
			//explode
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (targ->s.origin);
			gi.multicast (targ->s.origin, MULTICAST_PHS);

			T_RadiusDamage (targ, attacker, 45, targ, 40, MOD_EXPLOSIVE);
			T_Damage (targ, attacker, attacker, vec3_origin, targ->s.origin, vec3_origin, 50, 0, 0, MOD_TANKHIT);
			gi.positioned_sound (targ->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("bullet/metal1.wav"), 1, ATTN_NORM, 0);

			//burn
			PBM_Ignite (targ, attacker, point);
//			safe_bprintf (PRINT_HIGH, "ignite \n");

			targ->client->tank_hit = true;

			targ->client->flame_rnd = 0;


			for (i = 0; i <9; i ++)
			{

				edict_t *fire;

				fire = G_Spawn();
				fire->s.modelindex = MD2_FIRE;
					fire->s.frame      = FRAME_FIRST_SMALLIGNITE;
					fire->s.skinnum    = SKIN_FIRE_RED;
				VectorClear (fire->mins);
				VectorClear (fire->maxs);
					VectorCopy (targ->s.origin, fire->s.origin);
				VectorClear (fire->s.angles);
				VectorClear (fire->velocity);
				fire->movetype     = MOVETYPE_TOSS;
				fire->clipmask     = MASK_SHOT;
				fire->solid        = SOLID_BBOX;
				fire->takedamage   = DAMAGE_NO;
				fire->s.effects    = 0;
				fire->s.renderfx   = RF_FULLBRIGHT;
					fire->owner        = fire;
					fire->master       = attacker;
				fire->classname    = "fire";
				fire->classnameb = FIRE;
				fire->touch        = PBM_FireDropTouch;
				fire->burnout      = level.time + 2 + random() * 3;
				fire->timestamp    = level.time;
				fire->nextthink    = level.time + FRAMETIME;
				fire->think        = PBM_CheckFire;
	//				VectorCopy (30, fire->pos1);
	//				VectorCopy (30, fire->pos2);
					fire->dmg_radius   = 30;
					fire->dmg          = 1;//blast_chance;

				fire->s.origin[0] = targ->s.origin[0] + crandom() * 100;
				fire->s.origin[1] = targ->s.origin[1] + crandom() * 100;
				fire->s.origin[2] = targ->s.origin[2] + crandom() * 100;


				v[0] = 50 + 100 * crandom();
				v[1] = 50 + 100 * crandom();
				v[2] = 50 + 100 * crandom();
				VectorMA (targ->velocity, 2, v, fire->velocity);


				gi.linkentity (fire);
			}



			return;
		}














	//Wheaty: Panzer Deflection (too lazy to make it modular)
	// pbowens: moved to be damage loc-independant
	if (rand() % 100 < 5.0 &&
		targ->client->pers.weapon &&
		targ->client->pers.weapon->classnameb == WEAPON_PANZER &&
		result != LEG_WOUND &&
		result != STOMACH_WOUND &&
		targ->client &&
		targ->stanceflags != STANCE_STAND &&
		targ->client->aim &&
		targ->client->resp.team_on)
	{
	//	gi.sound   (targ, CHAN_BODY, gi.soundindex ("world/ric2.wav"), 1, ATTN_NORM, 0);


					if (random() < .333)
						gi.sound (targ, CHAN_AUTO, gi.soundindex("bullet/metal1.wav") , 1, ATTN_NORM, 0);
					else if (random() < .5)
						gi.sound (targ, CHAN_AUTO, gi.soundindex("bullet/metal2.wav") , 1, ATTN_NORM, 0);
					else
						gi.sound (targ, CHAN_AUTO, gi.soundindex("bullet/metal3.wav") , 1, ATTN_NORM, 0);




		//safe_cprintf (targ, PRINT_HIGH, "DEFLECTION\n");
		damage=0;
		result = 0; // make it nothing

		//Wheaty: Spark at point of deflection
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPARKS);
		gi.WritePosition (point);
		gi.WriteDir (point);
		gi.multicast (point, MULTICAST_PVS);

		targ->client->damage_div=0;

		targ->client->kick_angles[0] += 3;
		targ->client->kick_angles[1] += 3;
		targ->client->kick_angles[2] -= 3;



		return;
	}

	//Wheaty: Per Darwin's request... SMG/LMG can no longer inflict headshots
	if ((mod == MOD_LMG || mod == MOD_SHOTGUN2 || mod == MOD_SUBMG) && result == HEAD_WOUND)
		result = CHEST_WOUND;

	if (targ->deadflag)
		result = CHEST_WOUND;

	switch(result)
	{
		case LEG_WOUND:
			damage*=1.15;
			//if(targ->client) safe_cprintf(targ,PRINT_HIGH,"You've been hit in the leg!\n");
			wound_location |= LEG_WOUND;

			if (targ->client)
				targ->client->damage_div=1.7;

			gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hitleg.wav"), 1, ATTN_NORM, 0);

			break;

		case STOMACH_WOUND:
			damage*=1.5;//0.75
			//if(targ->client) safe_cprintf(targ,PRINT_HIGH,"You've been hit in the stomach!\n");
			wound_location |=STOMACH_WOUND;

			if(!targ->die_time)
				die_time=level.time+5;
			else
				die_time-=20;
			//targ->enemy=attacker;


			if (targ->client)
				targ->client->damage_div=1.4;

			gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hittorso.wav"), 1, ATTN_NORM, 0);
			break;

		case CHEST_WOUND:


//bcass start - random dropping weapon
			srand(rand());
			randnum=rand()%100;

			// wheaty: Don't let drop shot affect Morphine/Flamethrower/Fists/Binocs
			if(!targ->ai &&
				randnum > DROP_SHOT && IsValidPlayer(targ) &&
				!(targ->client->newweapon) && //faf:  if dropping/changing weap, dont hit gun
				targ->client->pers.weapon &&
				targ->client->pers.weapon->position != LOC_GRENADES &&//faf
				targ->client->pers.weapon->classname &&
				(targ->client->pers.weapon->classnameb != WEAPON_FISTS &&
				targ->client->pers.weapon->classnameb != WEAPON_MORPHINE &&
				targ->client->pers.weapon->classnameb != WEAPON_FLAMETHROWER &&
				targ->client->pers.weapon->classnameb != WEAPON_BINOCULARS))
				{
					Drop_Shot (targ, targ->client->pers.weapon);
					damage*=0;//faf
				}
//bcass end

			else//faf:  dont do damage if dropping weapon
			{
				damage*=2;//1.1;
				//if(targ->client) safe_cprintf(targ,PRINT_HIGH,"You've been hit in the chest!\n");
				wound_location |=CHEST_WOUND;

				if(!targ->die_time)
					die_time=level.time+5;

				else
					die_time-=45;


				if (targ->client)
				{
					targ->client->damage_div=1.3;
				}

				gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hittorso.wav"), 1, ATTN_NORM, 0);


			}


			break;

		case HEAD_WOUND:
			if(targ->client)
			{
				if(targ->client->pers.inventory[ITEM_INDEX(FindItem("Helmet"))] //if they got helmet
				&& ( !(targ->client->pers.weapon && !Q_stricmp(targ->client->pers.weapon->pickup_name, "Fists") && targ->client->aim))
					)
				{
					srand(rand());
					randnum=rand()%100;
					if(randnum > HELMET_SAVE)
					{
						edict_t  *dropped;
						vec3_t move_angles;
						vec3_t s_up={0.0,0.0,3.0};

						gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hithelm.wav"), 1, ATTN_NORM, 0);
						safe_cprintf(targ,PRINT_HIGH,"You lucky bastard! Your helmet deflected the shot!\n");
						damage = 0;
						targ->client->kick_angles[0] += 3;
						targ->client->kick_angles[1] -= 3;
						targ->client->kick_angles[2] += 3;
						targ->client->damage_div=0;
						saved=true;
						targ->client->resp.scopewobble = 0;//faf: shakes screen


						dropped = G_Spawn();
						dropped->classname = "Helmet";
						dropped->item = FindItem("Helmet");
						dropped->spawnflags = DROPPED_ITEM;
						//dropped->s.effects = item->world_model_flags;
				//faf        dropped->s.renderfx = RF_GLOW;
						VectorSet (dropped->mins, -4, -4, 0);
						VectorSet (dropped->maxs, 4, 4, 4);
						gi.setmodel (dropped, dropped->item->world_model);
						dropped->solid = SOLID_TRIGGER;
						dropped->movetype = MOVETYPE_TOSS;
						dropped->touch = Touch_Item;
						dropped->owner = targ;
						dropped->gravity = 1;


						vectoangles (targ->velocity, move_angles);

						VectorCopy (targ->s.origin, dropped->s.origin);

						dropped->s.origin[2]+= targ->viewheight;
						VectorCopy (move_angles, dropped->s.angles);


						dropped->nextthink = level.time + 5;
						dropped->think = G_FreeEdict;
						gi.linkentity (dropped);

						VectorScale (s_up, 60, dropped->velocity);

						dropped->movetype = MOVETYPE_TOSS;//STEP;

						dropped->touch = Helmet_touch;


						VectorSet (dropped->avelocity, (500 - 1000 * random()), (500 - 1000 * random()), (500 - 1000 * random()));
						targ->client->pers.inventory[ITEM_INDEX(FindItem("Helmet"))]=0;

					}
				}

				if(!saved)
				{
					if (!targ->deadflag) {
						gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hithead.wav"), 1, ATTN_NORM, 0);
						safe_cprintf(targ,PRINT_HIGH,"Your head's been shot off!\n");
					}
					damage *= 100;

				//Wheaty: This was missing, for some reason :p
					wound_location |= HEAD_WOUND;

				// Dont fade if death from headshot
					targ->client->resp.deathblend = 1;
				}
				else
				{
					//Wheaty: Spark at point of deflection
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPARKS);
					gi.WritePosition (point);
					gi.WriteDir (point);
					gi.multicast (point, MULTICAST_PVS);
				}


			}


			break;

		default: return;
		}
	}





	//faf:  stop teams from destroying their own objectives
	if (targ->classnameb &&	(targ->classnameb == FUNC_EXPLOSIVE_OBJECTIVE ||
		targ->classnameb == OBJECTIVE_VIP ||
	targ->classnameb == FUNC_TRAIN)
		&& 	objective_protect->value == 1)
	{
		if (attacker->client &&
			attacker->client->resp.team_on)
		{
			if(	attacker->client->resp.team_on->index == targ->obj_owner &&
			targ->obj_gain > 0)
				return;
			//faf:  so outpost works.
			else if (attacker->client->resp.team_on->index != targ->obj_owner &&
				targ->obj_loss < 0 && targ->obj_gain < 0)
				return;

			//so we can show how much damage is left on objective

		}
	}


 	if (!targ->takedamage)
		return;

	//faf:  bloodsprays are coming from the outside point of the hitbox, need to
	//      change it so it comes from the body
	VectorSet (blood_orig, targ->s.origin[0], targ->s.origin[1], point[2]);


	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if (attacker != NULL    &&
		targ != attacker	&&
		targ->client		&&
		attacker->client)
	{

		if (OnSameTeam (targ, attacker))
		{
			if (team_kill->value == 1 || team_kill->value == 2) { // ff w/ penalties

				//JABOT
				if (targ->ai && dflags == DAMAGE_BULLET)
				{
					targ->ai->tktime = level.time;
				}

				mod |= MOD_FRIENDLY_FIRE;
				targ->wound_location |= wound_location;
				targ->die_time       += die_time;

				//Wheaty: Spray Blood
				if (result && result == HEAD_WOUND)
				{
					if (!saved)
						SprayBlood(targ, blood_orig, dir, damage, mod);
				}
				else
				{
					SprayBlood(targ, blood_orig, dir, damage, mod);
				}

			}
			else
			{
				damage = 0;
				knockback = 0;
			}
		} else {

			targ->wound_location |= wound_location;
			targ->die_time       -= die_time;

			//Wheaty: Spray Blood
			if (result == HEAD_WOUND)
			{
				if (!saved)
					SprayBlood(targ, blood_orig, dir, damage, mod);
			}
			else
			{
				SprayBlood(targ, blood_orig, dir, damage, mod);
			}
		}

		WeighPlayer(targ);
	}

	meansOfDeath = mod;

	// easy mode takes half damage
	if (skill->value == 0 && deathmatch->value == 0 && targ->client)
	{
		damage *= 0.5;
		if (!damage)
			damage = 1;
	}

	client = targ->client;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

// bonus damage for suprising a monster
	//faf: get rid of this?
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && attacker && (attacker->client) && (!targ->enemy) && (targ->health > 0))
		damage *= 2;

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
		{
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;

		if (targ->client  && attacker == targ)
				VectorScale (dir, 1600.0 * (float)knockback / mass, kvel);	// the rocket jump hack...
			else
				VectorScale (dir, 500.0 * (float)knockback / mass, kvel);

			VectorAdd (targ->velocity, kvel, targ->velocity);
		}
	}

	take = damage;
	save = 0;

	//psave = CheckPowerArmor (targ, point, normal, take, dflags);
	//take -= psave;

//	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
//	take -= asave;

	//treat cheat/powerup savings the same as armor
//	asave += save;

	// team damage avoidance
	//if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
	//	return;

// do the damage
	if (take)
	{
		if ((targ->svflags & SVF_MONSTER) || (client))
		{}//faf			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);

		if (attacker)
			targ->enemy=attacker;
		else if (mod==MOD_WOUND)
			attacker=targ->enemy;

		targ->health = targ->health - take;

		if (targ->health <= 0)
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			//if(mod==MOD_WOUND) targ->enemy=attacker;

			//gi.dprintf("attacker on i%\n",attacker->team_on->kills+5);

			Killed (targ, inflictor, attacker, take, point);



			if (attacker && attacker->client)
				attacker->client->last_obj_health = 0;

			return;
		}
	}

	if (targ->svflags & SVF_MONSTER)
	{
		M_ReactToDamage (targ, attacker);

		if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
		{
			targ->pain (targ, attacker, knockback, take);

			// nightmare mode monsters don't go into pain frames often
			if (skill->value == 3)
				targ->pain_debounce_time = level.time + 5;
		}

	}
	else if (client)
	{
		if (!(targ->flags & FL_GODMODE) && (take))
			targ->pain (targ, attacker, knockback, take);
	}
	else if (take)
	{
		if (targ->pain)
			targ->pain (targ, attacker, knockback, take);
	}


	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client)
	{

		client->damage_blendtime = (level.time + BLEND_TIME);
//		client->damage_parmor += psave;
//		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
	}


	if (targ->classname &&
		(targ->classnameb == FUNC_EXPLOSIVE_OBJECTIVE ||
		targ->classnameb == OBJECTIVE_VIP ||
		targ->classnameb == FUNC_TRAIN) && targ->obj_gain)
	{
		if (attacker->client &&			attacker->client->resp.team_on)
		{
			//gi.dprintf ("%i\n",targ->health);
			attacker->client->last_obj_health = targ->health;
			attacker->client->last_obj_dmg_time = level.time;
			attacker->client->last_obj_name = targ->obj_name;
			//so we can show how much damage is left on objective

		}
	}
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;
	float knockb;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);


		if (inflictor->classnameb == HGRENADE && ent->client)
			points = points * 0.5;

		if (!OnSameTeam(attacker,ent) && inflictor->classnameb == TNT && ent->client && ent->client->spawn_safe_time > level.time - .3)
			points = 0;

		if (!OnSameTeam(attacker,ent) && inflictor->classnameb == ROCKET && ent->client && ent->client->spawn_safe_time > level.time - .3)
			points = points * .1;

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				knockb = (int)points;
				if (inflictor->classnameb == ROCKET)	{
					if (ent != attacker)
						knockb *= 3;
					else
						knockb = 0;
				}
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, knockb, DAMAGE_RADIUS, mod);

				//gi.dprintf ("%f %i \n",  VectorLength (v), (int)points);
			}
		}
	}

	//pbowens: for explosion effects, yes this assumes its an explosion
	SetExplosionEffect(inflictor, damage, radius);

	return;
}

void Remove_Gib (edict_t *ent);


void BloodSprayThink (edict_t *self)
{
	//Wheaty: Borrowed from AQ

	Remove_Gib(self);//faf
//	G_FreeEdict(self);
}

void blood_spray_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//Wheaty: Borrowed from AQ

        if (other == ent->owner)
                return;
//        ent->think = G_FreeEdict;
//		ent->think = Remove_Gib;//faf
//      ent->nextthink = level.time + 0.1;

Remove_Gib(ent);//faf

}


void SprayKnifeBlood(edict_t *self, vec3_t point, vec3_t angle, int damage, int mod)
{
	edict_t *blood;
	int		speed;

	int i;

	vec3_t direction;

	//faf
	if (level.gib_count > max_gibs->value)
		return;



	for (i=0 ; i<3 ; i++)
	{
		level.gib_count++;//faf

		VectorSet(direction, (random()-.5), (random()-.5), 1);
		speed = 300 * random();

		blood = G_Spawn();
		VectorNormalize(direction);
		VectorCopy (point, blood->s.origin);
		VectorCopy (direction, blood->movedir);

		vectoangles (direction, blood->s.angles);
		VectorScale (direction, speed, blood->velocity);


		blood->movetype = MOVETYPE_TOSS;
		blood->clipmask = MASK_SHOT;
		blood->solid = SOLID_BBOX;
		blood->s.effects |= EF_GIB;
		VectorClear (blood->mins);
		VectorClear (blood->maxs);
		blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
		blood->owner = self;
		blood->nextthink = level.time + 3;//faf speed/200;
		blood->touch = blood_spray_touch;
		blood->think = BloodSprayThink;
		blood->dmg = damage;
		blood->classname = "blood_spray";

		gi.linkentity (blood);
	}

}





void SprayBlood(edict_t *self, vec3_t point, vec3_t angle, int damage, int mod)
{

	//Wheaty: Borrowed from AQ

	edict_t *blood;
	int		speed;


	//faf
	if (level.gib_count > max_gibs->value)
		return;





	//faf
	if (damage == 0)
		return;

	//faf
	if (mod == MOD_KNIFE || mod == MOD_BAYONET)
	{
		SprayKnifeBlood (self, point, angle, damage, mod);
		return;
	}




	switch (mod)
	{

	case MOD_PISTOL:
		speed = 500;//250;
		break;
	case MOD_SHOTGUN:
		speed = 1000;//300;
		break;
	case MOD_RIFLE:
		speed = 1000;//400;
		break;
	case MOD_LMG:
		speed = 1000;//400;
		break;
	case MOD_HMG:
		speed = 1000;//700;
		break;
	case MOD_SUBMG:
		speed = 1000;//500;
		break;
	case MOD_SNIPER:
		speed = 1600;//800;
		break;
	case 69:
		speed = 2000;
	default:
		speed = 1600;//500;

	}

	//Wheaty: To prevent fireworks bloodspray
	if (mod == MOD_PISTOL || mod == MOD_SHOTGUN || mod == MOD_RIFLE || mod == MOD_LMG
		|| mod == MOD_HMG || mod == MOD_SUBMG || mod == MOD_SNIPER || mod == MOD_KNIFE || mod == 69)
	{
	level.gib_count++;//faf

	blood = G_Spawn();
    VectorNormalize(angle);
    VectorCopy (point, blood->s.origin);
    VectorCopy (angle, blood->movedir);

	vectoangles (angle, blood->s.angles);
    VectorScale (angle, speed, blood->velocity);

	//debug
	//safe_cprintf(self, PRINT_HIGH,"MOD: %d   VEL: %f\n", mod, VectorLength (blood->velocity));

    blood->movetype = MOVETYPE_TOSS;
    blood->clipmask = MASK_SHOT;
    blood->solid = SOLID_BBOX;
    blood->s.effects |= EF_GIB;
    VectorClear (blood->mins);
    VectorClear (blood->maxs);
    blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
    blood->owner = self;
    blood->nextthink = level.time + 3;//faf speed/200;
    blood->touch = blood_spray_touch;
    blood->think = BloodSprayThink;
    blood->dmg = damage;
    blood->classname = "blood_spray";
    gi.linkentity (blood);

	}
}

// pbowens: since we cant actually tell when an explosion happens, call this after the WriteByte
void SetExplosionEffect (edict_t *inflictor, float damage, float radius)
{
//	int		j;
	float	effect_radius;
	vec3_t	v;
	edict_t	*dmgef_ent = NULL;

	// pbowens: explosive effects
	//	 NOTE: this assumes whatever calls T_RadiusDamage is an explosion
	effect_radius = radius + damage;

	//gi.dprintf("effect_radius: %f\n       radius: %f\n       damage: %f\n",
	//	effect_radius, radius, damage);

	while ((dmgef_ent = findradius(dmgef_ent, inflictor->s.origin, effect_radius)) != NULL)
	{
		if (!dmgef_ent->client)
			continue;

		dmgef_ent->client->dmgef_startframe = level.framenum;
		dmgef_ent->client->dmgef_sway_value = 999; // start the sway count

		VectorAdd (dmgef_ent->mins, dmgef_ent->maxs, v);
		VectorMA (dmgef_ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		dmgef_ent->client->dmgef_intensity = effect_radius - VectorLength (v);

		//gi.dprintf("VectorLength: %f (flash is < 275)\n", VectorLength (v));
		if (VectorLength (v) < 275)
			dmgef_ent->client->dmgef_flash = true;
		else
			dmgef_ent->client->dmgef_flash = false;


	//	gi.dprintf("dmgef_startframe = %i\n", dmgef_ent->client->dmgef_startframe);
	//	gi.dprintf("dmgef_intensity  = %f\n", dmgef_ent->client->dmgef_intensity);
	}
}
