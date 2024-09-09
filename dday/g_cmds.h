/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_cmds.h,v $
 *   $Revision: 1.11 $
 *   $Date: 2002/07/23 21:11:37 $
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

 /*
  g_cmds.h

  contains definitions to allow dynamic functionality added to Quake2

  vjj   01/20/98
  */

typedef struct g_cmds_s
{
    char *command;
    int numArgs;
    void (*cmdfunc)();
} g_cmds_t;




void InsertCmds (g_cmds_t *cmds, int numCmds, char *src);
void CleanUpCmds (void);
g_cmds_t *FindCommand (char *cmd);
void PrintCmds (void);
void Cmd_GameVersion_f (edict_t *ent);
void SelectNextItem (edict_t *ent,int itflags);
void SelectPrevItem (edict_t *ent,int itflags);
void ValidateSelectedItem (edict_t *ent);
void Cmd_Give_f (edict_t *ent);
void Cmd_God_f (edict_t *ent);
void Cmd_Notarget_f (edict_t *ent);
void Cmd_Noclip_f (edict_t *ent);
void Cmd_Use_f (edict_t *ent);
void Cmd_Drop_f (edict_t *ent);
void Cmd_Inven_f (edict_t *ent);
void Cmd_InvUse_f (edict_t *ent);
void Cmd_WeapPrev_f (edict_t *ent);
void Cmd_WeapNext_f (edict_t *ent);
void Cmd_InvDrop_f (edict_t *ent);
void Cmd_Kill_f (edict_t *ent);
void Cmd_PutAway_f (edict_t *ent);
void ClientCommand (edict_t *ent);
void Cmd_Stance (edict_t *ent);
//void Cmd_MOS(edict_t *ent);
//void Cmd_Create_Team (edict_t *ent);
//void Cmd_Join_team (edict_t *ent);
void Cmd_List_team (edict_t *ent);
//void Cmd_Quit_team (edict_t *ent);
void Cmd_Shout_f (edict_t *ent);
void Cmd_DDebug_f (edict_t *ent);
void Cmd_DDHelp_f (edict_t *ent);
void Cmd_Maplist_f (edict_t *ent);
void Cmd_AutoPickUp_f (edict_t *ent);
void Cmd_Menu_Main_f (edict_t *ent);
void Cmd_Menu_Team_f (edict_t *ent);
void Cmd_Menu_Class_f (edict_t *ent);
void Cmd_PlayerID_f (edict_t *ent);
void Cmd_Medic_Call_f (edict_t *ent);
void Cmd_MOTD (edict_t *ent);
void Cmd_Arty_f (edict_t *ent);
void Cmd_Objectives_Toggle (edict_t *ent);


#define NUM_ID_CMDS     48
// Knightmare- made this var extern to fix compile on GCC
extern g_cmds_t id_GameCmds[NUM_ID_CMDS];

struct cmd_list_t
{
    char source[NUM_ID_CMDS];
    struct cmd_list_t *next;
    g_cmds_t *commands;
    int numCmds;
};
