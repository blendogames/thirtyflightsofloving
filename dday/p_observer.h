/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_observer.h,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:49 $
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

void ChooseMOS(edict_t *ent,qboolean smos);
void ChooseTeam(edict_t *ent);
void SyncUserInfo(edict_t *ent, qboolean pers);
void ClearShow (edict_t *ent);

void stuffcmd(edict_t *ent, char *s);
void DoEndOM(edict_t *ent);

void M_Team_Join	(edict_t *ent, pmenu_t *p, int choice);
void M_MOS_Join		(edict_t *ent, pmenu_t *p, int choice);
void M_Main_Menu	(edict_t *ent, pmenu_t *p, int choice);
void M_Team_Choose	(edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits	(edict_t *ent, pmenu_t *p, int choice);
void MainMenu(edict_t *ent);
void M_View_Credits_Shaef (edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits_Vipersoft (edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits_GBR (edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits_ITA (edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits_JPN (edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits_etc (edict_t *ent, pmenu_t *p, int choice);
void M_Class_Choose (edict_t *ent, pmenu_t *p, int choice);
void Cmd_FlyingNunMode_f(edict_t *ent);
void Cmd_DDHelp_f(edict_t *ent);
