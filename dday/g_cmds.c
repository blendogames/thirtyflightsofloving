/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_cmds.c,v $
 *   $Revision: 1.62 $
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

#include "g_local.h"
#include "m_player.h"
#include "g_cmds.h"
#include "stddef.h"
//#include "p_classes.h"

void EndObserverMode(edict_t *ent);
void Show_Mos(edict_t *ent);
qboolean Cmd_Scope_f(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void SwitchToObserver(edict_t *ent);
void SyncUserInfo(edict_t *ent, qboolean pers);
void ClientSetMaxSpeed (edict_t *ent, qboolean sync);
void check_unscope (edict_t *ent);//faf
void Cmd_Spot(edict_t *ent);
void Cmd_RemoveDoors(edict_t *ent);
void ShowLinks(edict_t *ent);
void SV_Remove_Node(edict_t *ent);

#define MEDIC_CALL_TIME 30

// Knightmare- moved this var here to fix compile on GCC
g_cmds_t id_GameCmds[NUM_ID_CMDS];

extern	int			GlobalAliciaModeVariable;
void ClientBeginDeathmatch (edict_t *ent);

// Ref/Observer Mode
void Cmd_FlyingNunMode_f(edict_t *ent)
{
	char* pw = NULL;
	pw = gi.args();


//		if (ent->client->resp.AlreadySpawned)
//	{
//		safe_cprintf(ent, PRINT_HIGH, "Observer Mode is only available before you join a team.\n");
//		return;
//	}

/*	if (Q_stricmp(flyingnun_password->string, "") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Observer Mode Not Available\n");
		return;
	}
	else
	if (Q_stricmp(flyingnun_password->string, pw) != 0
		&& !ent->flyingnun) //faf
	{
		safe_cprintf(ent, PRINT_HIGH, "Observer Mode Password Incorrect\n");
		return;
	}
*/


	if (ent->flyingnun)
	{
		ent->flyingnun = false;
		ent->client->chasetarget = NULL;
		safe_bprintf (PRINT_HIGH, "%s is no longer an Observer.\n", ent->client->pers.netname);

		ent->client->resp.mos = NONE;
		ent->client->resp.team_on = NULL;
		ent->client->resp.AlreadySpawned = false;
		InitClientResp (ent->client);
		PutClientInServer (ent);
		SwitchToObserver(ent);
	}
	else
	{
		if (!ent->deadflag && !ent->client->limbo_mode && ent->health > 0)
		{
			ent->flags &= ~FL_GODMODE;
			ent->health = 0;
			meansOfDeath = MOD_SUICIDE;
			player_die (ent, ent, ent, 100000, vec3_origin);
		}

		ent->stance_view=20;//faf 22;
		ent->stanceflags = STANCE_STAND;

		ent->flyingnun = true;
		safe_bprintf (PRINT_HIGH, "%s is now an Observer.\n", ent->client->pers.netname);
		safe_cprintf (ent,PRINT_HIGH, "Type \"Observer\" to leave observer mode.\n");

		//faf
		ent->client->layout_type = SHOW_NONE;


		//faf
		ent->client->speedmax[0]=120;
		ent->client->speedmax[1]=120;
		ent->client->speedmax[2]=120;
		ent->client->syncspeed = true;
		ent->client->display_info = false;


		if (ent->ai)
		{
			ent->client->speedmax[0]=240;
			ent->client->speedmax[1]=240;
			ent->client->speedmax[2]=240;
		}

//		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		/*faf: needs work

		if (ent->health > 0)
		{
			ent->flags &= ~FL_GODMODE;
			ent->health = 0;
			meansOfDeath = MOD_SUICIDE;
			player_die (ent, ent, ent, 100000, vec3_origin);
		}


		ent->s.sound = 0;
		ent->s.modelindex = 0;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		ent->client->resp.team_on = 0;
		ent->client->resp.mos = 0;
		gi.linkentity (ent);  */


		ent->s.sound = 0;
		ent->s.modelindex = 0;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		ent->client->resp.team_on = NULL;
		ent->client->resp.mos = NONE;


	}
}

void Cmd_AliciaMode_f(edict_t *ent) // made it legible (pbowens 07/12/99)
{
	edict_t *tempent;
	int i;

	if (easter_egg->value==0)
		return;

	// undocumented "feature" make everyone in the game say it.
//	if (GlobalAliciaModeVariable) //once per a game
//		return;
	GlobalAliciaModeVariable = 1;

	for (tempent = &g_edicts[0], i = 0; i < game.maxentities; i++, tempent++)
	{
		if (tempent->inuse && tempent->client)
			stuffcmd(tempent,"say Alicia Silverstone Is the Loveliest Actress In the Whole Universe!");
	}
}


void Cmd_SexPistols_f(edict_t *ent)
{
	if (easter_egg->value==0)
		return;

	//undocumented feature that makes gunshots sound like sex.
	ent->sexpistols = (ent->sexpistols) ? false:true;

	if (ent->sexpistols)
		safe_cprintf(ent, PRINT_HIGH, "%s\n", "You have the Sex Pistols!");
}

g_cmds_t id_GameCmds[NUM_ID_CMDS] = // remember to set back to NUM_ID_COMDS
{
    "use",			1,	Cmd_Use_f,
    "drop",			1,	Cmd_Drop_f,
	"give",			1,	Cmd_Give_f,
	"god",			1,	Cmd_God_f,
	"notarget",		1,	Cmd_Notarget_f,
	"noclip",		1,	Cmd_Noclip_f,
	"help",			1,	Cmd_Help_f,
	"scoreboard",	1,	Cmd_Help_f,
	"inven",		1,	Cmd_Inven_f,
	"invnext",		2,	SelectNextItem,
	"invprev",		2,	SelectPrevItem,
	"invuse",		1,	Cmd_InvUse_f,
//	"invdrop",		1,	Cmd_InvDrop_f,
	"weapprev",		1,	Cmd_WeapPrev_f,
	"weapnext",		1,	Cmd_WeapNext_f,
	"kill",			1,	Cmd_Kill_f,
	"putaway",		1,	Cmd_PutAway_f,
	"wave",			1,	Cmd_Wave_f,
	"gameversion",	1,	Cmd_GameVersion_f,

	"stance",		1,	Cmd_Stance,
//	"airstrike",	1,	Cmd_Airstrike_f,
	"arty",			1,	Cmd_Arty_f,
	"attack2",			1,	Cmd_Arty_f,
	"class",		1,	Cmd_Menu_Class_f, //Cmd_MOS,
//	"create_team",	1,	Cmd_Create_Team,
	"join_team",	1,	Cmd_Menu_Team_f,//Cmd_Join_team,
	"team",			1,	Cmd_Menu_Team_f,//Cmd_Join_team,
	"main",			1,	Cmd_Menu_Main_f,//Cmd_Join_team,
	"menu",			1,	Cmd_Menu_Main_f,//Cmd_Join_team,
	"list_team",	1,	Cmd_List_team,
//	"quit_team",	1,	Cmd_Quit_team,
//	"feed_ammo",	1,	Feed_Ammo,
	"reload",		1,	(void *)Cmd_Reload_f,
//	"begin_mission",1,	EndObserverMode,
	"scope",		2,	Cmd_Scope_f,
	"shout",		3,	Cmd_Shout_f,
	"aliciamode",	1,	Cmd_AliciaMode_f,
	"iwannabeanarchy",1,Cmd_SexPistols_f,
	"observer",		1,	Cmd_FlyingNunMode_f,
//	"printcmds",	0,	PrintCmds,
//	"ddhelp",		1,	Cmd_DDHelp_f,
	"binds",		1,	Cmd_DDHelp_f,
	"ddebug",		1,	Cmd_DDebug_f,
	"maplist",		1,	Cmd_Maplist_f,
	"autopickup",	1,	Cmd_AutoPickUp_f,
	"id",			1,	Cmd_PlayerID_f,
	"medic",		1,  Cmd_Medic_Call_f,
	"motd",			1,	Cmd_MOTD,

	"dday_menu_main",	1,	Cmd_Menu_Main_f,
	"dday_menu_team",	1,	Cmd_Menu_Team_f,
	"dday_menu_class",	1,	Cmd_Menu_Class_f,
	"objectives",	1,	Cmd_Objectives_Toggle,//faf:  bring up objective pic
	"spot", 1, Cmd_Spot,//for mappers to find coordinates for map entities
	"removedoors", 1, Cmd_RemoveDoors,
	"showlinks", 1, ShowLinks,
	"removenode", 1, SV_Remove_Node
};


// MENUS
void ChooseTeam(edict_t *ent);
void M_ChooseMOS(edict_t *ent);
void MainMenu(edict_t *ent);

void Cmd_Menu_Main_f(edict_t *ent)
{
	if (!ent->client->display_info)//faf && level.framenum > ((int)level_wait->value * 10))
		MainMenu(ent);
}
void Cmd_Menu_Team_f(edict_t *ent)
{
	if (ent->flyingnun)
		Cmd_Menu_Main_f(ent);
	else if (!ent->client->display_info)//faf && level.framenum > ((int)level_wait->value * 10))
		ChooseTeam(ent);
}
void Cmd_Menu_Class_f(edict_t *ent)
{
	if (ent->flyingnun)
		Cmd_Menu_Main_f(ent);
	else if (!ent->client->display_info)//faf && level.framenum > ((int)level_wait->value * 10))
		M_ChooseMOS(ent);
}

//faf
void Cmd_Objectives_Toggle (edict_t *ent)
{
	//show objective pic
	if (ent->client->display_info)
	{
		ent->client->display_info = false;
		ent->client->layout_type = SHOW_NONE;
	}
	else
	{
		if (level.campaign)
		{
			ent->client->layout_type = SHOW_CAMPAIGN;
			ShowCampaign(ent);
		}
		ent->client->display_info = true;
	}
}

void Cmd_Spot (edict_t *ent)
{
/*	edict_t *e;
	int i;
	for (i=0 ; i<game.maxentities ; i++)
	{

		e = &g_edicts[i];

		if (!e->inuse)
			continue;
		if (!e->classname)
			continue;

		gi.dprintf ("%s  ", e->classname);
	}*/




	if (sv_cheats->value != 0)
	{
		safe_bprintf (PRINT_HIGH, "spot = %s    angle = %s.\n", vtos(ent->s.origin), vtos(ent->client->v_angle));
	}
}

void Cmd_RemoveDoors (edict_t *end)//for help with adding bot support
{
	edict_t *check;
	int e;

	check = g_edicts+1;
	if (sv_cheats->value != 0)
	{
		for (e = 1; e < globals.num_edicts; e++, check++)
		{
			if (!check->inuse)
				continue;
			if (!check->classnameb || check->classnameb != FUNC_DOOR_ROTATING)
				continue;
			gi.dprintf("DOOR REMOVED\n");//
			G_FreeEdict (check);
		}
	}
}

qboolean	frame_output = false;
void Cmd_DDebug_f (edict_t *ent)
{
#ifndef	DEBUG
	return;
//#endif
#else	// Knightmare- unreachable code in release builds
	int		i;
	float	weight = 0;
	char	*arg;

	arg = gi.args();
/*
	if (Q_stricmp(arg, "00endlevel") == 0)
	{
		Last_Team_Winner = 1; // testing
		EndDMLevel();
		return;
	}
*/
	if (Q_stricmp(arg, "frames") == 0 && !dedicated->value)
	{
		frame_output = (frame_output) ? false : true;
		return;
	}
	else if (Q_stricmp(arg, "limp") == 0 && !dedicated->value)
	{
		ent->wound_location = LEG_WOUND;
		return;
	}

	safe_cprintf (ent, PRINT_HIGH, "D-DAY DEBUG INFO: %s : %s\n", GAMEVERSION, __DATE__);

	// pbowens
	// This is basically a scrap function called to print out defined statistics.
	// no argument prints everything

		safe_cprintf(ent, PRINT_HIGH, "modelindex =  %i\nmodelindex2 = %i\nmodelindex3 = %i\nmodelindex4 = %i\n",
			ent->s.modelindex, ent->s.modelindex2, ent->s.modelindex3, ent->s.modelindex4);
		safe_cprintf(ent, PRINT_HIGH, "skinnum = %i\n", ent->s.skinnum);
		safe_cprintf(ent, PRINT_HIGH, "cur playermodel = %s\ndef playermodel = %s\n",
			ent->model, ent->client->resp.team_on->playermodel);
		safe_cprintf(ent, PRINT_HIGH, "client->aim = %s\n",
			(ent->client->aim)?"true":"false");
		safe_cprintf(ent, PRINT_HIGH, "limbo mode = %s\n",
			(ent->client->limbo_mode)?"true":"false");
		safe_cprintf(ent, PRINT_HIGH, "changeteam = %s\n",
			(ent->client->resp.changeteam)?"true":"false");
		safe_cprintf(ent, PRINT_HIGH, "teamname = %s\n",
			ent->client->resp.team_on->teamname);
		safe_cprintf(ent, PRINT_HIGH, "class = %s\n",
			ent->client->resp.team_on->mos[ent->client->resp.mos]->name);
//		safe_cprintf(ent, PRINT_HIGH, "MZ_MACHINEGUN: %d (is_silenced: %d)\n", MZ_MACHINEGUN, (MZ_MACHINEGUN | is_silenced));
		safe_cprintf(ent, PRINT_HIGH, "team1->map: %s\nteam2->map: %s\n", team_list[0]->nextmap, team_list[1]->nextmap);
		safe_cprintf(ent, PRINT_HIGH, "waterlevel = %i\n", ent->waterlevel);

		for (i=0;i<MAX_ITEMS;i++)
			if (ent->client->pers.inventory[i])
				weight+=((itemlist[i].weight) * (ent->client->pers.inventory[i]));

		weight -= ent->client->resp.team_on->mos[ent->client->resp.mos]->normal_weight; 	//subtract normal carring weight for mos from weight.

		safe_cprintf(ent, PRINT_HIGH, "def speedmod = %f\n", ent->client->resp.team_on->mos[ent->client->resp.mos]->speed_mod);
		safe_cprintf(ent, PRINT_HIGH, " norm weight = %f\n", ent->client->resp.team_on->mos[ent->client->resp.mos]->normal_weight);

		safe_cprintf(ent, PRINT_HIGH, "  cur weight = %f\n", weight);
		safe_cprintf(ent, PRINT_HIGH, "------------   ----------\n");
		safe_cprintf(ent, PRINT_HIGH, "cur speedmod = %f\n", ent->client->speedmod);
#endif	// Knightmare- end unreachable code in release builds
}

void Cmd_DDHelp_f(edict_t *ent)
{
	safe_cprintf(ent,PRINT_HIGH,
		"\n\n -- DDAY NORMANDY BINDS/HINTS --\n\n"

		"Commands:\n"
		" use weapon  - To look down the gun sites/alternate firing mode\n"
		" reload      - Reload\n"
		" stance      - Toggle crouch/crawl/stand\n"
		" autopickup  - Toggle if you can pickup items\n"
		" drop weapon - Drop your weapon and all of its ammo\n"
		" drop ammo   - Drop ammo for the current weapon\n"
		" arty/attack2- Use turret, use bayonet, cancel arty\n"
		" name        - Change your name (name xxx)\n"
		" messagemode - Chat\n"
		" messagemode2- Chat to teammates only\n"
		" main        - Main menu\n"
		" team        - Team menu\n"
		" class       - Class menu\n"
		" medic       - Call for a medic (Or hit F8)\n"
		" objectives  - Toggle objectives screen\n"
		" scoreboard  - Display the scoreboard (Double click for more stats)\n"
		" shout xxx   - Play xxx.wav in your team shout directory\n"
		" binds       - This dialog\n"
		"Edit your autoexec.cfg in your dday folder to bind these commands to the keys you want.\n"
		"To vote, type \"Vote yes\" or \"Vote map xxx\" in the console.\n"
		"To open the console, hit the key above the TAB key on your keyboard.\n"

	);
}


void Cmd_GameVersion_f (edict_t *ent)
{
	safe_cprintf (ent, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
}



void turret_off (edict_t *self);

qboolean CheckForTurret(edict_t *ent);

qboolean Cmd_Scope_f(edict_t *ent)
{

	if (ent->client->chasetarget)
	{
		if (ent->client->aim == 0)
		{
			ent->client->aim = 1;
			safe_centerprintf(ent,"(1) 1st person chase mode\n");
		}
		else if (ent->client->aim == 1)
		{
			ent->client->aim = 2;
			safe_centerprintf(ent,"(2) 1st person chase with weapons mode\n");
		}
		else if (ent->client->aim == 2)
		{
			ent->client->aim = 3;
			safe_centerprintf(ent,"(3) Auto 1st person with weapons/\n3rd person mode\n");
		}
		else if (ent->client->aim == 3)
		{
			ent->client->aim = 4;
			safe_centerprintf(ent,"(4) Bird's eye view\n");
		}
		else if (ent->client->aim == 4)
		{
			ent->client->aim = 0;
			safe_centerprintf(ent,"(5) 3rd person chase mode\n");
		}

		return true;
	}



	if (!ent->client->pers.weapon ||
 		 ent->client->pers.weapon->classnameb == WEAPON_SANDBAGS ||
 	//	 ent->client->pers.weapon->position== LOC_GRENADES ||
//bcass start - TNT
		 ent->client->pers.weapon->position== LOC_TNT)
//bcass end
		 return true;


if (ent->client->pers.weapon->position == LOC_GRENADES)
{
	if (!ent->client->aim){
		ent->client->aim = true;
		safe_centerprintf(ent, "Long range throw!\n");
	}
	else
	{
		ent->client->aim = false;
		safe_centerprintf(ent, "Short range throw!\n");
	}
	return true;
}


	if (ent->client->weaponstate == WEAPON_RAISE ||
ent->client->weaponstate == WEAPON_LOWER)
return false;

	//faf:  crosshair cheat prevention measures
//	if (!ent->client->aim)
//	{
		ent->client->crosshair_offset_x = random() * 4 - 2;
		ent->client->crosshair_offset_y = random() * 4 - 2;
	//	gi.dprintf ("%f\n", ent->client->crosshair_offset_x);


//	}
//	else
//		ent->client->crosshair_offset = 0;

	if (ent->client->pers.weapon->position == LOC_H_MACHINEGUN || //no hmg or engineer cuz tracers/rockets need more work
		ent->client->pers.weapon->position == LOC_ROCKET ||
		ent->client->pers.weapon->position == LOC_SNIPER ||
		ent->client->pers.weapon->position == LOC_HELMET ||
		ent->client->pers.weapon->position == LOC_FLAME ||
		ent->client->pers.weapon->position == LOC_GRENADES ||
		ent->client->pers.weapon->position == LOC_KNIFE ||
		ent->client->pers.weapon->position == LOC_TNT ||
		ent->client->pers.weapon->position == LOC_SPECIAL ||
		ent->client->pers.weapon->position == LOC_SHOTGUN)

	{
		ent->client->crosshair_offset_x = 0;
		ent->client->crosshair_offset_y = 0;
	}




	//faf:  turret stuff
	if (ent->client->pers.weapon && ent->client->pers.weapon->classnameb == WEAPON_FISTS
		&& !ent->client->aim)
	{
		if (ent->client->turret)
		{
			turret_off (ent);
			ent->client->turret = NULL;
			return true;
		}
		else if (CheckForTurret(ent))
			return true;
	}


if (!ent->client->aim &&
	ent->client->pers.weapon->classnameb == WEAPON_FISTS &&
	!ent->client->pers.inventory[ITEM_INDEX(FindItem("Helmet"))])
	return true;


	if (ent->client->pers.weapon->classnameb == WEAPON_BINOCULARS)
	{
		if (ent->client->aim == true)
		{
			ent->client->aim = false;
			ent->client->ps.gunframe = 8;
		}
		else
		{
			ent->client->aim = true;
		}
		return true;
	}



//faf
       if (ent->client->pers.weapon &&
		   (ent->client->pers.weapon->classnameb == WEAPON_MAUSER98K ||
			ent->client->pers.weapon->classnameb == WEAPON_ARISAKA ||
			ent->client->pers.weapon->classnameb == WEAPON_CARCANO ||
			ent->client->pers.weapon->classnameb == WEAPON_ENFIELD) &&
               (ent->client->weaponstate == WEAPON_LOWER ||
			   (!ent->client->aim && ent->client->weaponstate == WEAPON_FIRING)))
               return false;



	// do not let a sniper reload bolt if there is no ammo
	if (ent->client->pers.weapon &&
		ent->client->pers.weapon->position == LOC_SNIPER &&
		ent->client->p_rnd &&
		*ent->client->p_rnd == 0)
		return true;

	//Wheaty: Don't allow them to switch to truesight/out of truesight while reloading
	if (ent->client->weaponstate != WEAPON_RELOADING &&
		ent->client->weaponstate != WEAPON_RAISE)
	{

		if (ent->client->aim) {
			ent->client->aim		 = false;
			ent->client->weaponstate = WEAPON_LOWER;
		} else {
			ent->client->aim		 = true;
			ent->client->weaponstate = WEAPON_RAISE;
			// rezmoth - begin sniper scope wobble
			if (ent->client->pers.weapon->position == LOC_SNIPER)
			{
				ent->client->resp.scopewobble = 0;
			}
		}
		return true;
	}

	// pbowens: switched to WeighPlayer
	WeighPlayer(ent);
//	ClientSetMaxSpeed(ent, true);
	//ent->client->syncspeed = true;
	return false;
}


struct cmd_list_t  *GlobalCommandList = NULL;

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	Q_strncpyz (value, sizeof(value), Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}




void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chasetarget)
		return;

	i = ent->client->chasetarget - g_edicts;
	do {
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (!e->flyingnun)
			break;
	} while (e != ent->client->chasetarget);

	ent->client->chasetarget = e;
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chasetarget)
		return;

	i = ent->client->chasetarget - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (!e->flyingnun)
			break;
	} while (e != ent->client->chasetarget);

	ent->client->chasetarget = e;
}






