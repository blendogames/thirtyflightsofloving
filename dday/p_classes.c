/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_classes.c,v $
 *   $Revision: 1.10 $
 *   $Date: 2002/07/23 22:48:27 $
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
#include "p_classes.h"

// g_classes.c
// D-Day: Normandy Player Classes
// this file is for definitions for classes in DDay.


//fills a gun with bullets
void Load_Weapon (edict_t *ent, gitem_t	*item)
{
	gitem_t	*ammo_item;

	// Loads primary weapon when spawning
	ammo_item = FindItem(item->ammo);
	if (!ammo_item)
	{
		gi.dprintf("WARNING: in Give_Class_Weapon %s spawned with no ammo for %s -> %s\n", ent->client->pers.netname, item->pickup_name, item->ammo);
		return;
	}


	if (!strcmp(item->ammo, "mauser98k_mag") && !strcmp(item->pickup_name, "Mauser 98k"))
		ent->client->mags[1].rifle_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "mauser98k_mag") && !strcmp(item->pickup_name, "Mauser 98ks"))
		ent->client->mags[1].sniper_rnd = ammo_item->quantity;

        else if (!strcmp(item->dllname, team_list[1]->teamid))  //faf:  if its a team 1 weap...(usually grm)
        {
                if (item->position == LOC_PISTOL)
                        ent->client->mags[1].pistol_rnd = ammo_item->quantity;
                else if (item->position == LOC_SUBMACHINEGUN)
                        ent->client->mags[1].submg_rnd = ammo_item->quantity;
                else if (item->position == LOC_L_MACHINEGUN)
                        ent->client->mags[1].lmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_H_MACHINEGUN)
                        ent->client->mags[1].hmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_ROCKET)
                        ent->client->mags[1].antitank_rnd = ammo_item->quantity;
                else if ((item->position == LOC_RIFLE))
                        ent->client->mags[1].rifle_rnd = ammo_item->quantity;
                else if ((item->position == LOC_SNIPER))
                        ent->client->mags[1].sniper_rnd = ammo_item->quantity;
                else if ((item->position == LOC_SHOTGUN))
                        ent->client->mags[1].shotgun_rnd = ammo_item->quantity;
                 else if (item->position == LOC_SUBMACHINEGUN2)
                        ent->client->mags[1].submg2_rnd = ammo_item->quantity;
       }

        else if (!strcmp(item->dllname, team_list[0]->teamid))  //usually allied weapons here...
        {
                if (item->position == LOC_PISTOL)
                        ent->client->mags[0].pistol_rnd = ammo_item->quantity;
                else if (item->position ==  LOC_RIFLE)
                        ent->client->mags[0].rifle_rnd = ammo_item->quantity;
                else if (item->position == LOC_SNIPER)
                        ent->client->mags[0].sniper_rnd = ammo_item->quantity;  //faf:  not used for usa but so plugin team 1 can use same ammo for inf rifle and sniper rifle
                else if (item->position == LOC_SUBMACHINEGUN)
                        ent->client->mags[0].submg_rnd = ammo_item->quantity;
                else if (item->position == LOC_L_MACHINEGUN)
                        ent->client->mags[0].lmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_H_MACHINEGUN)
                        ent->client->mags[0].hmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_ROCKET)
                        ent->client->mags[0].antitank_rnd = ammo_item->quantity;
                else if (item->position == LOC_SNIPER)
                        ent->client->mags[0].sniper_rnd = ammo_item->quantity;
                else if (item->position == LOC_SHOTGUN)
                        ent->client->mags[0].shotgun_rnd = ammo_item->quantity;
                 else if (item->position == LOC_SUBMACHINEGUN2)
                        ent->client->mags[0].submg2_rnd = ammo_item->quantity;
       }

        else if (!strcmp(item->ammo, "flame_mag"))
                ent->client->flame_rnd = ammo_item->quantity;
	
}



