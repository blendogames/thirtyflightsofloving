/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_client.c,v $
 *   $Revision: 1.63 $
 *   $Date: 2002/07/23 07:26:15 $
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
#include "m_player.h"
#include "g_cmds.h"
#include "x_fire.h"
//#include "p_menus.h"

void ShowGun(edict_t *ent);

void SwitchToObserver(edict_t *ent);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
//void weapon_grenade_fire (edict_t *ent, int team, float time)
//bcass start - TNT
void weapon_tnt_fire (edict_t *ent);
//bcass end
void SP_misc_teleporter_dest (edict_t *ent);
void change_stance(edict_t *self, int stance);
qboolean Cmd_Scope_f(edict_t *ent);
void Drop_Weapon (edict_t *ent, gitem_t *item);
void weapon_grenade_fire (edict_t *ent);
void check_unscope (edict_t *ent);//faf
void turret_off (edict_t *self);



//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ( (!self->targetname) || (Q_stricmp(self->targetname, spot->targetname) != 0) )
			{
			//	gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if (Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}

/* needs testing
void SP_info_team_start(edict_t *ent);

//faf:  for maps with no info_team_starts
void Create_InfoTeamStarts (edict_t *self)
{
	edict_t	*spot;
	spot = NULL;

	spot = G_Find(spot, FOFS(classname), "info_team_start");
	if (spot)
		return;

	gi.dprintf("WARNING: No info_team_start entities found.  Creating from scratch.\n");

	spot = G_Spawn();
	spot->classname = "info_team_start";
	spot->dmg = 50;
	spot->map = "dday1";
	spot->count = 99;
	spot->pathtarget = "usa";
	spot->obj_owner = 0;
	spot->message = "Allies";

	SP_info_team_start(spot);

   	
	spot = G_Spawn();
	spot->classname = "info_team_start";
   	spot->dmg = 50;
	spot->map = "dday1";
	spot->count = 99;
	spot->pathtarget = "grm";
	spot->obj_owner = 1;
	spot->message = "Axis";

	SP_info_team_start(spot);
}


*/

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
the normal starting point for a level.
*/
void Create_CTB_Entities (edict_t *self);

void SP_info_player_start(edict_t *self)
{Create_CTB_Entities (self);

	//faf
//	Create_InfoTeamStarts(self);

	if (!coop->value)
		return;
	if (Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/

void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{

	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ( (Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0) )
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/* QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
he deathmatch intermission point will be at one of these
se 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
/*
void SP_info_player_intermission(void){}
void SP_info_reinforcement_start(edict_t *ent);
void SP_info_Infantry_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_L_Gunner_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_H_Gunner_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Sniper_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Engineer_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Medic_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Flamethrower_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Special_Start(edict_t *ent){SP_info_reinforcement_start(ent);}
void SP_info_Officer_Start(edict_t *ent) {SP_info_reinforcement_start(ent);}
*/

void SP_info_player_intermission(void){}
void SP_info_reinforcement_start(edict_t *ent);
void SP_info_Infantry_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][INFANTRY].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_L_Gunner_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][L_GUNNER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_H_Gunner_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][H_GUNNER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Sniper_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][SNIPER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Engineer_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][ENGINEER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Medic_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][MEDIC].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Flamethrower_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][FLAMER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Special_Start(edict_t *ent)
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][SPECIAL].limit = ent->count;
	SP_info_reinforcement_start(ent);
}
void SP_info_Officer_Start(edict_t *ent) 
{
	if (ent->count)
		mapclasslimits[ent->obj_owner][OFFICER].limit = ent->count;
	SP_info_reinforcement_start(ent);
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
//	char		*info;

//	if (!ent->client)
//		return false;

//	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
//	if (info[0] == 'f' || info[0] == 'F')
//		return true;
	return false;
}





qboolean OnSameTeam(edict_t *self,edict_t *target);
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod,i;
	char		*message;
	char		*message2;
	char		*message3;

	qboolean	ff;
	edict_t *Tent;


	
/*-----/ PM /-----/ MODIFIED:  Condition split up for portability. /-----*/
	if (coop->value)
		if (attacker->client)
			meansOfDeath |= MOD_FRIENDLY_FIRE;
/*-----------------------------------------------------------------------*/

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "fell to his death";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "drowned";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "tripped on barbedwire";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			if (inflictor->deathtarget)
				message = inflictor->deathtarget;
			else
				message = "was in the wrong place";
			break;
		case MOD_CHANGETEAM:
			message = "changed teams";
			break;
		case MOD_CHANGETEAM_WOUNDED:
			message = "changed teams";
			break;
		case MOD_PENALTY:
			message = NULL; // penalties have their own obituaries
			break;
		case MOD_NOMOD:
			message = NULL;
			break;
		}


		if (message) //already have message = killed self somehow
		{
			if (self->ai && self->client && self->client->resp.team_on)
			{
				int teamnum = self->client->resp.team_on->index;

				if (botchat->value && (int)rand()%((int)botchatfreq->value) == 1 && botchat_self_count[teamnum])
				{
					self->ai->chatdelay = 10;
					self->ai->chat = botchat_self[teamnum][(int)rand()%(botchat_self_count[teamnum])];
					ParseBotChat (self->ai->chat, self);
				}
			}
		}
		if (attacker == self)
		{

			if (self->ai && self->client && self->client->resp.team_on)
			{
				int teamnum = self->client->resp.team_on->index;

				if (botchat->value && botchat_self_count[teamnum])
				{
					self->ai->chatdelay = 10;
					self->ai->chat = botchat_self[teamnum][(int)rand()%(botchat_self_count[teamnum])];
					ParseBotChat (self->ai->chat, self);
				}
			}



			switch (mod)
			{
			case MOD_HELD_TNT:
				message = "forgot to drop the TNT";
				break;				

			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_SPAWNCAMP://faf
				message = "was killed for being in a spawn area";
				break;
			case MOD_AIRSTRIKE_SPLASH: //faf
				if (IsFemale(self))
					message = "called an airstrike on herself";
				else
					message = "called an airstrike on himself";
				break;

		
/*-----/ PM /-----/ NEW:  Fire related obituaries for suicides. /-----*/
                        case MOD_FIRE_SPLASH:
                        case MOD_FIREBALL:
				if (IsFemale(self))
                                        message = "burned herself";
				else
                                        message = "burned himself";
				break;
                        case MOD_ON_FIRE:
                                message = "became toast";
				break;
/*--------------------------------------------------------------------*/

			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}



		}
		if (message)
		{
			//faf: if they were wounded before they got themselves killed, name who wounded them.
			if (self->client->last_wound_inflictor && self->client->last_wound_inflictor->client && self->client->last_wound_inflictor->client->resp.team_on)
			{
				self->client->last_wound_inflictor->client->resp.team_on->kills++;

				message2 = "(kill credited to";
//				if (OnSameTeam (self->client->last_wound_inflictor, self)
//				{}

			

				// Means of Death msgs now server customizable
				if (death_msg->value == 0)	// print to self only
					safe_cprintf(self, PRINT_HIGH, "%s %s %s %s).\n", self->client->pers.netname, message, message2, self->client->last_wound_inflictor->client->pers.netname);
				else if (death_msg->value == 1 || death_msg->value == 3) // print to everybody
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client)
							continue;

//						if (self->client->resp.team_on->index == Tent->client->resp.team_on->index)
//						{
						if (OnSameTeam(Tent,self)){
							safe_cprintf (Tent, PRINT_MEDIUM, "\2%s ", self->client->pers.netname);}
						else{
							safe_cprintf (Tent, PRINT_MEDIUM, "%s ", self->client->pers.netname);}

							safe_cprintf (Tent, PRINT_MEDIUM, "%s %s ", message, message2);

						if (OnSameTeam(Tent,self->client->last_wound_inflictor)){
							safe_cprintf (Tent, PRINT_MEDIUM, "\2%s", self->client->last_wound_inflictor->client->pers.netname);}
						else{
							safe_cprintf (Tent, PRINT_MEDIUM, "%s", self->client->last_wound_inflictor->client->pers.netname);}
							safe_cprintf (Tent, PRINT_MEDIUM, ").\n");
                            							
					}
					if (death_msg->value == 3)
					{
						gi.cprintf (NULL, PRINT_MEDIUM, "%s ", self->client->pers.netname);
						gi.cprintf (NULL, PRINT_MEDIUM, "%s %s ", message, message2);
						gi.cprintf (NULL, PRINT_MEDIUM, "%s", self->client->last_wound_inflictor->client->pers.netname);
						gi.cprintf (NULL, PRINT_MEDIUM, ").\n");

					}
							
				}
				else if (death_msg->value == 2)  // print to team
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client || !Tent->client->resp.team_on || !Tent->client->resp.mos)
							continue;

						if (self->client->resp.team_on->index == Tent->client->resp.team_on->index)
							safe_cprintf (Tent, PRINT_MEDIUM, "%s %s %s %s).\n", self->client->pers.netname, message, message2, self->client->last_wound_inflictor->client->pers.netname);
					}
				}
			}
			else
			{
				if (death_msg->value == 0)	// print to self only
					safe_cprintf(self, PRINT_HIGH, "%s %s.\n", self->client->pers.netname, message);
				else if (death_msg->value == 1 || death_msg->value == 3) // print to everybody
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client)
							continue;

						if (OnSameTeam(Tent,self)){
							safe_cprintf (Tent, PRINT_MEDIUM, "\2%s", self->client->pers.netname);}
						else{safe_cprintf (Tent, PRINT_MEDIUM, "%s", self->client->pers.netname);}

						safe_cprintf (Tent, PRINT_MEDIUM, " %s.\n", message);
					}
					if (death_msg->value == 3)
					{
						gi.cprintf (NULL, PRINT_MEDIUM, "%s", self->client->pers.netname);
						gi.cprintf (NULL, PRINT_MEDIUM, " %s.\n", message);
					}
				}
				else if (death_msg->value == 2)  // print to team
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client || !Tent->client->resp.team_on || !Tent->client->resp.mos)
							continue;

						if (self->client->resp.team_on->index == Tent->client->resp.team_on->index)
							safe_cprintf (Tent, PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
					}
				}
			}




			if (deathmatch->value)
				self->client->resp.score--;
			if (!self->ai)
				self->enemy = NULL;
			return;
		}



		self->enemy = attacker;
		if (attacker && attacker->client)
		{			

			switch (mod)
			{
			case MOD_PISTOL:
				message = "was capped by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SHOTGUN2:
				message = "was shotgunned by";
				break;
			case MOD_RIFLE:
				message = "was shot down by";
				message2 = "'s rifle";
				break;
			case MOD_LMG:
				message = "was machinegunned by";
				message2 = "'s light machine gun";
				break;
			case MOD_HMG:
				message = "was killed by";
				message2 = "'s heavy machine gun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "did not survive";
				message2 = "'s explosive attack";
				break;
			case MOD_AIRSTRIKE:  //faf
				message = "ate";
				message2 = "'s airstrike";
				break;
			case MOD_AIRSTRIKE_SPLASH:  //faf
				message = "was killed by";
				message2 = "'s airstrike";
				break;

			case MOD_SUBMG:
				message = "was gunned down by";
				message2 = "'s submachinegun";
				break;
			case MOD_SNIPER:
				message = "was sniped by";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
//bcass start - TNT
			case MOD_TNT:
				message = "caught";
				message2 = "'s TNT";
				break;
			case MOD_TNT_SPLASH:
				message = "didn't see";
				message2 = "'s TNT";
				break;				
			case MOD_TNT1_SPLASH:
				if (IsFemale(self))
				message = "tripped on her own TNT";
				else
				message = "tripped on his own TNT";
				break;				
//bcass end
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_WOUND:
				message = "died of severe wounds inflicted by";
				break;

			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
/*-----/ PM /-----/ NEW:  Fire related obituaries for frags. /-----*/
			case MOD_FIRE_SPLASH:
				message = "was scorched by";
				break;
            case MOD_ON_FIRE:
				message = "was cremated by";
				break;
            case MOD_FIREBALL:
				message = "got flamed by";
				break;
/*-----------------------------------------------------------------*/
			case MOD_KNIFE:
	
				if (attacker->client->pers.weapon && 
					attacker->client->pers.weapon->classnameb == WEAPON_KATANA)
					message = "was sliced in half by";
				else if (attacker->client->pers.weapon && 
					attacker->client->pers.weapon->classnameb == WEAPON_SABRE)
					message = "was sliced in half by";
				else
				{
					message = "was castrated by";
					if (self->client && !self->ai && attacker && attacker->client && !attacker->ai && !OnSameTeam(self,attacker))
						attacker->client->resp.stat_castrations++;
				}
				break;
			case MOD_FISTS:
				message = "got knocked out by";
					if (self->client && !self->ai && attacker && attacker->client && !attacker->ai && !OnSameTeam(self,attacker))
						attacker->client->resp.stat_fists++;
				break;
			case MOD_HELMET:
				message = "was helmeted by"; //faf
					if (self->client && !self->ai && attacker && attacker->client && !attacker->ai && !OnSameTeam(self,attacker))
						attacker->client->resp.stat_helmets++;
				break;
			case MOD_BAYONET:
				message = "was bayoneted by";
				break;//faf
			case MOD_PLONK:
				message = "was flattened by";
				break;//faf
			case MOD_BOTTLE:
				message = "was molotoved by";
				break;//faf
			case MOD_TANKHIT:
				message = "got blown up by";
				break;





			}

			//faf:  announce team kills
			if (OnSameTeam(attacker, self) && attacker != self)
			{	message3 = " (Friendly fire";
				if (attacker->ai && attacker->client && attacker->client->resp.team_on)
				{
					int teamnum = attacker->client->resp.team_on->index;
					if (botchat->value && (int)rand()%((int)botchatfreq->value) == 1 &&  botchat_sorry_count[teamnum])
					{
						attacker->ai->chatdelay = 10+rand()%9;
						attacker->ai->chat = botchat_sorry[teamnum][(int)rand()%(botchat_sorry_count[teamnum])];
						ParseBotChat (attacker->ai->chat, self);
					}
				}
				if (self->ai && self->client && self->client->resp.team_on)
				{
					int teamnum = self->client->resp.team_on->index;
					if (botchat->value && (int)rand()%((int)botchatfreq->value) == 1 && botchat_forgive_count[teamnum])
					{
						int randchat = (int)rand()%(botchat_forgive_count[teamnum]);
						self->ai->chatdelay = 20+rand()%9;
						self->ai->chat = botchat_forgive[teamnum][randchat];
						ParseBotChat (self->ai->chat, attacker);
					}
				}

			}
			else
			{
				message3 = "";

				if (self->ai && self->client && self->client->resp.team_on)
				{
					int teamnum = self->client->resp.team_on->index;
					if (botchat->value && (int)rand()%((int)botchatfreq->value) == 1 && botchat_killed_count[teamnum])
					{
						self->ai->chatdelay = 10;
						self->ai->chat = botchat_killed[teamnum][(int)rand()%(botchat_killed_count[teamnum])];
						ParseBotChat (self->ai->chat, attacker);
					}
				}

			}


			if (message)
			{
				// Means of Death msgs now server customizable
				if (death_msg->value == 0) // print to self only
					safe_cprintf(self, PRINT_HIGH, "%s %s %s%s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, message3);
				else if (death_msg->value == 1 || death_msg->value == 3) // print to everybody
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client)
							continue;

//						if (self->client->resp.team_on->index == Tent->client->resp.team_on->index)
//						{
						if (OnSameTeam(Tent,self)){
							safe_cprintf (Tent, PRINT_MEDIUM,"\2%s", self->client->pers.netname);}
						else{safe_cprintf (Tent, PRINT_MEDIUM,"%s", self->client->pers.netname);}

							safe_cprintf (Tent, PRINT_MEDIUM," %s ", message);
						if (OnSameTeam(Tent,attacker)){
							safe_cprintf (Tent, PRINT_MEDIUM,"\2%s", attacker->client->pers.netname);}
						else{safe_cprintf (Tent, PRINT_MEDIUM,"%s", attacker->client->pers.netname);}
							safe_cprintf (Tent, PRINT_MEDIUM,"%s%s", message2, message3);

						if (OnSameTeam(attacker, self) && attacker != self)
						{
							if (self->client->last_wound_inflictor && self->client->last_wound_inflictor->client){
								safe_cprintf (Tent, PRINT_MEDIUM,", Kill credited to ");
								if (OnSameTeam(Tent,self->client->last_wound_inflictor))
									safe_cprintf (Tent, PRINT_MEDIUM,"\2%s", self->client->last_wound_inflictor->client->pers.netname);
								else
									safe_cprintf (Tent, PRINT_MEDIUM,"%s", self->client->last_wound_inflictor->client->pers.netname);

							}

							safe_cprintf (Tent,PRINT_MEDIUM,")\n");
						}
						else safe_cprintf (Tent,PRINT_MEDIUM,"\n");
						
							


					}
						if (death_msg->value == 3) //print to server console
						{
							gi.cprintf (NULL, PRINT_MEDIUM,"%s", self->client->pers.netname);
							gi.cprintf (NULL, PRINT_MEDIUM," %s ", message);
							gi.cprintf (NULL, PRINT_MEDIUM,"%s", attacker->client->pers.netname);
							gi.cprintf (NULL, PRINT_MEDIUM,"%s%s\n", message2, message3);
						}
				}
				else if (death_msg->value == 2) // print to team
				{
					for (i = 1; i <= game.maxclients; i++)
					{
						Tent = &g_edicts[i];
						if (!Tent->inuse || !Tent->client || !Tent->client->resp.team_on || !Tent->client->resp.mos)
							continue;

						if (self->client->resp.team_on->index == Tent->client->resp.team_on->index)
							safe_cprintf(Tent, PRINT_MEDIUM, "%s %s %s%s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, message3);
					}

				}

				//give credit to original attacker when wounded player is tk'ed.
				if (OnSameTeam(attacker, self) && attacker != self && self->client->last_wound_inflictor && self->client->last_wound_inflictor->client)
				{
					if (self->client->last_wound_inflictor->client->resp.team_on)
					{
						self->client->last_wound_inflictor->client->resp.team_on->kills++;
						self->client->last_wound_inflictor->client->resp.score++;
						if (self->client->aim)
							self->client->last_wound_inflictor->client->resp.stat_bot_plus++;
						else
							self->client->last_wound_inflictor->client->resp.stat_human_plus++;

					}

				}

				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

/*
	if (!mod == MOD_NOMOD) {
		safe_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
		if (deathmatch->value)
			self->client->resp.score--;
	}
	*/
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
//	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;


	if (self->client->weaponstate == WEAPON_DROPPING)
		return;//faf:  fixes dropping 2 weaps with drop_shot



	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if ( item && 
		((Q_stricmp (item->pickup_name, "Morphine")   == 0) ||
		 (Q_stricmp (item->pickup_name, "Fists")      == 0) ||
		 (Q_stricmp (item->pickup_name, "TNT")      == 0) ||
		 (Q_stricmp (item->pickup_name, "Sandbags")      == 0) ||
		 (Q_stricmp (item->pickup_name, "Binoculars") == 0) ))
		item = NULL;


	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	// pbowens: fixed the 0 rnd count for persistant rnds on client death
	if (item)
	{
		self->client->v_angle[YAW] -= spread;
//		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
//		drop->spawnflags = DROPPED_PLAYER_ITEM;

		// pbowens: drop ammo
		if (item->ammo &&
			item->position &&
			(item->position != LOC_KNIFE)		&&
			(item->position != LOC_GRENADES)	&&
			(item->position != LOC_TNT)			&&
			(item->position != LOC_SPECIAL))
		{
			edict_t	*dropped;
			gitem_t *ammo_item;
			int		 ammo_index;

			ammo_item	= FindItem(item->ammo);
			ammo_index	= ITEM_INDEX(ammo_item);

			if (self->client->pers.inventory[ammo_index])
			{
				if (!ammo_item->drop)
					return;

				dropped					= Drop_Item (self, ammo_item);
				dropped->count			= self->client->pers.inventory[ammo_index];
				dropped->item->quantity = ammo_item->quantity;

				self->client->pers.inventory[ammo_index] = 0;
			}
		}

		Drop_Weapon(self, item);

	}
/*
	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
*/
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}
/*
=====================
SyncUserInfo
This is called to update the player status
=====================
*/
void SyncUserInfo(edict_t *ent, qboolean pers) {

	char		userinfo[MAX_INFO_STRING];

	memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));

	if (pers == true)
		InitClientPersistant (ent->client);

	globals.ClientUserinfoChanged (ent, userinfo);
}