void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;//, current_player, new_chase;
	gitem_t		*it;

	cl = ent->client;


	if (cl->chasetarget)
	{
		ChaseNext(ent);
		return;
	}


   	if (cl->menu)
	{
		PMenu_Next(ent);
		return;
	}




	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;

}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;


	if (cl->chasetarget)
 	{
		ChasePrev(ent);
		return;
	}




   	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	}


	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}
/*
	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		//gitem_armor_t	*info;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		if (!give_all)
			return;
	}

*/
	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;

		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;
/*test	gitem_t	*item;


int i;


	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		gi.dprintf("%s\n", item->classname);
	}
*/





	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";

		ent->stance_view=20;
		ent->stanceflags = STANCE_STAND;
	}

	safe_cprintf (ent, PRINT_HIGH, msg);
}

///////////////////////////////////////////////////////////////////////////////////////////
/*
gitem_t	*FindNextPos (edict_t *ent,int position)
{
	int		i;
	gitem_t	*it;
	gitem_t *tempit=NULL;

	for (it = ent->client->pers.weapon, i = ITEM_INDEX(it); 1 ; )
	{
		if (!it || !ent->client->pers.weapon)
			return NULL;

		//starts list off at current weapon

		if (!tempit)
			tempit=it;

		it++;
		i++;  //adds one to it and i each time through

		if ( i >= game.num_items)
		{
			it = itemlist+1; //if it > number of items, recycle to begining
			i  = ITEM_INDEX(it);
		}

		if (!it || !it->pickup_name)
			continue;		//keep going if NULL

		if (ent->client->pers.weapon == it)
				return tempit;	//if complete circut through list, return

		if (it->position == position)
		{
			tempit = it;
			if (ent->client->pers.inventory[ITEM_INDEX(it)])
				return it;	//if we found the next item in the list at that location return
		}
	}

	return NULL;
}
*/

