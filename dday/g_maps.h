/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_maps.h,v $
 *   $Revision: 1.7 $
 *   $Date: 2002/06/04 19:49:46 $
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

FILE *DDay_OpenFile (char *filename_ptr); 
void DDay_CloseFile (FILE *fp);

#define MAX_MAPS           64 
#define MAX_MAPNAME_LEN    16 

#define ML_ROTATE_SEQ          0 
#define ML_ROTATE_RANDOM       1 
#define ML_ROTATE_NUM_CHOICES  2 
  


typedef struct 
{ 
   char filename[21];     // filename on server (20-char max length) 
   int  nummaps;          // number of maps in list 
   char mapnames[MAX_MAPS][MAX_MAPNAME_LEN]; 
   char rotationflag;     // set to ML_ROTATE_* 
   int  currentmap;       // index to current map 
} maplist_t; 
  
// Knightmare- made this var extern to fix compile on GCC
extern maplist_t maplist; 
  

int  LoadMapList         (char *filename); 
void ClearMapList        (void); 
void Cmd_Maplist_f       (edict_t *ent); 
void Svcmd_Maplist_f     (void); 
void DisplayMaplistUsage (edict_t *ent); 
void ShowCurrentMaplist  (edict_t *ent);