/*

void Bloodpool_Think(edict_t *ent)
{
	if (!ent->owner->inuse)
	{
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + .1;
		return;
	}

	if (ent->s.frame < 25)
	{
		ent->s.frame++;
		ent->think = Bloodpool_Think;
		ent->nextthink = level.time + .2;
	}
	else if (ent->s.frame < 33)
	{
		ent->s.frame++;
		ent->think = Bloodpool_Think;
		ent->nextthink = level.time + .3;
	}
	else if (ent->s.frame < 39)
	{
		ent->s.frame++;
		ent->think = Bloodpool_Think;
		ent->nextthink = level.time + .4;
	}
	else
	{
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + 20;
	}

}



void Spawn_Bloodpool(edict_t *ent)
{

	edict_t *bloodpool;
	trace_t tr;
	vec3_t end, norm, start;
	vec3_t down = { 0, 0, -1};

	float angle;


	bloodpool = G_Spawn();
	bloodpool->movetype = MOVETYPE_NONE;
	bloodpool->solid = SOLID_TRIGGER;
	bloodpool->s.modelindex = gi.modelindex ("models/objects/bloodpool/tris.md2");

	bloodpool->think = Bloodpool_Think;
	bloodpool->nextthink = level.time + .3;
	bloodpool->owner = ent;
	bloodpool->clipmask = MASK_SHOT;
	
	VectorClear (bloodpool->mins);
	VectorClear (bloodpool->maxs);
	
	bloodpool->classname = "bloodpool";

	VectorCopy (ent->s.origin, bloodpool->s.origin);

//	bloodpool->s.renderfx = RF_TRANSLUCENT;
	VectorCopy (ent->s.angles, bloodpool->s.angles);

	VectorCopy (bloodpool->s.origin, start);
	start[2]+=1;
	VectorMA (start, 100, down, end);
	tr = gi.trace (start, NULL, NULL, end, bloodpool, MASK_SHOT);
	
	if (tr.fraction < 1.0)
	{
		VectorCopy (tr.plane.normal, norm);
		VectorNegate (norm, norm);
		if (norm[2])
			angle = atan (norm[0]/norm[2]);
		else
			angle = atan (norm[0]/.000001);
	
		bloodpool->s.angles[0] = (cos((M_PI*bloodpool->s.angles[1])/180) * angle)* 180 / M_PI;
		bloodpool->s.angles[2] = -(sin((M_PI*bloodpool->s.angles[1])/180) * angle)* 180 / M_PI;


		VectorCopy (tr.endpos, bloodpool->s.origin);
		bloodpool->s.origin[2]+=1;
		gi.linkentity(bloodpool);
	}
	else
		G_FreeEdict(bloodpool);
}
*/
/*
void AlignToGround(edict_t *ent)
{
	trace_t tr;
	vec3_t end, norm, start;
	vec3_t down = { 0, 0, -1};
	float angle;

	VectorCopy (ent->s.origin, start);

	VectorMA (start, 100, down, end);
	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);
	
	if (tr.fraction < 1.0)
	{
		safe_bprintf (PRINT_HIGH, "xxx\n"); 
		VectorCopy (tr.plane.normal, norm);
		VectorNegate (norm, norm);
		if (norm[2])
			angle = atan (norm[0]/norm[2]);
		else
			angle = atan (norm[0]/.000001);
	
		ent->s.angles[0] = (cos((M_PI*ent->s.angles[1])/180) * angle)* 180 / M_PI;
		ent->s.angles[2] = -(sin((M_PI*ent->s.angles[1])/180) * angle)* 180 / M_PI;


		VectorCopy (tr.endpos, ent->s.origin);

	}

}
*/
void turret_off (edict_t *self);
void CopyToBodyQue (edict_t *ent);
/*
==================
player_die
=================
*/
void temp_player_corpse_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	int		 n;

	if (self->health < -400)
{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//		for (n= 0; n < 4; n++)
//			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sexyleg/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/skull/tris.md2", damage, GIB_ORGANIC);

		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
		self->solid = SOLID_NOT;
}
	

}
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		i, n;
	vec3_t	forward, right;
	vec3_t	s_up={0.0,0.0,1.0}, s_forward={0.0,1.0,1.0}, s_behind={0.0,-1.0,1.0}, s_right={1.0,0.0,1.0}, s_left={-1.0,0.0,1.0};

	int mod;


	self->s.modelindex3 = 0;//faf:  ctb code
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("briefcase"))])
		Drop_Briefcase (self, FindItemByClassname("briefcase"));


	VectorClear (self->avelocity);
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;
	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	self->s.sound = 0;
	self->client->weapon_sound = 0;
	self->maxs[2] = -8;
//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