// pbowens: based off of previous function by KMM
gitem_t	*FindNextPickup (edict_t *ent, int location)
{
	int		index;
	gitem_t	*it;
	gitem_t *it_start = NULL;

	it = ent->client->pers.weapon; // start the cycle with player's weapon
	index = ITEM_INDEX(it);

	while (it != it_start)
	{
		if (!it_start)
			 it_start = it;

		if (index >= MAX_ITEMS) // reached end of list, reset to beginning
			it = itemlist;
		else
			it++;

		index = ITEM_INDEX(it); //gi.dprintf("index: %i\n", index);

		if (!it || !it->pickup_name )
			continue;

		// sneak location stuff in here too
		if (location && it->position == location)
		{
			if (ent->client->pers.inventory[index])
				return it;
			else
				continue;
		}
		else if (location)	// Skip the stuff below if its looking for a specific position
			continue;



		if (it == FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1) )
			continue;
//		if (ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2 &&
//			it == FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2) )
//			continue;
		if (ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades &&
			it == FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades) )
			continue;
//		if (ent->client->resp.team_on->mos[ent->client->resp.mos]->special &&
//			it == FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->special) )
//			continue;
		if (it == FindItem("Knife") )
			continue;
		if (it == FindItem("Fists") )
			continue;
		if (it == FindItem("Helmet") )
			continue;
		if (!(it->flags & IT_WEAPON) && it->position != LOC_GRENADES)
			continue;

		if (ent->client->pers.inventory[index])
			return it;	// new weapon in inventory
	}

	return it_start; // no other weaps could be found, use same weapon
}

/*
==================
Cmd_Use_f

Use an inventory item
==================
*/

void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;
	char		itemBuf[MAX_OSPATH];	// Knghtmare added

//	s = gi.args();
//	it = FindItem (s);
	// Knightmare- copy args over to local buffer for safe manipulation
	Q_strncpyz (itemBuf, sizeof(itemBuf), gi.args());
	it = FindItem (itemBuf);
	s = itemBuf;
	// end Knightmare

	if (ent->client->chasetarget)
	{
		if (Q_stricmp(s, "weapon") == 0)
			Cmd_Scope_f(ent);
	}

	if (ent->client->limbo_mode || ent->deadflag)
		return;
	if ( !IsValidPlayer(ent) )
		return;

	if (Q_stricmp(s, "gibmachine") == 0)
	{
		if (easter_egg->value == 0)
			return;

		if (ent->client->gibmachine == flame_normal)
		{
			safe_cprintf(ent, PRINT_HIGH, "You've got the gib machine!.\n");
			ent->client->gibmachine = flame_gib;
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Goodbye gib machine!.\n");
			ent->client->gibmachine = flame_normal;
		}
		return;
	}

	if (!it)
	{
//////////////
/// NEW USE SYSTEM
//////////////
		// FIXME!! Knightmare- re-allocate these based on src size
		if (Q_stricmp(s, "weapon") == 0)
		{
			if (Cmd_Scope_f(ent) == false)
				ent->client->scopetry = true;
			else
				ent->client->scopetry = false;

			return;
		}
		else if (Q_stricmp(s, "weapon1") == 0)
		{
			if (it = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1))
			//	strncpy (s, ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1);
				Q_strncpyz (itemBuf, sizeof(itemBuf), ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1);
			else
				it = ent->client->pers.weapon;
		}
		else if (Q_stricmp(s, "weapon2") == 0)
		{
			if (it = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2))
			//	strncpy (s, ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2);
				Q_strncpyz (itemBuf, sizeof(itemBuf), ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2);
			else
				it = ent->client->pers.weapon;
		}
		/*
		else if (Q_stricmp(s, "grenades") == 0)
		{
			if (it = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades))
			//	strncpy (s, ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades);
				Q_strncpyz (itemBuf, sizeof(itemBuf), ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades);
			else
				it = ent->client->pers.weapon;
		}
		*/
		else if (Q_stricmp(s, "special") == 0)
		{
			if (it = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->special))
			//	strncpy (s, ent->client->resp.team_on->mos[ent->client->resp.mos]->special);
				Q_strncpyz (itemBuf, sizeof(itemBuf), ent->client->resp.team_on->mos[ent->client->resp.mos]->special);
			else
				it = ent->client->pers.weapon;
		}
		else if (Q_stricmp(s, "grenades") == 0)
		{
			it = FindNextPickup(ent, LOC_GRENADES);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}
		else if (Q_stricmp(s, "melee") == 0)
		{
			it = FindNextPickup(ent, LOC_KNIFE);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}
		else if (Q_stricmp(s, "pickup") == 0)
		{
			it = FindNextPickup(ent, LOC_NONE);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		//faf
		else if (Q_stricmp(s, "sniper") == 0)
		{
			it = FindNextPickup(ent, LOC_SNIPER);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		else if (Q_stricmp(s, "pistol") == 0)
		{
			it = FindNextPickup(ent, LOC_PISTOL);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		else if (Q_stricmp(s, "rifle") == 0)
		{
			it = FindNextPickup(ent, LOC_RIFLE);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		else if (Q_stricmp(s, "smg") == 0)
		{
			it = FindNextPickup(ent, LOC_SUBMACHINEGUN2);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
			it = FindNextPickup(ent, LOC_SUBMACHINEGUN);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		else if (Q_stricmp(s, "lmg") == 0)
		{
			it = FindNextPickup(ent, LOC_L_MACHINEGUN);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}

		else if (Q_stricmp(s, "hmg") == 0)
		{
			it = FindNextPickup(ent, LOC_H_MACHINEGUN);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}
		else if (Q_stricmp(s, "rocket") == 0)
		{
			it = FindNextPickup(ent, LOC_ROCKET);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}
		else if (Q_stricmp(s, "flamer") == 0)
		{
			it = FindNextPickup(ent, LOC_FLAME);
		//	strncpy (s, it->pickup_name);
			Q_strncpyz (itemBuf, sizeof(itemBuf), it->pickup_name);
		}
		// end faf

		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Unknown item: %s\n", s);
			return;
		}

	}
	if (!it || (it && !it->use))
	{
		safe_cprintf (ent, PRINT_HIGH, "Item %s is not usable.\n", it->pickup_name);
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
	//	safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}

//bcass start - was here
/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	if (ent->client->limbo_mode || ent->deadflag == DEAD_DEAD)
		return;

	if (!ent->client->resp.team_on || !ent->client->resp.mos || ent->client->grenade || ent->client->grenade_index )
		return;

	s = gi.args();
	it = FindItem (s);

//bcass start - drop
	if (Q_stricmp(s, "gun") == 0 || //pbowens: do the same thing if it is the current weapon
		(ent->client->pers.weapon && it && (ent->client->pers.weapon == it)) )
	{
		it = (ent->client->pers.weapon);
		index = ITEM_INDEX(it);

//not weapons or live grenades
		if ((!it || !ent->client->pers.inventory[index]) ||
			(ent->client->pers.weapon->classnameb == WEAPON_FISTS) ||		 //Wheaty: Use 'Classname' instead of 'Pickupname'
			(ent->client->pers.weapon->position == LOC_SPECIAL) ||
			(ent->client->pers.weapon->position == LOC_FLAME) ||
			(ent->client->pers.weapon->position == LOC_TNT) ||
			ent->client->grenade ||	!it->drop) // check and see if it *can* drop in the first place
			return;

//pbowens: stop firing
		ent->client->buttons |= BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

//move us to a new weapon
		ent->client->drop = true;
		Cmd_WeapNext_f (ent);

//take it from the inventory and drop it
		it->drop (ent, it);
		WeighPlayer(ent);
		return;
	}


//bcass - taking away is bugged
// pbowens: not anymore
	if (Q_stricmp(s, "ammo") == 0)
	{
		edict_t	*dropped;
		gitem_t *ammo_item;
		int		 ammo_index;

		if (!ent->client->pers.weapon ||
			(ent->client->pers.weapon->position == LOC_KNIFE)	||
			(ent->client->pers.weapon->position == LOC_TNT)		||
			(ent->client->pers.weapon->position == LOC_SPECIAL) ) // pbowens: client does not have weapon, etc
			return;

		// rezmoth - ammo_item definition has crashed the server twice
		ammo_item	= FindItem(ent->client->pers.weapon->ammo);
		ammo_index	= ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index] || !ammo_item->drop)
			return;

		// pbowens: Drop_Item actually does the dropping, so do checks *beforehand*
		dropped		= Drop_Item (ent, ammo_item);

		// pbowens: clarification:
		//		ent->count			== number to add to inventory (should be 1)
		//		ent->item->quantity	== number of rounds

		dropped->count			= 1;
		dropped->item->quantity = ammo_item->quantity;

		ent->client->pers.inventory[ammo_index]--;

		ValidateSelectedItem (ent);
		WeighPlayer(ent);

		return;
	}

	// pbowens: drop weapon drops both gun and ammo
	if (Q_stricmp(s, "weapon") == 0)
	{
		gitem_t *item;

		it = ent->client->pers.weapon;
		index = ITEM_INDEX(it);

//not weapons or live grenades
		if ((!it || !ent->client->pers.inventory[index]) ||
			(ent->client->pers.weapon->classnameb == WEAPON_FISTS) ||		 //Wheaty: Use 'Classname' instead of 'Pickupname'
			(ent->client->pers.weapon->position == LOC_SPECIAL) ||
			(ent->client->pers.weapon->position == LOC_FLAME) ||
			(ent->client->pers.weapon->position == LOC_TNT) ||
			ent->client->grenade ||	!it->drop) // check and see if it *can* drop in the first place
			return;

//pbowens: stop firing
		ent->client->buttons |= BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		// pbowens: drop ammo
		item = it;
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

			if (ent->client->pers.inventory[ammo_index])
			{
				if (!ammo_item->drop)
					return;

				dropped					= Drop_Item (ent, ammo_item);
				dropped->count			= ent->client->pers.inventory[ammo_index];
				dropped->item->quantity = ammo_item->quantity;

				ent->client->pers.inventory[ammo_index] = 0;
			}
		}

//move us to a new weapon
		ent->client->drop = true;
		Cmd_WeapNext_f (ent);

//take is from the inventory and drop it
		it->drop (ent, it);
		WeighPlayer(ent);
		return;
	}

//why not add another easter egg?
//future site of a gerbil model on ground =)
	if (Q_stricmp(s, "GerbilsAreTheGreatestInTheWorld") == 0)
	{
		if (easter_egg->value==0)
			return;

		safe_bprintf(PRINT_CHAT, "***NOTICE*** Species loves Gerbil Pr0n\n");
		return;
	}
//bcass end

	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
	WeighPlayer(ent);
}
//bcass end

