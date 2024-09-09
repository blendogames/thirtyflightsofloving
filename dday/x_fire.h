/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/x_fire.h,v $
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
//  x_fire.h -- by Patrick Martin               Last updated:  6-18-1998
//--------------------------------------------------------------------------
//  This is the header file for x_fire.c.  Any files that use the
//  functions in x_fire.c must include this file.
//========================================================================*/

/***************/
/*  CONSTANTS  */
/***************/

/* Time in seconds the entity on fire will burn. */
#define BURN_TIME   30

/* Default radius for fire splash damage. */
#define RADIUS_FIRE_SPLASH   80

/* Fireflags.
        SHIFT_Z      =  Vertical flame shift.
        DOWN         =  Shift flame down, not up.
        DELTA_ALIVE  =  Adjust only if target is alive.
        DELTA_BASE   =  Base adjustments (+4 if alive, -18 if dead).
        DELTA_VIEW   =  Adjust w/ viewheight (clients only).
        IGNITE       =  Target can ignite.
*/
#define FIREFLAG_SHIFT_Z        0x000000ff
#define FIREFLAG_DOWN           0x00000100
#define FIREFLAG_DELTA_ALIVE    0x00000200
#define FIREFLAG_DELTA_BASE     0x00000400
#define FIREFLAG_DELTA_VIEW     0x00000800
#define FIREFLAG_IGNITE         0x00001000

/* Important frames in my flame model. */
#define FRAME_FIRST_SMALLIGNITE   0
#define FRAME_LAST_SMALLIGNITE    2
#define FRAME_FIRST_SMALLFIRE     3
#define FRAME_LAST_SMALLFIRE     14
#define FRAME_FIRST_LARGEIGNITE  15
#define FRAME_LAST_LARGEIGNITE   20
#define FRAME_FIRST_LARGEFIRE    21
#define FRAME_LAST_LARGEFIRE     32
#define FRAME_FIRST_FLARE        33
#define FRAME_LAST_FLARE         38
#define FRAME_FLAMEOUT           39

/* Skins in my flame model. */
#define SKIN_FIRE_RED   0
#define SKIN_FIRE_BLUE  1

/* Path to my flame model. */
#define MD2_FIRE  gi.modelindex("models/fire/tris.md2")
//#define MD2_FIRE  gi.modelindex("sprites/faf/fire3.sp2")

/* New 'Means of Death'.
        FIRE         =  Unknown fire-based attack.  (NOT used)
        FIRE_SPLASH  =  Radius damage.
        ON_FIRE      =  Entity is on fire.
        FIREBALL     =  Direct hit by fireball.
*/
#define MOD_FIRE          100
#define MOD_FIRE_SPLASH   101
#define MOD_ON_FIRE       102
#define MOD_FIREBALL      103


/****************/
/*  PROTOTYPES  */
/****************/

/*
// x_fire.c
*/
void check_firedodge
 (edict_t *self, vec3_t start, vec3_t dir, int speed);

void     PBM_BecomeSmoke (edict_t *self);
void     PBM_BecomeSteam (edict_t *self);
void     PBM_SmallExplodeThink (edict_t *self);
void     PBM_StartSmallExplosion (edict_t *self);
void     PBM_BecomeSmallExplosion (edict_t *self);

void     PBM_KillAllFires (void);
qboolean PBM_InWater (edict_t *ent);
qboolean PBM_Inflammable (edict_t *ent);
qboolean PBM_ActivePowerArmor (edict_t *ent, vec3_t point);
qboolean PBM_FireResistant (edict_t *ent, vec3_t point);
void     PBM_BurnDamage
 (edict_t *victim, edict_t *fire, vec3_t point, vec3_t damage, int dflags,
  int mod);
void     PBM_BurnRadius
 (edict_t *fire, float radius, vec3_t damage, edict_t *ignore);
void     PBM_CheckMaster (edict_t *fire);
void     PBM_FireSpot (vec3_t spot, edict_t *ent);
qboolean PBM_FlameOut (edict_t *self);
void     PBM_Burn (edict_t *self);
void     PBM_Ignite (edict_t *victim, edict_t *attacker, vec3_t point);

void     PBM_CheckFire (edict_t *self);
void     PBM_FireDropTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FireDrop (edict_t *attacker, vec3_t spot, vec3_t damage, vec3_t radius_damage,  int blast_chance, vec3_t vel);
void     PBM_EasyFireDrop (edict_t *self);

void     PBM_CloudBurst (edict_t *self);
void     PBM_CloudBurstDamage (edict_t *self);
void     PBM_FlameCloud
 (edict_t *attacker, vec3_t start, vec3_t cloud, vec3_t timer,
  qboolean deadly, vec3_t damage, vec3_t radius_damage, int rain_chance,
  int blast_chance);

void     PBM_FireAngleSpread (vec3_t spread, vec3_t dir);
void     PBM_FireballTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FireFlamer
 (edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance);
void     PBM_FlameThrowerTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FlameThrowerThink (edict_t *self);

#ifndef USER_EXCLUDE_FUNCTIONS

void     PBM_FireFlameThrower
 (edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance);

#endif
/*===========================/  END OF FILE  /===========================*/