//temp removing corpse stuff 	self->die = temp_player_corpse_die;

	turret_off (self);

	
	if (!self->deadflag)
	{
		self->leave_limbo_time = level.time + RI->value; //faf

		self->client->respawn_time = level.time + 1.0;
		//LookAtKiller (self, inflictor, attacker);
		//self->client->deathfade = 1;
		self->client->killer_yaw = self->s.angles[YAW]; //pbowens: always look forward
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

		if ((!(self->ai && meansOfDeath == MOD_SUICIDE)) &&
			meansOfDeath != MOD_CHANGETEAM)
		TossClientWeapon (self);

		//gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM, 0);

		if (meansOfDeath != MOD_CHANGETEAM ||
			meansOfDeath != MOD_CHANGETEAM_WOUNDED) {
			if (deathmatch->value && !self->client->menu)
				Cmd_Help_f (self);		// show scores
		} 

		if (self->client->grenade)
			weapon_grenade_fire(self);

		//bcass start - tnt
		if (self->client->tnt)
		{
			weapon_tnt_fire(self);
		}
		//bcass end
	}
	
	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;

	if ((mod == MOD_HELD_GRENADE) 
		|| (mod == MOD_GRENADE) 
		|| (mod == MOD_HANDGRENADE) 
		|| (mod == MOD_ROCKET) 
		|| (mod == MOD_R_SPLASH)
		|| (mod == MOD_G_SPLASH) 
		|| (mod == MOD_HG_SPLASH)		
		|| (mod == MOD_EXPLOSIVE)
		|| (mod == MOD_TANKHIT)
		|| (mod == MOD_TNT)
		|| (mod == MOD_TNT1)
		|| (mod == MOD_HELD_TNT)
		|| (mod == MOD_TNT_SPLASH)
		|| (mod == MOD_TNT1_SPLASH)) 
	{	//Wheaty: Explosive death --  gib 'em


		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n=0; n<3; n++)
		{
			SprayBlood(self,self->s.origin, s_up, damage, 69);
			SprayBlood(self,self->s.origin, s_forward, damage, 69);
			SprayBlood(self,self->s.origin, s_behind, damage, 69);
			SprayBlood(self,self->s.origin, s_right, damage, 69);
			SprayBlood(self,self->s.origin, s_left, damage, 69);
		}

		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sexyleg/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/skull/tris.md2", damage, GIB_ORGANIC);

		// pbowens: fade quickly (hopefully reduces overflows)
		self->client->resp.deathblend = 0.6;

		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
	else
	{ // normal death
		if (!self->deadflag)
		{
			static int i;
				i = (i+1)%3;



				
				
				// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->stanceflags==STANCE_DUCK)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else if (self->stanceflags==STANCE_CRAWL)
			{
				self->s.frame = FRAME_crawldeath01-1;
				self->client->anim_end = FRAME_crawldeath07;
			}
			else if (meansOfDeath == MOD_FALLING)//fell to death while standing.
			{
				self->s.frame = 184;
				self->client->anim_end = 189;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}


			/* Wheaty 10/27/01: Moved inside initial 'deadflag' if statement
			to reduce code redundancy. */

			if (attacker && attacker->client && self->wound_location == HEAD_WOUND && meansOfDeath != MOD_KNIFE) //Wheaty: Don't let a knife throw a brain out
			{
				//Wheaty: Head Shot, throw a head gib
				AngleVectors(attacker->client->v_angle, forward, right, NULL);
				point[2] = point[2] + self->viewheight;//faf
				HeadShotGib (self, point, forward, right, self->velocity);
			}
	
			//faf:  dont play 2 death sounds
			if (meansOfDeath != MOD_FALLING || (self->client->resp.team_on && !strcmp(self->client->resp.team_on->teamid, "gbr")))
				gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}

		self->client->v_angle[0] = 0;
	}

	self->deadflag = DEAD_DEAD;

	// remove rnds
	for (i = 0; i < MAX_TEAMS; i++)
	{
		self->client->mags[i].antitank_rnd	= 0;
		self->client->mags[i].hmg_rnd		= 0;
		self->client->mags[i].lmg_fract		= 0;
		self->client->mags[i].lmg_rnd		= 0;
		self->client->mags[i].pistol_fract	= 0;
		self->client->mags[i].pistol_rnd	= 0;
		self->client->mags[i].rifle_fract	= 0;
		self->client->mags[i].rifle_rnd		= 0;
		self->client->mags[i].sniper_fract	= 0;
		self->client->mags[i].sniper_rnd	= 0;
		self->client->mags[i].submg_fract	= 0;
		self->client->mags[i].submg_rnd		= 0;		
		self->client->mags[i].submg2_rnd		= 0;		
		self->client->mags[i].shotgun_rnd		= 0;		
	}

	self->client->last_wound_inflictor = NULL;//faf

	gi.linkentity (self);

//	Spawn_Bloodpool(self);


	//jabot
	if (self->ai)
	{
		if (self->ai->camp_targ > -1)
		{
			camp_spots[self->ai->camp_targ].owner = NULL;
		}
		self->ai->camp_targ = -1;
		if (self->ai->objective)
		{
			self->ai->objective = NULL;
			self->ai->reached_obj_time = 0;
		}
		self->ai->actual_camp_start = -1;
		self->ai->start_camp_time = 0;

	}
	
 
	/*temp removing corpse stuff 
	self->svflags = self->svflags | SVF_MONSTER | SVF_DEADMONSTER;
     self->monsterinfo.aiflags |= AI_GOOD_GUY; 
	*/ 

}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
//	gitem_t		*item;

	float human_kills_save = client->pers.stat_human_kills;
	float human_deaths_save = client->pers.stat_human_deaths;
	float bot_kills_save = client->pers.stat_bot_kills;
	float bot_deaths_save = client->pers.stat_bot_deaths;
	char *ip_save = client->pers.ip;
	char stat_chat[150];
	int		afk_check_time;
	vec3_t	last_angles;

	afk_check_time = client->pers.afk_check_time;
	VectorCopy (client->pers.last_angles, last_angles);

	Com_sprintf (stat_chat, sizeof(stat_chat), "%s", client->pers.stat_chat);

	memset (&client->pers, 0, sizeof(client->pers));


//	client->pers.weapon=item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 5;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 5;
	client->pers.max_grenades	= 2;
//bcass start - TNT
	client->pers.max_tnt		= 1;
//bcass end
	client->pers.max_30cal		= 10;
	client->pers.max_slugs		= 8;
	client->pers.max_flame		= 6;

	client->pers.connected = true;
    
	client->pers.stat_human_kills = human_kills_save;
	client->pers.stat_human_deaths = human_deaths_save;
	client->pers.stat_bot_kills = bot_kills_save;
	client->pers.stat_bot_deaths = bot_deaths_save;
	client->pers.ip = ip_save;

	Com_sprintf (client->pers.stat_chat, sizeof(client->pers.stat_chat), "%s", stat_chat);

	client->pers.afk_check_time = afk_check_time;
	VectorCopy (last_angles, client->pers.last_angles);


}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
//		if (coop->value)
//			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
//	if (coop->value)
//		ent->client->resp.score = ent->client->pers.score;
}



/*
======================================================================
  SelectSpawnPoint
=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (vec3_t spot_origin)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];


		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot_origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;


	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
	
		count++;
		range = PlayersRangeFromSpot(spot->s.origin);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;


	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

		if (spot == spot1 || spot == spot2)

			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectRandomDDaySpawnPoint

specify ent, go to a random point // faf, not this:, but NOT the two points closest to other players
================
*/
edict_t *SelectRandomDDaySpawnPoint (char *spawn_point, int team)
{
	edict_t	*spot, *spotb, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range1, range2;
	edict_t *e;
	int i,otherteam;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find_Team (spot, FOFS(classname), spawn_point, team)) != NULL)
	{
		count++;
//		range = PlayersRangeFromSpot(spot->s.origin);
//		if (range < range1)
//		{
//			range1 = range;
//			spot1 = spot;
//		}
//		else if (range < range2)
//		{
//			range2 = range;
//			spot2 = spot;
//		}
	}

	if (!count)
		return NULL;

	//gi.dprintf("found %i of %s\n", count, spawn_point);

//	if (count <= 2)
//	{
//		spot1 = spot2 = NULL;
//	}
//	else
//		count -= 2;


//	gi.dprintf("count: %i ********\n",count);

	selection = (int) (random() * count);
//	gi.dprintf("selection: %i ********\n",selection);


	spot = NULL;
	do
	{
		spot = G_Find_Team (spot, FOFS(classname), spawn_point, team);

//		if (spot == spot1 || spot == spot2)

//			selection++;
	} while(selection--);

	if (spot->obj_owner == -1)
		spot->obj_owner = team;
	else return spot;



//this team just took a -1 spawn point, make sure other team has a spawn point or give it them here
	otherteam = (team+1)%2;

	for (i=0 ; i<game.maxentities ; i++)
	{
		e = &g_edicts[i];

		if (!e->inuse)
			continue;
		if (strcmp(e->classname, "info_reinforcements_start"))
			continue;
		if (e->obj_owner == otherteam)
			return spot;  //they already have one, get out
	}



	spotb = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
	count = 0;

	while ((spotb = G_Find_Team (spotb, FOFS(classname), spawn_point, otherteam)) != NULL)	{
		count++;
	}

	if (!count)
		return NULL;

	selection = (int) (random() * count);

	spotb = NULL;
	do	{
		spotb = G_Find_Team (spotb, FOFS(classname), spawn_point, otherteam);
	} while(selection--);

	spotb->obj_owner = otherteam;





	return spot;
}



edict_t *SelectNearestSpawnPoint (int team)
{
	edict_t	*bestspot;
	float	bestdistance;
	edict_t	*spot;
	vec3_t mean_origin;
	int i;
    edict_t *check_ent;
	float temp_distance, nearest_distance = 9999999;
	vec3_t dist;
	int playercount = 0;
	bestdistance = 0;

	VectorClear (mean_origin);
 	spot = NULL;
	bestspot = NULL;
       
	for (i = 1; i <= maxclients->value; i++)
    {
         check_ent = g_edicts + i;
         if (!check_ent->inuse)
			 continue;
		 if (!check_ent->client ||
			 !check_ent->client->resp.team_on ||
			 check_ent->health <1)
			 continue;
		 if (check_ent->deadflag)
			 continue;
		if (check_ent->client->resp.AlreadySpawned == false)
			continue;
		if (check_ent->client->limbo_mode == true)
			continue;
		if (check_ent->client->spawntime > level.time - 1)
			continue;


		 if (check_ent->client->resp.team_on->index == team)
		 {
			 playercount++;
			 mean_origin[0] = mean_origin[0] + check_ent->s.origin [0];
			 mean_origin[1] = mean_origin[1] + check_ent->s.origin [1];
			 mean_origin[2] = mean_origin[2] + check_ent->s.origin [2];
		 }


	}

	if (playercount < 1)
		return NULL;

	mean_origin[0]= mean_origin[0] / playercount;
	mean_origin[1]= mean_origin[1] / playercount;
	mean_origin[2]= mean_origin[2] / playercount;


/*	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (mean_origin);
	gi.multicast (mean_origin, MULTICAST_PVS);
	gi.dprintf ("%i   %s",playercount,vtos(mean_origin));
*/

	while ((spot = G_Find (spot, FOFS(classname), "info_reinforcements_nearest")) != NULL)
	{
		if (spot->obj_owner != team)
			continue;


		VectorSubtract (spot->s.origin, mean_origin, dist);
		
		temp_distance = VectorLength(dist);

//		gi.dprintf ("%s %f \n", vtos(spot->s.origin),temp_distance);

		if (temp_distance < nearest_distance)
		{
			nearest_distance = temp_distance;
			bestspot = spot;
		}

	}


	if (bestspot)

	{

		return bestspot;

	}


	return spot;
}



/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/

edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{

		bestplayerdistance = PlayersRangeFromSpot (spot->s.origin);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)

	{
		return bestspot;

	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{

	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();

	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}

	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

/*	if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();

	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);
*/
	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);

	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================
/*
void Find_Mission_Start_Point(edict_t *ent)

  find the entry position for the units to be spawned to when the mission begins
*/

void Find_Mission_Start_Point(edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;
	int team = ent->client->resp.team_on->index;

	/*
	while( (spot = G_Find (spot, FOFS(classname), ent->client->resp.team_on->mos[ent->client->resp.mos]->MOS_Spaw_Point))!=NULL)
	{
		if (spot->obj_owner==ent->client->resp.team_on->index) 
			break;
	}

	if (!spot)
	{
		while( (spot = G_Find (spot, FOFS(classname),"info_reinforcements_start"))!=NULL)
		{
			if (spot->obj_owner==ent->client->resp.team_on->index) 
				break;
		}
	}
	*/
	//faf
	if (!spot)
		spot = SelectNearestSpawnPoint(team);


	if (ent->ai)
		spot = SelectRandomDDaySpawnPoint ("bot_spawn", team);




	// first, find the class spot
	if (!spot)
		spot = SelectRandomDDaySpawnPoint (ent->client->resp.team_on->mos[ent->client->resp.mos]->MOS_Spaw_Point, team);

	if (!spot)
		spot = SelectRandomDDaySpawnPoint ("info_reinforcements_start", team);
	if (!spot)
		spot = SelectRandomDDaySpawnPoint ("info_reinforcements_nearest", team);



	if (!spot) 
		spot = G_Find (spot, FOFS(classname),"info_player_start");
	

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;

	VectorCopy (spot->s.angles, ent->s.angles);
	VectorCopy (spot->s.angles, ent->client->ps.viewangles);
	VectorCopy (spot->s.angles, ent->client->v_angle);


}