/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]


	cl = ent->client;


	if (ent->client->menu) {
		PMenu_Close(ent);
		return;
	}

	if (cl->layout_type == SHOW_INVENTORY)
	{
		cl->layout_type = SHOW_NONE;
		return;
	}
	cl->layout_type = SHOW_INVENTORY;


	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}


void Cmd_Objectives (edict_t *ent)
{
	int			i,j,y;
	gclient_t	*cl = NULL;
	char	string[1024];
	char	entry[1024];
	char	entryb[1024];
	int	stringlength;
	char  *bp = NULL;
	char  *objective_name = NULL;


	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	cl = ent->client;


	if (!ent->client->resp.team_on)
		return;

	if (ent->client->menu) {
		PMenu_Close(ent);
		return;
	}


	if (cl->layout_type != 	SHOW_OBJECTIVES_TEMP)
		cl->layout_type = SHOW_OBJECTIVES;



	string[0] = 0;

	// send the layout
//	Com_sprintf (string, sizeof(string),
//		"xv 32 yv 8 picn scorehead ");
	Com_sprintf (string, sizeof(entry),	"xr -200 160 yt 120  string \" Objectives\" xr -200 160 yt 130  string \" ----------\" ");



	stringlength = (int)strlen(string);

	y=2;
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		edict_t *e;
		e = &g_edicts[i];

//		if (!e->inuse)
//			continue;
		if (!e->classnameb)
			continue;
		if (!(e->classnameb == OBJECTIVE_TOUCH  || e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE || e->classnameb == FUNC_TRAIN || e->classnameb == OBJECTIVE_VIP))
			continue;

		if (e->classnameb == FUNC_TRAIN &&
			!e->obj_gain)
			continue;

		if (e->obj_name)
			objective_name = e->obj_name;
		else if (e->message)
			objective_name = e->message;
		else if (e->classnameb == OBJECTIVE_TOUCH)
			objective_name = "Objective";
		else if (e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE)
			objective_name = "Destroyable Objective";

		//put ! next to own objectives that can't be recapped
		if (ent->client->resp.team_on &&
			e->obj_perm_owner &&
			e->obj_perm_owner%2 == ent->client->resp.team_on->index)
		{ bp ="!";}
		else
		{ bp = " ";}

		if (ent->client->resp.team_on &&
			e->obj_owner == ent->client->resp.team_on->index)
			Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string2 \" %-22.22s\" ",120+(y*10), objective_name);
		else
		Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string \"%s%-22.22s\" ",120+(y*10),  bp, objective_name);

		j = (int)strlen(entry);
		if (stringlength + j > 1024)
			break;
	//	strncpy (string + stringlength, entry);
		Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;
		y++;

	}

	if (team_list[ent->client->resp.team_on->index]->kills_and_points &&
		team_list[ent->client->resp.team_on->index]->kills < team_list[ent->client->resp.team_on->index]->need_kills)
	{
		if (strlen(string) < 1000)
		{
			Com_sprintf (entryb, sizeof(entryb),	"xr -200 yt %i string \" %i Kills\" ",120+(y*10),team_list[ent->client->resp.team_on->index]->need_kills);
		//	strncpy (string + strlen(string), entryb);
			Q_strncpyz (string + strlen(string), sizeof(string) - stringlength, entryb);

		}
		y++;
	}
	//gi.dprintf("%s\n",string);

	if (y==2)// no objectives
	{
		cl->layout_type = SHOW_NONE;
		return;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}



//not necessary
void Cmd_Objectives_ToggleX (edict_t *ent)
{
	if (ent->client->layout_type == SHOW_OBJECTIVES || ent->client->layout_type == SHOW_OBJECTIVES_TEMP)
	{
		ent->client->layout_type = SHOW_NONE;
		return;
	}
	else
	{
		ent->client->layout_type = SHOW_OBJECTIVES;
		Cmd_Objectives (ent);
	}
}
/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (cl->chasetarget)
	{
		ChasePrev(ent);
		return;
	}

	check_unscope(ent);//faf

	cl->ps.fov = STANDARD_FOV;

	if (!cl->pers.weapon)
		return;

	if (cl->limbo_mode)
		return;

	//faf
//blah	if (ent->client->weaponstate != WEAPON_READY)
//blah		return;


	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;


	if (cl->chasetarget)
	{
		ChaseNext(ent);
		return;
	}

//	if (!cl->pers.weapon)
//		return;

	if (cl->limbo_mode || ent->deadflag)
		return;

	//faf
//blah	if (ent->client->weaponstate != WEAPON_READY)
//blah		return;


	check_unscope(ent);//faf

	cl->ps.fov = STANDARD_FOV;

//	gi.dprintf("change\n");

	if (cl->pers.weapon)
		selected_weapon = ITEM_INDEX(cl->pers.weapon);
	else
		selected_weapon = ITEM_INDEX(FindItem("Knife"));

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chasetarget)
	{
		ChasePrev(ent);
		return;
	}
	check_unscope(ent);//faf

	cl->ps.fov = STANDARD_FOV;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	if (cl->limbo_mode)
		return;

	//faf
//blah	if (ent->client->weaponstate != WEAPON_READY)
//blah		return;


	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	if (ent->client->menu)
		PMenu_Close(ent);

	ValidateSelectedItem (ent);

	if (ent->client->limbo_mode)
		return;

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
	WeighPlayer(ent);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	safe_cprintf(ent, PRINT_HIGH, "Kill who? ME? HAHAHAHAhahaha....\n");
	/*
	if ((level.time - ent->client->respawn_time) < 5)
		return;

	//T_Damage(ent,ent,ent, ent->maxs, ent->s.origin, NULL, 999, 0,  DAMAGE_NO_PROTECTION,
	//			MOD_SUICIDE);
	/*
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	ent->deadflag = 0; // PLAY the animations
	player_die (ent, ent, ent, 100000, vec3_origin);
	respawn (ent);
	*/
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->layout_type = SHOW_NONE;
	PMenu_Close(ent);
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			Q_strncatz (large, sizeof(large), "...\n");
			break;
		}
		Q_strncatz (large, sizeof(large), small);
	}

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent, int wave)
{
	int		i;

	if (ent->ai)
		i = wave;
	else
		i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		safe_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}


//faf:  based on aq2 code
void GetNearbyTeammates (edict_t *self, char *buf, size_t bufSize)
{
	unsigned char nearby_teammates[10][16];
	int nearby_teammates_num, l;
	edict_t *ent = NULL;

	nearby_teammates_num = 0;

	while ((ent = findradius(ent, self->s.origin, 750)) != NULL)
	{
		if (ent == self || !ent->client || !CanDamage(ent, self) ||
			!OnSameTeam(ent, self))
			continue;

		strncpy(nearby_teammates[nearby_teammates_num], ent->client->pers.netname, 15);
		nearby_teammates[nearby_teammates_num][15] = 0; // in case their name is 15 chars...
		nearby_teammates_num++;
		if (nearby_teammates_num >= 10)
			break;
	}

	if (nearby_teammates_num == 0)
	{
		Q_strncpyz (buf, bufSize, "");
		return;
	}

	for (l = 0; l < nearby_teammates_num; l++)
	{
		if (l == 0)
		{
			Q_strncpyz (buf, bufSize, nearby_teammates[l]);
		}
		else
		{
			if (nearby_teammates_num == 2)
			{
				Q_strncatz (buf, bufSize, " and ");
				Q_strncatz (buf, bufSize, nearby_teammates[l]);
			}
			else
			{
				if (l == (nearby_teammates_num - 1))
				{
					Q_strncatz (buf, bufSize, ", and ");
					Q_strncatz (buf, bufSize, nearby_teammates[l]);
				}
				else
				{
					Q_strncatz (buf, bufSize, ", ");
					Q_strncatz (buf, bufSize, nearby_teammates[l]);
				}
			}
		}

	}
}

