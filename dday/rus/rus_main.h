/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_main.h,v $
 *   $Revision: 1.6 $
 *   $Date: 2002/06/04 19:49:50 $
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

#define USER_EXCLUDE_FUNCTIONS 1

#include "rus_defines.h"
#include "../g_local.h"
#include "../g_cmds.h"

// Knightmare- made this var extern to fix compile on GCC
extern int rus_index;

void Weapon_Pistol_Fire (edict_t *ent);
void Weapon_Rifle_Fire (edict_t *ent);
void Weapon_Submachinegun_Fire (edict_t *ent);
void Weapon_LMG_Fire (edict_t *ent);
void Weapon_HMG_Fire (edict_t *ent);
void Weapon_Rocket_Fire (edict_t *ent);
void Weapon_Sniper_Fire (edict_t *ent);
void Weapon_Ppsh41_Fire (edict_t *ent);
void Weapon_Pps43_Fire (edict_t *ent);
void Weapon_Molotov_Fire (edict_t *ent);

void Weapon_tt33(edict_t *ent);
void Weapon_m9130(edict_t *ent);
void Weapon_ppsh41(edict_t *ent);
void Weapon_pps43(edict_t *ent);
void Weapon_dpm(edict_t *ent);
void Weapon_RPG1(edict_t *ent);
void Weapon_m9130s(edict_t *ent);
void Weapon_Molotov(edict_t *ent);


void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_LFIRE_LAST, int FRAME_LIDLE_LAST, int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST,
					 int FRAME_DEACTIVATE_LAST, int FRAME_RAISE_LAST,int FRAME_AFIRE_LAST, int FRAME_AIDLE_LAST,
					 int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