void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;


	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)

	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}


void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//		for (n= 0; n < 4; n++)
//			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sexyleg/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/skull/tris.md2", damage, GIB_ORGANIC);


		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;

	}
}






void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;
	float ang;
	int temp1;
	vec3_t   a,b;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);
	gi.unlinkentity (body);

	body->s = ent->s;
	body->s.number = body - g_edicts;
/*-----/ PM /-----/ NEW:  Don't let body make sounds. /-----*/
        body->s.sound = 0;
/*----------------------------------------------------------*/


/*-----/ PM /-----/ MODIFIED:  Treat body as a dead monster. /-----*/
        body->svflags = ent->svflags | SVF_MONSTER | SVF_DEADMONSTER;
        body->monsterinfo.aiflags |= AI_GOOD_GUY;
/*-----------------------------------------------------------------*/

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);


	//in case it didn't get to end of death sequence (could happen when killed on ladder)
	if (ent->client && ent->client->anim_end)
	{
		body->s.frame = ent->client->anim_end;
	}
		



// use prone vaules so gibs dont block shots	
//faf:  handle in player_die	body->mins[2]	= -24;
//								body->maxs[2]   = -12;//faf 8; 

//	safe_bprintf (PRINT_HIGH, "frame %i .\n", ent->s.frame); 
//	safe_bprintf (PRINT_HIGH, "ang %s .\n", vtos(ent->s.angles)); 
//	safe_bprintf (PRINT_HIGH, "bang %s .\n", vtos(body->s.angles)); 

	//faf
	if (ent->s.frame == 177)//crouch death
	{
		VectorSet (body->mins, -8, -5, -24);
		VectorSet (body->maxs, 8, 37, -14);
	}
	else if (ent->s.frame == 183)//death1
	{
		VectorSet (body->mins, -57, -10, -24);
		VectorSet (body->maxs, -8, 15, -14);
	}
	else if (ent->s.frame == 189)//death2
	{
		VectorSet (body->mins, -10, -10, -24);
		VectorSet (body->maxs, 43, 10, -14);
	}
	else if (ent->s.frame == 197)//death3
	{
		VectorSet (body->mins, -100, -11, -24);
		VectorSet (body->maxs, -55, 20, -14);
	}
	else// if (ent->s.frame == 240)//crawldeath
	{
		VectorSet (body->mins, -37, -11, -24);
		VectorSet (body->maxs, 10, 10, -14);
	}


//	safe_bprintf (PRINT_HIGH, "ent mins1 %s .\n", vtos(body->mins)); 
//	safe_bprintf (PRINT_HIGH, "ent maxs1 %s .\n", vtos(body->maxs)); 

//	safe_bprintf (PRINT_HIGH, "ang %f .\n", body->s.angles[1]); 

	ang = anglemod (body->s.angles[1]);

//	safe_bprintf (PRINT_HIGH, "ang %f .\n", ang); 


	VectorCopy (body->mins,a);
	VectorCopy (body->maxs,b);
//		safe_bprintf (PRINT_HIGH, "%f a1 %f .\n", a[0],a[1]); 
//		safe_bprintf (PRINT_HIGH, "%f b1 %f .\n", b[0],b[1]); 

	if (ang > 315 || ang < 45)
	{
//		ang = 0;
	}
	else if (ang >=45 && ang < 135)
	{
		body->mins[0]= -1 * a[1];
		body->mins[1]= a[0];
		body->maxs[0]= -1 * b[1];
		body->maxs[1]= b[0];
	}
	else if (ang >= 135 && ang < 225)
	{
		body->mins[0]= -1 * a[0];
		body->mins[1]= -1 * a[1];
		body->maxs[0]= -1 * b[0];
		body->maxs[1]= -1 * b[1];
	}
	else //270 degrees
	{
		body->mins[0] = a[1];
		body->mins[1] = -1 * a[0];
		body->maxs[0] = b[1];
		body->maxs[1] = -1 * b[0];
	}

//	safe_bprintf (PRINT_HIGH, "ang %f .\n", ang); 

//	angle = ang * (M_PI*2 / 360);
//	body->mins[0] = body->mins[0] * cos(angle) - body->mins[1] * sin (angle);
//	body->mins[1] = body->mins[1] * cos(angle) - body->mins[0] * sin (angle);
//
//	body->maxs[0] = body->maxs[0] * cos(angle) - body->maxs[1] * sin (angle);
//	body->maxs[1] = body->maxs[1] * cos(angle) - body->maxs[0] * sin (angle);

	
	if (body->mins[0] > body->maxs[0])
	{//switch mins and maxs if needed
		temp1 = body->mins[0];
		body->mins[0] = body->maxs[0];
		body->maxs[0] = temp1;
	}
	if (body->mins[1] > body->maxs[1])
	{//switch mins and maxs if needed
		temp1 = body->mins[1];
		body->mins[1] = body->maxs[1];
		body->maxs[1] = temp1;
	}

	
//	X' = X cos(T) - Y sin(T) 
//	Y' = Y cos(T) + X sin(T) 

//safe_bprintf (PRINT_HIGH, "ent mins2 %s angles %s.\n", vtos(body->mins), vtos(ent->s.angles)); 
//safe_bprintf (PRINT_HIGH, "ent maxs1 %s .\n", vtos(body->maxs)); 


//safe_bprintf (PRINT_HIGH, "ent mins1 %s .\n", vtos(ent->mins)); 


	body->absmax[2] = body->absmin[2] + 16;
	body->size[2]   = 16;

//pbowens: ent is now SOLID_NOT, so reset to be solid so it can gib
//	body->solid = ent->solid;
	body->solid	= SOLID_BBOX;
	
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;
/*-----/ PM /-----/ NEW:  Tranfer fire from entity to body. /-----*/
    if (ent->health > 0)
		body->health = 0;
    else
		body->health = ent->health;

	//body->health = 10000;
    body->gib_health = ent->gib_health;
    body->groundentity = ent->groundentity;
    body->mass = ent->mass;
    body->burnout = ent->burnout;
    if (ent->burner)
	{
		body->burner = ent->burner;
        body->burner->enemy = body;
        ent->burner = NULL;
	}
    body->fireflags = ent->fireflags & ~FIREFLAG_DELTA_VIEW;
/*----------------------------------------------------------------*/


	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);

}


void M_ChooseMOS(edict_t *ent);
void respawn (edict_t *self)
{
	// JABot[start]
	if (self->ai && self->ai->is_bot){
		BOT_Respawn (self);
		return;
	}
	// JABot[end]

//	if (deathmatch->value || coop->value)
//	{
		CopyToBodyQue (self);
		PutClientInServer (self);
		// add a teleportation effect
		//self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_LAND;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

	//	if (observer_on_death->value)
	//	{
			// enter spirit world as observer 
			self->movetype = MOVETYPE_NOCLIP; 
			self->solid = SOLID_NOT; 
			self->svflags |= SVF_NOCLIENT; 
			self->client->ps.gunindex = 0;
			self->client->pers.weapon=NULL;
			self->client->limbo_mode=true;
			gi.linkentity (self);
			return;
	//	}
		// Knightmare- unreachable code
	/*	gi.linkentity (self);		

		DoEndOM(self);
		
		return; */
//	}
	// restart the entire server
//	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;

	int		i;


	client_persistant_t	saved;
	client_respawn_t	resp;

	ent->stanceflags = STANCE_STAND;


	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	//faf: due to overflow problems when first connecting, going to start the player in the 
	//void outside the map where there's less lag when they first join the game for a split second
	if (!level.intermissiontime && !ent->ai && ent->client->resp.enterframe > level.framenum - 1)
		VectorSet (spawn_origin, -4032, -4032, -4032);



	//the above line should put the player in a staging room...
	index = ent-g_edicts-1;
	client = ent->client;


	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		SyncUserInfo(ent, true);
		resp = client->resp;
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		globals.ClientUserinfoChanged (ent, userinfo);
//		if (resp.score > client->pers.score)
//			client->pers.score = resp.score;
	}
	else 
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

// clear entity values
	ent->groundentity = NULL;

	ent->client = &game.clients[index];

	ent->takedamage = DAMAGE_YES;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 20;//faf 22;
	ent->inuse = true;
	ent->classname = "private";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
/*-----/ PM /-----/ NEW:  Kill any flames and set new attributes. /-----*/
	ent->burnout = 0;
	ent->gib_health = -30;
    ent->fireflags = 18 | FIREFLAG_DOWN | FIREFLAG_DELTA_VIEW | FIREFLAG_IGNITE;
    ent->style = 1;
/*----------------------------------------------------------------------*/

	if (   ent->client->resp.team_on 
		&& ent->client->resp.mos
		&& ent->client->resp.AlreadySpawned)

		ent->model = va("players/%s/tris.md2", ent->client->resp.team_on->playermodel);
	else
	{
		ent->model = va("players/%s/tris.md2", team_list[0]->teamid);

//faf: removed for team dll support		ent->model = "players/usa/tris.md2";
	}
		

	gi.setmodel (ent, ent->model);

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);
// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));


	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = STANDARD_FOV;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = STANDARD_FOV;
		else if (client->ps.fov > MAX_FOV)
			client->ps.fov = MAX_FOV;
	}

	//if (client->pers.weapon)
	//	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = (MAX_MODELS-1);		// will use the skin specified model
	ent->s.modelindex2 = (MAX_MODELS-1);		// custom gun model

	ShowGun(ent);                   //pbowens: v_wep

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle

	for (i=0 ; i<3 ; i++)

		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

//bcass start - gibola
	client->gibmachine = flame_normal;
//bcass end


	VectorCopy (ent->s.origin, ent->client->last_pos1);
	VectorCopy (ent->s.origin, ent->client->last_pos2);
	VectorCopy (ent->s.origin, ent->client->last_pos3);


	//JABot[start]
	if (ent->ai && ent->ai->is_bot == true)
		return;
	//JABot[end]


	//pbowens: this caused telefrags
/*
	if (!KillBox (ent))
	{	// could't spawn in?

	}
*/
	//if (!client->resp.team_on)
		gi.linkentity (ent);



	// force the current weapon up
	//client->newweapon = client->pers.weapon;
	//ChangeWeapon (ent);
	
	ent->stance_max=ent->stance_min=0; //these 2 lines are for stances that have to be set
	ent->stance_view=20;//faf 22;				//in each clientthink.
	change_stance(ent, STANCE_STAND);

	ent->die_time=0;
	ent->client->limbo_mode=false;		//when you die, you enter the spirit world

	ent->client->jump_stamina = JUMP_MAX;
	ent->flyingnun = false;

	ent->client->syncspeed = true;//faf:  so we're frozen when in death view

	ent->oldstance = STANCE_STAND;//faf

	ent->client->last_fire_time = 0;

	stuffcmd(ent, "crosshair 0;");


}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);








//faf	if (level.framenum > ((int)level_wait->value * 10))
	ent->client->resp.scopewobble = 192;
	
	ent->client->resp.AlreadySpawned=false;
	ent->client->resp.mos = NONE;


	// locate ent at a spawn point
	PutClientInServer (ent);
	//SyncUserInfo(ent, false);


		if (level.objectivepic)
			ent->client->display_info = true;
		else
			ent->client->display_info = false;


	if (level.campaign)
	{
		ent->client->layout_type = SHOW_CAMPAIGN;
		ShowCampaign (ent);
	}
	else if (serverimg->string)
	{
		ent->client->layout_type = SHOW_SERVERIMG;
		ShowServerImg (ent);
	}



	ent->client->resp.autopickup = false; // default to OFF
	ent->client->resp.show_id	 = true;  // default to ON

/*
	memcpy(&ent->client->menu_main,    menu_main,    sizeof(menu_main   ));
	memcpy(&ent->client->menu_teams,   menu_teams,   sizeof(menu_teams  ));
	memcpy(&ent->client->menu_classes, menu_classes, sizeof(menu_classes));
	memcpy(&ent->client->menu_credits, menu_credits, sizeof(menu_credits));
*/
	SwitchToObserver(ent);

	// send effect
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_LOGIN);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

#ifdef DEBUG
		safe_cprintf (ent, PRINT_HIGH, "D-Day: Normandy [DEVELOPMENT BUILD]\n"
                                     "DATE:     %s\n"
									 "VERSION:  %s\n", __DATE__, DEVVERSION);