//faf
void GetNearestMedic (edict_t *self, char *buf, size_t bufSize)
{
//	char nearest_medic[16];
	int i;
	edict_t *e;
	edict_t *nearest = NULL;
	float temp_distance, nearest_distance = 9999999;
	vec3_t dist;


	for (i=0 ; i < game.maxclients ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse || e->flyingnun || !e->client)
			continue;
		if (!OnSameTeam(e, self))
			continue;
		if (e->client->resp.mos &&
			!(e->client->resp.mos == MEDIC))
			continue;
		if (e == self)
			continue;

		VectorSubtract (e->s.origin, self->s.origin, dist);

		temp_distance = VectorLength(dist);

		if (temp_distance < nearest_distance)
		{
			nearest_distance = temp_distance;
			nearest = e;
		}
	}

	if (nearest != NULL)
	{
		Q_strncpyz (buf, bufSize, nearest->client->pers.netname);
	}
	else
		Q_strncpyz (buf, bufSize, "Medic");
}


//faf
void SP_map_location (edict_t *self)
{
}

void GetNearbyLocation (edict_t *self, char *buf, size_t bufSize)
{
//	unsigned char nearest_location[16];
	int  i;

	float temp_distance;
	float nearest_distance = 9999999;

	vec3_t dist, dist2;
	edict_t	*e = NULL, *closest = NULL;


	for (i=0 ; i<globals.num_edicts ; i++)
	{
		e = &g_edicts[i];

		if (!e->inuse)
			continue;
		if (!( e->classnameb == OBJECTIVE_TOUCH  ||
			e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE  ||
			(!strcmp(e->classname, "map_location")) ))
			continue;

		VectorSubtract (e->mins, self->s.origin, dist);
		VectorSubtract (e->maxs, self->s.origin, dist2);

		if (VectorLength(dist) < VectorLength(dist2))
			temp_distance = VectorLength(dist);
		else
			temp_distance = VectorLength(dist2);

		if (!strcmp(e->classname, "map_location"))
		{
			VectorSubtract (e->s.origin, self->s.origin, dist);
			temp_distance = VectorLength(dist);
		}
	//	safe_bprintf (PRINT_HIGH, "%s\n", vtos(e->maxs)	);

	//	if (temp_distance > 500)//too far
	//		continue;

		if (temp_distance < nearest_distance)
		{
			nearest_distance = temp_distance;
			closest = e;

		}
	}
	if (nearest_distance > 500)
	{
		Q_strncpyz (buf, bufSize, "somewhere");
	}
	else
	{
		if (closest->obj_name)
			Q_strncpyz (buf, bufSize, closest->obj_name);
		else if (closest->message)
			Q_strncpyz (buf, bufSize, closest->message);
		else
			Q_strncpyz (buf, bufSize, "somewhere");
	}

	return;
}

void GetClass (edict_t *self, char *buf, size_t bufSize)
{

/*	int i;

	if (self->client->resp.team_on)
		i = self->client->resp.team_on->index;
	else
	{
		Q_strncpyz (buf, bufSize, "");
		return;
	}
	if (self->client->resp.team_on &&
		self->client->resp.team_on->mos)
		Q_strncpyz (buf, bufSize, self->client->resp.team_on->mos[1]->name);
	else
		Q_strncpyz (buf, bufSize, "");

	return;
*/
	if (self->client->resp.mos)
	{
		switch (self->client->resp.mos)
		{
		case OFFICER:
			Q_strncpyz (buf, bufSize, "Officer");
			break;
		case INFANTRY:
			Q_strncpyz (buf, bufSize, "Infantry");
			break;
		case L_GUNNER:
			Q_strncpyz (buf, bufSize, "Light Gunner");
			break;
		case H_GUNNER:
			Q_strncpyz (buf, bufSize, "Heavy Gunner");
			break;
		case SNIPER:
			Q_strncpyz (buf, bufSize, "Sniper");
			break;
		case SPECIAL:
			Q_strncpyz (buf, bufSize, "Airborne");
			break;
		case ENGINEER:
			Q_strncpyz (buf, bufSize, "Engineer");
			break;
		case MEDIC:
			Q_strncpyz (buf, bufSize, "Medic");
			break;
		case FLAMER:
			Q_strncpyz (buf, bufSize, "Flamethrower");
			break;
		default:
			Q_strncpyz (buf, bufSize, "x");
			break;
		}
	}
}


char *SeekBufEnd (char *buf)
{
	while (*buf != 0)
		buf++;
	return buf;
}


void ParseSayText (edict_t *ent, char *text)
{
	static char buf[10240], infobuf[10240];    // Knightmare- was unsigned
	char	*p, *pbuf;
	size_t		bufLen;		// Knightmare added

	p = text;
	pbuf = buf;
	*pbuf = 0;
	bufLen = pbuf - &buf[0];	// Knightmare added

	while (*p != 0)
	{
		if (((ptrdiff_t)pbuf - (ptrdiff_t)buf) > 150)
		{
			break;
		}
		if (*p == '%')
		{
			switch (*(p+1))
			{
			case 'L':
				GetNearbyLocation (ent, infobuf, sizeof(infobuf));
			//	strncpy (pbuf, infobuf);
				Q_strncpyz (pbuf, sizeof(buf) - bufLen, infobuf);
				pbuf = SeekBufEnd(pbuf);
				bufLen = pbuf - &buf[0];	// Knightmare added
				p += 2;
				continue;
			case 'T':
				GetNearbyTeammates (ent, infobuf, sizeof(infobuf));
			//	strncpy (pbuf, infobuf);
				Q_strncpyz (pbuf, sizeof(buf) - bufLen, infobuf);
				pbuf = SeekBufEnd(pbuf);
				bufLen = pbuf - &buf[0];	// Knightmare added
				p += 2;
				continue;
			case 'M':
				GetNearestMedic (ent, infobuf, sizeof(infobuf));
			//	strncpy (pbuf, infobuf);
				Q_strncpyz (pbuf, sizeof(buf) - bufLen, infobuf);
				pbuf = SeekBufEnd(pbuf);
				bufLen = pbuf - &buf[0];	// Knightmare added
				p +=2;
				continue;
			case 'C':
				GetClass (ent, infobuf, sizeof(infobuf));
			//	strncpy (pbuf, infobuf);
				Q_strncpyz (pbuf, sizeof(buf) - bufLen, infobuf);
				pbuf = SeekBufEnd(pbuf);
				bufLen = pbuf - &buf[0];	// Knightmare added
				p +=2;
				continue;
			}
		}
		*pbuf++ = *p++;
	}

	*pbuf = 0;

	strncpy (text, buf, 150);
	text[150] = 0; // in case it's 150
}

/*
==================
Cmd_Say_f
==================
*/

void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0, qboolean saved)
{
	int			i,j, offset_of_text;
	edict_t		*entR = NULL;
	edict_t		*entG = NULL;
	char		*p = NULL;
	char		text[2048];
	gclient_t	*cl = NULL;
	char teamname[5];

	if (ent->client->resp.team_on)
	{
		Com_sprintf (teamname, sizeof(teamname), "%s ",ent->client->resp.team_on->playermodel);
	}
	else
		Com_sprintf (teamname, sizeof(teamname), "");


	if (saved)
	{
		Com_sprintf (text, sizeof(text), "%s", ent->client->resp.chatsave);
		team =  ent->client->resp.chatsavetype;
		ent->client->resp.chatsave[0] = '\0';
	}
	else
	{
		if (gi.argc () < 2 && !arg0)
			return;


	/*	if (ent->leave_limbo_time && level.time < ent->leave_limbo_time)
		{
				safe_cprintf(ent, PRINT_HIGH, "You just died!  You can't talk for %i more seconds\n", 1+(int)(ent->leave_limbo_time - level.time));
				return;
		}
	*/




		if (team)
			Com_sprintf (text, sizeof(text), "(Team)%s: ", ent->client->pers.netname);
		else{
			Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);


		}

		p = gi.args();

		offset_of_text = (int)strlen(text);  //FB 5/31/99

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		Q_strncatz (text, sizeof(text), p);

		// Wheaty: Avoid the 'null spamming'...
		if (!team) {
			if (2 == strlen(text) - strlen(ent->client->pers.netname)) {
				return;
			}
		} else {
			if (8 == strlen(text) - strlen(ent->client->pers.netname)) {
				return;
			}
		}

		// don't let text be too long for malicious reasons
		if (strlen(text) > 150)
			text[150] = 0;

		//faf: from aq2
		if (ent->solid != SOLID_NOT && ent->deadflag != DEAD_DEAD)
					ParseSayText (ent, text + offset_of_text);  //FB 5/31/99 - offset change
									// this will parse the % variables,

		Q_strncatz (text, sizeof(text), "\n");

		if (flood_msgs->value) {
			cl = ent->client;

			if (level.time < cl->flood_locktill) {
				safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
					(int)(cl->flood_locktill - level.time));
				return;
			}
			i = cl->flood_whenhead - flood_msgs->value + 1;
			if (i < 0)
				i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
			if (cl->flood_when[i] &&
				level.time - cl->flood_when[i] < flood_persecond->value) {
				cl->flood_locktill = level.time + flood_waitdelay->value;
				safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
					(int)flood_waitdelay->value);
				return;
			}
			cl->flood_whenhead = (cl->flood_whenhead + 1) %
				(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
			cl->flood_when[cl->flood_whenhead] = level.time;
		}


		if (team == 1 /*people didn't like public chat delay*/ && ent->client->forcespawn && ent->client->forcespawn  - FORCE_RESPAWN + (10 *(int)RI->value) > level.framenum)
		{
			safe_cprintf(ent, PRINT_HIGH, "You just died! Chat will be delayed.\n");
			Com_sprintf (ent->client->resp.chatsave, sizeof(ent->client->resp.chatsave), "%s", text);

			ent->client->resp.chatsavetype = team;
			ent->client->resp.chatsavetime = ent->client->forcespawn  - FORCE_RESPAWN + (10 *(int)RI->value);
			return;
		}
	}


	if (team)//say only to team
	{
		if (!ent->client->resp.team_on ||
			!ent->client->resp.mos) {
			safe_cprintf(ent, PRINT_HIGH, "You aren't on a team!\n");
			return;
		}

	//	if (ent->deadflag || ent->client->limbo_mode)
	//		return;

		for (j = 1; j <= game.maxclients; j++)
		{
			entR=&g_edicts[j];
			if (!entR->inuse || !entR->client)
				continue;

			if (entR->client->resp.team_on	&&
				entR->client->resp.mos		&&
				entR->client->resp.team_on == ent->client->resp.team_on)
			{safe_cprintf(entR, PRINT_HIGH, "%s", teamname);
			safe_cprintf(entR, PRINT_CHAT, "%s", text);}
		}

	}
	else //say to everyone
	{
		if (dedicated->value)
			safe_cprintf(NULL, PRINT_CHAT, "%s", text);

		for (j = 1; j <= game.maxclients; j++)
		{
			entR=&g_edicts[j];
			if (!entR->inuse || !entR->client)
				continue;
			safe_cprintf(entR, PRINT_HIGH, "%s", teamname);
			safe_cprintf(entR, PRINT_CHAT, "%s", text);
		}


		//save a random chat message for stats page
		if (!saved && strlen(p)>9 && rand()%25 == 1 || (!strcmp (ent->client->pers.stat_chat , "") && rand()% 3 ==1))
		{
			Com_sprintf (ent->client->pers.stat_chat, sizeof(ent->client->pers.stat_chat), "%s", p);
		}
	}

	if (ent->flyingnun || ent->deadflag || ent->health < 1)
		return;

	ent->client->anim_priority = ANIM_WAVE;
	if (ent->stanceflags == STANCE_STAND)
	{
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
	}
	else if (ent->stanceflags == STANCE_DUCK)
	{
		ent->s.frame = 168;
		ent->client->anim_end = 171;
	}
	else if (ent->stanceflags == STANCE_CRAWL)
	{
		ent->s.frame = 229;
		ent->client->anim_end = 232;
	}

}