gitem_t *FindTeamItem (char *dllname, int position);
void Give_Class_Weapon(edict_t *ent)
{
	gitem_t		*item;
	gclient_t	*client;
	gitem_t		*ammo_item;

	client=ent->client;


	if (knifefest->value)
	{
		item = FindItem("Helmet");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Fists");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Knife");
		client->pers.inventory[ITEM_INDEX(item)] = 5;
		client->pers.selected_item=ITEM_INDEX(item);
		client->newweapon=item;
		ChangeWeapon(ent);
		return;
	}


	if (client->resp.mos == ENGINEER)
	{
		item = FindItem("Sandbags");
		client->pers.inventory[ITEM_INDEX(item)]= 2;

	}

	//give everyone a knife & fists & helmet

		item = FindItem("Helmet");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Fists");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Knife");
		client->pers.inventory[ITEM_INDEX(item)] = 1;

	// faf rifle-only code  //ddaylife 
	if ((mauser_only->value == 1) && !(client->resp.mos == MEDIC))
	{
			item= FindTeamItem(team_list[1]->teamid, LOC_RIFLE);
	}
	else if ((sniper_only->value == 1) && !(client->resp.mos == MEDIC))
	{
			item= FindTeamItem(team_list[(client->resp.team_on->index)]->teamid, LOC_SNIPER);
	}
	else if (swords->value == 1 && client->resp.mos != MEDIC)
	{
			item= FindItem("Sword");
	}

	else
		item=FindItem(client->resp.team_on->mos[client->resp.mos]->weapon1);


	// Loads primary weapon when spawning
	Load_Weapon (ent, item);
	

	if (!item) { //pbowens: prevents from crashing the game
		safe_cprintf(ent, PRINT_HIGH, "weapon1 item not found!\n");
		return;
	}

	client->pers.selected_item=ITEM_INDEX(item);
	client->newweapon=item;
	client->pers.inventory[client->pers.selected_item]=1;

	item = NULL;
	//if (client->resp.team_on->mos[client->resp.mos]->weapon2)
	//{
		if (item=FindItem(client->resp.team_on->mos[client->resp.mos]->weapon2))
		client->pers.inventory[ITEM_INDEX(item)]=1;

	// Loads secondary weapon, if existant, when spawning
	if (item)
	{
		ammo_item = FindItem(item->ammo);
		if (!strcmp(item->dllname, team_list[1]->teamid) && item->position == LOC_PISTOL)
			ent->client->mags[1].pistol_rnd = ammo_item->quantity;
		else if (!strcmp(item->dllname, team_list[0]->teamid) && item->position == LOC_PISTOL)
			ent->client->mags[0].pistol_rnd = ammo_item->quantity;

		/*
		ammo_item = FindItem(item->ammo);
		if (!strcmp(item->ammo, "p38_mag"))
			ent->client->mags[1].pistol_rnd = ammo_item->quantity;
		else if (!strcmp(item->ammo, "colt45_mag"))
			ent->client->mags[0].pistol_rnd = ammo_item->quantity;
		*/
	}

	//}
	//if (client->resp.team_on->mos[client->resp.mos]->grenades)
	//{

	if (!no_nades->value)//ddaylife
		if (item=FindItem(client->resp.team_on->mos[client->resp.mos]->grenades) )
			client->pers.inventory[ITEM_INDEX(item)]=client->resp.team_on->mos[client->resp.mos]->grenadenum;
	//}
	//if (client->resp.team_on->mos[client->resp.mos]->special)
	//{
		if (item=FindItem(client->resp.team_on->mos[client->resp.mos]->special) )
		client->pers.inventory[ITEM_INDEX(item)]=client->resp.team_on->mos[client->resp.mos]->specnum;
	//}

	ChangeWeapon(ent);
}
			