#endif

	//ala MOTd
//faf	if (level.framenum > ((int)level_wait->value * 10))
//		Cmd_MOTD(ent);


	//JABot[start]
	AI_EnemyAdded(ent);
	//[end]



	// make sure all view stuff is valid
	ClientEndServerFrame (ent);


}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;
	
	
	num_clients++;



	ent->client = game.clients + (ent - g_edicts - 1);
	ent->client->resp.AlreadySpawned=false;




	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		//LevelStartUserDLLs(ent);

		stuffcmd(ent, "cl_forwardspeed 200;cl_sidespeed 200;cl_upspeed 200;");
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "private";
//		ent->client->resp.team_on = 0;
		ent->client->resp.mos = NONE;
		InitClientResp (ent->client);
		PutClientInServer (ent);
		//SyncUserInfo(ent, false);
		SwitchToObserver(ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			safe_bprintf (PRINT_HIGH, "%s has joined the battle.\n", ent->client->pers.netname);
		}
	}


	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	//LevelStartUserDLLs(ent);

}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;


	//gi.dprintf("+ %s->userinfo changed\n", ent->client->pers.netname);

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
	//	strncpy (userinfo, "\\name\\badinfo\\skin\\usa/USMC");
		Q_strncpyz (userinfo, MAX_INFO_STRING, "\\name\\badinfo\\skin\\usa/USMC");	// userinfo is always length of MAX_INFO_STRING 
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;
	
	if (ent)
	{
		char skin[64];

		if (ent->client->resp.team_on && ent->client->resp.mos) 
		{
			//faf:  has the mapper set a custom skin?
/*			if (team_list[ent->client->resp.team_on->index]->skin)
			{
				Q_strncpyz (skin, sizeof(skin), va("%s/%s",
					ent->client->resp.team_on->playermodel, 
					team_list[ent->client->resp.team_on->index]->skin) );
				if (ent->client->resp.mos == MEDIC)
					Q_strncatz (skin, sizeof(skin), "_medic");
			}*/

			if (team_list[ent->client->resp.team_on->index]->skin)
			{
				Q_strncpyz (skin, sizeof(skin), va("%s/%s_",
					ent->client->resp.team_on->playermodel, 
					team_list[ent->client->resp.team_on->index]->skin)
					);
				if (ent->client->resp.mos == MEDIC)
					Q_strncatz (skin, sizeof(skin), "medic");
				else if (ent->client->resp.mos == INFANTRY)
					Q_strncatz (skin, sizeof(skin), "infantry");
				else if (ent->client->resp.mos == OFFICER)
					Q_strncatz (skin, sizeof(skin), "officer");
				else if (ent->client->resp.mos == L_GUNNER)
					Q_strncatz (skin, sizeof(skin), "lmg");
				else if (ent->client->resp.mos == H_GUNNER)
					Q_strncatz (skin, sizeof(skin), "hmg");
				else if (ent->client->resp.mos == SNIPER)
					Q_strncatz (skin, sizeof(skin), "sniper");
				else if (ent->client->resp.mos == SPECIAL)
					Q_strncatz (skin, sizeof(skin), "special");
				else if (ent->client->resp.mos == ENGINEER)
					Q_strncatz (skin, sizeof(skin), "engineer");
				else if (ent->client->resp.mos == FLAMER)
					Q_strncatz (skin, sizeof(skin), "flamethrower");
				else 
					Q_strncatz (skin, sizeof(skin), "infantry");
			}
			else
			{			
				Q_strncpyz (skin, sizeof(skin), va("%s/%s",
					ent->client->resp.team_on->playermodel, 
					ent->client->resp.team_on->mos[ent->client->resp.mos]->skinname) );
			}

			gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s", ent->client->pers.netname, skin));
			
		//	Info_SetValueForKey (userinfo, "skin", skin);

		//	if ( Q_stricmp(s, skin) )
		//		stuffcmd(ent, va("skin %s", skin));

			// pbowens: grm officer model HACK
			/*
			if (IsValidPlayer(ent)) 
			{
				if (ent->client->resp.mos == OFFICER )
					gi.setmodel(ent, va("players/%s/tris2.md2", ent->client->resp.team_on->playermodel) );
				else
					gi.setmodel(ent, va("players/%s/tris.md2", ent->client->resp.team_on->playermodel) );
			}
			*/
		}
		else
		{
			// default to allied infantry
			Q_strncpyz (skin, sizeof(skin), "grm/class_infantry");

			gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s",
				ent->client->pers.netname, skin) );
		}
	/*	if (ent->ai && !strcmp(ent->client->pers.netname, "Hitler"))
		{
			Q_strncpyz (skin, sizeof(skin), "grm/h");
			gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s",
				ent->client->pers.netname, skin) );
		}*/


	}

	// combine name and skin into a configstring
	else 
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );


    


	// handedness
	s = Info_ValueForKey (userinfo, "hand");

	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	ShowGun(ent);  //pbowens: v_wep
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()

Changing levels will NOT cause this to be called again, but

loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	static int Already_Done=0;
	char	*value, *f=NULL, *ip;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

//	f = strdup (value);
	f = G_CopyString (value);	// Knightmare- use G_CopyString instead
	ip = strtok(f, ":");


	//for people with dynamic ip's, let them save their stats by name if they set gender to cyborg
	if (!strcmp (Info_ValueForKey (userinfo, "gender"), "cyborg"))
		ent->client->pers.ip = ent->client->pers.netname;
	else
		ent->client->pers.ip = ip;
	//gi.dprintf("xxxx %s\n", userinfo);

	if (ent->ai)
		BOT_RemoveBot("",ent);


	//faf:  using this from the quake 3.21 source code
	value = Info_ValueForKey (userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && 
	strcmp(password->string, value)) 
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		// Nick - This next lines to stop the passworded (Quake2) server crash
		// when a player 'reconnects' after crashing out whilst in intermission,
		// or when the 'ghost' gets killed as the crashed client reconnects.
		// Removes the 'ghost' from a player slot completely.
		ent->solid = SOLID_NOT;
		ent->inuse = false;
		G_FreeEdict (ent);

		return false;
	}


	// check for a password
//	value = Info_ValueForKey (userinfo, "password");
//	if (strcmp(password->string, value) != 0)
//		return false;

	if (!Already_Done)
	{ 
		//the client connect function gets called a number of times. we need to set
        //this variable up to allow functions to be called only once.
        Already_Done = 1;
        InsertCmds(id_GameCmds, NUM_ID_CMDS, "id");
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	globals.ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected.\n", ent->client->pers.netname);

	ent->client->pers.connected = true;


	if (stats->value)
		SetPlayerRating(ent);

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.

Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;



	if (!level.intermissiontime)
		Write_Player_Stats(ent);

	num_clients--;


	change_stance(ent, STANCE_STAND);
	
	turret_off(ent);

	stuffcmd(ent, "cl_forwardspeed 200;cl_sidespeed 200;cl_upspeed 200;");
	safe_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);


	//faf:  ctb code
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("briefcase"))])
		Drop_Briefcase (ent, FindItemByClassname("briefcase"));



	// Remove them from team

	if (ent->client->resp.team_on)
	{
//faf:  not used now		team_list[ent->client->resp.team_on->index]->total--;
//		team_list[ent->client->resp.team_on->index]->units[ent->client->resp.unit_index] = NULL;
	}

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);

	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;

	gi.configstring (CS_PLAYERSKINS+playernum, "");


	//JABot[start]
	AI_EnemyRemoved (ent);
	//[end]
}

//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask

trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}

void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

void ClientSetMaxSpeed (edict_t *ent, qboolean sync)
{
	int			i;
	int			speed_new[3], 
				speed_standard[3];
	//char		cmd[MAX_CMD_BUFFER];
	qboolean	truesight = true;

	if (!ent || !ent->client || ent->client->resp.changeteam || ent->deadflag) // morphine_use on other ents
		return; 

	// We're going under the assumption that most clients have run enabled
	speed_standard[0] = PSPEED_FORWARD;
	speed_standard[1] = PSPEED_SIDE;
	speed_standard[2] = PSPEED_UP;

	if (!ent->client->limbo_mode)
	{ 
		// TRUE-SIGHT exceptions
		if (!ent->client->pers.weapon ||
			ent->client->pers.weapon->position == LOC_KNIFE		||
			ent->client->pers.weapon->position == LOC_SPECIAL	||
			ent->client->pers.weapon->position == LOC_FLAME		||
//faf			ent->client->pers.weapon->position == LOC_ROCKET	||
			ent->client->pers.weapon->position == LOC_GRENADES	||
			ent->client->pers.weapon->position == LOC_TNT)
			truesight = false;
			
		for (i = 0; i < 3; i++) {
			if (ent->client->aim) 
			{
				// standing 
				if (ent->stanceflags == STANCE_STAND) 
				{

					if (!truesight)
						speed_new[i] = speed_standard[i] * ent->client->speedmod;
					else
						speed_new[i] = speed_standard[i] * ent->client->speedmod/TRUESIGHT_FACTOR;

			
				
				}
				//crouching
				else if (ent->stanceflags == STANCE_DUCK)
				{
					if (!truesight)
						speed_new[i] = speed_standard[i] * ent->client->speedmod;
					else
						speed_new[i] = speed_standard[i] * ent->client->speedmod/TRUESIGHT_FACTOR;
					
					if (ent->client->pers.weapon && 
						ent->client->pers.weapon->position == LOC_H_MACHINEGUN)
						speed_new[i] = 0; // Freezed if crouch/crawl and truesight with HMG
				}
				//prone
				else
				{
					speed_new[i] = speed_standard[i] * (ent->client->speedmod * 0.80);
					
					if (ent->client->pers.weapon && 
						ent->client->pers.weapon->position == LOC_H_MACHINEGUN)
						speed_new[i] = 0; // Freezed if crouch/crawl and truesight with HMG
				}

				//faf:  slow down flamer when firing:
				//if (ent->client->pers.weapon &&
				//ent->client->pers.weapon->position == LOC_FLAME &&
				//ent->client->buttons & BUTTON_ATTACK)
				//	speed_new[i]=0;

			}
			else
				speed_new[i] = speed_standard[i] * ent->client->speedmod;



			// Nick - Lower speed for flamer.
			//faf
		//	if (ent->client->pers.weapon &&
		//	ent->client->pers.weapon->position == LOC_FLAME &&
		//				ent->client->buttons & BUTTON_ATTACK)
		//	{
		//		speed_new[i] *= .75;
		//	}

		}

		
		if ((ent->stanceflags != STANCE_STAND && ent->waterlevel != 3) || 
			 ent->client->jump_stuck == true)

			speed_new[2] = 0;
	} 	
	else	// they are in limbo so freeze 'em
	{
		for (i = 0; i < 3; i++)
			speed_new[i] = 0;
	}




	for (i = 0; i <3; i++)
	{
		if (ent->ai)
			speed_new[i]*=2;

		
		// sometimes, there's no difference, so don't call it twice if unneeded
		if (ent->client->speedmax[i] != speed_new[i])  {
			ent->client->speedmax[i]  = speed_new[i];// for clients with run enabled

			ent->client->syncspeed = (sync) ? true : false;
		}
	}

	//gi.dprintf("speednew: %s\n", vtos(speed_new));
	//gi.dprintf("speedmax  %s\n", vtos(ent->client->speedmax));
}

void Set_Stance_Max(edict_t *ent)
{

	if (ent->s.frame == 293) ent->stance_max = -9;
	else if (ent->s.frame == 283) ent->stance_max = 24;//23;   //23 causes bugs
	else if (ent->s.frame == 284) ent->stance_max = 22;
	else if (ent->s.frame == 285) ent->stance_max = 20;
	else if (ent->s.frame == 286) ent->stance_max = 16;
	else if (ent->s.frame == 287) ent->stance_max = 11;
	else if (ent->s.frame == 288) ent->stance_max = 9;
	else if (ent->s.frame == 289) ent->stance_max = 9;
	else if (ent->s.frame == 290) ent->stance_max = 4;
	else if (ent->s.frame == 291) ent->stance_max = -3;
	else if (ent->s.frame == 292) ent->stance_max = -5;
/*	else if (ent->s.frame == 293) ent->stance_max = -9; weirdest bug ever*/
	else if (ent->s.frame == 294) ent->stance_max = -12;
	else if (ent->stanceflags == STANCE_DUCK) ent->stance_max = 4;
	else if (ent->stanceflags == STANCE_CRAWL) ent->stance_max = -15; //changing this to -16 causes weird  2 player crawl to sky bug


	else ent->stance_max = 24;//23;


}
			
	
void SetWaterLevel (edict_t *ent)
{
	vec3_t		point;
	int			cont;


//
// get waterlevel
//
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;	
	cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	if (ent->stanceflags == STANCE_CRAWL)
		point[2] += 4;
	else if (ent->stanceflags == STANCE_DUCK)
		point[2] +=18  ;
	else
		point[2] += 26;
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	if (ent->stanceflags == STANCE_CRAWL)
		point[2] += 3;
	else if (ent->stanceflags == STANCE_DUCK)
		point[2] +=12  ;
	else
		point[2] += 18;
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
	{
		ent->waterlevel = 3;

	//	if (ent->stanceflags == STANCE_CRAWL &&
		//	ent->client->v_angle[0] < -35)
		//	change_stance (ent, STANCE_STAND);
	}
}
	