/*
This routine is for changing the stances and to allow lowcrawl type  manuevers.
*/

void change_stance(edict_t *self, int stance)
{
	vec3_t	end;
	vec3_t	dist = {0, 0, 16};
	trace_t	tr;

	if (self->health < 1)
		return;


	if (self->stanceflags == stance ) // if the ent is already in the stance, return
//	{ gi.dprintf("Same Stance\n");
		return;
//	}

	if (!self->client->limbo_mode && !self->deadflag)
	{
		// pbowens: check to see if there is a player above them and if so return
		if (self->stanceflags != STANCE_STAND)
		{
			VectorCopy(self->s.origin, end);
			VectorAdd(end, dist, end);

			tr = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_PLAYERSOLID);
			if (tr.ent && tr.ent != self && tr.ent->client)
			{
				safe_centerprintf(self, "There is a soldier on top of you!\nYou can not get up!\n");
				return;
			}
		}
	}

	if (stance==STANCE_DUCK) //if we're supposed to duck
	{
		self->stance_view= 4;//faf6;//faf -2;
//		self->stance_max=9;//faf 16;
	}

	else if (stance==STANCE_CRAWL)	//if we're supposed to crawl
	{
		if (!Q_stricmp(level.mapname, "soviet1")) //lolhack
			self->stance_view = -16;
		else
			self->stance_view= -17;//faf -14;	//10
//		self->stance_max =-12;//faf -8;

//bcass start - make a thud sound when you go prone
		if (!self->waterlevel) //Wheaty: Don't play sound if they are in water
			gi.sound (self, CHAN_BODY, gi.soundindex ("misc/prone2.wav"), .4, ATTN_NORM, 0);
//bcass end

	}
	else // 					//if we're supposed to stand
	{
		self->stance_view=20;//faf 22;
//		self->stance_max=24;//faf32;
	}

	self->client->anim_priority = ANIM_CHANGESTANCE; //faf

	if (self->s.frame < FRAME_standtokneel03 || self->s.frame > FRAME_kneeltoprone06)
		self->oldstance = self->stanceflags;//faf

	self->stanceflags = stance;				//change stanceflags


	if (self->stanceflags == STANCE_CRAWL)
	{
		self->client->anim_end = FRAME_kneeltoprone06;
	}
	else if (self->stanceflags == STANCE_DUCK)
	{
		if (self->oldstance == STANCE_CRAWL)
			self->client->anim_end = FRAME_kneeltoprone01;
		else
			self->client->anim_end = FRAME_standtokneel06;
	}
	else if (self->stanceflags == STANCE_STAND)
	{
//		self->client->anim_end = FRAME_standtokneel03;
	}


	WeighPlayer(self);
	//ent->client->syncspeed = true;

	gi.linkentity (self);
}
/*********************************************************************************************
**  This command is for joining teams, and should be done by each captain
**
**
*********************************************************************************************/
//extern SMos_t def_MOS_List;
//void SP_info_team_start(edict_t * ent );
/*
void Cmd_Create_Team (edict_t *ent)
{
	char *teamname;
	char *mission;
	char *dllname;
	int i;
	edict_t *team; //temp var for team to be spawned
	qboolean found=false;

	if (gi.argc () < 4)
	{
		safe_cprintf(ent, PRINT_HIGH, "usage: create_team teamname dll_name offense|defence|patrol\n");
		return;
	}

	teamname = gi.argv(1);
	dllname = gi.argv(2);
	mission = gi.argv(3);

	for (i=0;i<MAX_TEAMS;i++)
	{

		if (team_list[i])
		{
			if (Q_stricmp(team_list[i]->teamname, teamname) == 0)
			{
				safe_cprintf(ent,PRINT_HIGH,"Team name already in use! ");
				i=MAX_TEAMS;
			}
			else continue;
		}
		else
		{
			team = G_Spawn();	// create temp entity for team spawn

			team->obj_owner = (team_list && team_list[0])?1:0;

			// Knightmare- use gi.TagMalloc instead to avoid memory leak
		//	if ( !(team->message = malloc(sizeof(teamname))) ) break; // create some space for the team name
			if ( !(team->message = gi.TagMalloc(sizeof(teamname), TAG_LEVEL)) ) break; // create some space for the team name

		//	strncpy (team->message, teamname);	// FIXME
			team->message = G_CopyString (teamname);	// Knightmare- use G_CopyString instead to avoid overflowing original buffer
			if ( (Q_stricmp(mission, "Offence") == 0) || (Q_stricmp(mission, "1") == 0) )
				team->count = TEAM_OFFENCE;
			else if ( (Q_stricmp(mission, "Patrol") == 0) || (Q_stricmp(mission, "2") == 0) )
				team->count = TEAM_PATROL;
			else team->count = TEAM_DEFENCE;

			// Knightmare- use gi.TagMalloc instead to avoid memory leak
		//	if ( !(team->pathtarget = malloc(sizeof(dllname))) ) break;	// create some space for the dll name
			if ( !(team->pathtarget = gi.TagMalloc(sizeof(dllname), TAG_LEVEL)) ) break;	// create some space for the dll name

		//	strncpy (team->pathtarget, dllname);	// FIXME
			team->pathtarget =  G_CopyString (dllname);	// Knightmare- use G_CopyString instead to avoid overflowing original buffer

			found = true;

			SP_info_team_start (team);
			free (team->pathtarget); // clean up string memmory
			free (team->message);
			G_FreeEdict (team);		// kill temp entity

			if (ent->client->resp.team_on)
			{
				team_list[i]->units[0] = ent;
				ent->client->resp.team_on = team_list[i];
				team_list[i]->total++;
			}
			break;
		}

	}
	if (!found)
		safe_cprintf (ent, PRINT_HIGH, "Unable to create team.\n");
	else
		safe_bprintf (PRINT_CHAT, "Team %s created.\n",teamname);
}

void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void ChooseTeam(edict_t *ent);
void M_ChooseMOS(edict_t *ent);

void AfterSwitch(edict_t *ent, int type)
{
/*	gi.unlinkentity (ent);
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->ps.gunindex = 0; */

/*	ent->client->limbo_mode=true;

	switch (type) {
	default:
		respawn(ent);
		return;
		break;

	case 1: // Choose Team
		ChooseTeam(ent);
		return;
		break;

	case 2: // Choose class
		M_ChooseMOS(ent);
		return;
		break;

	}
}


void Cmd_Join_team(edict_t *ent)
{
	int					teamnum, i, j;
	qboolean			foundspot;
	char				*arg;
	TeamS_t				*temp;

	if (ent->client->display_info) {
		safe_cprintf(ent, PRINT_HIGH, "You can't join a team while reading objective information! Press Fire!\n");
		return;
	}

	if (gi.argc() < 2)
	{
		safe_cprintf(ent, PRINT_HIGH, "List of teams:\n\n");

		for (i=0;i<MAX_TEAMS;i++)
		{
			if (!team_list[i])
				continue;

			safe_cprintf(ent,PRINT_HIGH," [%d] %s\n",(i + 1), team_list[i]->teamname);
		}

		safe_cprintf(ent, PRINT_HIGH,"\nUsage: join_team #\n");

		return;
	}

	// This is cheating, but it beats working around typecasts
	arg = gi.argv(1);
	if      (Q_stricmp(arg, "1") == 0)
		teamnum = 0;
	else if (Q_stricmp(arg, "2") == 0)
		teamnum = 1;
	else
		teamnum = -1;

	foundspot=false;

	if (teamnum == -1) {
		safe_cprintf(ent, PRINT_HIGH, "Invalid Team Number: %s!\n", arg);
		return;
	}

	if (ent->client->resp.team_on) { //if trying to join team that they are already on
		if (Q_stricmp(team_list[teamnum]->teamname, ent->client->resp.team_on->teamname) == 0) {
			safe_cprintf(ent, PRINT_HIGH, "You're already on the %s team!\n", team_list[teamnum]->teamname);
			return;
		}
	}

	//if list slot is empty
	if (teamnum > MAX_TEAMS || !team_list[teamnum]) {
		safe_cprintf(ent, PRINT_HIGH, "Team %d does not exist!\n", teamnum);
		return;
	}

	for (i = 0; i < MAX_TEAM_MATES; i++)
	{
		if (!team_list[teamnum]->units[i]) // Found an empty slot
		{
			if (ent->client->resp.team_on) // Already on a team
			{
				for (j=0;j<MAX_TEAM_MATES;j++)
				{
					if (ent->client->resp.team_on->units[j]==ent)
					{
						temp=ent->client->resp.team_on;
						temp->units[j]=NULL;
						temp->total--;

						//gi.TagFree(ent->client->resp.team_on);
						ent->client->resp.team_on = 0;
						//j=MAX_TEAM_MATES;
						break;
					}
				}
			}

			ent->client->resp.team_on=team_list[teamnum];
			team_list[teamnum]->total++;
			team_list[teamnum]->units[i]=ent;
			foundspot=true;

			ent->client->resp.mos = NONE; // reset MOS

			// Appropriate Player Model/Skin
			SyncUserInfo(ent, false);

			i=MAX_TEAM_MATES;
		}
	}

	if (!foundspot)
		safe_cprintf(ent,PRINT_HIGH,"No room left on the %s team.\n", team_list[teamnum]->teamname);
	else
	{
		safe_cprintf(ent,PRINT_HIGH,"You have joined the %s team.\n", team_list[teamnum]->teamname);
		ent->flags &= ~FL_GODMODE;
		ent->health = 0;
		meansOfDeath = MOD_CHANGETEAM;

		player_die (ent, ent, ent, 100000, vec3_origin);
		ent->deadflag = DEAD_DEAD;

		AfterSwitch(ent, 2);
	}
}*/
/*
void Cmd_Quit_team(edict_t *ent)
{
	int					j;
	TeamS_t				*temp;

    if (!ent->client->resp.team_on)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not on any teams.\n");
		return;
	}

	if ((level.time - ent->client->respawn_time) < 5)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have to wait before changing teams again.\n");
		return;
	}


	for (j=0;j<MAX_TEAM_MATES;j++)
	{
		if (ent->client->resp.team_on->units[j]==ent)
		{
			temp=ent->client->resp.team_on;
			temp->units[j]=NULL;
			temp->total--;

			//j=MAX_TEAM_MATES;
			break;
		}
	}

	safe_cprintf(ent,PRINT_HIGH,"Removed from the %s team.\n",temp->teamname);

//	gi.TagFree(ent->client->resp.team_on);
	ent->client->resp.team_on = 0;
	ent->client->resp.mos = NONE; //null

	/*
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;

	meansOfDeath = MOD_CHANGETEAM;
	player_die (ent, ent, ent, 100000, vec3_origin);

	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;

	//SwitchToObserver(ent);
	respawn (ent);
	* /

	AfterSwitch(ent, 1);
}
*/