//this function is for giving the player ammo...
void Give_Class_Ammo(edict_t *ent)
{
	gitem_t *item,*item2;
	
	if (ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo1 )
	{
		if ((mauser_only->value == 1) && !(ent->client->resp.mos == MEDIC))
			item= FindTeamItem(team_list[1]->teamid, LOC_RIFLE);
		else if ((sniper_only->value == 1) && !(ent->client->resp.mos == MEDIC))
			item= FindTeamItem(team_list[(ent->client->resp.team_on->index)]->teamid, LOC_SNIPER);
		else
			item=FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1);

		item2=FindItem(item->ammo);
		Add_Ammo(ent,item2,ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo1);
	}

	if (ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo2 )
	{
		item=FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2);
		item2=FindItem(item->ammo);
		Add_Ammo(ent,item2,ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo2);
	}
	
	
}


void Show_Mos(edict_t *ent)
{
/*	int i;
	
	if (!ent->client->resp.team_on || !ent->client->resp.mos)
	{
		safe_cprintf(ent,PRINT_HIGH,"Must be on a team to view the open class slots.\n");
		return;
	}

	safe_cprintf(ent,PRINT_HIGH,"\nOpen class slots for %s: \n",ent->client->resp.team_on->teamname);

	for (i=1; i < MAX_MOS; i++) {
		if (ent->client->resp.team_on->mos[i]->available == 99) 
		{
			safe_cprintf(ent, PRINT_HIGH," %10s -- unlimited\n", ent->client->resp.team_on->mos[i]->name);
		} else {
			safe_cprintf(ent, PRINT_HIGH," %10s -- %i\n", 
			ent->client->resp.team_on->mos[i]->name,
			ent->client->resp.team_on->mos[i]->available);
		}
	} */
}
 


void InitMOS_List(TeamS_t *team, SMos_t *mos_list)
{
	int		i;
	
	SMos_t **MOS;
	team->mos = MOS = gi.TagMalloc( (sizeof(SMos_t)*MAX_MOS),TAG_LEVEL);
	
	MOS[0] = NULL;
	for (i=1;i<MAX_MOS;i++)
	{
		MOS[i] = gi.TagMalloc( sizeof(SMos_t), TAG_LEVEL);
		MOS[i]->name = gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->weapon1 = gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->weapon2 = gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->grenades = gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->special = gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->MOS_Spaw_Point = gi.TagMalloc( sizeof(mos_list[i].MOS_Spaw_Point), TAG_LEVEL);

		Q_strncpyz (MOS[i]->name, MAX_MOS_NAME_CHAR, mos_list[i].name);

		if (mos_list[i].weapon1)
			Q_strncpyz (MOS[i]->weapon1, MAX_MOS_NAME_CHAR, mos_list[i].weapon1);
		MOS[i]->ammo1 = mos_list[i].ammo1;

		if (mos_list[i].weapon2) 
			Q_strncpyz (MOS[i]->weapon2, MAX_MOS_NAME_CHAR, mos_list[i].weapon2);
		MOS[i]->ammo2 = mos_list[i].ammo2;
		
		if (mos_list[i].grenades) 
			Q_strncpyz (MOS[i]->grenades, MAX_MOS_NAME_CHAR, mos_list[i].grenades);
		MOS[i]->grenadenum = mos_list[i].grenadenum;
		
		if (mos_list[i].special)
			Q_strncpyz (MOS[i]->special, MAX_MOS_NAME_CHAR, mos_list[i].special);

		MOS[i]->specnum = mos_list[i].specnum;
		MOS[i]->mos = mos_list[i].mos;
		MOS[i]->available = mos_list[i].available;
		MOS[i]->normal_weight = mos_list[i].normal_weight;
		MOS[i]->max_weight = mos_list[i].max_weight;
		MOS[i]->speed_mod = mos_list[i].speed_mod;
		MOS[i]->MOS_Spaw_Point = mos_list[i].MOS_Spaw_Point;
		MOS[i]->skinname = mos_list[i].skinname;
	}

	//team->language = language;
	//team->playermodel = playermodel;
	//team->teamid = teamid;

	//team->mos gi.TagMalloc( (sizeof(char)*playermodel),TAG_LEVEL);
}