void Count_Votes (void)
{
	int i;
	int highestvotecount = 0;
	int highmap = 0;
	
	for (i =0; i<5; i++)
	{
		if (mapvotes[i] > highestvotecount)
		{
			highestvotecount = mapvotes[i];
			highmap = i;
		}
	}

	level.changemap = votemaps[highmap];
}


qboolean Setup_Map_Vote (void)
{
	char		*maps;
	int			i, j, k, c;
	char		*s, *f = NULL;
	int			mapcount;
	int			newmapcount;
	int			x;
	int			randnum = 0;
	int			removed;
	char		*possible_maps[300];
	char		*maplisttxt[300];
	qboolean	gotmap;
	int			count = 0;
	qboolean	changefirstmap;
 	char		filename[MAX_OSPATH] = "";	// Knightmare added

//	maps = ReadEntFile("dday/votemaps.txt");
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/votemaps.txt", GameDir());
	maps = ReadEntFile(filename);

	mapcount = 0;

	if (maps)
	{
		c = 0;
	//	f = strdup (maps);
		f = G_CopyString (maps);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		while (c < 300)
		{
			if (s != NULL) 
			{
				if (MapExists(s))
				{
					maplisttxt[c] = s;
					mapcount++;
					c++;
				}
				else
					gi.dprintf("WARNING: Map '%s' in maplist.txt not found on server!\n",s);



				s = strtok (NULL, "\n");
			}
			else
			{maplisttxt[c] = ""; 
			c++;}

		}

	}
	else
	{
		gi.dprintf ("Problem loading vote maps!\n");
		return false;
	}

	removed = 0;
	//remove most recently played maps
	for (i = 0; i<20 && removed != mapcount-4 && last_maps_played[i]; i++) //for last_maps_played
	{
		for (j=0; j < mapcount; j++)
		{
			if (!strcmp (maplisttxt[j],""))
				continue;

			if (!strcmp (last_maps_played[i], maplisttxt[j]))
			{	
				maplisttxt[j]= "";
				//gi.dprintf("EEEEEEEE Removing %s\n",campaign_spots[checknum].bspname);
				removed++;
				break;
			}
		}
	} 


	newmapcount = 0;
	//sort possible_maps
	for (x=0; x < 300; x++)
	{
		if (!strcmp (maplisttxt[x],""))
			continue;
		possible_maps[newmapcount] = maplisttxt[x];

		newmapcount++;
	}

/*	for (j = 0; j<50; j++)	{
		if (possible_maps[j] > -1)	{
			gi.dprintf ("%s\n",campaign_spots[possible_maps[j]].bspname);
		}
	}	*/

	changefirstmap = false;
	for (i = 0; i < 4; i++)
	{
		if (i == 0 && level.nextmap)
		{
			//if nextmap is same as current map or one played recently, list that last instead of 4th
			if (!strcmp(level.nextmap, level.mapname))
			{
				changefirstmap = true;
				//votemaps[0] = level.nextmap;
				//continue;
			}
			for (k = 0; k<20 && last_maps_played[k]; k++)
			{
				if (!strcmp (last_maps_played[k], level.nextmap))
				{	
					changefirstmap = true;
					//votemaps[0] = votemaps[3];
					//votemaps[3] = level.nextmap;
				}
			}
			if (!changefirstmap)
			{
				votemaps[0] = level.nextmap;
				continue;
			}
		} 

		if (i==3 && changefirstmap)
		{
				votemaps[3] = level.nextmap;
				continue;
		}

	
		gotmap = false;
		count = 0;
		while (!gotmap && count < 1000)
		{
			count++;
			gotmap = true;
			randnum = (int)(random()*newmapcount);
			for (j = 0; j < i; j++)
			{
				if (!strcmp (possible_maps[randnum], votemaps[j]))
					gotmap = false;
			}
		}

		if (!gotmap){
			gi.dprintf ("Problem loading vote maps !\n");
			return false;
		}

		votemaps[i] = possible_maps[randnum];
		//gi.dprintf ("%i %s\n",randnum, votemaps[i]);
					
	}

	//if nextmap was played in last 10 maps, list it 4th
	if (level.nextmap)
	{

	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	return true;
}



void turret_driver_link (edict_t *self);
/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{

	gclient_t	*client;
	edict_t		*other;
	pmove_t		pm;
	int			i, j;
	int			 nWoundFrame;

	// dday
	qboolean oob_pitch=false;
	qboolean found = false;
	int CT_DUCKED=0;
	int pronedist=12;//8 causes getting stuck in doors;//faf 12;
	char cmd[MAX_CMD_BUFFER];

	// pbowens: more trace stuff
	vec3_t	start, dist;
	vec3_t	end = {0, 0, -8192};
	trace_t	tr;

	float time;


	level.current_entity = ent;
	client = ent->client;

	VectorCopy(ent->s.origin, start);	// initial value
	VectorAdd(start, end, end);			// add distance for end

	tr = gi.trace (start, ent->mins, ent->maxs, end, ent, MASK_SOLID);
	//tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);
	VectorSubtract(ent->s.origin, tr.endpos, dist);

//	gi.dprintf("%i %s\n", ent->waterlevel, vtos(ent->client->v_angle));


	if (!chile->value && ent->client && ent->client->pers.weapon)
	{ 
		time = 10 * (level.time - ent->client->last_fire_time);

		if (time < 30)
		{
			if (ent->client->pers.weapon->position == LOC_SNIPER)
			
			{
				if ((time < 20) && time != 0)
					ent->client->kick_angles[0] = ((2 * cos((time/4) -4)) * ((20 -time)/20));

			}
			else if (ent->client->pers.weapon->position == LOC_RIFLE)
			{
				if ((time < 2) && time != 0)
					ent->client->kick_angles[0] = ((2 * cos((time) -4)) * ((20 -time)/20));
			}
			else if (ent->client->pers.weapon->position == LOC_ROCKET)
			{
				if ((time < 2) && time != 0)
					ent->client->kick_angles[0] = ((2 * cos((time) -4)) * ((20 -time)/20));
			}
			else if (ent->client->pers.weapon->position == LOC_SHOTGUN)
			{
				if ((time < 2) && time != 0)
					ent->client->kick_angles[0] = ((2 * cos((time) -4)) * ((20 -time)/20));
			}
			else if (ent->client->pers.weapon->position == LOC_PISTOL)
			{
				if ((time < 2) && time != 0)
					ent->client->kick_angles[0] = ((3 * cos((time) -4)) * ((20 -time)/20));
			}
		}

	}




/*
	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (campaign_winner > -1)
		{
			if (level.time > level.intermissiontime + 20 && (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;
		}
		else if (level.time > level.intermissiontime + 10.0
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}*/



	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (campaign_winner > -1)
		{
			if (level.time > level.intermissiontime + 20 && (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;
		}
		else if ( (!mapvoting->value || level.map_vote_time == -1) && 
			level.time > level.intermissiontime + 8.0 && (constant_play->value || (ucmd->buttons & BUTTON_ANY)))
			level.exitintermission = true;

		else if (mapvoting->value && level.time > level.intermissiontime + 3.0
			&& (constant_play->value || (ucmd->buttons & BUTTON_ANY)) )
		{
			if (!level.map_vote_time)
			{
				if (Setup_Map_Vote())
					level.map_vote_time = level.time;
				else 
					level.map_vote_time = -1;
	}

			if (level.map_vote_time != -1 && !ent->client->vote_started)
			{
				MapVote(ent);
				ent->client->vote_started = true;
			}
		}
		if (level.map_vote_time > 0)
		{
			if (level.time > level.map_vote_time + 15)
			{
                Count_Votes ();
				level.exitintermission = true;
			}
			else if (level.time > level.intermissiontime + 6.0 &&
				level.last_vote_time < level.time - 1 &&
				mapvotes[0]+mapvotes[1]+mapvotes[2]+mapvotes[3] >= num_clients)
			{
                Count_Votes ();
				level.exitintermission = true;
			}
		}

		return;
	}




	//DDAY

	if (ent->client->resp.scopewobble < 40)
	{
		++ent->client->resp.scopewobble;
		if (ent->client->resp.scopewobble > 10)
		{
			// X Axis
			ent->client->kick_angles[0] = sin(ent->client->resp.scopewobble * 10) * (1 - (float)(ent->client->resp.scopewobble) / 40) * 2.5;
			// Y Axis
			ent->client->kick_angles[1] = sin((ent->client->resp.scopewobble + 45) * 6) * (1 - (float)(ent->client->resp.scopewobble) / 40) * 2.5;
			// Slight crosshair rotation
			//ent->client->kick_angles[2] = sin((ent->client->resp.scopewobble - 30) * 8) * (1 - (float)(ent->client->resp.scopewobble) / 40) * 1;
		}
	}

  //these are the premove routines needed to make sure things work the way they are supposed to...
	if (client->limbo_mode && ent->flyingnun && !ent->client->resp.team_on)
	{
		if (!client->chasetarget)
		{
			ucmd->forwardmove *= 3;
			ucmd->sidemove *= 3;
			ucmd->upmove *= 3;
		}
		else
		{
			ucmd->forwardmove	 = 0;
			ucmd->sidemove		 = 0;
			ucmd->upmove		 = 0;
		}
	}
	else if (client->limbo_mode)
	{
		VectorSet(ucmd->angles,0,0,0);

		ucmd->forwardmove	 = 0;
		ucmd->sidemove		 = 0;
		ucmd->upmove		 = 0;
	}
	else 
	{
//		if ( (ent->velocity[0] != 0 || ent->velocity[1] != 0) && ent->wound_location == LEG_WOUND )
//		{
//			ent->client->kick_angles[0] += ( sin(level.framenum  * 0.5) * ( abs(ucmd->forwardmove) + abs(ucmd->sidemove) ) / 190 );
//			ent->client->kick_angles[1] += ( sin(level.framenum  * 1.0) * ( abs(ucmd->forwardmove) + abs(ucmd->sidemove) ) / 190 );
//		}

	//faf: reenabling this (old limp system)
		if (ent->wound_location == LEG_WOUND
			&& ent->client->movement
			&& ent->stanceflags == STANCE_STAND)
		{
			nWoundFrame = level.framenum % 6;
			if (nWoundFrame == 1)
				ent->client->kick_angles[0] += .3;
			else if (nWoundFrame == 2)
				ent->client->kick_angles[0] += .2;
			else if (nWoundFrame == 3)
				ent->client->kick_angles[0] += .1;

	//faf			ucmd->forwardmove *= .9;
	//faf			ucmd->sidemove *= .9;
		

	}


//		if (ent->client->aim && VectorLength(dist) > 25 && ent->velocity[2] != 0)
// Nick 01/05/2003 - added && ent->stanceflags == STANCE_STAND to fix slope bug
 //       if (ent->client->aim && VectorLength(dist) > 25 && ent->velocity[2] != 0
   //            && ent->stanceflags == STANCE_STAND)
//		{
			//gi.dprintf("kicked out: %f %s %s\n", VectorLength(dist), vtos(dist), vtos(ent->velocity));
	//		Cmd_Scope_f(ent);
///		}

		// jump cheat fix
/*
		gi.dprintf("1 %3i (up %3i) jump_stuck: %5s jump_last: %5s %s\n",
			(int)client->jump_stamina , ucmd->upmove,
			(client->jump_stuck) ? "true" : "false",
			(client->jump_last)  ? "true" : "false",
			vtos(ent->velocity) );
*/
		if (client->jump_last && ent->velocity[2] <= 0) 
			client->jump_last = false;
			
		if (client->jump_stuck && !client->jump_last)
			ucmd->upmove = 0;
/*
		gi.dprintf("2 %3i (up %3i) jump_stuck: %5s jump_last: %5s %s\n",
			(int)client->jump_stamina , ucmd->upmove,
			(client->jump_stuck) ? "true" : "false",
			(client->jump_last)  ? "true" : "false",
			vtos(ent->velocity) );
*/
		if (ucmd->forwardmove != 0 || ucmd->sidemove != 0 || ucmd->upmove != 0)
			client->movement = true;
		else
			client->movement =  false;

		//faf:  for Parts' running anim
		if (ucmd->sidemove > 0 && ucmd->forwardmove == 0)
			client->sidestep_anim = MOVE_LEFT;
		else if (ucmd->sidemove < 0 && ucmd->forwardmove ==0)
			client->sidestep_anim = MOVE_RIGHT;
		else if (ucmd->forwardmove > 0)
			client->sidestep_anim = MOVE_FORWARD;
		else if (ucmd->forwardmove < 0) 
			client->sidestep_anim = MOVE_BACKWARD;
		else
			client->sidestep_anim = 0; //end faf




		// ClientSetMaxSpeed() should update cl_ prediction vars to reflect ucmd
		// theoretically, none of these should actually be used, but there are 
		// potential cheaters out there and the possibility of the stuffcmd NOT
		// going through (crosshair hack interference)

		//gi.dprintf("f%i s%i u%i\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
		//gi.dprintf(" jump_push: %i\n",  client->jump_push);
        //gi.dprintf("    upmove: %i\n",  ucmd->upmove);

		if (ucmd->forwardmove >  (ent->client->speedmax[0] * 2)) // run
			ucmd->forwardmove =   ent->client->speedmax[0];
		
		if (ucmd->forwardmove <  -(ent->client->speedmax[0] * 2)) // run
			ucmd->forwardmove =   -ent->client->speedmax[0];

		if (ucmd->sidemove    >  (ent->client->speedmax[1] * 2)) // run
			ucmd->sidemove    =   ent->client->speedmax[1];

		if (ucmd->sidemove    <  -(ent->client->speedmax[1] * 2)) // run
			ucmd->sidemove    =   -ent->client->speedmax[1];


		if (client->jump_stamina <= JUMP_MINIMUM) 
		{
			if (ent->velocity[2] != 0 && !client->jump_stuck) 
				client->jump_last = true;

			client->jump_stuck = true;

			//ClientSetMaxSpeed(ent, true);
			//ucmd->upmove = 0;
		}
		else 
		{
			if (client->jump_stuck) 
			{
				client->jump_stuck = false;
				WeighPlayer(ent);
//				ClientSetMaxSpeed(ent, true);
			}
		}
/////////
//				gi.dprintf(	" buttons:    %i\n"
//					" oldbuttons: %i\n"
//					" latched:    %i\n",
//			client->buttons, client->oldbuttons, client->latched_buttons);
				/////////////////////////
		
		if (ent->stanceflags != STANCE_STAND && ent->waterlevel != 3) 
		{
			ucmd->upmove -= 300; // always crouching

			if (ent->stanceflags == STANCE_CRAWL)
				pronedist *= 2; // double 'length'
		}
		else 
		{
			ent->stance_max=24;//faf32;
		}


		if (ent->stanceflags != STANCE_STAND && ent->waterlevel == 3)
			if (ucmd->upmove > 0)
                	change_stance (ent, STANCE_STAND);
	

	}

	if (ucmd->buttons & BUTTON_ATTACK) 
	{
		if (ent->client->display_info)
		{
			if (ent->client->layout_type == SHOW_CAMPAIGN)
				ent->client->layout_type = SHOW_NONE;

			if (ent->client->layout_type == SHOW_SERVERIMG)
				ent->client->layout_type = SHOW_NONE;


			ent->client->display_info = false;
			//faf
			if (!ent->client->resp.team_on)
				SwitchToObserver(ent);
		}

		if (ent->client->aim &&  (
			(ent->stanceflags == STANCE_CRAWL && ent->client->pers.weapon && ent->client->pers.weapon->position != LOC_GRENADES) ||
			ent->client->pers.weapon && ent->client->pers.weapon->classnameb == WEAPON_MORPHINE) )			
		{
			// pbowens: this directly undermines the purpose of ClientSetMaxSpeed :(
			for (i = 0; i < 3; i++) {
				if (ent->client->speedmax[i] != 0) {
					ent->client->speedmax[i]  = 0;

					ent->client->syncspeed = true;
					ucmd->upmove = ucmd->sidemove = ucmd->forwardmove = 0;
				}
			}
		}
	} else
		WeighPlayer(ent);
//		ClientSetMaxSpeed(ent, true);
	


	// spits out commands to make sure they are taken
	if (level.time >= ent->client->cmdtime || client->syncspeed == true) 
	{
		if (client->syncspeed) 
		{
			//faf : player properly frozen in death view
			if (ent->client->limbo_mode &&
					!ent->flyingnun) //observer mode
			{
				ent->client->speedmax[0]=0;
				ent->client->speedmax[1]=0;
				ent->client->speedmax[2]=0;
			}

			// be careful with this. it can overflow clients if used too much
			Com_sprintf (cmd, sizeof(cmd), "cl_forwardspeed %i; cl_sidespeed %i; cl_upspeed %i;",
				ent->client->speedmax[0],
				ent->client->speedmax[1], 
				ent->client->speedmax[2]);

			client->syncspeed = false;

			client->cmdtime = CMD_DELAY + level.time;
			stuffcmd(ent, cmd);
		}
	/*	else
		{
			// *really* need to do transparent crosshair instead
			if (client->crosshair)
				Q_strncpyz (cmd, sizeof(cmd), "crosshair 1");
			else
				Q_strncpyz (cmd, sizeof(cmd), "crosshair 0");
		} */
	}

	
	//ClientSetMaxSpeed(ent, true);

	//END DDAY

	pm_passent = ent;
	
	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (client->turret)
		client->ps.pmove.pm_type = PM_NORMAL;

	else if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != (MAX_MODELS-1))
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	//DDAY
	else if (client->limbo_mode)
		client->ps.pmove.pm_type = PM_FREEZE;
	//END DDAY
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	//DDAY
	if (!client->landed && client->resp.mos == SPECIAL)
	{
		//gi.dprintf("groundentity: %s\n", (ent->groundentity) ? "true" : "false");

		if (ent->groundentity || ent->velocity[2] > 0)//faf: velocity check for water landings
		{
			client->ps.pmove.gravity = sv_gravity->value;
		}
		else
		{
		//	ucmd->upmove += -200;
			client->ps.pmove.gravity = .25 * (sv_gravity->value) ; //parchute factor
//faf			client->landed = false;
		}
	}
	else
		client->ps.pmove.gravity = sv_gravity->value;
	//END DDAY


	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{

		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}


	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	//DDAY
	if (!ent->deadflag)
	{

		if (ent->s.frame > 197 &&
		ent->s.frame < 235 ||
		ent->s.frame > 290)
			pronedist = 24;
		else
			pronedist = 16;

		pm.maxs[1]		= pronedist;
		pm.mins[1]		= (-pronedist);
		pm.maxs[0]		= pronedist;
		pm.mins[0]		= (-pronedist);
		
		Set_Stance_Max (ent);
		pm.maxs[2]		= ent->stance_max;

		//gi.dprintf("maxs  %f\n", pm.maxs[2]);
		//gi.dprintf("maxs  %s\n", vtos (pm.maxs));
		//gi.dprintf("mins  %s\n", vtos (pm.mins));
	}
	//END DDAY
	//

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	client->jump_push = false;//faf

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		if (!ent->burnout) //on fire
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);


		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		
		//if (!ent->ai)
			client->jump_stamina -= JUMP_TAKE;//faf

		ucmd->upmove      = ent->client->speedmax[2];//faf
		client->jump_push = true;

		client->last_jump_time = level.time;//faf

		//faf
		if (ent->client->aim == true &&
			ent->client->pers.weapon &&
			ent->client->pers.weapon->position != LOC_KNIFE  &&
			ent->client->pers.weapon->position != LOC_HELMET &&
			ent->client->pers.weapon->classnameb != WEAPON_BINOCULARS)

		{
			ent->client->aim = false;

			check_unscope(ent);//faf

			ent->client->ps.fov = STANDARD_FOV;
		}

		turret_off (ent);

	}