void Cmd_List_team(edict_t *ent)
{
	int i,j;
	safe_cprintf(ent,PRINT_HIGH,"\n\n");
	for (i=0;i<MAX_TEAMS;i++)
	{
		if (!team_list[i]) continue;
		safe_cprintf(ent,PRINT_HIGH,"Team: %s\n",team_list[i]->teamname);
		for (j=0;j<MAX_TEAM_MATES;j++)
			if ( (team_list[i]->units[j]) && (team_list[i]->units[j]->client) )
				safe_cprintf(ent,PRINT_HIGH,"    %s\n",team_list[i]->units[j]->client->pers.netname);
		safe_cprintf(ent,PRINT_HIGH,"\n");
	}
}


/*
==================
Cmd_Reload_f()
Handles weapon reload requests
==================
*/

qboolean Cmd_Reload (edict_t *ent)
{
//	int rds_left;           //+BD - Variable to handle rounds left
	int	mags_left;

	gitem_t *ammo_item = NULL;
	int		ammo_index = 0, *ammo_amount = NULL;

	if (!ent ||
		!ent->client ||
		!ent->client->pers.weapon ||
		!ent->client->ammo_index || !ent->client->p_rnd ||
		!ent->client->pers.inventory[ent->client->ammo_index] ||
		 ent->client->limbo_mode || ent->deadflag == DEAD_DEAD)
		return true;

	//hack
	if (ent->client->pers.weapon->classnameb != WEAPON_PANZERFAUST)
	{if (ent->client->weaponstate == WEAPON_FIRING ||
		ent->client->weaponstate == WEAPON_LOWER)
		return false;}

	if (ent->client->weaponstate &&
		ent->client->weaponstate == WEAPON_RELOADING &&
		ent->client->pers.weapon &&
		ent->client->pers.weapon->position == LOC_SHOTGUN)
	{
		ent->client->weaponstate = WEAPON_READY;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"),1, ATTN_NORM, 0);//silences reload sound
		return true;
	}


	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
	{
		ent->client->crosshair_offset_x = 0;
		ent->client->crosshair_offset_y = 0;
	}


	if (ent->client->pers.weapon->ammo)
	{
		ammo_item = FindItem(ent->client->pers.weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		ammo_amount = &ent->client->pers.inventory[ammo_index];
	}

    // Grab the current magazine max count...
	if ( ent->client->pers.weapon->topoff)
	{

		// rezmoth - bug here crashes because of a null round variable?
		// hardcode a full clip into each gun on weapon pickup to prevent it?
		if (ent->client->pers.weapon->topoff==3 &&
			 ent->client->p_rnd &&
			*ent->client->p_rnd)
		{
			if (ent->client->pers.weapon->position == LOC_ROCKET)
				safe_cprintf(ent, PRINT_HIGH, "%s already loaded!\n", ent->client->pers.weapon->pickup_name);
			else
				safe_cprintf(ent, PRINT_HIGH, "You can't top off the %s!\n", ent->client->pers.weapon->pickup_name);

			return true;
		}

		mags_left= ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo))];
	} else
		return true;

	// rezmoth - bug here crashes for what reason?
	if (*ammo_amount &&
		ent->client->p_rnd && //faf
		*ent->client->p_rnd == ammo_item->quantity) {
			safe_cprintf(ent, PRINT_HIGH, "You still have a full magazine left!\n");
			return true;
	}

	WeighPlayer(ent);

	if (mags_left)
	{
		if ( (ent->client->weaponstate != WEAPON_END_MAG) &&
			(mags_left==1)
		  /*(ent->client->pers.inventory[ent->client->ammo_index] < rds_left)*/)
		{
/* eh don't even need this			if (ent->client->pers.weapon->position == LOC_RIFLE)
				safe_cprintf(ent, PRINT_HIGH, "Last Clip!\n");
			else if (ent->client->pers.weapon->position == LOC_ROCKET)
				safe_cprintf(ent, PRINT_HIGH, "Last Rocket!\n");
			else if (ent->client->pers.weapon->position == LOC_PISTOL)
				safe_cprintf(ent, PRINT_HIGH, "Last Clip!\n");
			else if (ent->client->pers.weapon->position == LOC_FLAME) //Wheaty: Flamethrower
				safe_cprintf(ent, PRINT_HIGH, "Last Fuel Tank!\n");
			else
				safe_cprintf(ent, PRINT_HIGH, "Last Magazine!\n");
*/


			ent->client->weaponstate = WEAPON_RELOADING;
//bcass start - truesite speed after reload
			ent->client->aim=false;  //they're out of truesite now
//			ClientSetMaxSpeed(ent, true); //make them move the right speed
			WeighPlayer(ent);
//bcass end

			return true;
		}
        else
		{
			ent->client->weaponstate = WEAPON_RELOADING;
//bcass start - truesite speed after reload
			ent->client->aim=false;  //they're out of truesite now
//			ClientSetMaxSpeed(ent, true); //make them move the right speed
			WeighPlayer(ent);
//bcass end
			return true;
		}
	}
    else
	{
		ent->client->weaponstate = WEAPON_READY;
		safe_cprintf(ent,PRINT_HIGH,"You're out of ammo!\n");
	}
	return true;
}

void Cmd_Reload_f (edict_t *ent)
{
	if (Cmd_Reload(ent) == false)
		ent->client->reloadtry = true;
	else
		ent->client->reloadtry = false;

	return;
}
void turret_off (edict_t *self);

void Cmd_Stance(edict_t *ent)
{
	vec3_t	start;
	vec3_t	end;
	vec3_t  up;
	trace_t	tr;

//	safe_centerprintf(ent, "11111\n");

	if (ent->flyingnun)
	{
		ent->s.origin[2]-=10;
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if (!ent->client->resp.AlreadySpawned || ent->deadflag)
		return;
//	safe_centerprintf(ent, "22222\n");

//	if (ent->waterlevel > 1)
//		return;

	//safe_cprintf(ent, PRINT_HIGH, "Use +movedown instead.\n");



	turret_off (ent);

	//faf:  keep player's view inside the map
	if (ent->stanceflags == STANCE_CRAWL)
	{
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		VectorSet(up, 0, 0, 1);
		VectorMA(start, 48, up, end);

		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);


		if (tr.fraction < 1.0 )	//can't stand so go to crouch
		{

				change_stance(ent, STANCE_DUCK);
				return;

		}
	}

//	safe_centerprintf(ent, "33333\n");

	if (ent->stanceflags == STANCE_STAND) change_stance(ent, STANCE_DUCK);
	else if (ent->stanceflags == STANCE_DUCK) change_stance(ent, STANCE_CRAWL);
	else if (ent->stanceflags == STANCE_CRAWL) change_stance(ent, STANCE_STAND);
	WeighPlayer(ent);
}


#define MAX_FILENAME_LENGTH 20

