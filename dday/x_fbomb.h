/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/x_fbomb.h,v $
 *   $Revision: 1.5 $
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
 
 /*==========================================================================
//  x_fbomb.h -- by Patrick Martin              Last updated:  3-5-1998
//--------------------------------------------------------------------------
//  This is the header file for x_fbomb.c.
//========================================================================*/

static void FireGrenade_Explode (edict_t *ent);
static void FireGrenade_Touch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_flamegrenade
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius);
void fire_flamegrenade2
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius, qboolean held);
void firerocket_touch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_flamerocket
 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
  float damage_radius, int radius_damage);


/*=============================  END OF FILE =============================*/