//	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	

	SetWaterLevel(ent);


//gi.dprintf("%i\n",ent->waterlevel);






	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
/*		client->ps.viewangles[ROLL] = client->killer_yaw;	//40
		client->ps.viewangles[PITCH] = client->killer_yaw;	//-15
		client->ps.viewangles[YAW] = client->killer_yaw; */
	}
	else
	{


		VectorCopy (pm.viewangles, client->v_angle);		
		VectorCopy (pm.viewangles, client->ps.viewangles);
/* faf
		//Wheaty: Limit the PITCH angle when in prone mode
		if (ent->stanceflags == STANCE_CRAWL) //prone
		{
			if (client->ps.viewangles[PITCH] < -25)
			{
				client->ps.viewangles[PITCH] = -25;
				client->ps.gunangles[PITCH]  = -25;
			}
		} */

		//DDAY
		/*
		if (ent->oldwaterlevel != ent->waterlevel)
		{
			if (ent->waterlevel == 3 && ent->stanceflags != STANCE_STAND) 
				change_stance(ent, STANCE_STAND);

			ent->oldwaterlevel = ent->waterlevel;
			ent->oldwatertype  = ent->watertype;
			WeighPlayer(ent); // Reset Speed Modifier
		}
		*/
		//END DDAY
	}





	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];

		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	/*
	gi.dprintf( "oldbuttons: %i\n   buttons: %i\n   latched: %i\n",
		client->oldbuttons,
		client->buttons,
		client->latched_buttons);
	*/
	if (client->heldfire && !(client->buttons & BUTTON_ATTACK))
		client->heldfire = false;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;



	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (!client->weapon_thunk) 
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client->weaponstate != client->weaponstate_last) {
		if (client->weaponstate_last == WEAPON_RAISE ||			
			client->weaponstate_last == WEAPON_LOWER )
		{
//			ClientSetMaxSpeed(ent, true);
			WeighPlayer(ent);
		}
		
		client->weaponstate_last = client->weaponstate;
	}
	
	// DDAY
#if 0	// Check to see if they are still feeding ammo...
		if (ent->client->feedammo == true)
			Feed_Ammo(ent);
#endif //0


	/*faf:  moved to somewhere else
	// Do Jump-Stamina increase
	    if (client->jump_stamina < JUMP_MAX)
			client->jump_stamina += JUMP_REGEN;
*/
		if (ent->die_time &&
			!ent->client->movement)
			ent->die_time = level.time + .5;


	// Check to see if its time to die from a wound...		
		if ( (ent->die_time) && (level.time > ent->die_time))//faf
		{
			int temp_damage;

			if (ent->wound_location & STOMACH_WOUND)
				temp_damage = 2;
			if (ent->wound_location & CHEST_WOUND)
				temp_damage = 4;
			else
				// rezmoth - changed to 0 to prevent leg wound dmg
				temp_damage = 0;

			if (ent->client->last_wound_inflictor)
			T_Damage (ent, ent->client->last_wound_inflictor, ent->client->last_wound_inflictor, ent->maxs, ent->s.origin, NULL,temp_damage, 0,  DAMAGE_NO_PROTECTION,
				MOD_WOUND);

			// rezmoth - made bleed interval random
			//ent->die_time = level.time + (crandom() + 1) * 2;
			ent->die_time = level.time + 2;



			//faf: making it so you only bleed when you are moving
			if (!ent->client->bleedwarn && temp_damage != 0)
			{
				safe_cprintf(ent, PRINT_HIGH, "You are bleeding!  Stop moving and wait for a medic!\n");
				ent->client->bleedwarn = true;
			}
		}

		// rezmoth - the infamous nade bug lies below
		// Check to see if they have a live grenade...
		//if (ent->client->grenade_time) {
/*		if (false) {
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}
		}
*/

		/*
		if (client->grenade) // disable the grenade from triggering self, but still leave timers on
		{
			VectorCopy(ent->s.origin, client->grenade->s.origin);
			client->grenade->s.modelindex = 0; // set model to null
			//client->grenade_caught->touch = NULL;
			client->grenade->s.origin[2]+= 100;//faf:  this helps fix nade slope bug
		}*/


//bcass start - TNT
		if (client->tnt)
		{
			VectorCopy(ent->s.origin, client->tnt->s.origin);
			client->tnt->s.modelindex = 0; // set model to nulllient->tnt_blew_up = true;
		}

//bcass end
		
	//JABot[start]
	AITools_DropNodes(ent);
	//JABot[end]


/*		if (ent->waterlevel > 1)
		{
			if (ent->stanceflags != STANCE_STAND)
				change_stance(ent,  STANCE_STAND);
		}
*/

	//END DDAY
}

edict_t *Nearest_Player(edict_t *ent)
{
	int i;
    edict_t *e;
	edict_t *nearest = NULL;
	float temp_distance, nearest_distance = 9999999;
	vec3_t dist;


	for (i=0 ; i < game.maxclients ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse || !e->client || !e->client->resp.team_on)
			continue;
		if (e == ent)
			continue;
		
		VectorSubtract (e->s.origin, ent->s.origin, dist);
		
		temp_distance = VectorLength(dist);
		if (temp_distance < nearest_distance)
		{
			nearest_distance = temp_distance;
			nearest = e;
		}
	}

	if (nearest)
		return nearest;
	else 
		return NULL;
}
	
   