void Cmd_Shout_f(edict_t *ent)
{
	char filename[MAX_FILENAME_LENGTH];
	char soundfile[50];
	int i;
	qboolean newshout = true;

	//faf
	if (ent->deadflag || ent->client->limbo_mode)
		return;
	if (ent->client->last_shout_time > (level.time - .5))
		return;

	if (ent->burnout) //on fire
		return;

	if (!ent->client->resp.team_on || ent->client->resp.mos == NONE || ent->deadflag == DEAD_DEAD)
		return;

	if (gi.argc()<1)
	{
		safe_cprintf(ent, PRINT_HIGH,"Usage: shout wavefile (no \".wav\")\n");
		return;
	}

	if (strlen(gi.argv(1))>MAX_FILENAME_LENGTH-6)
	{
		safe_cprintf(ent, PRINT_HIGH, "Filename must be less than %i characters long.\n",MAX_FILENAME_LENGTH-6);
		return;
	}


	if (!strcmp(gi.argv(1), "thanks1") ||
		!strcmp(gi.argv(1), "thanks2") ||
		!strcmp(gi.argv(1), "yes1") ||
		!strcmp(gi.argv(1), "yes2") ||
		!strcmp(gi.argv(1), "no1") ||
		!strcmp(gi.argv(1), "no2") ||
		!strcmp(gi.argv(1), "cover1") ||
		!strcmp(gi.argv(1), "cover2") ||
		!strcmp(gi.argv(1), "cover3") ||
		!strcmp(gi.argv(1), "move1") ||
		!strcmp(gi.argv(1), "move2") ||
		!strcmp(gi.argv(1), "left1") ||
		!strcmp(gi.argv(1), "left2") ||
		!strcmp(gi.argv(1), "right1") ||
		!strcmp(gi.argv(1), "right2") ||
		!strcmp(gi.argv(1), "medic") ||
		!strcmp(gi.argv(1), "attack1") ||
		!strcmp(gi.argv(1), "attack2") ||
		!strcmp(gi.argv(1), "retreat1") ||
		!strcmp(gi.argv(1), "retreat2") ||
		!strcmp(gi.argv(1), "shoulders1") ||
		!strcmp(gi.argv(1), "shoulders2") ||
		!strcmp(gi.argv(1), "incoming1") ||
		!strcmp(gi.argv(1), "incoming2") ||
		!strcmp(gi.argv(1), "follow1") ||
		!strcmp(gi.argv(1), "follow2") ||
		!strcmp(gi.argv(1), "funny1") ||
		!strcmp(gi.argv(1), "funny2") ||
		!strcmp(gi.argv(1), "funny3") ||
		!strcmp(gi.argv(1), "sniper1") ||
		!strcmp(gi.argv(1), "sniper2") ||
		!strcmp(gi.argv(1), "cease1") ||
		!strcmp(gi.argv(1), "cease2") ||
		!strcmp(gi.argv(1), "weapon1") ||
		!strcmp(gi.argv(1), "weapon2") ||
		!strcmp(gi.argv(1), "grenade1") ||
		!strcmp(gi.argv(1), "eye1") ||
		!strcmp(gi.argv(1), "thanks1") ||
		!strcmp(gi.argv(1), "smoke1") ||
		!strcmp(gi.argv(1), "help1"))
	{
		newshout = false;
	}

	Q_strncpyz (filename, sizeof(filename), gi.argv(1));

	for (i = 0; filename[i]; i++)
		filename[i] = tolower(filename[i]);

	Q_strncatz (filename, sizeof(filename), ".wav\0");

	Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/", ent->client->resp.team_on->teamid));
	Q_strncatz (soundfile, sizeof(soundfile), filename);

	if (newshout)
	{
		for (i=1; i<= user_shout_count; i++)
		{
			//gi.dprintf("s:%s  u:%s\n", soundfile, user_shouts[i]);
			if (!strcmp (soundfile, user_shouts[i]) )
				newshout= false;

		}
		if (newshout)
		{
			if (user_shout_count >19)
			{
				safe_cprintf(ent, PRINT_HIGH, "Too many user shouts already, sorry!\n");
				return;
			}

			user_shout_count++;

			Com_sprintf (user_shouts[user_shout_count], sizeof(user_shouts[user_shout_count]), "%s", soundfile);


			//gi.dprintf("usc: %i   %s\n",user_shout_count,user_shouts[user_shout_count]);
		}
	}






	gi.sound (ent, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);
}

void Cmd_AutoPickUp_f (edict_t *ent) {

	if (!ent->client->resp.AlreadySpawned)
		return;


	if (!ent->client->resp.autopickup) {
		if (nohud->value)
			safe_cprintf(ent, PRINT_HIGH, "Auto Item pickup enabled.\n");
		ent->client->resp.autopickup = true;
	} else {
		if (nohud->value)
			safe_cprintf(ent, PRINT_HIGH, "Auto Item pickup disabled.\n");
		ent->client->resp.autopickup = false;
	}
}

void Cmd_PlayerID_f (edict_t *ent) {
//	if (!ent->client->resp.AlreadySpawned)
//		return;

	if (!ent->client->resp.show_id) {
		safe_cprintf(ent, PRINT_HIGH, "player id enabled\n");
		ent->client->resp.show_id = true;
	} else {
		safe_cprintf(ent, PRINT_HIGH, "player id disabled\n");
		ent->client->resp.show_id = false;
	}

}

void Cmd_Medic_Call_f (edict_t *ent) {

	//gi.sound (ent, CHAN_VOICE, gi.soundindex("shout/medic.wav"), 1, ATTN_NORM, 0);

	if (ent->client->medic_call < level.framenum)
		ent->client->medic_call = (level.framenum + MEDIC_CALL_TIME);
}

void Cmd_MOTD (edict_t *ent)
{
	//ala MOTD
	char	filename[256];	// Knightmare added
	FILE	*motd_file;
	char	motd[1000];
	char	line[100];
#ifndef _WIN32
	int		i;
	size_t  len;
#endif // _WIN32

//	if (motd_file = fopen(GAMEVERSION "/motd.txt", "r") )
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/motd.txt", GameDir());
	motd_file = fopen(filename, "r");
	if (motd_file != NULL)
	// end Knightmare
	{
		// we successfully opened the file "motd.txt"
		if ( fgets(motd, 900, motd_file) )
		{
		// Knightmare- replace carriage returns on Linux
#ifndef _WIN32
			len = strlen(motd);
			for (i = 0; i < len; i++) {
				if (motd[i] == '\r')
					motd[i] = ' ';
			}
#endif // _WIN32
			// we successfully read a line from "motd.txt" into motd
			// ... read the remaining lines now
			while ( fgets(line, 100, motd_file) )
			{
		// Knightmare- replace carriage returns on Linux
#ifndef _WIN32
				len = strlen(line);
				for (i = 0; i < len; i++) {
					if (line[i] == '\r')
						line[i] = ' ';
				}
#endif // _WIN32
				// add each new line to motd, to create a BIG message string.
				// we are using strncatz: STRing conCATenation function here.
				Q_strncatz (motd, sizeof(motd), line);
			}

			// print our message.
			safe_centerprintf (ent, motd);
		}
		// be good now ! ... close the file
		fclose(motd_file);
	}
}

/*
=================
ClientCommand
=================
*/
/*
  A couple of rules here. first off, I had to select something reasonable
  for the number of arguments that could be specified on the command line.
  I chose three. If you need more, declare your routine as needing 0 and
  manipulate the gi.argv structure in your routine.
  Secondly, the first command found is the one that gets executed.
  Thirdly, the routine is not particularly efficient in finding and managing
  the entries. That will be a future upgrade.
  */


/*
  takes a cmd structure and inserts it into the GlobalCommandList.
  instead of allocating additional memory, it utilizes the memory allocated
  to hold the commands and inserts a pointer to it.

  Note that we are just keeping a list of all the commands from each structure
  together.
  */
void InsertCmds (g_cmds_t *cmds, int numCmds, char *src)
{
    struct cmd_list_t **ptr;
    struct cmd_list_t *tmp;

        gi.dprintf("processing %s commands\n",src);
    ptr = &GlobalCommandList;
    while(*ptr)
        ptr = &((*ptr)->next);

      /*at this point, ptr is pointing to a pointer var whose value is NULL*/
      /*not sure if I should be using malloc*/
    tmp = (struct cmd_list_t *) gi.TagMalloc(sizeof(struct cmd_list_t), TAG_GAME);
    tmp->commands = cmds;
    //tmp->numCmds = sizeof(cmds) / sizeof(struct g_cmds_s);
    tmp->numCmds = numCmds;
    strncpy(tmp->source,src,32);
    tmp->next = NULL;

//      gi.dprintf("sizeof(*cmds) = %d, sizeof(struct g_cmds_s) = %d\n", sizeof(*cmds),sizeof(struct g_cmds_s));
//      gi.dprintf("number of commands processed = %d\n",tmp->numCmds);

    *ptr = tmp;
}


/*
  This function walks the global command list and prints out all the commands that it finds.
*/
void PrintCmds (void)
{
    struct cmd_list_t *ptr;
    g_cmds_t *tmp;
        int i;

    ptr = GlobalCommandList;
    while(ptr)
    {
        gi.dprintf("printing <%s> commands:\n",ptr->source);
        tmp = ptr->commands;
        for (i=0;i < ptr->numCmds; i++, tmp++)
                gi.dprintf("%s has %d args.\n",tmp->command, tmp->numArgs);
        ptr = ptr->next;
    }
	gi.dprintf("%i\n",game.num_items);
}


/*
  this function frees all the memory that has been allocated by the
  InsertCmds function.
*/
void CleanUpCmds (void)
{
    struct cmd_list_t *tmp1, *tmp2;

    tmp1 = GlobalCommandList;
    while (tmp1)
    {
        tmp2 = tmp1->next;
        gi.TagFree(tmp1);
        tmp1 = tmp2;
    }
    GlobalCommandList = NULL;
}


/*
  this is a crucial function. It searches the GlobalCommandList, looking
  for the command that is passed in. It performs a two dimensional search,
  first going from one set of commands to another and searching inside
  for the command. It returns a pointer to the g_cmds_t structure that has
  the command in it.

  I recommend that the commands be placed in individual namespaces, that is,
  each mod have it's own prefix that is placed in front of the command.
  */
g_cmds_t *FindCommand(char *cmd)
{
    struct cmd_list_t *sets;
    g_cmds_t *cmds;
    int i, found;
    char nmspace[64], *ptr;

        //we first need to find the namespace, if any
    nmspace[0] = '\0';
    ptr = cmd; i = found = 0;
    while (*ptr && i<32 && !found)
    {
        if (*ptr == '.')
            found = 1;
        else
        {
            nmspace[i] = *ptr;
            i++;
            ptr++;
        }
    }
    if (found)
    {
        cmd = ++ptr;
        nmspace[i] = '\0';
    }
    else
        nmspace[0] = '\0';

    sets = GlobalCommandList;
    while (sets)
    {
        if (found)
            if (strcmp(nmspace,sets->source))
            {
                sets = sets->next;
                continue;
            }

        cmds = sets->commands;

        for (i=0;i<sets->numCmds;i++) {
			if (!cmd || !cmds[i].command)
				return NULL;

            if (Q_stricmp(cmd, cmds[i].command) == 0)
                return &(cmds[i]);
		}
        sets = sets->next;
    }
    return NULL;
}


void ClientCommand (edict_t *ent)
{
    char        *cmd;
    g_cmds_t *cmdptr;

    if (!ent->client)
        return;         // not fully in game yet

	//JABot[start]
	if (BOT_Commands(ent))
		return;
	//[end]


    cmd = gi.argv(0);

	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f(ent, false, false, false);
		return;
	}
	else if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f(ent, true, false, false);
		return;
	}


    cmdptr = FindCommand(cmd);
    if (cmdptr)
    {
        switch (cmdptr->numArgs)
        {
          case 0:
              (cmdptr->cmdfunc)();
              break;
          case 1:
              (cmdptr->cmdfunc)(ent);
              break;
          case 2:
              (cmdptr->cmdfunc)(ent,gi.argv(1));
              break;
          case 3:
              (cmdptr->cmdfunc)(ent,gi.argv(1),gi.argv(2));
              break;
          case 4:
              (cmdptr->cmdfunc)(ent,gi.argv(1),gi.argv(2),gi.argv(3));
              break;

		}
    }
	else
		safe_cprintf(ent, PRINT_HIGH, "Invalid Command: %s!\n", cmd);
}