void EndObserverMode(edict_t *ent);
/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;
	int delay;

	edict_t *chase;

	if (level.intermissiontime)
		return;

	client = ent->client;

//		gi.dprintf("    %i\n", ent->client->ps.gunframe);//faf test


	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;




		if (!ent->groundentity)
		{
			ent->client->in_air = true;
		}
		else
		{
			if (ent->client->in_air)//landed
			{
				ent->client->footstep_framenum = level.framenum;
				if (ent->stanceflags == STANCE_STAND)
					Play_Footstep_Sound(ent);
				ent->client->in_air = false;
			}
		}







	//faf:  moved from clientthink
	if (client->grenade) // disable the grenade from triggering self, but still leave timers on
	{
		VectorCopy(ent->s.origin, client->grenade->s.origin);
		client->grenade->s.modelindex = 0; // set model to null
		//client->grenade_caught->touch = NULL;
		client->grenade->s.origin[2]+= 58;//faf:  this helps fix nade slope bug

		//faf .11
		if (gi.pointcontents(ent->client->grenade->s.origin) & MASK_SOLID)
		{
			client->grenade->s.origin[2]-= 58;

		}

	}

	/*
 	if (client->jump_stamina <= JUMP_MINIMUM  &&
		level.framenum%13 ==0)		 
	{
		stuffcmd(ent, "play inland/heart.wav");
	}*/

	if (ent->flyingnun)
	{
		if (ent->client->chasetarget && !ent->client->chasetarget->inuse)
			ent->client->chasetarget=NULL;


		if (client->latched_buttons & BUTTON_ATTACK)
		{
			if (ent->client->chasetarget)
				ent->client->chasetarget = NULL;
			else 
			{
				/*for (n = 1; n <= maxclients->value; n++)
				{
					player = &g_edicts[n];

					if (!player->inuse)
						continue;
					if (!player->client)
						continue;
					if (!player->client->resp.team_on)
						continue;

					chase = player;
				}*/
				chase = Nearest_Player(ent);
				if (!chase)
				{
					//safe_cprintf (ent, PRINT_HIGH, "No one to chase.\n");
					ent->client->chasetarget = NULL;
					
				}
				else
					ent->client->chasetarget= chase;
			}
		}
		client->latched_buttons = 0;
		return;
	}




//faf:  players press fire to bring up class or team menu when they need it
	else if (ent->client->limbo_mode)
	{
		if (client->latched_buttons & BUTTON_ATTACK)
		{
			delay = 0;
			if (ent->client->resp.team_on)
				delay = ent->client->resp.team_on->delay;


			if (!ent->client->resp.team_on)
				MainMenu(ent);
			else if (!ent->client->resp.mos &&
				!ent->client->resp.newmos)
			{
				M_ChooseMOS(ent);
				client->latched_buttons = 0;
			}
			if (level.framenum > (10 * (int)(level_wait->value  + delay)) &&
			ent->leave_limbo_time < level.time - .1)
			{
				EndObserverMode(ent); //faf
				client->latched_buttons = 0;
			}
		}
	}




	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			//gi.dprintf("%i : %i (%i)\n", level.framenum, client->forcespawn, client->limbo_mode);
			if ( ( client->latched_buttons & buttonMask ) ||
				 ( deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) ||
				 ( client->forcespawn <= level.framenum && client->limbo_mode == false && client->resp.changeteam == false) )
			{
				if (ent->client->resp.changeteam || ent->ai)
				{
					if (ent->client->forcespawn < level.framenum)
					{
						respawn(ent);
						client->latched_buttons = 0;
					}
				}
				else
				{
					respawn(ent);
					client->latched_buttons = 0;
				}
			}
		}
		return;
	}





	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;
	
	delay = 0;
	if (ent->client->resp.team_on)
		delay = ent->client->resp.team_on->delay;

	if (level.framenum > (10 * (int)(level_wait->value  + delay))  &&
	(ent->client->limbo_mode) &&
	(ent->leave_limbo_time < level.time) &&
	(ent->client->menu == 0))  // so you dont spawn while choosing a class
	{
		if (ent->client->resp.team_on)
		{
				if (ent->client->resp.newmos ||
				ent->client->resp.mos)
				{
					EndObserverMode(ent); //faf
				}
				else if (!ent->client->display_info)
					ChooseTeam(ent);
		}
//		else if	(!ent->client->display_info)
//			MainMenu(ent);
		
	}	
}


void Write_Player_Stats (edict_t *ent)
{
	char	statsfilename[MAX_QPATH] = "";
	char	*ip;
	int		c;
	char	*s, *f=NULL;
	char	*statsc;
	char	*name;
	int		games = 0;
	int		ping = 0;
	int		human_kills = 0;
	int		human_deaths = 0;
	int		bot_kills = 0;
	int		bot_deaths = 0;
	int		games_won = 0;
	int		games_lost = 0;
	int		played_allies = 0;
	int		played_axis = 0;
	int		infantry = 0;
	int		officer = 0;
	int		lgunner = 0;
	int		hgunner = 0;
	int		sniper = 0;
	int		special = 0;
	int		engineer = 0;
	int		medic = 0;
	int		flamer = 0;
	int		castrations = 0;
	int		helmets = 0;
	int		fists = 0;
	char	*chat;
 	char	filename[MAX_OSPATH] = "";
	FILE	*fn;

//	gi.dprintf ("Write_Player_Stats\n");

	if (ent->ai)
		return;

	// dont count maps that end really fast
//	if (level.time < 60)
//		return;

	ip = ent->client->pers.ip;

	if (!ip)
		return;

//	Com_sprintf (statsfilename, sizeof(statsfilename), "dday/stats/%s.stats", ip);
	// Knightmare- use SavegameDir() instead for compatibility on all platforms
	Com_sprintf (statsfilename, sizeof(statsfilename), "%s/stats/%s.stats", SavegameDir(), ip);

    statsc = ReadEntFile(statsfilename);

	if (statsc)
	{
		c = 0;
	//	f = strdup (statsc);
		f = G_CopyString (statsc);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");

		if (s != NULL) {
			name = s;
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			games = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			ping = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			human_kills = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			human_deaths = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			bot_kills = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			bot_deaths = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			games_won = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			games_lost = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			played_allies = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			played_axis = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			infantry = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			officer = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			lgunner = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			hgunner = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			sniper = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			special = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			engineer = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			medic = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			flamer = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			castrations = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			helmets = atoi (s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			fists = atoi (s);
			s = strtok (NULL, "\n");
		}	
		if (s != NULL) {//not used here
			chat = s;
			s = strtok (NULL, "\n");
		}

	}

	if (level.intermissiontime && ent->client->resp.team_on)
	{
		if (ent->client->resp.team_on->index == Last_Team_Winner)
			games_won++;
		else
			games_lost++;

		if (ent->client->resp.team_on->index == 0)
			played_allies++;
		else
			played_axis++;


		if (ent->client->resp.mos)
		{
			if (ent->client->resp.mos == INFANTRY)
				infantry++;
			if (ent->client->resp.mos == OFFICER)
				officer++;
			if (ent->client->resp.mos == L_GUNNER)
				lgunner++;
			if (ent->client->resp.mos == H_GUNNER)
				hgunner++;
			if (ent->client->resp.mos == SNIPER)
				sniper++;
			if (ent->client->resp.mos == SPECIAL)
				special++;
			if (ent->client->resp.mos == ENGINEER)
				engineer++;
			if (ent->client->resp.mos == MEDIC)
				medic++;
			if (ent->client->resp.mos == FLAMER)
				flamer++;

		}

	}

// if (level.intermissiontime)
	games = games + 1;

	//plus_minus = ent->client->resp.plus_minus;
	//plus_minus = ent->client->resp.stat_plus - ent->client->resp.stat_minus;

/*	if (plus_minus > 10)
		plus_minus = 10;
	if (plus_minus < -10)
		plus_minus = -10; 

	average = ((average * (games - 1)) + plus_minus)/games;	 */

//	gi.dprintf ("%i %f\n", deaths, ratio);

	human_kills = human_kills + ent->client->resp.stat_human_plus;
	human_deaths = human_deaths + ent->client->resp.stat_human_minus;
	bot_kills = bot_kills + ent->client->resp.stat_bot_plus;
	bot_deaths = bot_deaths + ent->client->resp.stat_bot_minus;

	castrations = castrations + ent->client->resp.stat_castrations;
	helmets = helmets + ent->client->resp.stat_helmets;
	fists = fists + ent->client->resp.stat_fists;

//	Com_sprintf (filename, sizeof(filename), "dday/stats/%s.stats", ip);
	// Knightmare- use SavegameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/stats/%s.stats", SavegameDir(), ip);

	fn = fopen (filename, "w");
	if (!fn)
	{
		gi.error ("Couldn't open %s, you may need to create a 'dday/stats' folder.", filename);
	}

	fprintf (fn, "%s\n", ent->client->pers.netname);
	fprintf (fn, "%i\n", games);
	fprintf (fn, "%i\n", ent->client->ping);
	fprintf (fn, "%i\n", human_kills);
	fprintf (fn, "%i\n", human_deaths);
	fprintf (fn, "%i\n", bot_kills);
	fprintf (fn, "%i\n", bot_deaths);
	fprintf (fn, "%i\n", games_won);
	fprintf (fn, "%i\n", games_lost);
	fprintf (fn, "%i\n", played_allies);
	fprintf (fn, "%i\n", played_axis);
	fprintf (fn, "%i\n", infantry);
	fprintf (fn, "%i\n", officer);
	fprintf (fn, "%i\n", lgunner);
	fprintf (fn, "%i\n", hgunner);
	fprintf (fn, "%i\n", sniper);
	fprintf (fn, "%i\n", special);
	fprintf (fn, "%i\n", engineer);
	fprintf (fn, "%i\n", medic);
	fprintf (fn, "%i\n", flamer);
	fprintf (fn, "%i\n", castrations);
	fprintf (fn, "%i\n", helmets);
	fprintf (fn, "%i\n", fists);
	fprintf (fn, "%s\n", ent->client->pers.stat_chat);


	fclose (fn);

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
}

//writes players stat average from .stat file to pers
void SetPlayerRating (edict_t *ent)
{
	char	statsfilename[MAX_QPATH] = "";
	int		c;
	char	*s, *f = NULL;
	char	*statsc;
	char	*name;
	float	ratio = 0.0;
	int		games = 0;
	int		ping = 0;
	int		human_kills = 0;
	int		human_deaths = 0;
	int		bot_kills = 0;
	int		bot_deaths = 0;
	char	*chat = "";
	int		i;

	if (!ent->client->pers.ip)
		return;

//	Com_sprintf (statsfilename, sizeof(statsfilename), "dday/stats/%s.stats", ent->client->pers.ip);
	// Knightmare- use SavegameDir() instead for compatibility on all platforms
	Com_sprintf (statsfilename, sizeof(statsfilename), "%s/stats/%s.stats", SavegameDir(), ent->client->pers.ip);

//	gi.dprintf ("%s\n", statsfilename);

    statsc = ReadEntFile(statsfilename);

	if ( !statsc )
		return;

	if (stats->value)
	{
		c = 0;
	//	f = strdup (statsc);
		f = G_CopyString (statsc);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");


		if (s != NULL) {
			name = s;
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			games = atoi (s);
			//gi.dprintf ("s: %i\n",games);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			ping = atoi (s);
			//gi.dprintf ("s: %i\n",games);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			human_kills = atoi (s);
			//gi.dprintf ("s: %i\n",kills);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			human_deaths = atoi (s);
			//gi.dprintf ("s: %f\n",deaths);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			bot_kills = atoi (s);
			//gi.dprintf ("s: %i\n",kills);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			bot_deaths = atoi (s);
			//gi.dprintf ("dfssdf s: %i\n",bot_deaths);
			s = strtok (NULL, "\n");
		}
		//don't need next 16 stats
		for (i=0; i<16; i++){
			s= strtok (NULL, "\n");
		}
		if (s != NULL) {
			chat = s;
			s = strtok(NULL, "\n");
		}


	}

//g	i.dprintf ("ratio: %f\n",ratio);
				
	ent->client->pers.stat_human_kills = human_kills;
	ent->client->pers.stat_bot_kills = bot_kills;
	ent->client->pers.stat_human_deaths = human_deaths;
	ent->client->pers.stat_bot_deaths = bot_deaths;

	Com_sprintf (ent->client->pers.stat_chat, sizeof(ent->client->pers.stat_chat), "%s", chat);
//	ent->client->pers.stat_chat[0] = '\0';	

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
}
