/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_weapon.c,v $
 *   $Revision: 1.49 $
 *   $Date: 2002/07/25 01:44:49 $
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

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void NoAmmoWeaponChange (edict_t *ent);
//qboolean (*Pickup_Weapon)(edict_t *, edict_t *);
void Cmd_WeapNext_f (edict_t *ent);


//faf:  tidies up coming out of the scope for the sniper.
//      it wasnt showing the gun being lowered before
void check_unscope (edict_t *ent)
{
	if (ent->client &&
		ent->client->pers.weapon &&
		ent->client->pers.weapon->position != LOC_SNIPER)
		return;

	if (ent->client->ps.fov == SCOPE_FOV)
	{
		ent->client->unscopetime = level.time;
		ent->client->weaponstate = WEAPON_LOWER;
		ent->client->ps.fov = STANDARD_FOV;
	}

}



/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
	}
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


int calcVspread(edict_t *ent,trace_t *tr)
{
	int half_max,max_y;
	half_max = ent->client->pers.weapon->max_range;
	half_max = half_max >> 2;
	max_y=ent->client->pers.weapon->max_y;
	if(rand()<ent->client->resp.team_on->mos[ent->client->resp.mos]->marksmanship)
		return(DEFAULT_BULLET_VSPREAD*rand());
	return ( (max_y*( (tr->endpos[PITCH]<half_max)?tr->endpos[PITCH]:ent->client->pers.weapon->max_range-tr->endpos[PITCH] ))/half_max);
}














qboolean Surface(char *name, int type);

int Play_Bullet_Hit(edict_t *ent, char *surface, vec3_t endpos, edict_t *impact_ent)
{
	int sound;
	int soundtype;


	if (!surface)
		surface = "concrete";

//	false;// .02 faf

// safe_bprintf(PRINT_HIGH, "%s\n", surface);
	if (Surface(surface, SURF_WOOD)) // wood
		{
			if (random() < 0.33)
				sound = gi.soundindex("bullet/wood1.wav");
			else if (random() < 0.50)
				sound = gi.soundindex("bullet/wood2.wav");
			else
				sound = gi.soundindex("bullet/wood3.wav");

			soundtype = SOUND_WOOD;

//			safe_bprintf (PRINT_HIGH, "wood\n");
		}
		else if (Surface(surface, SURF_METAL)) // metal
		{
			if (random() < 0.33)
				sound = gi.soundindex("bullet/metal1.wav");
			else if (random() < 0.50)
				sound = gi.soundindex("bullet/metal2.wav");
			else
				sound = gi.soundindex("bullet/metal3.wav");
//			safe_bprintf (PRINT_HIGH, "metal\n");

			soundtype = SOUND_METAL;
		}	
		else if (Surface(surface, SURF_GLASS)) // metal
		{
			if (random() < 0.50)
				sound = gi.soundindex("bullet/glass1.wav");
			else 
				sound = gi.soundindex("bullet/glass2.wav");
//			safe_bprintf (PRINT_HIGH, "glass\n");
			soundtype = SOUND_GLASS;
		}
		else if ( (impact_ent && impact_ent->classnameb == SANDBAGS) ||
			(Surface(surface, SURF_SAND) || Surface(surface, SURF_GRASS)) )// sand (beach)
		{
			if (random() < 0.33)
				sound = gi.soundindex("bullet/sand1.wav");
			else if (random() < 0.50)
				sound = gi.soundindex("bullet/sand2.wav");
			else 
				sound = gi.soundindex("bullet/sand3.wav");
//			safe_bprintf (PRINT_HIGH, "sand\n");

			soundtype = SOUND_SAND;
		}
	else 
		{
			if (random() < 0.33)
				sound = gi.soundindex("bullet/concrete1.wav");
			else if (random() < 0.50)
				sound = gi.soundindex("bullet/concrete2.wav");
			else
				sound = gi.soundindex("bullet/concrete3.wav");
//			safe_bprintf (PRINT_HIGH, "default\n");

			soundtype = SOUND_CONCRETE;
		}	



//gi.positioned_sound (endpos, g_edicts, CHAN_AUTO, sound, .75, ATTN_NORM, 0);
gi.sound (ent, CHAN_ITEM, sound, .5, ATTN_NORM, 0);

return soundtype;

//if (loud)
//gi.positioned_sound (endpos, g_edicts, CHAN_AUTO, sound, 1, ATTN_NORM, 0);
}



void Play_Bullet_Whiz (edict_t *self, vec3_t start, vec3_t end, edict_t *hit_ent)
{
	vec3_t		from;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	vec3_t  whizrangemax = {40,40,40};
	vec3_t  whizrangemin = {-40,-40,-40};

	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, whizrangemin, whizrangemax, end, ignore, mask);

		if (tr.contents & (CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_MONSTER|CONTENTS_MIST))		
		{
			mask &= ~(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_MONSTER|CONTENTS_MIST);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) /* && (tr.ent != hit_ent)*/ &&  (tr.ent->client))
			{  
				//safe_bprintf(PRINT_HIGH,"whiz\n");
			
				if (random() < 0.33)
				{
					gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex("bullet/whiz1.wav"), 1, ATTN_STATIC, 0);
					//safe_bprintf(PRINT_HIGH,"whiz1\n");
				}
				else if (random() < 0.50)
				{
					gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex("bullet/whiz2.wav"), 1, ATTN_STATIC, 0);
					//safe_bprintf(PRINT_HIGH,"whiz2\n");
				}
				else
				{
					gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex("bullet/whiz3.wav"), 1, ATTN_STATIC, 0);
					//safe_bprintf(PRINT_HIGH,"whiz3\n");
				}
				ignore = NULL;

			}
		}

		VectorCopy (tr.endpos, from);
	}	
//	PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}







// rezmoth - tool function to display vector contents
void showvector(char* namevector, vec3_t showvector)
{
	gi.dprintf("%s (%f, %f, %f)\n", namevector, showvector[0], showvector[1], showvector[2]);
}

// rezmoth - new function to fire bullet
void fire_gun_old(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
{
	// standard defines
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	edict_t  *hit_ent = NULL;

	// rezmoth - start dist trace
	vec3_t	diststart, dist;
	vec3_t	distend = {0, 0, -8192};
	trace_t	disttr;



	if(self->client)
	self->client->resp.accuracy_misses++;



	VectorCopy(self->s.origin, diststart);	// initial value
	//VectorAdd(start, ent->mins, start); // go to the bottom of the player
	VectorAdd(diststart, distend, distend);			// add distance for end

	disttr = gi.trace (diststart, self->mins, self->maxs, distend, self, MASK_SOLID);
	VectorSubtract(self->s.origin, disttr.endpos, dist);
	// rezmoth - end dist trace

	// Extra debugging propaganda
	//gi.dprintf("self    %s\n", self->client->pers.netname);
	//gi.dprintf("damage  %i\n", damage);
	//gi.dprintf("kick    %i\n", kick);
	//gi.dprintf("mod     %i\n", mod);
	//gi.dprintf("calcv   %s\n", (calcv) ? "true":"false");

	// Useful debugging information
	//gi.dprintf("hspread %i\n", hspread);
	//gi.dprintf("vspread %i\n", vspread);
	//showvector("start   ", start);
	//showvector("aimdir  ", aimdir);

	// fetch trace results
	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

	// if the trace hit anything before distance termination
	if (!(tr.fraction < 1.0))
	{
		// seperate the aimdir into three parts
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		// rezmoth - TODO: fix this part
		// random spread calculation
		calcv = false;
		//r = (calcv) ? (crandom() * hspread) : hspread;
		//u = (calcv) ? (crandom() * vspread) : vspread;


		// add spread to hip shots
		if (self->client && !self->client->aim)
		{
			r = 300 - crandom() * 600;
			u = crandom() * 600;
		}
		else 
		{
			r = (crandom() * (hspread)) - ((hspread)/2);
			u = (crandom() * (vspread));
		}

		if (self->burnout) //on fire
		{
			r= crandom() * 2000;
			u = crandom() * 2000;
		}

		if (VectorLength(dist) > 20 && self->velocity[2] != 0)
		{
			r = 800 - crandom() * 1600;
			u = crandom() * 1600;
		}

		// end = start[i] + 8192 * forward[i]
		VectorMA (start, 8192, aimdir, end); //faf:  aim fix by kermit, "forward" changed to "aimdir"
		// scale right angle by calculated horizontal spread
		VectorMA (end, r, right, end);
		// scale up angle by calculated vertical spread
		VectorMA (end, u, up, end);

		// if trace starts in water?
		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			// remove water from possible impacts during trace
			content_mask &= ~MASK_WATER;
		}


			//faf
			if (self->client)
			{
				AngleVectors (self->client->v_angle, NULL, right, up);
			
					if (self->client->pers.hand == LEFT_HANDED)
						VectorMA (end, (self->client->ps.gunangles[YAW]) * 143, right, end);
					else
					{
						VectorMA (end, (self->client->ps.gunangles[YAW]) * (-143), right, end);
					}

					VectorMA (end, (self->client->ps.gunangles[PITCH]) * (-143), up, end);

					if (self->client->pers.hand == LEFT_HANDED)
						VectorMA (end, (self->client->explosion_angles[YAW]) * 143, right, end);
					else
						VectorMA (end, (self->client->explosion_angles[YAW]) * (-143), right, end);


					VectorMA (end, (self->client->explosion_angles[PITCH]) * (-143), up, end);
			}






		// retrace from point of impact with water
		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// if trace impacts dead player
		if (tr.contents & MASK_DEADSOLID)
//			SprayBlood(self, tr.endpos, up, 0, MOD_UNKNOWN);
			SprayBlood(self, tr.endpos, aimdir, 0, 69);//faf

		// more spread calculation
		if(calcv) calcVspread(self,&tr);

		// if the trace impacts water?
		if (tr.contents & MASK_WATER)
		{
			int		color;
			water = true;

			// copy trace's impact with water (end) to water_start
			VectorCopy (tr.endpos, water_start);

			// if the trace's start and end were not the same
			if (!VectorCompare (start, tr.endpos))
			{
				// if trace impacts water
				if (tr.contents & CONTENTS_WATER)
				{
					gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex(va("bullet/water%i.wav",1+rand()%2)), 1, ATTN_NORM, 0);	

					// if water is brown
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					// if water is blue
					else
						color = SPLASH_BLUE_WATER;
				}
				// if trace impacts slime
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				// if trace impacts lava
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				// if trace impacts unknown water
				else
					color = SPLASH_UNKNOWN;

				// if trace impacted known water
				if (color != SPLASH_UNKNOWN)
				{
					// display water splash particles
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, aimdir, end); //faf:  aim fix found by kermit, "forward" changed to "aimdir"
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// retrace starting from impact with water
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// if trace does not impact a surface and the surface is not the sky
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		// if the trace impacted anything before distance termination
		if (tr.fraction < 1.0)
		{
			// if the impacted THING can take damage
			if (tr.ent->takedamage)
			{
				if (self->client)
					self->client->resp.accuracy_misses--;

				// damage impacted player
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
				hit_ent = tr.ent;
	
				if (self->client && tr.ent->client)
				{
					if(!OnSameTeam(tr.ent, self))
						self->client->resp.accuracy_hits++;
                    else
						self->client->resp.accuracy_misses++;
				}


				if (!tr.ent->client && !(tr.ent->svflags & SVF_MONSTER))//objective or destroyable thing
				{
					Play_Bullet_Hit(self, tr.surface->name, tr.endpos, tr.ent);

					if (self->client && (!tr.ent->obj_owner || tr.ent->obj_owner != self->client->resp.team_on->index) && (rand() % 100) < 30)
					{
						if (tr.ent->health > 0)
						{
							if (tr.ent->classnameb != OBJECTIVE_VIP && !tr.ent->svflags & SVF_DEADMONSTER)
								ThrowDebris (self, "models/objects/debris2/tris.md2", 1, tr.endpos);
						}
					}

				}
			}
			else
			{
				// if the trace impacted a surface other than the sky
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					int soundtype;

					soundtype = Play_Bullet_Hit(self, tr.surface->name, tr.endpos, tr.ent) == SOUND_METAL;

					// display impact on surface
					gi.WriteByte (svc_temp_entity);

					if (soundtype == SOUND_METAL)
					{
						if (crandom() < 0.5)
							gi.WriteByte (TE_GUNSHOT);
						else
							gi.WriteByte (TE_BULLET_SPARKS);
					}
					else
						gi.WriteByte (TE_GUNSHOT);

					//if (crandom() < 0.5)
					//	gi.WriteByte (TE_GUNSHOT);
					//else
					//	gi.WriteByte (TE_BULLET_SPARKS);

					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);


					// output impact sound
//					if (self->client)
//						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}

			}
		}
	}

	// if trace impacted water
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		// display bubble trail
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}

	//Play_Bullet_Whiz(self, start, tr.endpos, hit_ent);
}

/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod, qboolean calcv)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		// pbowens: TEST-FIX (need to see ingame results)
		calcv = false;

		// the reason why I disabled the spread calculation is because we've noticed some 
		// servers that sometimes have guns sparadically fire everywhere. other servers don't
		// do this, and then again some do but rarely. so, my only conclusion was this HAD to 
		// be something concerning a random element. each release, we'll try a different random
		// generation point and see if it eradicates it or not.. who knows

		r = (calcv) ? (crandom() * hspread) : hspread;
		u = (calcv) ? (crandom() * vspread) : vspread;

		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		if (tr.contents & MASK_DEADSOLID) // dead player
			SprayBlood(self, tr.endpos, up, 0, MOD_UNKNOWN);

		if(calcv) calcVspread(self,&tr);
		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex(va("bullet/water%i.wav",1+rand()%2)), 1, ATTN_NORM, 0);	



			water = true;
			VectorCopy (tr.endpos, water_start);


			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}



			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}

/*
=================
fire_fragment

Used for grenade fragments
=================
*/
void fire_fragment (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod, false);

}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean tracers_on)
{

//the next line is for monsters who don't like to shoot tracers...
	if (!tracers_on)
		fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod,true);
	else
	{
//		self->numfired++;
//		if (self->numfired % TRACERSHOT == 1) 
//			fire_tracer (self, start, aimdir, damage, mod); 
//		else 
			fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod,true);
	}

}


/*
=================
fire_tracer

Fires a single tracer bolt.  
=================
*/
void tracer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, self->mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		if (surf)
		Play_Bullet_Hit(self, surf->name, self->s.origin, other);

	}

	G_FreeEdict (self);
}



void tracer_effect_delay (edict_t *self)
{
	self->s.effects |= EF_HYPERBLASTER;
	
	self->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	self->nextthink = level.time + 10;
	self->think = G_FreeEdict;
}

void fire_tracer (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod)
{
	int speed=2100;//1500;//850;  //bullet speed
	int effect=EF_HYPERBLASTER;
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	// rezmoth - new tracer origin
	//VectorCopy (start, bolt->s.origin);
	//VectorCopy (start, bolt->s.old_origin);
	VectorCopy (self->s.origin, bolt->s.origin);
	VectorCopy (self->s.origin, bolt->s.old_origin);
	bolt->s.origin[2] += self->viewheight;
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	//bolt->s.effects |= effect;
	bolt->s.effects = 0;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	//bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.modelindex = 0;
	// rezmoth - new bullet whiz sound
	//bolt->s.sound = gi.soundindex ("weapons/machgf5b.wav");
	bolt->s.sound = gi.soundindex ("bullet/whiz1.wav");
	//bolt->s.sound = gi.soundindex ("misc/bwhiz.wav");
	bolt->owner = self;
	bolt->touch = tracer_touch;
	bolt->nextthink = level.time + .1;//10;// + 2;
	bolt->think = tracer_effect_delay;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	bolt->spawnflags = 1;
	bolt->mod = mod; // pbowens: carry the means of death
	gi.linkentity (bolt);

//	if (self->client)
//		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void make_splash (edict_t *ent)
{
	ent->dmg *=.5;
	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BUBBLETRAIL);
	gi.WritePosition (ent->move_origin);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->think = G_FreeEdict;
	ent->nextthink = level.time +.6;


}


void fire_underwater (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod)
{
	int speed= 400 + random()*500;//1500;//850;  //bullet speed
	int effect=EF_HYPERBLASTER;
	edict_t	*underwater_bullet;
	trace_t	tr;

	damage *= .75;

	
	gi.sound(self, CHAN_BODY, gi.soundindex(va("bullet/water%i.wav",1+rand()%2)), 1, ATTN_NORM, 0);


					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (start);
					gi.WriteDir (vec3_origin);
					gi.WriteByte (SPLASH_BLUE_WATER);
					gi.multicast (start, MULTICAST_PVS);




	VectorNormalize (dir);

	underwater_bullet = G_Spawn();
	// rezmoth - new tracer origin
	//VectorCopy (start, underwater_bullet->s.origin);
	//VectorCopy (start, underwater_bullet->s.old_origin);
	VectorCopy (self->s.origin, underwater_bullet->s.origin);
	VectorCopy (self->s.origin, underwater_bullet->s.old_origin);
	underwater_bullet->s.origin[2] += self->viewheight;
	vectoangles (dir, underwater_bullet->s.angles);
	VectorScale (dir, speed, underwater_bullet->velocity);
	underwater_bullet->movetype = MOVETYPE_FLYMISSILE;
	underwater_bullet->clipmask = MASK_SHOT;
	underwater_bullet->solid = SOLID_BBOX;
//	underwater_bullet->s.effects |= effect;
	VectorClear (underwater_bullet->mins);
	VectorClear (underwater_bullet->maxs);
//	underwater_bullet->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	// rezmoth - new bullet whiz sound
	//underwater_bullet->s.sound = gi.soundindex ("weapons/machgf5b.wav");
//	underwater_bullet->s.sound = gi.soundindex ("misc/bwhiz.wav");
	underwater_bullet->owner = self;
	underwater_bullet->touch = tracer_touch;
	underwater_bullet->nextthink = level.time + .2;// + 2;
	underwater_bullet->think = G_FreeEdict;
	underwater_bullet->think = make_splash;
	underwater_bullet->dmg = damage;
	underwater_bullet->classname = "underwater_bullet";
	underwater_bullet->spawnflags = 1;
	underwater_bullet->mod = mod; // pbowens: carry the means of death
	gi.linkentity (underwater_bullet);

	VectorCopy (start, underwater_bullet->move_origin);

//	if (self->client)
//		check_dodge (self, underwater_bullet->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, underwater_bullet->s.origin, underwater_bullet, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (underwater_bullet->s.origin, -10, dir, underwater_bullet->s.origin);
		underwater_bullet->touch (underwater_bullet, tr.ent, NULL, NULL);
	}
}	




//faf
void fire_hmg_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod)
{
	int speed=2100;//1500;//850;  //bullet speed
	int effect=EF_HYPERBLASTER;
	edict_t	*hmg_bullet;
	trace_t	tr;

	VectorNormalize (dir);

	hmg_bullet = G_Spawn();
	// rezmoth - new tracer origin
	//VectorCopy (start, hmg_bullet->s.origin);
	//VectorCopy (start, hmg_bullet->s.old_origin);
	VectorCopy (self->s.origin, hmg_bullet->s.origin);
	VectorCopy (self->s.origin, hmg_bullet->s.old_origin);
	hmg_bullet->s.origin[2] += self->viewheight;
	vectoangles (dir, hmg_bullet->s.angles);
	VectorScale (dir, speed, hmg_bullet->velocity);
	hmg_bullet->movetype = MOVETYPE_FLYMISSILE;
	hmg_bullet->clipmask = MASK_SHOT;
	hmg_bullet->solid = SOLID_BBOX;
//	hmg_bullet->s.effects |= effect;
	VectorClear (hmg_bullet->mins);
	VectorClear (hmg_bullet->maxs);
//	hmg_bullet->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	// rezmoth - new bullet whiz sound
	//hmg_bullet->s.sound = gi.soundindex ("weapons/machgf5b.wav");
//	hmg_bullet->s.sound = gi.soundindex ("misc/bwhiz.wav");
	hmg_bullet->owner = self;
	hmg_bullet->touch = tracer_touch;
	hmg_bullet->nextthink = level.time + 10;// + 2;
	hmg_bullet->think = G_FreeEdict;
	hmg_bullet->dmg = damage;
	hmg_bullet->classname = "hmg_bullet";
	hmg_bullet->spawnflags = 1;
	hmg_bullet->mod = mod; // pbowens: carry the means of death
	gi.linkentity (hmg_bullet);

//	if (self->client)
//		check_dodge (self, hmg_bullet->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, hmg_bullet->s.origin, hmg_bullet, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (hmg_bullet->s.origin, -10, dir, hmg_bullet->s.origin);
		hmg_bullet->touch (hmg_bullet, tr.ent, NULL, NULL);
	}
}	



void Play_WepSound(edict_t *ent, char *sound)
{
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(sound), 1, ATTN_NORM, 0);
}
/*
=================
fire_blaster

Fires a single blaster bolt.  Monsters require this....
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_SUBMG;
		else
			mod = MOD_PISTOL;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (hyper)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

//	if (self->client)
//		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


//add view whiteness simulating smoke to players within certain radius of blast
void Smoke_Effect (vec3_t origin, float strength)
{
	edict_t	*e;
	int i;
	vec3_t distvector;
	float	distance;

    
	for (i=0 ; i<game.maxclients ; i++)
	{	
		e = g_edicts + 1 + i;
	
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->s.origin)
			continue;
		if (e->ai)
			continue;
		if (e->deadflag)
			continue;
		if (e->health <= 0)
			continue;

		VectorSubtract(e->s.origin, origin, distvector);
		distance = VectorLength( distvector);

		if (distance < 1000)
		{
			e->client->smoke_effect_goal += (strength * (1000-distance))/1000;
		}
		if (e->client->smoke_effect_goal > .10)
			e->client->smoke_effect_goal = .10;

	}


}



//faf: play bullet hit noises for nades/tnt/ etc.
void Play_Ricochet_Noise (edict_t *ent, vec3_t origin)
{
    vec3_t offset, end;
	trace_t tr;
	vec3_t mins, maxs;

	qboolean gotwood, gotmetal;

	gotwood = false;
	gotmetal = false;


	VectorSet (mins, -10,-10,-10);
	VectorSet (maxs, 10,10,10);


	VectorCopy (origin, offset);
	VectorCopy (origin, end);
	offset[2]+=30;


	end[2]+=100; //up
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if (tr.fraction < 1.0 && tr.surface){
		if (tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
			

	VectorCopy (offset, end);
	end[2]-=100;//down
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if ((!gotwood || !gotmetal) && tr.fraction < 1.0 && tr.surface){
		if (!gotwood && tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (!gotmetal && tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
	VectorCopy (offset, end);
	end[0]+=100;//left
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if ((!gotwood || !gotmetal) && tr.fraction < 1.0 && tr.surface){
		if (!gotwood && tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (!gotmetal && tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
	VectorCopy (offset, end);
	end[0]-=100;//right
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if ((!gotwood || !gotmetal) && tr.fraction < 1.0 && tr.surface){
		if (!gotwood && tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (!gotmetal && tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
	VectorCopy (offset, end);
	end[1]+=100;//forward
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if ((!gotwood || !gotmetal) && tr.fraction < 1.0 && tr.surface){
		if (!gotwood && tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (!gotmetal && tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
	VectorCopy (offset, end);
	end[1]-=100;//back
	tr = gi.trace (offset, mins, maxs, end, ent, MASK_SHOT);
	if ((!gotwood || !gotmetal) && tr.fraction < 1.0 && tr.surface){
		if (!gotwood && tr.surface && Surface2(tr.surface->name) == SURF_WOOD)
			gotwood = true;
		else if (!gotmetal && tr.surface && Surface2(tr.surface->name) == SURF_METAL)
			gotmetal = true;
	}
	
	if (gotwood)
		Play_Bullet_Hit(ent, "wood", origin, ent);
	if (gotmetal)
		Play_Bullet_Hit(ent, "metal", origin, ent);


}


/*
=================
fire_grenade
=================
*/

#define SCHRAPNEL_DAMAGE 35

void Shrapnel_Explode (edict_t *ent)
{
	vec3_t        origin;     
    
    int             mod;
	int i;

 

	//faf
	edict_t	*checkent;

	//faf:  so nades dont go off after game ends
	if (level.intermissiontime)
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client &&
		(!ent->owner->client->resp.team_on ||
	ent->owner->client->resp.team_on->index != ent->obj_owner))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->takedamage = DAMAGE_NO;



	Play_Ricochet_Noise (ent, ent->s.origin);
	Smoke_Effect (ent->s.origin, .1);

	//faf:  for when player is carrying a nade and the timer runs out.  Moved
	//	    from clientthink
	for (i=0 ; i<maxclients->value ; i++)
	{
		checkent = g_edicts + 1 + i;
		if (!checkent->inuse)
			continue;
		if (!checkent->client)
			continue;

		if (checkent->client->grenade &&
			checkent->client->grenade == ent) // disable the grenade from triggering self, but still leave timers on
		{
			VectorCopy(checkent->s.origin, checkent->client->grenade->s.origin);
			checkent->client->grenade->s.modelindex = 0; // set model to null
			//client->grenade_caught->touch = NULL;
		}

	}






    if (ent->owner && ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

 
	//pbowens: quick hack to make only USA Grenade fire fragments
	if ( ent->obj_owner == 0 )
	{
		int i,j;
		vec3_t offset, grenade1;

    VectorSet(offset,0,0,32);
    VectorAdd(offset,ent->s.origin,offset);
    VectorCopy (offset, ent->s.origin);

		for (i = 0; i < 3; i++) 
		{
			for(j=0;j<3;j++)
			{
				VectorSet(grenade1,((rand()%40)-20),((rand()%40)-20),5);
				fire_fragment (ent, offset, grenade1, SCHRAPNEL_DAMAGE, 2, 4500, 4500, MOD_G_SPLASH);
			}
		}
	}


    if (ent->takedamage)
	{
		float    points;
        vec3_t    v;
        vec3_t    dir;
		VectorAdd (ent->mins, ent->maxs, v);
        VectorMA (ent->s.origin, 0.5, v, v);
        VectorSubtract (ent->s.origin, v, v);
        points = ent->dmg - 0.5 * VectorLength (v);
        VectorSubtract (ent->s.origin, ent->s.origin, dir);
        if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
        else
			mod = MOD_GRENADE;
        T_Damage (ent, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
    else if (ent->spawnflags & 1)
        mod = MOD_HG_SPLASH;
    else
        mod = MOD_G_SPLASH;





    if (ent->owner && ent->owner->client && ent->owner->client->grenade && ent->owner->client->grenade == ent)
	{
        T_Damage (ent->owner, ent->owner, ent->owner, ent->maxs, ent->s.origin, vec3_origin, 100, 0, 0, MOD_HELD_GRENADE);
	}



    if (1)
	{
		int i;
		edict_t *cl_ent;

		for (i=0 ; i<maxclients->value ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse || !cl_ent->client)
				continue;
			if (cl_ent->client->grenade &&
				cl_ent->client->grenade == ent)
			{
			        T_Damage (cl_ent, ent->owner, ent->owner, ent->maxs, ent->s.origin, vec3_origin, 100, 0, 0, MOD_HANDGRENADE);
			}
		}
		
	}



    T_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius, mod);
	
    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
        else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
    else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
        else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    G_FreeEdict (ent);
}

// pbowens: new dud.. muahaha
void Shrapnel_Dud (edict_t *ent)
{
	if (!ent->owner || !ent->owner->client)
		return;

	safe_centerprintf(ent->owner, "Your grenade did not go off!\n");

//	if (ent == ent->owner->client->grenade)
//		ent->owner->client->pers.inventory[ent->owner->client->grenade_index]--;

	//faf: leave dud around for a bit, kinda glitchy though and don't feel like working on it
//	ent->think = G_FreeEdict;
//	ent->nextthink = level.time + 20;
//	ent->touch = NULL;
	
	G_FreeEdict (ent);
}

void Shrapnel_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int	index;



		
    if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
        return;
	}

	if (other->deadflag == DEAD_DEAD)
		return;

//	if (other->client &&
//		other->client->grenade == ent)
//			return;


//    if (!other->takedamage || !other->client)
	if (ent->avelocity[0] + ent->avelocity[1] + ent->avelocity[2] != 0 && // still bouncing/spinning 
		(!other->takedamage || !other->client) && (other != ent) ) //GaP prevent stupid sound
	{
	
		/*buggy?
		gi.dprintf("%f %f %f\n", ent->velocity[0],ent->velocity[1],ent->velocity[2]);

		//stop spinning
		if (ent->velocity[0] < 100 && ent->velocity[0] > -100 &&
			ent->velocity[1] < 100 && ent->velocity[1] > -100 &&
			ent->velocity[2] < 100 && ent->velocity[2] > -100
			)
		{
			VectorClear (ent->avelocity) ;
		}
*/
			//gi.dprintf("%s\n", ent->classname);

		//if (!strcmp (ent->classname, "M24 Grenade"))
			ent->s.angles[2] = 0;

		if (surf && Surface(surf->name, SURF_METAL) )
			Play_Bullet_Hit(ent, surf->name, ent->s.origin, other);

		else if (surf && (Surface(surf->name, SURF_SAND) || Surface(surf->name, SURF_GRASS)) )
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("bullet/sand3.wav"), .4, ATTN_NORM, 0);

		else if (surf && Surface(surf->name, SURF_WOOD))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("bullet/wood3.wav"), .3, ATTN_NORM, 0);

		else
		{
			//if (surf && Surface (surf->name, SURF_WOOD)) //play two sounds for wood bounces
			//	gi.sound (ent, CHAN_VOICE, gi.soundindex ("bullet/wood3.wav"), .3, ATTN_NORM, 0);

			if (ent->spawnflags & 1)
			{
				if (random() > 0.5)
					gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
			}
		}
        return;
	}
//else pick it up


	if (!other->client || 
		other->client->grenade) // they already have a grenade

		//(teamgren->value == 1 && other->client->resp.team_on->index == ent->obj_owner &&
		//(ent->owner != other))
		return;	


	if (other->client && other->client->throw_grenade_time > level.time - .2)
		return;


	//faf:  trying to fix a crash that happens every so often.
	if (!other->client->pers.inventory)
		return;
	if (!ent->item)
		return;

	index= ITEM_INDEX(ent->item);

	ent->touch = NULL; // so it doesnt keep upping the count

	//other->client->pers.inventory[index]++;
	other->client->newweapon = ent->item;
//	other->client->grenade_time=ent->nextthink;
	other->client->grenade = ent; // set their caught grenade
	other->client->aim = false;

	ChangeWeapon(other);
/*
	other->client->latched_buttons |= BUTTON_ATTACK;
	other->client->ps.gunframe = 10;
	other->client->weaponstate = WEAPON_ACTIVATING;
*/
	safe_cprintf(other, PRINT_HIGH, "You have a live grenade!\n");
	
//	other->client->pers.lastweapon = other->client->pers.weapon;
//	other->client->pers.weapon = other->client->newweapon;
//	other->client->newweapon = NULL;
	
	ent->s.modelindex = 0; // set model to null
//	G_FreeEdict(ent);
//	other->armed_grenade=true;
	//other->client->weaponstate = WEAPON_FIRING;
    //ent->enemy = other;
    //Shrapnel_Explode (ent);
}


void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
//	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);




	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->nextthink = level.time + timer;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "nade";
	
	grenade->think = Shrapnel_Explode;
	grenade->touch = Shrapnel_Touch;
	
	gi.linkentity (grenade);
}

void grenade_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
G_FreeEdict (self);

}
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float time, float damage_radius, int team)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	char grenadefilename[MAX_QPATH];  //faf

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	if (self->client->grenade || self->client->grenade_index)
	{
		grenade = self->client->grenade; // use the same grenade
		self->client->grenade = NULL; // reset it

//		if (grenade->item && grenade->item->world_model)
//			grenade->s.modelindex =  gi.modelindex(grenade->item->world_model);
//		else
/*
		if (grenade->item)
			team = Q_stricmp(grenade->item->ammo , "Mk 2 Grenade");
		else // this should not happen, but does
			team = self->client->resp.team_on->index;

			grenade->s.modelindex =  gi.modelindex (va("models/objects/%s/tris.md2", (team) ? "masher" : "grenade2"));

*/

		//              if (grenade->item && grenade->item->world_model)
//                      grenade->s.modelindex =  gi.modelindex(grenade->item->world_model);
//              else
		if (grenade->item)
		{
			// team = Q_stricmp(grenade->item->ammo , "Mk 2 Grenade");  //faf: team dll support, replaced with below
			if (!strcmp(grenade->item->dllname, team_list[0]->teamid))
					team = 0;
			else if (!strcmp(grenade->item->dllname, team_list[1]->teamid))
					team = 1;
			else
			{
				safe_cprintf (self, PRINT_HIGH, "*** fire_grenade2 error");
				team = self->client->resp.team_on->index;
			}
		}
		else // this should not happen, but does
			team = self->client->resp.team_on->index;

		if (grenade->item && grenade->item->ammo)
		{
			if (!strcmp(grenade->item->ammo, "Mk 2 Grenade"))
				grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
			else if (!strcmp(grenade->item->ammo, "M24 Grenade"))
				grenade->s.modelindex = gi.modelindex ("models/objects/masher/tris.md2");
			else
			{
				Q_strncpyz (grenadefilename, sizeof(grenadefilename),"models/objects/");
				Q_strncatz (grenadefilename, sizeof(grenadefilename), team_list[team]->teamid);
				Q_strncatz (grenadefilename, sizeof(grenadefilename),"grenade/tris.md2");
				grenade->s.modelindex =  gi.modelindex (grenadefilename);
			}
		}
		else
			grenade->s.modelindex =  gi.modelindex (va("models/objects/%s/tris.md2", (team) ? "masher" : "grenade2"));

		// faf:  for team dll support-> new grenade models must be named
		//        "[name of dll]grenade/tris.md2" to work.  usa & grm will keep old names.

		grenade->touch = Shrapnel_Touch;
		gi.linkentity (grenade);

	}
	else
		return; // just get out of here


	grenade->oldenemy = self;// last person to throw for obituary//faf



	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
 

	if (grenade->item && grenade->item->ammo && !strcmp(grenade->item->ammo, "M24 Grenade"))
	{
		VectorCopy(self->s.angles, grenade->s.angles);
		grenade->s.angles[YAW]-=90;
		VectorSet (grenade->avelocity, crandom()*20 -20, 0, 1000);
	}
	else
		VectorSet (grenade->avelocity, 300, 300, 300);


	//faf
	if (self->client &&
		self->stanceflags == STANCE_CRAWL)
	{
		grenade->s.origin[2] +=10;
	}



	//faf:  play part of wave animation when throwing nade
	if (self->client && self->stanceflags == STANCE_STAND) 
	{
		self->client->anim_priority = ANIM_WAVE;
		self->s.frame = 116;//(FRAME_wave05);
		self->client->anim_end = 121;// (FRAME_wave10);
	} //end faf
	else if (self->stanceflags == STANCE_DUCK) 
	{
		self->client->anim_priority = ANIM_WAVE;
		self->s.frame = 169;//(FRAME_crpain1);
		self->client->anim_end = 172;// (FRAME_crpain4);
	} //end faf
	else if (self->stanceflags == STANCE_CRAWL) 
	{
		self->client->anim_priority = ANIM_WAVE;
		self->s.frame = 222;//(FRAME_crawlattck02);
		self->client->anim_end = 225;// (FRAME_crawlattck05);
	} //end faf

	/*
	VectorSet (grenade->mins, -15,-15,-15);
	VectorSet (grenade->maxs, 15,15,15);
	grenade->takedamage = DAMAGE_YES;
	grenade->health = 1;
	grenade->mass = 10;
	grenade->die = grenade_die; */

	//grenade->movetype = MOVETYPE_STEP;


/*	else
	{
		grenade = G_Spawn();

		grenade->movetype = MOVETYPE_BOUNCE;
		grenade->clipmask = MASK_SHOT;
		grenade->solid = SOLID_TRIGGER;
//		grenade->s.effects |= EF_GRENADE;
		
		VectorClear (grenade->mins);
		VectorClear (grenade->maxs);
		//gi.dprintf("grenade timer set to %f (%f)\n", time, level.time);

		grenade->s.modelindex = gi.modelindex (va("models/objects/%s/tris.md2", (team) ? "masher" : "grenade2"));
		grenade->owner = self;
		grenade->nextthink = time;
		grenade->dmg = damage;
		grenade->dmg_radius = damage_radius;
		grenade->classname = "hgrenade";
		grenade->obj_owner = team;
		grenade->item=FindItem(va("%s", (team) ? "M24 Grenade" : "Mk 2 Grenade" ));
	
		grenade->think = Shrapnel_Explode;
		grenade->touch = Shrapnel_Touch;

//`		gi.linkentity (grenade);

//		if (held)
//			grenade->spawnflags = 3;
//		else
			grenade->spawnflags = 1;

//		grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
//
//		if (timer <= 0.0)
//			Shrapnel_Explode(grenade);
//		else
//		{
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
//			gi.linkentity (grenade);
//		}
	}
*/

	// .. snip .. 
}


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		//if (!deathmatch->value && !coop->value)
		//{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		//}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);


	Play_Ricochet_Noise (ent, ent->s.origin);
	Smoke_Effect (ent->s.origin, .1);


	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}



//faf
void airstrike_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;


	vec3_t  end, world_up, start;
	
	trace_t	tr;

	
	VectorSet(world_up, 0, 0, 1);
	VectorCopy(ent->s.origin, start);
	VectorMA(start, 8192, world_up, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	//this happens on invade1.  bomb gets dropped inside the pill
	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{ 
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + .1;
		return;
	}





	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_AIRSTRIKE);
	}
	else
	{
		// don't throw any debris in net games
		//if (!deathmatch->value && !coop->value)
		//{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		//}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_AIRSTRIKE_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	Play_Ricochet_Noise (ent, ent->s.origin);
	Smoke_Effect (ent->s.origin, .05);



	G_FreeEdict (ent);
}

void panzer_effect(edict_t *rocket)
{
	rocket->s.effects = EF_GRENADE;
	rocket->nextthink = level.time + 12;
	rocket->think = G_FreeEdict;
}

void BotWarnThink (edict_t *ent)
{

	//this "turns it on" //not sure this even works
	ent->classnameb = BOTWARN;

	if (!ent->owner || ent->owner && !ent->owner->inuse)
	{
		ent->think = G_FreeEdict;
	}
	ent->nextthink = level.time + .1;

}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

//****************warn bots to move****************
	trace_t		tr;
	vec3_t		end;
	edict_t *botwarn;


	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
//faf	rocket->nextthink = level.time + 8000/speed;
//faf	rocket->think = G_FreeEdict;

	//faf
	if (self->client && self->client->pers.weapon &&
		self->client->pers.weapon->classnameb == WEAPON_PANZER)
	{
		rocket->s.effects = EF_ROCKET;
		rocket->nextthink = level.time + .2;
		rocket->think = panzer_effect;
	}
	else
	{
		rocket->s.effects |= EF_GRENADE;
		rocket->nextthink = rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
	}


	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	rocket->classnameb = ROCKET;

//	if (self->client)
//		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);


	VectorMA (start, 8192, dir, end);

	tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		botwarn = G_Spawn();
		botwarn->owner = rocket;
		botwarn->movetype = MOVETYPE_NONE;
		botwarn->solid = SOLID_TRIGGER;
		botwarn->think = BotWarnThink;
		if (skill->value >= 2)
			botwarn->nextthink = level.time + .1;
		else if (skill->value >= 1)
			botwarn->nextthink = level.time + .2;
		else 
			botwarn->nextthink = level.time + .3;

		VectorClear (botwarn->mins);
		VectorClear (botwarn->maxs);
//		botwarn->classname = "botwarn";
		botwarn->classnameb = BOTWARN;
		VectorCopy (tr.endpos, botwarn->s.origin);
	}

}

void panzerf_effect(edict_t *rocket)
{
	rocket->s.effects = EF_GRENADE;
	rocket->nextthink = level.time + 12;
	rocket->gravity = .5;
	rocket->think = G_FreeEdict;
}


//faf:  adds gravity to the rocket (for piat)
void fire_rocket_piat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float gravity)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_BOUNCE,//MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;

	rocket->gravity =  gravity;//1;//.9; //faf
	
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	
	//faf
	if (self->client && self->client->pers.weapon &&
		self->client->pers.weapon->classnameb == WEAPON_PANZERFAUST)
	{
		rocket->s.effects = EF_ROCKET;
		rocket->nextthink = level.time + .6;
		rocket->think = panzerf_effect;
		rocket->gravity = .2;
	}
	else
	{
		rocket->s.effects |= EF_GRENADE;
		rocket->nextthink = rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
	}
	
	
	
	
	
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	rocket->classnameb = ROCKET;
	


//faf:causing error and dont think is needed	if (self->client)
//faf		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

//faf:  adds gravity to the rocket (for piat)
void fire_rocket_panzerfaust (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float gravity)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_BOUNCE,//MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/panzrocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;

	rocket->gravity =  gravity;//1;//.9; //faf
	
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	
	//faf
	if (self->client && self->client->pers.weapon &&
		self->client->pers.weapon->classnameb == WEAPON_PANZERFAUST)
	{
		rocket->s.effects = EF_ROCKET;
		rocket->nextthink = level.time + .6;
		rocket->think = panzerf_effect;
		rocket->gravity = .2;
	}
	else
	{
		rocket->s.effects |= EF_GRENADE;
		rocket->nextthink = rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
	}
	
	
	
	
	
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	rocket->classnameb = ROCKET;
	


//faf:causing error and dont think is needed	if (self->client)
//faf		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}





//faf
void fire_airstrike (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*airstrike;

	airstrike = G_Spawn();
	VectorCopy (start, airstrike->s.origin);
	VectorCopy (dir, airstrike->movedir);
	vectoangles (dir, airstrike->s.angles);
	VectorScale (dir, speed, airstrike->velocity);
	airstrike->movetype = MOVETYPE_FLYMISSILE; 
	airstrike->clipmask = MASK_SHOT;
	airstrike->solid = SOLID_BBOX;
	airstrike->s.effects |= 0; //faf
	//airstrike->s.effects |= EF_ROCKET;
	VectorClear (airstrike->mins);
	VectorClear (airstrike->maxs);
	airstrike->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	airstrike->owner = self;
	airstrike->touch = airstrike_touch;
	airstrike->nextthink = level.time + 8000/speed;
	airstrike->think = G_FreeEdict;
	airstrike->dmg = damage;
	airstrike->radius_dmg = radius_damage;
	airstrike->dmg_radius = damage_radius;
	airstrike->s.sound = gi.soundindex ("weapons/rockfly.wav");
	airstrike->classname = "airstrike";
//	airstrike->gravity = .5; // faf


//	if (self->client)
//		check_dodge (self, airstrike->s.origin, dir, speed);

	gi.linkentity (airstrike);
}


void fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv);
void ricochetthink(edict_t *ricochet)
{
	fire_gun (ricochet->owner, ricochet->s.origin, ricochet->s.angles, ricochet->dmg, ricochet->count, 20, 20, ricochet->style, ricochet->is_step);

	ricochet->think = G_FreeEdict;
	ricochet->nextthink = level.time +.1;
}


void ThrowDebris2 (int soundtype, vec3_t origin);
/*
=================
fire_rifle
=================
*/
void fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
//void fire_rifle (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr = {0};
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	int     soundtype = 0;
	int		ric_chance;

	float dot;

//	vec3_t		right, up;

	int			hits = 0;

	float r,u;
	vec3_t forward, right, up; 

	AngleVectors(aimdir, forward, right, up);


//	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);



	if ((gi.pointcontents(start) & MASK_WATER) > 0)
	{
		fire_underwater (self, start, aimdir, damage, mod);
		return;
	}



	//faf
	if (self->client)
	{
		/*if (self->client->pers.weapon->position != LOC_SNIPER)//faf
		{
			AngleVectors (self->client->v_angle, NULL, right, up);
			
			if (self->client->pers.hand == LEFT_HANDED)
				VectorMA (end, (self->client->ps.gunangles[YAW]) * 143, right, end);
			
			else
			{
				VectorMA (end, (self->client->ps.gunangles[YAW]) * (-143), right, end);
			}

			VectorMA (end, (self->client->ps.gunangles[PITCH]) * (-143), up, end);
			
			if (self->client->pers.hand == LEFT_HANDED)
				VectorMA (end, (self->client->kick_angles[YAW]) * 143, right, end);
			else
				VectorMA (end, (self->client->kick_angles[YAW]) * (-143), right, end);

			VectorMA (end, (self->client->kick_angles[PITCH]) * (-143), up, end);
		}

		if (self->client->pers.hand == LEFT_HANDED)
			VectorMA (end, (self->client->explosion_angles[YAW]) * 143, right, end);
		else
			VectorMA (end, (self->client->explosion_angles[YAW]) * (-143), right, end);

			VectorMA (end, (self->client->explosion_angles[PITCH]) * (-143), up, end);
*/
	}	


		if (self->client->jump_stamina < 60 && self->client->pers.weapon->position == LOC_SNIPER)
		{
			hspread += 200;
			vspread += 200;
		}

		// add spread to hip shots
		if (self->client && !self->client->aim)
		{
			r = 300 - crandom() * 600;
			u = crandom() * 600;
		}
		else 
		{
			r = (crandom() * (hspread)) - ((hspread)/2);
			u = (crandom() * (vspread));
		}

		if (self->burnout) //on fire
		{
			r= crandom() * 2000;
			u = crandom() * 2000;
		}



		//gi.dprintf("%f %f\n", r ,u);

		// end = start[i] + 8192 * forward[i]
		VectorMA (start, 8192, aimdir, end); //faf:  aim fix by kermit, "forward" changed to "aimdir"
		// scale right angle by calculated horizontal spread
		VectorMA (end, r, right, end);
		// scale up angle by calculated vertical spread
		VectorMA (end, u, up, end);

			//faf
			if (self->client)
			{
				AngleVectors (self->client->v_angle, NULL, right, up);
			
					if (self->client->pers.hand == LEFT_HANDED)
						VectorMA (end, (self->client->ps.gunangles[YAW]) * 143, right, end);
					else
					{
						VectorMA (end, (self->client->ps.gunangles[YAW]) * (-143), right, end);
					}

					VectorMA (end, (self->client->ps.gunangles[PITCH]) * (-143), up, end);

					if (self->client->pers.hand == LEFT_HANDED)
						VectorMA (end, (self->client->explosion_angles[YAW]) * 143, right, end);
					else
						VectorMA (end, (self->client->explosion_angles[YAW]) * (-143), right, end);


					VectorMA (end, (self->client->explosion_angles[PITCH]) * (-143), up, end);
			}




	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
			{	ignore = NULL;
							soundtype = Play_Bullet_Hit(self, tr.surface->name, tr.endpos, tr.ent);
							//gi.dprintf("hit\n");
			}

			if ((tr.ent != self) && (tr.ent->takedamage))
			{
				//gi.dprintf("%i\n",damage);
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
				damage = (int)damage/2;
				if (tr.ent->client && !OnSameTeam(tr.ent, self))
					hits++;
			}
		}

		VectorCopy (tr.endpos, from);
	}



	if (hits)
		self->client->resp.accuracy_hits+=hits;
	else
		self->client->resp.accuracy_misses++;


	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{	gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);//(TE_GUNSHOT);  //TE_BULLET_SPARKS
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);

/*		if (random() < .3 && (soundtype == SOUND_WOOD ||
			soundtype == SOUND_CONCRETE))
		{
			ThrowDebris2 (soundtype, tr.endpos);
		} */
	}
	else
		return;
	
//	gi.dprintf("a %f %f %f\n", aimdir[0],aimdir[1],aimdir[2]);
//	gi.dprintf("s %f %f %f\n", tr.plane.normal[0],tr.plane.normal[1],tr.plane.normal[2]);

dot = DotProduct (tr.plane.normal, aimdir);
// gi.dprintf("%f\n",fabs(dot));

 
ric_chance = random();

if (soundtype != SOUND_SAND && ric_chance < .4 && .4-random() > fabs(dot) 
	//&&		!(gi.pointcontents (tr.endpos) & MASK_WATER)
	)
{
	vec3_t ricvec;
	int randn;
edict_t *ricochet;
randn = rand()%2+1;

gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex(va("world/ric%i.wav",randn)), 1, ATTN_NORM, 0);	
gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex(va("world/ric%i.wav",randn)), 1, ATTN_NORM, 0);	


//r = a - 2<a, n> n


ricvec[0] = aimdir[0] -2*(dot)* tr.plane.normal[0];
ricvec[1] = aimdir[1] -2*(dot)* tr.plane.normal[1];
ricvec[2] = aimdir[2] -2*(dot)* tr.plane.normal[2];


	ricochet = G_Spawn ();
	ricochet->owner = self;
	VectorCopy (tr.endpos,ricochet->s.origin);
	VectorCopy (ricvec, ricochet->s.angles);
	ricochet->dmg = damage/2;
	ricochet->count= kick;
	ricochet->style = mod;
	ricochet->is_step = calcv;
	ricochet->think = ricochetthink;
	ricochet->nextthink = level.time+.1;
	gi.linkentity (ricochet);



}

	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);


}


edict_t *ApplyFirstAid (edict_t *ent)
{
	//this function returns the entity that gets the first aid.
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;

    trace_t tr; //detect whats in front of you up to range "vec3_t end"


    vec3_t end;


    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, vec3_origin, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

    // Figure out what we hit, if anything:

//    VectorMA (start, KNIFE_RANGE, forward, end);  //calculates the range vector                      
    VectorMA (start, 40, forward, end);  //faf:  make it easier to heal

    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
                        // figuers out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
                //This tells us to damage the thing that in our path...hehe
				return tr.ent;
            }        
            else        
            {                
                return NULL;
            }    
        }
    }
    return NULL;
} 

#define MIN_ANARCHY_SHOTS 7
#define MAX_ANARCHY_SHOTS 13

int DoAnarchyStuff (edict_t *ent,char *sound)
{
	static int lastone;
	int soundindexM = 0, soundindexF = 0;
	float RanMale ,RanFemale;

	if(!ent->sexpistols) return gi.soundindex(sound);
	
	srand((unsigned)time( NULL ) );
	RanMale=rand()%26;
	RanFemale=rand()%26;
	ent->anarchy++;

	if(lastone==1)
	{
		if(RanMale<=24 && RanMale>=21) soundindexM=gi.soundindex("player/male/jump1.wav");
		else if(RanMale<=20 && RanMale>=16) soundindexM=gi.soundindex("soldier/solidle1.wav");
		else if(RanMale<=15 && RanMale>=14) soundindexM=gi.soundindex("player/male/fall2.wav");
		else if(RanMale<=13 && RanMale>=12) soundindexM=gi.soundindex("player/male/pain25_2");
		else if(RanMale<=11 && RanMale>=10) soundindexM=gi.soundindex("player/male/pain50_2.wav");
		else if(RanMale<=9 && RanMale>=8) soundindexM=gi.soundindex("player/male/pain100_1.wav");
		else if(RanMale<=7 && RanMale>=6) soundindexM=gi.soundindex("player/male/pain100_2.wav");	
		else if(RanMale<=5 && RanMale>=4) soundindexM=gi.soundindex("player/male/pain75_1.wav");	
		else if(RanMale<=3) soundindexM=gi.soundindex("player/male/pain75_2.wav");
		lastone=0;
	}
	else
	{
		//now the girls turn
		if(RanFemale<=24 && RanMale>=20) soundindexF=gi.soundindex("chick/chkdeth2.wav");
		else if(RanFemale<=19 && RanMale>=15) soundindexF=gi.soundindex("chick/chkidle1.wav");
		else if(RanFemale<=14 && RanMale>=13) soundindexF=gi.soundindex("chick/chkidle2.wav");
		else if(RanFemale<=12 && RanMale>=11) soundindexF=gi.soundindex("chick/chksrch2.wav");
		else if(RanFemale<=10 && RanMale>=9) soundindexF=gi.soundindex("player/female/fall2.wav");
		else if(RanFemale<=8 && RanMale>=8) soundindexF=gi.soundindex("chick/cchkidle2.wav");
		else if(RanFemale<=7 && RanMale>=6) soundindexF=gi.soundindex("chick/chkpain1.wav");	
		else if(RanFemale<=5 && RanMale>=4) soundindexF=gi.soundindex("chick/chkpain2.wav");	
		else if(RanFemale<=3) soundindexF=gi.soundindex("player/male/chick/chkpain3.wav");
		lastone=1;
	}
	if(RanMale==25 || RanFemale==25 || ent->anarchy ==MAX_ANARCHY_SHOTS)
	{ //end of act
		if(ent->anarchy>MIN_ANARCHY_SHOTS)
		{
			soundindexF=gi.soundindex("chick/chkatck1.wav");
			soundindexM=gi.soundindex("player/male/death1.wav");
			ent->sexpistols=false;
			ent->anarchy=0;
		}
		else
		{
			soundindexF=gi.soundindex("chick/chkdeth2.wav");
			soundindexM=gi.soundindex("player/male/jump1.wav");
		}
	}
//	gi.dprintf("m: %f f: %f\n",RanMale,RanFemale);
	return( (lastone==0) ? soundindexM : soundindexF);			
} 

void Weapon_Pistol_Fire (edict_t *ent)
{
	int		i;		//temp var
	int		kick=2;
	
	vec3_t		offset;
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		angles;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	
	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	ent->client->ps.gunframe++;     

	if (!ent->client->mags[mag_index].pistol_rnd)
	 {
		 ent->client->ps.gunframe = guninfo->LastFire+1;
		 ent->client->aim = false; // Nick - One bloody line.
		 if (level.time >= ent->pain_debounce_time)
		 {
          gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
          ent->pain_debounce_time = level.time + 1;
		 }
//Make the user change weapons MANUALLY!
	
//	if(auto_weapon_change->value) NoAmmoWeaponChange (ent);
	return;
	}

	for (i=1 ; i<3 ; i++)
	{
		// rezmoth - changed for new firing system
		//ent->client->kick_origin[i] = crandom() * 0.35;
		//ent->client->kick_angles[i] = crandom() * 0.7;
	}
	// rezmoth - changed for new firing system
	//ent->client->kick_origin[0] = crandom() * 0.35;
	//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;



	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);

	// rezmoth - changed for new firing system
	if (ent->client->pers.weapon->position == LOC_PISTOL)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

//	if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || 

	fire_gun(ent, start, forward, damage, kick, PISTOL_SPREAD, PISTOL_SPREAD, mod, false);
	// rezmoth - cosmetic recoil

	//faf if (ent->client->aim)
	//	ent->client->kick_angles[0] -= 2;
	//else
	//	ent->client->kick_angles[0] -= 5;

	//faf:  smoother kick
	ent->client->last_fire_time = level.time;//faf


	/*faf:  this  doesnt really add anything to the game
	if(ent->client->mags[mag_index].pistol_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}
	*/

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);	

	ent->client->mags[mag_index].pistol_rnd--;
/*

	ent->client->anim_priority = ANIM_ATTACK;i
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1;
                ent->client->anim_end = FRAME_crattak9;
            }
            else
            {
                ent->s.frame = FRAME_attack1 - 1;
                ent->client->anim_end = FRAME_attack8;
            }
*/    
	gi.sound(ent, CHAN_WEAPON, DoAnarchyStuff(ent,guninfo->FireSound), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//if(!ent->sexpistols)Play_WepSound(ent,guninfo->FireSound);
	//PlayerNoise(ent, start, PNOISE_WEAPON);

//	if(ent->client->mags[mag_index].pistol_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;	
	
} 


void Weapon_Rifle_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			kick=200;
	int			i;
	
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int	mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;


//faf	if (ent->client->mags[mag_index].rifle_rnd != 1) 
	ent->client->ps.gunframe++;




	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
		{
			ent->client->ps.gunframe = guninfo->LastAFire;
		}
		else
			ent->client->ps.gunframe = guninfo->LastFire;

		ent->client->machinegun_shots = 0;
//		ent->client->ps.gunframe++;
		ent->client->buttons |= BUTTON_ATTACK;
		//ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}

	if (ent->client->next_fire_frame > level.framenum)
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;


	if ( *ent->client->p_rnd == 0 )
	{
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;

		 if (level.time >= ent->pain_debounce_time)
		 {
			 gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = level.time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);
		return;
	}

//	ent->client->ps.gunframe++;
	if (ent->client->next_fire_frame > level.framenum)
		return;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;



	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.

		//faf
		if (ent->client->pers.weapon->classnameb == WEAPON_M1)
		{
			ent->client->ps.gunframe=guninfo->LastReload+1;
			ent->client->weaponstate = WEAPON_END_MAG;
		}
		
		Play_WepSound(ent,guninfo->LastRoundSound);

	}

	/*
	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);

	}
	
	else
		ent->client->ps.gunframe++;
		*/


	for (i=1 ; i<3 ; i++)
	{
		// rezmoth - changed for new firing system
		//ent->client->kick_origin[i] = crandom() * 0.35;
		//ent->client->kick_angles[i] = crandom() * 0.7;
	}
	// rezmoth - changed for new firing system
	//ent->client->kick_origin[0] = crandom() * 0.35;
	//ent->client->kick_angles[0] = -5.25;     
	ent->client->machinegun_shots++;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:7,  ent->viewheight-8);
	if (ent->client->pers.weapon->position == LOC_RIFLE)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//2
	else
		gi.dprintf("*** Firing System Error\n");

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
	// rezmoth - cosmetic recoil
//	if (ent->client->aim)
//		ent->client->kick_angles[0] -= 2.5;
//	else
//		ent->client->kick_angles[0] -= 5;

	ent->client->last_fire_time = level.time;//faf

/*
	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);

	}
*/
	// rezmoth - changed to new firing code
	//fire_rifle (ent, start, forward, damage, kick, mod);

/*
    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	ent->client->mags[mag_index].rifle_rnd--;
	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
//	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}


void Weapon_Submachinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;

	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].submg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}

	if (!ent->client->aim)
	{
		for (i=0 ; i<3 ; i++)
		{
			// Thompson extra-kick (1.7 to 1.9)
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = (crandom() * 0.5)-1.5;
			//ent->client->kick_angles[i] = (crandom() * 1.9)-1.5;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.38;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.8;

	}
	else
	{
		for (i=1 ; i<3 ; i++)
		{	
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = crandom() * 0.35;
			//ent->client->kick_angles[i] = crandom() * 0.7;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	}

	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	ent->client->machinegun_shots++;
	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;
//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	// rezmoth - cosmetic recoil
	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= .5;
		else
			ent->client->kick_angles[0] = -.5;
	}

	// pbowens: for darwin's 3.2 kick
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	//fire_gun(ent, start, forward, damage, kick, SMG_SPREAD, SMG_SPREAD, mod, false);
	fire_gun(ent, start, forward, damage, kick, 20, 20, mod, false);
	ent->client->crosshair_offset_x += (random() -.5) * 1.1;
	//ent->client->crosshair_offset_y += -.15 + (random() -.5) *1.1; //rise slightly
	ent->client->crosshair_offset_y += (random() -.5) *1.1; //don't rise slightly
	


	/*faf:  this  doesnt really add anything to the game
	if(ent->client->mags[mag_index].submg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}*/

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].submg_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}

void Weapon_LMG_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right,up;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;

	if (ent->client->next_fire_frame > level.framenum)
		return;
	
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}
	ent->client->ps.gunframe++;

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe=guninfo->LastAFire-1;
		else 
			ent->client->ps.gunframe=guninfo->LastAFire;
	}
	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (ent->client->mags[mag_index].lmg_rnd < 1)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire + 1:guninfo->LastFire + 1;
		ent->client->weaponstate = WEAPON_READY;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
//		if(auto_weapon_change->value) NoAmmoWeaponChange (ent);

		return;
	}

	ent->client->ps.gunframe++;

	if (!ent->ai)
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
		//ent->client->machinegun_shots++;
		//if (ent->client->machinegun_shots > 7)
			//ent->client->machinegun_shots = 7;
//	}

	// get start / end positions

	//rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8);

	if (ent->client->pers.weapon->position == LOC_L_MACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	if (!ent->client->aim)
	{
/*		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 0.5)-1.5;
			ent->client->kick_angles[i] = (crandom() * 0.7)-1.5;
		}
		//rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.8;
*/

		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	else
	{
/*		for (i=1 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.30;
			ent->client->kick_angles[i] = crandom() * 0.5;
		}
*/		//rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.0;

		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, NULL);
	}


	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//fire_gun(ent, start, forward, damage, kick, LMG_SPREAD, LMG_SPREAD, mod, false);

	fire_gun(ent, start, forward, damage, kick, 20, 20, mod, false);
	ent->client->crosshair_offset_x += (random() -.5) * 1.4;
	ent->client->crosshair_offset_y +=  (random() -.5) *1.4; 
	

	/*faf:  this  doesnt really add anything to the game
	if(ent->client->mags[mag_index].lmg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}
	*/

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);

	Play_WepSound(ent,guninfo->FireSound);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	//PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->mags[mag_index].lmg_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}



void Weapon_HMG_Fire (edict_t *ent)
{
	int			i;
	int			shots=1;
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		angles;
	int			kick = 30;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	trace_t tr; //faf
    vec3_t end; //faf
	vec3_t g_offset; //faf


	if (ent->client->next_fire_frame > level.framenum)
		return;

	//Wheaty: Disable HMG while standing, totally
	//faf:  hmgers can now rest hmg on sandbags/objects in front of them
	if (ent->stanceflags == STANCE_STAND && (ent->client->buttons & BUTTON_ATTACK))
	{
		VectorCopy (vec3_origin,g_offset);

	    AngleVectors (ent->client->v_angle, forward, right, NULL);
	    VectorSet(offset, 24, 8, ent->viewheight-25);
	    VectorAdd (offset, g_offset, offset);
	    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	    VectorScale (forward, -2, ent->client->kick_origin);
		
		VectorMA (start, 15, forward, end);  //calculates the range vector  //faf: 10 = range
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);// figures out what in front of the player up till "end"

		if (ent->client->movement || tr.fraction >= 1.0 || ent->client->v_angle[0] > 40)
		{
			if (ent->client->gunwarntime && ent->client->gunwarntime > level.time - 1)
				return;

			safe_cprintf(ent, PRINT_HIGH, "You need to rest that thing on something to shoot it!\n");
			ent->client->gunwarntime = level.time;
			return;
		}
	}

// this is for when the trigger is released
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (!ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastFire;
		else 
			ent->client->ps.gunframe=guninfo->LastAFire;

		ent->client->weapon_sound = 0;
		ent->client->machinegun_shots=0;
		
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		return;
	}

	/*
// if you are still firing, recycle back to first firing
	if (!ent->client->aim)
	{		
		 if(ent->client->ps.gunframe == guninfo->LastFire) 
			 ent->client->ps.gunframe = guninfo->LastFire-1;
		 else 
			 ent->client->ps.gunframe = guninfo->LastFire;
	}
// else give another guninfo
	else
	{
		if (ent->client->ps.gunframe >= guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-2;
		else 
			ent->client->ps.gunframe++;
		
	}
//	*/

	// pbowens: the following assumes HMGs use only 2 firing frames
	i = (level.framenum % 2) ? 1 : 0;

	if (ent->client->aim)
		ent->client->ps.gunframe = guninfo->AFO[i];
	else
		ent->client->ps.gunframe = guninfo->FO[i];


/*
	if (ent->client->mags[mag_index].hmg_rnd < shots)
		shots = ent->client->mags[mag_index].hmg_rnd;
*/
	if (ent->client->p_rnd && *ent->client->p_rnd == 0)
	{
		if (ent->client->weaponstate != WEAPON_FIRING)
			return;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
//		if(auto_weapon_change->value) NoAmmoWeaponChange (ent);

		 if (ent->client->aim) 
			 ent->client->ps.gunframe = guninfo->LastAFire;
		 else 
			 ent->client->ps.gunframe = guninfo->LastFire;

		 ent->client->weaponstate = WEAPON_READY;

		 return;
	}

	//ent->client->ps.gunframe++;

	// get start / end positions
	
	//if not crouched, make gun jump sporadicly
//faf	if (ent->stanceflags == STANCE_STAND || !ent->client->aim)
	if (!ent->client->aim)
	{	
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
			ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
		}
		//rezmoth - changed for new firing system
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
		// Raise HMG faster
		ent->client->machinegun_shots += 2;
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	else
	{
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] += crandom() * 0.7;
		}
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	// Instead of limit, force the aim down and start over for jumpiness
	if (ent->client->machinegun_shots > 10)
		ent->client->machinegun_shots -= 10;

//	for (i=0 ; i<shots ; i++)
//	{
		// get start / end positions

		//rezmoth - changed for new firing system
		//if(!ent->client->aim) 
		//	//VectorSet(offset, 50, 9, ent->viewheight-6);
		//	VectorSet(offset, 50, 9, crandom() * 40);
		//else 
		//	VectorSet(offset, 50, 0, ent->viewheight-1);

		//rezmoth - changed for new firing system
		/*
		if (!strcmp(ent->client->pers.weapon->ammo, "mg42_mag"))
		{
			VectorSet(offset, 200, 0, ent->viewheight + 4);
		} else if (!strcmp(ent->client->pers.weapon->ammo, "hmg_mag")) {
			VectorSet(offset, 0, 0, ent->viewheight + 20);
		} else {
			gi.dprintf("*** Firing System Error\n");
		}
		*/

		//start[2] += ent->viewheight;
		//start[2] += ent->viewheight;

		VectorSet(offset, 0, 0, ent->viewheight - 0);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// rezmoth - tracers moved to here
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);
		++ent->numfired;

		if (ent->numfired % TRACERSHOT == 1)
		{
			VectorNormalize (right);
			if (ent->client->pers.hand)
				VectorScale (right, .0175 * ent->client->crosshair_offset_x, right);
			else
				VectorScale (right, -.0175 * ent->client->crosshair_offset_x, right);

			VectorAdd (forward, right, forward);
			forward[2] += -.0175 * ent->client->crosshair_offset_y;// (forward, right, forward);
			fire_tracer (ent, start, forward, damage, mod);
		}
		else
//			fire_hmg_bullet (ent, start, forward, damage, mod);//faf:  removing after testing
		{
			//fire_gun(ent, start, forward, damage, kick, HMG_SPREAD, HMG_SPREAD, mod, false);

			fire_gun(ent, start, forward, damage, kick, 20, 20, mod, false);
			ent->client->crosshair_offset_x += (random() -.5) * 2.5;
			ent->client->crosshair_offset_y += (random() -.5) * 2.5; 

			if (ent->client->crosshair_offset_x > 1)
				ent->client->crosshair_offset_x -= .5;
			if (ent->client->crosshair_offset_x < -1)
				ent->client->crosshair_offset_x += .5;
			if (ent->client->crosshair_offset_y > 1)
				ent->client->crosshair_offset_y -= .5;
			if (ent->client->crosshair_offset_y < -1)
				ent->client->crosshair_offset_y += .5;

			ent->client->crosshair_offset_x = (random() -.5) * 2;
			ent->client->crosshair_offset_y = (random() -.5) * 2; 


//			gi.dprintf("%f\n", ent->client->crosshair_offset_x);


		}

		// rezmoth - changed to new firing code
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);

//	}

	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	/*
	if (ent->client->p_rnd && *ent->client->p_rnd==1)
	  { 
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload + 1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	  }
	  */


	//if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	//	ent->client->pers.inventory[ent->client->ammo_index] -= shots;
	ent->client->mags[mag_index].hmg_rnd-=shots;
//	if(ent->client->mags[mag_index].hmg_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}


void Weapon_Rocket_Fire (edict_t *ent)
{
//	float scale;
	vec3_t	offset, start;
	vec3_t	forward, right;

	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	radius_damage = guninfo->damage_radius; //The *damage* within the radius
	int	damage		= guninfo->damage_direct;

	float	damage_radius; // The *radius* of the damage

	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (ent->stanceflags == STANCE_STAND ||
		ent->stanceflags == STANCE_CRAWL ||
//faf		ent->client->movement			 ||
	    !ent->client->aim				 ||
	    gi.pointcontents(ent->s.origin) & MASK_WATER && //Wheaty: Don't let them fire in water
		ent->client->weaponstate == WEAPON_READY) //faf
	{
		if (ent->client->gunwarntime && ent->client->gunwarntime > level.time - 1)
		{}else{
			safe_cprintf(ent, PRINT_HIGH, "You must kneel (crouch), be on dry land, and aim before firing that thing!\n");
			ent->client->gunwarntime = level.time;
		}
		//gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		//ent->pain_debounce_time = level.time + 1;
		ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
														guninfo->LastFire  + 1;
		ent->client->weapon_sound = 0;
		ent->client->weaponstate=WEAPON_READY;
		return;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastAFire+1;
		else
			ent->client->ps.gunframe = guninfo->LastFire+1;
		return;
	}

	// pbowens: rasied rocket dmg from 175 to 225
	damage_radius = 175;//225;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -1;

	VectorSet(offset, 4, 4, ent->viewheight);//faf -8);			//z,x,y
//	VectorSet(offset, 0, 0, 0);//faf -8);			//z,x,y

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (!ent->client->mags[mag_index].antitank_rnd)
	 {

		ent->client->ps.gunframe = (ent->client->aim) ? guninfo->LastAFire+1 : guninfo->LastFire+1;
		 if (level.time >= ent->pain_debounce_time)
		 {
			 gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = level.time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);

		return;
	}

	/*faf:  this was screwing up the firing animation
	if (ent->client->mags[mag_index].antitank_rnd == 1) { // last round fire sounds

		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);

	}*/


/*
	if (ent->client->pers.inventory[ent->client->ammo_index] == 1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}

	if (!ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = guninfo->LastFire;
		
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		//Make the user change weapons MANUALLY!

		if(auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);

		return;
	}
	*/

	VectorNormalize (right);
//meh
//-8	VectorScale (right, -.0165 * ent->client->crosshair_offset_x, right);
//+8	VectorScale (right, -.0185 * ent->client->crosshair_offset_x, right);
//	scale = ent->client->crosshair_offset_x *(-.0165 - (.002 * ((8 + ent->client->crosshair_offset_x)/16)));
//	VectorScale (right, scale, right);
//	VectorScale (right, -.0175 * ent->client->crosshair_offset_x, right);

//	VectorAdd (forward, right, forward);
//	forward[2] += -.0175 * ent->client->crosshair_offset_y;// (forward, right, forward);

	fire_rocket (ent, start, forward, damage, 800, damage_radius, 170);
	//fire_rocket (ent, start, forward, damage, 1000, damage_radius, radius_damage);
	// rezmoth - cosmetic recoil
		ent->client->kick_angles[0] -= 7;
		ent->client->kick_origin[2] -= 5;

	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	ent->client->ps.gunframe++;


	//ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->mags[mag_index].antitank_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}

//backblast area damage

void Weapon_Sniper_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
//	vec3_t      angles;
	int			kick=200;
//	int			i;

	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;



	if (!ent->client->mags[mag_index].sniper_rnd ||
		ent->client->sniper_loaded[ent->client->resp.team_on->index] == false)
	{
		if ((ent->client->ps.gunframe != guninfo->AFO[2]) &&
			(ent->client->buttons & BUTTON_ATTACK) )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}

			ent->client->ps.gunframe = 9;//first idle frame//faf

		}

		ent->client->weaponstate = WEAPON_READY;
		ent->client->aim = false;
		check_unscope(ent);//faf

		if(ent->client->ps.fov != STANDARD_FOV)
			ent->client->ps.fov = STANDARD_FOV;

		return;
	}

	//faf:  keep them from firing too fast
//	if (level.time < ent->client->last_fire_time + 2)
//	{
//		ent->client->ps.gunframe = 9;//first idle frame
//		return;//faf
//	}


	
	if (ent->client->aim)
	{

		if (ent->client->ps.gunframe == (guninfo->AFO[1] - 2) ||  ent->client->next_fire_frame > level.framenum)
		{
			check_unscope(ent);//faf

			ent->client->ps.fov = STANDARD_FOV;
			ent->client->crosshair = false;
			ent->client->weaponstate = WEAPON_LOWER;
//			if (ent->client->next_fire_frame < level.framenum)
//				ent->client->ps.gunframe++;
			return;
		}

	}
	
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	if (ent->client->aim)
		VectorSet(offset, 0, 0,  ent->viewheight);
	else
		VectorSet(offset, 0, 0,  ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	// rezmoth - changed to new firing code
	if (ent->client->aim)
	{
		if (!ent->ai && ent->client->movement && ent->client->ps.gunframe <=  guninfo->AFO[0] )
		{
			safe_cprintf(ent, PRINT_HIGH, "Inaccurate shot!  Hold still when you fire the sniper rifle!\n");
			fire_gun(ent, start, forward, damage, kick, 250, 250, mod, false);
		}
		else
		{
			fire_gun (ent, start, forward, damage, kick, 0,0,mod, false);

		}
	}
	else
		fire_gun(ent, start, forward, damage, kick, 100, 100, mod, false);

	ent->client->sniper_loaded[ent->client->resp.team_on->index] = false;
	// rezmoth - cosmetic recoil
//faf	ent->client->kick_angles[0] -= 3;

	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].sniper_rnd--;
	ent->client->ps.gunframe++;

//	gi.dprintf("    %i\n", ent->client->next_fire_frame);
	
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenlr1b.wav"), 1, ATTN_NORM, 0);
//	if(ent->client->mags[mag_index].sniper_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);

	ent->client->last_fire_time = level.time;//faf


}

//faf:  custom tnt damage radius
void TNT_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;
	float knockb;
	float	distance;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		distance = VectorLength (v);
		points = damage - 0.5 * distance;

		knockb = .3 * (radius - distance);
	
		// only half the distance of radius actually gets injured, rest is knockback only =)
		if (distance > radius/2)
			points = 0;
		//gi.dprintf("%f %s\n", distance, ent->classname);



		if (!OnSameTeam(attacker,ent) && ent->client && ent->client->spawn_safe_time > level.time - .3)
			points = 0;

		if (points > 0 || distance > radius/2)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, knockb, DAMAGE_RADIUS, mod);

				//gi.dprintf ("%f %i \n",  VectorLength (v), (int)points);
			}
		}
	}

	//pbowens: for explosion effects, yes this assumes its an explosion
	SetExplosionEffect(inflictor, damage, radius);

	return;
}






//bcass start - TNT
void TNT_Explode (edict_t *ent)
{
	vec3_t        origin;     
    vec3_t offset;

    if (ent->owner && ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    VectorSet(offset,0,0,32);
    VectorAdd(offset,ent->s.origin,offset);
    VectorCopy (offset, ent->s.origin);
    
/*	for (i = 0; i < 8; i++) //was 8
	{
		for(j=0;j<8;j++)
		{
			VectorSet(grenade1,((rand()%20)-40),((rand()%20)-40),5);
			fire_fragment (ent, offset, grenade1, SCHRAPNEL_DAMAGE, 2, 4500, 10000, MOD_G_SPLASH); //was 4500
		}
	}
*/


    if (ent->owner && ent->owner->client && ent->owner->client->tnt && ent->owner->client->tnt == ent)
	{
        T_Damage (ent->owner, ent->owner, ent->owner, ent->maxs, ent->s.origin, vec3_origin, 200, 0, 0, MOD_HELD_TNT);
	}
    

//    T_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius, MOD_TNT_SPLASH);
    TNT_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius * 2, MOD_TNT_SPLASH);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
/*
    if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
        else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
    else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
        else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
*/

	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/boom.wav"), 1, ATTN_NORM, 0);

	
	Play_Ricochet_Noise (ent, ent->s.origin);
	Smoke_Effect (ent->s.origin, .2);


    G_FreeEdict (ent);
}


static void TNT_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int	index;
//	trace_t		trace;
	
    if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
        return;
	}

//    if (!other->takedamage || !other->client)
//faf    if ( (!other->takedamage || !other->client) && (other != ent) ) //GaP prevent stupid sound
//faf	{

		// stop thudding sound when projectile is stuck in TNT
/*faf		if (other->item && 
			(other->item->position != LOC_KNIFE		&&
	 		 other->item->position != LOC_HELMET	&&
			 other->item->position != LOC_GRENADES)	) */
		
	if (!other->client && other !=ent)
	{
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/tnt/wall.wav"), 1, ATTN_NORM, 0); //grenlb1b.wav
		VectorClear (ent->velocity) ;
		VectorClear (ent->avelocity) ;
		ent->movetype = MOVETYPE_NONE;
       
		ent->touch = NULL;//faf
	}
	return;
//faf	}
/*  What a mess...  TNT pickup for RC1
//else pick it up



	index= ITEM_INDEX(ent->item);

	// rezmoth - following line has crashed a dozen times
	// something about picking up live TNT?
	other->client->pers.inventory[index]++;
	other->client->newweapon = ent->item;
	other->client->grenade_time=ent->nextthink;		
	ChangeWeapon(other);

	safe_cprintf(other, PRINT_HIGH, "You have live TNT!\n");
	
//	other->client->pers.lastweapon = other->client->pers.weapon;
//	other->client->pers.weapon = other->client->newweapon;
//	other->client->newweapon = NULL;
	
	G_FreeEdict(ent);
	other->armed_grenade=true;
	//other->client->weaponstate = WEAPON_FIRING;
	other->client->ps.gunframe = 6;
    //ent->enemy = other;
    //Shrapnel_Explode (ent);
*/

//	ent->enemy = other;
//	TNT_Explode (ent);
}

#define TNT_DEFUSE 60		//40% chance TNT will defuse while submerged

void TNT_Think (edict_t *ent)
{
//	vec3_t	normal;
	int randnum;
	int inwater;

	srand(rand());


	//gi.dprintf ("%i    %i \n",ent->owner->client->resp.team_on->index, ent->obj_owner);

	//switch teams exploit fix
	if (ent->owner && ent->owner->client)
	{
		if (!ent->owner->client->resp.team_on ||
			ent->owner->client->resp.team_on &&
			ent->owner->client->resp.team_on->index != ent->obj_owner)
		{
			ent->think = G_FreeEdict;
			ent->nextthink = level.time + .1;
			return;
		}
	}








	if (level.time > ent->delay)
	{
		// Wheaty: Store it in a variable, baby
		inwater = (gi.pointcontents(ent->s.origin) & MASK_WATER) ? 1 : 0;

		// Wheaty: Various modifyers if the TNT goes off under water
		if (inwater)
		{
			randnum=rand()%100;

			if(randnum > TNT_DEFUSE)
				G_FreeEdict (ent);
			else
			{
				ent->dmg = 1700;
				ent->dmg_radius = 600;
				TNT_Explode(ent);
			}

		}
		else
			TNT_Explode(ent);
	}
/*
gi.WriteByte (svc_temp_entity); 
gi.WriteByte (TE_BLASTER);
gi.WriteDir (normal);
gi.WritePosition(ent->s.origin); 
*/

		if(easter_egg->value==808)
		{
			gi.WriteByte (svc_temp_entity); 
			gi.WriteByte (TE_BOSSTPORT);
			gi.WritePosition(ent->s.origin); 
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SPARKS);
			gi.WritePosition (ent->s.origin);
			gi.WriteDir (vec3_origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		//gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		
	ent->think = TNT_Think;
	ent->nextthink = level.time + .1;
}

void fire_tnt (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float time, float damage_radius, int team)
{
	edict_t	*tnt;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	if (self->client->tnt)
	{
		tnt = self->client->tnt; // use the same grenade
		self->client->tnt = NULL; // reset it

//		if (grenade->item && grenade->item->world_model)
//			grenade->s.modelindex =  gi.modelindex(grenade->item->world_model);
//		else
			tnt->s.modelindex = gi.modelindex ("models/objects/tnt/tris.md2");
		tnt->touch = TNT_Touch;
		gi.linkentity (tnt);

	}
	else
		return; // just get out of here


	VectorCopy (start, tnt->s.origin);
	VectorScale (aimdir, speed, tnt->velocity);
//	VectorMA (tnt->velocity, 25 + crandom() * 10.0, up, tnt->velocity); //was 200
//	VectorMA (tnt->velocity, crandom() * 10.0, right, tnt->velocity);
	VectorSet (tnt->avelocity, 300, 300, 300);


//	if (self->client && self->stanceflags != STANCE_CRAWL)
		tnt->s.origin[2]+= 20;



	//tnt->s.sound = gi.soundindex("weapons/tnt/fizz.wav");
	//gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/tnt/fizz.wav"), 1, ATTN_NORM, 0);
	//in the great words of castrator .. snip .. 
}








//ctc .04, moved out of gbr dll
void Weapon_Vickers_Fire (edict_t *ent)  //faf:  this was taken from the game dll- weapon_ hmg_fire base
{
	int			i;
	int			shots=1;
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		angles;
	int			kick = 30;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	trace_t tr; //faf
    vec3_t end; //faf
	vec3_t g_offset; //faf

	if (ent->client->next_fire_frame > level.framenum)
		return;


	//Wheaty: Disable HMG while standing, totally
	//faf:  hmgers can now rest hmg on sandbags/objects in front of them
	if (ent->stanceflags == STANCE_STAND && (ent->client->buttons & BUTTON_ATTACK))
	{
		VectorCopy (vec3_origin,g_offset);

	    AngleVectors (ent->client->v_angle, forward, right, NULL);
	    VectorSet(offset, 24, 8, ent->viewheight-25);
	    VectorAdd (offset, g_offset, offset);
	    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	    VectorScale (forward, -2, ent->client->kick_origin);
		
		VectorMA (start, 15, forward, end);  //calculates the range vector  //faf: 10 = range
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);// figures out what in front of the player up till "end"

		if (ent->client->movement || tr.fraction >= 1.0 || ent->client->v_angle[0] > 40)
		{
			if (ent->client->gunwarntime && ent->client->gunwarntime > level.time - 1)
				return;

			safe_cprintf(ent, PRINT_HIGH, "You need to rest that thing on something to shoot it!\n");
			ent->client->gunwarntime = level.time;
			return;
		}
	}

	// Nick 22/11/2002 - Next lines taken out - was for the BIG Vickers.
	//if (!ent->client->aim && (ent->client->buttons & BUTTON_ATTACK))
	//{
	//	safe_cprintf(ent, PRINT_HIGH, "You must be aiming to fire!\n");
	//	return;
	//}


// this is for when the trigger is released
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (!ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastFire;
		else 
			ent->client->ps.gunframe=guninfo->LastAFire;

		ent->client->weapon_sound = 0;
		ent->client->machinegun_shots=0;
		
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		return;
	}

	// pbowens: the following assumes HMGs use only 2 firing frames
	i = (level.framenum % 2) ? 1 : 0;

	if (ent->client->aim)
		ent->client->ps.gunframe = guninfo->AFO[i];
	else
		ent->client->ps.gunframe = guninfo->FO[i];


/*
	if (ent->client->mags[mag_index].hmg_rnd < shots)
		shots = ent->client->mags[mag_index].hmg_rnd;
*/
	if (ent->client->p_rnd && *ent->client->p_rnd == 0)
	{
		if (ent->client->weaponstate != WEAPON_FIRING)
			return;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
//		if(auto_weapon_change->value) NoAmmoWeaponChange (ent);

		 if (ent->client->aim) 
			 ent->client->ps.gunframe = guninfo->LastAFire;
		 else 
			 ent->client->ps.gunframe = guninfo->LastFire;

		 ent->client->weaponstate = WEAPON_READY;

		 return;
	}

	//ent->client->ps.gunframe++;

	// get start / end positions
	
	//if not crouched, make gun jump sporadicly
	if (//ent->stanceflags == STANCE_STAND || 
		!ent->client->aim)
	{	
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
			ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
		}
		//rezmoth - changed for new firing system
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
		// Raise HMG faster
		ent->client->machinegun_shots += 2;
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);

	}
	
	else
	{
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] += crandom() * 0.7;
		}
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	// Instead of limit, force the aim down and start over for jumpiness
	if (ent->client->machinegun_shots > 10)
		ent->client->machinegun_shots -= 10;

//	for (i=0 ; i<shots ; i++)
//	{
		// get start / end positions

		//rezmoth - changed for new firing system
		//if(!ent->client->aim) 
		//	//VectorSet(offset, 50, 9, ent->viewheight-6);
		//	VectorSet(offset, 50, 9, crandom() * 40);
		//else 
		//	VectorSet(offset, 50, 0, ent->viewheight-1);

		//rezmoth - changed for new firing system
		/*
		if (!strcmp(ent->client->pers.weapon->ammo, "mg42_mag"))
		{
			VectorSet(offset, 200, 0, ent->viewheight + 4);
		} else if (!strcmp(ent->client->pers.weapon->ammo, "hmg_mag")) {
			VectorSet(offset, 0, 0, ent->viewheight + 20);
		} else {
			gi.dprintf("*** Firing System Error\n");
		}
		*/

		//start[2] += ent->viewheight;
		//start[2] += ent->viewheight;

		VectorSet(offset, 0, 0, ent->viewheight - 0);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// rezmoth - tracers moved to here
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);
		++ent->numfired;

		if (ent->numfired % TRACERSHOT == 1)
			fire_tracer (ent, start, forward, damage, mod);
		else
//			fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
			fire_gun(ent, start, forward, damage, kick, HMG_SPREAD, HMG_SPREAD, mod, false);

		
		//faf:  fire 2 bullets.  to simulate 900 rpm firing rate ****************
		if (ent->numfired % 2 == 1)
		{
			for (i=0 ; i<3 ; i++)
			{
				//rezmoth - changed for new firing system
				ent->client->kick_origin[i] = crandom() * 0.35;
				ent->client->kick_angles[i] += crandom() * 0.7;
			}
			VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
			AngleVectors (angles, forward, right, up);
			
			VectorSet(offset, 0, 0, ent->viewheight - 0);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			fire_gun(ent, start, forward, damage, kick, HMG_SPREAD, HMG_SPREAD, mod, false);
		}
		//**********************************************************************




		// rezmoth - changed to new firing code
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);

//	}

//	Play_WepSound(ent,guninfo->FireSound);//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//faf	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf
	if (ent->numfired % 2 == 1)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("gbr/vickers/firea.wav"), 1, ATTN_NORM, 0);//faf
	else
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("gbr/vickers/fireb.wav"), 1, ATTN_NORM, 0);//faf



//	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);
	
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);// | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (ent->client->p_rnd && *ent->client->p_rnd==1)
  { 
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload + 1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		
		//faf: this is equivalent to using the play_wepsound function
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf
		

	}


	//if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	//	ent->client->pers.inventory[ent->client->ammo_index] -= shots;

	if (ent->numfired % 2 == 1)
		ent->client->mags[mag_index].hmg_rnd-= 2;// faf  shots;
	else
		ent->client->mags[mag_index].hmg_rnd-= 1;// faf  shots;
//	if(ent->client->mags[mag_index].hmg_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}




void Weapon_PIAT_Fire (edict_t *ent)
{

	vec3_t	offset, start;
	vec3_t	forward, right;

	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	radius_damage = guninfo->damage_radius; //The *damage* within the radius
	int	damage		= guninfo->damage_direct;

	float	damage_radius; // The *radius* of the damage

	// Nick 28/11/2002 - flag for incorrect firing.
	short int		firewrong;
	firewrong = 0;


//faf: for testing	safe_bprintf (PRINT_HIGH, "%i machinegun shots\n", ent->client->machinegun_shots); 

	if (ent->client->next_fire_frame > level.framenum)
		return;

	// Nick 28/11/2002 - Tidied up the warnings somewhat (i.e. made it specific to the problem).
	// and of course 'Bristishised' them :).

	// Remove the old warnings first.

	//	if (ent->stanceflags == STANCE_STAND 
	//	ent->stanceflags == STANCE_CRAWL ||
	//	ent->client->movement			 ||
	//   !ent->client->aim				 ||
	//    gi.pointcontents(ent->s.origin) & MASK_WATER) //Wheaty: Don't let them fire in water
	//{
	//	safe_cprintf(ent, PRINT_HIGH, "You must kneel (crouch), be on dry land, and aim before firing that thing!\n");
		


		// Add the new ones.
		if (gi.pointcontents(ent->s.origin) & MASK_WATER) {
			if (ent->client->machinegun_shots == 0)
				safe_centerprintf(ent, "Get out the water to fire!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;
				}

		else if (!ent->client->aim) {

//			if (ent->client->machinegun_shots == 0)
//				safe_centerprintf(ent, "You have gotta AIM it, Tommy Atkins!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		else if (ent->client->movement) {

			if (ent->client->machinegun_shots == 0)
				safe_centerprintf(ent, "Stop bloody moving when you fire!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		else if ((ent->stanceflags == STANCE_STAND) || (ent->stanceflags == STANCE_CRAWL)) {

			if (ent->client->machinegun_shots == 0)
				safe_centerprintf(ent, "Kneel when you fire that bloody thing!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		//else if (ent->stanceflags == STANCE_CRAWL) {
		//	if (ent->client->machinegun_shots == 0)
		//		safe_centerprintf(ent, "Get up on one knee, Tommy Atkins!!\n");
		//	firewrong = 1;
		//	ent->client->machinegun_shots = 1;
		//		}

		if (firewrong == 1) {
		//gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		//ent->pain_debounce_time = level.time + 1;
		//ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
		//													guninfo->LastFire + 1;

		// Nick - 25/11/2002 Had to add this to allow the new 'empty' idle frames to work here.

			if (ent->client->mags[mag_index].antitank_rnd == 0) {
			ent->client->ps.gunframe = 46;
			} else {
			// Original code follows.
			ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
															guninfo->LastFire + 1;
			}
		// End Nick

 		ent->client->weapon_sound = 0;
		ent->client->weaponstate=WEAPON_READY;
		return;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastAFire+1;
		else
			ent->client->ps.gunframe = guninfo->LastFire+1;
		return;
	}

	// pbowens: rasied rocket dmg from 175 to 225
	damage_radius = 175;//faf 225;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);			//z,x,y
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (!ent->client->mags[mag_index].antitank_rnd)
	 {

		ent->client->ps.gunframe = (ent->client->aim) ? guninfo->LastAFire+1 : guninfo->LastFire+1;
		 if (level.time >= ent->pain_debounce_time)
		 {
			 gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = level.time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);

		return;
	}
/* 
	if (ent->client->mags[mag_index].antitank_rnd == 1) { // last round fire sounds

		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	} */
/*
	if (ent->client->pers.inventory[ent->client->ammo_index] == 1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}

	if (!ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = guninfo->LastFire;
		
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		//Make the user change weapons MANUALLY!

		if(auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);

		return;
	}
	*/

	fire_rocket_piat (ent, start, forward, damage, 1600, damage_radius, radius_damage, 1); //faf: was 1400 
	// rezmoth - cosmetic recoil
	ent->client->kick_angles[0] -= 7;
	ent->client->kick_origin[2] -= 5;

//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	ent->client->ps.gunframe++;


	//ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->mags[mag_index].antitank_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}


void Weapon_Sten_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;


	int randnum; //faf

	srand(rand());
	randnum=rand()%300;

	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].submg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}

	if (!ent->client->aim)
	{
		for (i=0 ; i<3 ; i++)
		{
			// Thompson extra-kick (1.7 to 1.9)
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = (crandom() * 0.5)-1.5;
			//ent->client->kick_angles[i] = (crandom() * 1.9)-1.5;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.38;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.8;

	}
	else
	{
		for (i=1 ; i<3 ; i++)
		{	
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = crandom() * 0.35;
			//ent->client->kick_angles[i] = crandom() * 0.7;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	}

	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	
	if (!ent->ai)
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;
//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

//faf for testing	safe_cprintf(ent, PRINT_HIGH, "%i randnum\n", randnum);

//	jamchance = rand() % 100;
	if (randnum == 5)
	{
		// Nick 23/11/2002 - add a specific Sten jam noise (also declared in weapon pre-cache).
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("gbr/sten/jam.wav"), 1, ATTN_NORM, 0);
		// End Nick
		safe_centerprintf(ent, "The bloody Sten jammed!\n"); // Nick 23/11/2002 - Britishised.
		ent->client->mags[mag_index].submg_rnd= 0;
		return;
	}


	// rezmoth - cosmetic recoil
	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;
//	ent->client->kick_angles[1] = ent->client->machinegun_shots * .3;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

//	fire_gun(ent, start, forward, damage, kick, SMG_SPREAD, SMG_SPREAD, mod, false);

	//fire_gun(ent, start, forward, damage, kick, SMG_SPREAD, SMG_SPREAD, mod, false);
	fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
	ent->client->crosshair_offset_x +=  random() -.5 -.1; //move right slightly as per Parts' pa description
	ent->client->crosshair_offset_y +=  random() -.5 -.1; //rise slightly
	



	if(ent->client->mags[mag_index].submg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	
	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].submg_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}





void Weapon_Bren_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;



	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].lmg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	if (!ent->ai &&
		((ent->stanceflags == STANCE_STAND) || (!ent->client->aim)))
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;

	if ((!ent->stanceflags == STANCE_STAND) && (ent->client->aim))
		ent->client->machinegun_shots = 0;
	else
	{
		trace_t tr; 
		vec3_t end;

		if (ent->stanceflags == STANCE_STAND && (ent->client->buttons & BUTTON_ATTACK))
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorSet(offset, 24, 8, ent->viewheight-25);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			VectorScale (forward, -2, ent->client->kick_origin);
			VectorMA (start, 15, forward, end);  //calculates the range vector  //faf: 10 = range
			tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);// figures out what in front of the player up till "end"

			if (!(ent->client->movement || tr.fraction >= 1.0 || ent->client->v_angle[0] > 40))
			{
				ent->client->machinegun_shots = 0;
			}
		}
	}




//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_L_MACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	// rezmoth - cosmetic recoil
	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_gun(ent, start, forward, damage, kick, LMG_SPREAD, LMG_SPREAD, mod, false);

	if(ent->client->mags[mag_index].lmg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//faf	Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//faf	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);// | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].lmg_rnd--;
}



void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	vec3_t	diststart, dist;
	vec3_t	distend = {0, 0, -8192};
	trace_t	disttr;

	VectorCopy(self->s.origin, diststart);
	VectorAdd(diststart, distend, distend);

	disttr = gi.trace (diststart, self->mins, self->maxs, distend, self, MASK_SOLID);
	VectorSubtract(self->s.origin, disttr.endpos, dist);
	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

	if (!(tr.fraction < 1.0))
	{
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		calcv = false;

		if (self->client->aim)
		{
			r = crandom() * 400;
			u = crandom() * 400;
		}
		else
		{
			r = crandom() * 400;
			u = crandom() * 400;
		}

		VectorMA (start, 8192, aimdir, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		if(calcv) calcVspread(self,&tr);

		if (tr.contents & MASK_WATER)
		{
			int		color;
			water = true;

			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, aimdir, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);


					if (crandom() < 0.5)
						gi.WriteByte (TE_GUNSHOT);
					else
						gi.WriteByte (TE_BULLET_SPARKS);

					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client) PlayerNoise(self, tr.endpos, PNOISE_IMPACT);						
				}
			}
		}
	}

	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);

		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
} 

/////////////////////////////////////////////////
// shotgun
/////////////////////////////////////////////////
void Weapon_Shotgun_Fire (edict_t *ent)
{
	int		kick=2, i;
	vec3_t		offset;
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		angles;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;
	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	
	//ent->client->aim = false;

	if (ent->client->next_fire_frame > level.framenum) return;
//	if (!(ent->client->buttons & BUTTON_ATTACK)) return;

	ent->client->ps.gunframe++;     
	

	if ((!ent->client->mags[mag_index].shotgun_rnd))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;

		ent->client->ps.gunframe = (ent->client->aim)?85:16;
		ent->client->weaponstate = WEAPON_READY;
		ent->client->next_fire_frame = level.framenum + 10;	

		return;
	}

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight - 0);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	for (i=0; i<DEFAULT_DEATHMATCH_SHOTGUN_COUNT; i++) fire_shotgun(ent, start, forward, damage, kick, 0, 0, MOD_SHOTGUN2, false);
	
	ent->client->kick_angles[0] -= 2;


	ent->client->mags[mag_index].shotgun_rnd--;
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);

	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_MACHINEGUN);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

//	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;	
}



qboolean fire_katana ( edict_t *self, vec3_t start, vec3_t dir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

	vec3_t	vec;
	float	dot;
	vec3_t	forward;


    // Figure out what we hit, if anything:

    VectorMA (start, 55, dir, end);  //calculates the range vector //50 is sword range
    tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figures out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {

				if (tr.ent->client && 
					tr.ent->client->pers.weapon &&
					tr.ent->client->pers.weapon->classnameb == WEAPON_KATANA)
				{
					AngleVectors (tr.ent->s.angles, forward, NULL, NULL);
					VectorSubtract (self->s.origin, tr.ent->s.origin, vec);
					VectorNormalize (vec);
					dot = DotProduct (vec, forward);
					//safe_bprintf (PRINT_HIGH, "dot = %f \n",dot);
					if (dot > 0.6  &&
						tr.ent->client->weaponstate != WEAPON_FIRING)
					{
						if (random() < .5)	
							gi.sound (self, CHAN_AUTO, gi.soundindex("jpn/katana/hit1.wav") , 1, ATTN_NORM, 0);
						else
							gi.sound (self, CHAN_AUTO, gi.soundindex("jpn/katana/hit2.wav") , 1, ATTN_NORM, 0);
							gi.WriteByte (svc_temp_entity);
							gi.WriteByte (TE_SPARKS);
							gi.WritePosition (tr.endpos);
							gi.WriteDir (tr.endpos);
							gi.multicast (tr.endpos, MULTICAST_PVS);
					}

					else
					{
						T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
					  gi.sound (self, CHAN_AUTO, gi.soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
					}

			
				}
				else
				{
					T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
	                gi.sound (self, CHAN_AUTO, gi.soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
				}

            }        
            else        
            {                
                gi.WriteByte (svc_temp_entity);    
                gi.WriteByte (TE_SPARKS);
                gi.WritePosition (tr.endpos);    
                gi.WriteDir (tr.plane.normal);
                gi.multicast (tr.endpos, MULTICAST_PVS);

				if (tr.surface && Surface(tr.surface->name, SURF_WOOD))
				{
					if (random() < .333)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/wood1.wav") , 1, ATTN_NORM, 0);
					else if (random() < .5)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/wood2.wav") , 1, ATTN_NORM, 0);
					else
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/wood3.wav") , 1, ATTN_NORM, 0);

				}
				else if ( tr.surface && (Surface(tr.surface->name, SURF_SAND) || Surface(tr.surface->name, SURF_GRASS)) )
				{
					if (random() < .333)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/sand1.wav") , 1, ATTN_NORM, 0);
					else if (random() < .5)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/sand2.wav") , 1, ATTN_NORM, 0);
					else
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/sand3.wav") , 1, ATTN_NORM, 0);

				}			
				else if (tr.surface && Surface(tr.surface->name, SURF_METAL))
				{
					if (random() < .333)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/metal1.wav") , 1, ATTN_NORM, 0);
					else if (random() < .5)	
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/metal2.wav") , 1, ATTN_NORM, 0);
					else
						gi.sound (self, CHAN_AUTO, gi.soundindex("bullet/metal3.wav") , 1, ATTN_NORM, 0);

				}
				else if (random() < .5)	
					gi.sound (self, CHAN_AUTO, gi.soundindex("jpn/katana/hit1.wav") , 1, ATTN_NORM, 0);
				else
					gi.sound (self, CHAN_AUTO, gi.soundindex("jpn/katana/hit2.wav") , 1, ATTN_NORM, 0);

            }    
			return true;
        }
    }
    return false;
} 
 


void Weapon_Katana_Fire (edict_t *ent)
{
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;

	vec3_t direction;

	ent->client->ps.gunframe++;//faf


	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

	VectorAdd (forward, right, direction);
	VectorAdd (direction, forward, direction);
	VectorNormalize (direction);
 
	if (!(fire_katana (ent, start, direction, 50, 0)))
	{
		VectorAdd (direction, forward, direction);
		VectorSubtract (forward, right, direction);
		VectorNormalize (direction);
		if (!(fire_katana (ent, start, direction, 50, 0)))
		{
			if (!(fire_katana (ent, start, forward, 50, 0)))
			{
				VectorAdd (direction, forward, direction);
				VectorSubtract (forward, right, direction);
				VectorNormalize (direction);
				if (!(fire_katana (ent, start, direction, 50, 0)))
				{}
			}

		}

	}



	
	ent->client->ps.gunframe++;

	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

	if (ent->client)
		ent->client->last_fire_time = level.time;
}


void Use_Weapon (edict_t *ent, gitem_t *item);
void PBM_CheckFire (edict_t *self);
void PBM_Ignite (edict_t *victim, edict_t *attacker, vec3_t point);
void PBM_FireDropTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );

void Molotov_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *attacker;
	int i;
	vec3_t v;

	edict_t	*e;
	vec3_t distvector;
	float	distance;

	vec3_t org;


    if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
        return;
	}

	if (ent->waterlevel)
	{
		ent->touch = NULL;
		ent->s.sound = 0;
		return;
	}


	org[0] = ent->s.origin[0] + crandom() * ent->size[0];
	org[1] = ent->s.origin[1] + crandom() * ent->size[1];
	org[2] = ent->s.origin[2] + crandom() * ent->size[2];
	ThrowDebris (ent, "models/objects/debris1/tris.md2", 1, org);
	org[0] = ent->s.origin[0] + crandom() * ent->size[0];
	org[1] = ent->s.origin[1] + crandom() * ent->size[1];
	org[2] = ent->s.origin[2] + crandom() * ent->size[2];
	ThrowDebris (ent, "models/objects/debris1/tris.md2", 1, org);




	attacker = ent->owner;

	//explode
/*	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/




    
	for (i=0 ; i<game.maxclients ; i++)
	{	
		e = g_edicts + 1 + i;
	
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->s.origin)
			continue;
		if (e->deadflag)
			continue;
		if (OnSameTeam(e,ent))
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (e->health < 1)
			continue;
		if (e->flyingnun)
			continue;

		VectorSubtract(e->s.origin, ent->s.origin, distvector);
		distance = VectorLength( distvector);

		if (distance < 80)
		{
			if (visible (e,ent))
				PBM_Ignite (e, attacker, ent->s.origin);
		}

	}





	explosion_effect (ent->s.origin, 2);



	//burn
	PBM_Ignite (other, attacker, ent->s.origin);

	T_Damage (other, ent, ent, vec3_origin, other->s.origin, vec3_origin, 50, 0, 0, MOD_BOTTLE);	
	//safe_bprintf (PRINT_HIGH, "ignite \n");

 
	for (i = 0; i <4; i ++)
	{
		edict_t *fire;

		fire = G_Spawn();
		fire->s.modelindex = gi.modelindex("models/fire/tris.md2");
		fire->s.frame      = 0;
		fire->s.skinnum    = 0;
		VectorClear (fire->mins);
		VectorClear (fire->maxs);
		VectorCopy (ent->s.origin, fire->s.origin);
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
		fire->touch        = PBM_FireDropTouch;
		fire->burnout      = level.time + 2 + random() * 3;
		fire->timestamp    = level.time;
		fire->nextthink    = level.time + FRAMETIME;
		fire->think        = PBM_CheckFire;
	//				VectorCopy (30, fire->pos1);
	//				VectorCopy (30, fire->pos2);
		fire->dmg_radius   = 30;
		fire->dmg          = 1;//blast_chance;

//		fire->s.origin[0] = ent->s.origin[0] + crandom() * 100;				
//		fire->s.origin[1] = ent->s.origin[1] + crandom() * 100;
//		fire->s.origin[2] = ent->s.origin[2] + crandom() * 100;

							
		v[0] = 75 + 50 * crandom();
		v[1] = 75 + 50 * crandom();
		if (random() < .5)
		{
			v[0] = -v[0];
		}
		if (random() < .5)
		{
			v[1] = -v[1];
		}


		v[2] = 75 + 50 * crandom();
		VectorMA (fire->velocity, 2, v, fire->velocity);


		gi.linkentity (fire);

		
	}
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("jpn/molotov/break.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict(ent);

}




void Molotov_Throw (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*molotov;
//	trace_t	tr;

	VectorNormalize (dir);

	molotov = G_Spawn();
	VectorCopy (start, molotov->s.origin);
	VectorCopy (start, molotov->s.old_origin);
	vectoangles (dir, molotov->s.angles);
	VectorScale (dir, 750, molotov->velocity);
//faf	molotov->movetype = MOVETYPE_STEP;
	molotov->clipmask = MASK_SHOT;
	molotov->solid = SOLID_BBOX;
//faf 	molotov->s.effects |= effect;


	molotov->movetype = MOVETYPE_TOSS;//STEP;
	VectorSet (molotov->mins, -4, -4, 0);
	VectorSet (molotov->maxs, 4, 4, 4);

	VectorSet (molotov->avelocity, (500 - 1000 * random()), (500 - 1000 * random()), (500 - 1000 * random()));

	molotov->s.modelindex = gi.modelindex("models/weapons/jpn/g_molotov/tris.md2");

	molotov->owner = self;
//faf: for separate helmet/molotov death messages  	molotov->touch = Blade_touch;
	molotov->touch = Molotov_Touch;//faf

	molotov->nextthink = level.time + 5;//faf 2;
	molotov->think = G_FreeEdict;
	molotov->dmg = 0;
	molotov->s.frame = 0;
	
	molotov->classname = "molotov";

//	molotov->s.effects = EF_GRENADE;

	molotov->s.sound = gi.soundindex("inland/fire.wav");

	molotov->spawnflags = 1;
	gi.linkentity (molotov);

  }


void Weapon_Molotov_Fire (edict_t *ent)
{
//	int molotovs;		//var to keep track of how many molotovs are left.
//	int molotov_index; //index to molotov
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;

	int mag_index=ent->client->pers.weapon->mag_index;


	ent->client->ps.gunframe++;//faf



	
	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;
 
//	if(ent->client->pers.inventory[molotov_index])
//	{
		start[2]+= 5;//faf
		Molotov_Throw(ent, start,forward);
//		ent->client->pers.inventory[molotov_index]--;
	//	ent->client->mags[jpn_index].antitank_rnd--;
//	ent->client->mags[jpn_index].antitank_rnd--;
		ent->client->pers.inventory[ent->client->ammo_index]--;

	
	if (ent->client->pers.inventory[ent->client->ammo_index] ==0)
	{
		ent->client->weaponstate=WEAPON_LOWER;
		Use_Weapon (ent, FindItem("fists"));
		return;
	} 
	
	ent->client->ps.gunframe++;
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/toss.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}










void Weapon_Ppsh41_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;



	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (ent->client->mags[mag_index].submg_rnd <= 0)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
			
		return;
	}


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	if (!ent->ai)
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;
//	}

	//faf: adding this to alternate firing sounds
	++ent->numfired;

	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

//faf for testing	safe_cprintf(ent, PRINT_HIGH, "%i randnum\n", randnum);

	// rezmoth - cosmetic recoil
	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;
//sten kick removed	ent->client->kick_angles[1] = ent->client->machinegun_shots * .3;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	fire_gun(ent, start, forward, damage, kick, SMG_SPREAD, SMG_SPREAD, mod, false);

//faf:  a second bullet comes out every other time.  This combined with the 
//		sound effects creates the illusion of a higher (consistant) firing rate
	if (ent->numfired % 2 == 1)
	{
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] += crandom() * 0.7;
		}

		fire_gun(ent, start, forward, damage, kick, SMG_SPREAD, SMG_SPREAD, mod, false);
	}

	if(ent->client->mags[mag_index].submg_rnd<=1) //faf ==
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf
	}

	//faf
	if (ent->numfired % 2 == 1)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("rus/ppsh41/firea.wav"), 1, ATTN_NORM, 0);//faf
	}
	else
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("rus/ppsh41/fireb.wav"), 1, ATTN_NORM, 0);//faf
	}

	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//faf
	if (ent->numfired % 2 == 1)
		ent->client->mags[mag_index].submg_rnd-=2;
	else
		ent->client->mags[mag_index].submg_rnd--;

	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}



void Weapon_Pps43_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;

	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].lmg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
//	if ((ent->stanceflags == STANCE_STAND) || (!ent->client->aim))
	
	if (!ent->ai)
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;

//	if ((!ent->stanceflags == STANCE_STAND) && (ent->client->aim))
//		ent->client->machinegun_shots = 0;

//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_L_MACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	// rezmoth - cosmetic recoil
	if (level.framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_gun(ent, start, forward, damage, kick, LMG_SPREAD,LMG_SPREAD, mod, false);

	if(ent->client->mags[mag_index].lmg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//faf	Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//faf	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);// | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].lmg_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}


void Weapon_MG34_Fire (edict_t *ent)
{
	int			i;
	int			shots=1;
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		angles;
	int			kick = 30;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	trace_t tr; //faf
    vec3_t end; //faf
	vec3_t g_offset; //faf


	if (ent->client->next_fire_frame > level.framenum)
		return;

	//Wheaty: Disable HMG while standing, totally
	//faf:  hmgers can now rest hmg on sandbags/objects in front of them
	if (ent->stanceflags == STANCE_STAND && (ent->client->buttons & BUTTON_ATTACK))
	{
		VectorCopy (vec3_origin,g_offset);

	    AngleVectors (ent->client->v_angle, forward, right, NULL);
	    VectorSet(offset, 24, 8, ent->viewheight-25);
	    VectorAdd (offset, g_offset, offset);
	    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	    VectorScale (forward, -2, ent->client->kick_origin);
		
		VectorMA (start, 15, forward, end);  //calculates the range vector  //faf: 10 = range
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);// figures out what in front of the player up till "end"

		if (ent->client->movement || tr.fraction >= 1.0 || ent->client->v_angle[0] > 40)
		{
			if (ent->client->gunwarntime && ent->client->gunwarntime > level.time - 1)
				return;

			safe_cprintf(ent, PRINT_HIGH, "You need to rest that thing on something to shoot it!\n");
			ent->client->gunwarntime = level.time;
			return;
		}
	}

// this is for when the trigger is released
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (!ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastFire;
		else 
			ent->client->ps.gunframe=guninfo->LastAFire;

		ent->client->weapon_sound = 0;
		ent->client->machinegun_shots=0;
		
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		return;
	}



	// pbowens: the following assumes HMGs use only 2 firing frames
	i = (level.framenum % 2) ? 1 : 0;

	if (ent->client->aim)
		ent->client->ps.gunframe = guninfo->AFO[i];
	else
		ent->client->ps.gunframe = guninfo->FO[i];


	if (ent->client->p_rnd && *ent->client->p_rnd <= 0)
	{
		if (ent->client->weaponstate != WEAPON_FIRING)
			return;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}


		 if (ent->client->aim) 
			 ent->client->ps.gunframe = guninfo->LastAFire;
		 else 
			 ent->client->ps.gunframe = guninfo->LastFire;

		 ent->client->weaponstate = WEAPON_READY;

		 return;
	}

	//ent->client->ps.gunframe++;

	// get start / end positions
	
	//if not crouched, make gun jump sporadicly
//faf	if (ent->stanceflags == STANCE_STAND || !ent->client->aim)
	if (!ent->client->aim)
	{	
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
			ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
		}
		//rezmoth - changed for new firing system
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
		// Raise HMG faster
		ent->client->machinegun_shots += 2;
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	else
	{
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] += crandom() * 0.7;
		}
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	// Instead of limit, force the aim down and start over for jumpiness
	if (ent->client->machinegun_shots > 10)
		ent->client->machinegun_shots -= 10;


		VectorSet(offset, 0, 0, ent->viewheight - 0);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// rezmoth - tracers moved to here
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);
		++ent->numfired;

//		if (ent->numfired % TRACERSHOT == 1)
//			fire_tracer (ent, start, forward, damage, mod);
//		else
//			fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
			fire_gun(ent, start, forward, damage, kick, LMG_SPREAD, LMG_SPREAD, mod, false);

		
		//faf:  fire 2 bullets.  to simulate 900 rpm firing rate ****************
		if (ent->numfired % 2 == 1)
		{
			for (i=0 ; i<3 ; i++)
			{
				//rezmoth - changed for new firing system
				ent->client->kick_origin[i] = crandom() * 0.35;
				ent->client->kick_angles[i] += crandom() * 0.7;
			}
			VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
			AngleVectors (angles, forward, right, up);
			
			VectorSet(offset, 0, 0, ent->viewheight - 0);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			fire_gun(ent, start, forward, damage, kick, LMG_SPREAD, LMG_SPREAD, mod, false);
		}
		//**********************************************************************




//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	if (ent->numfired % 2 == 1)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/mg34/firea.wav"), 1, ATTN_NORM, 0);//faf
	else
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/mg34/fireb.wav"), 1, ATTN_NORM, 0);//faf

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);// | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);


	if (ent->numfired % 2 == 1)
		ent->client->mags[mag_index].hmg_rnd-= 2;
	else
		ent->client->mags[mag_index].hmg_rnd-= 1;

	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}




qboolean fire_sabre ( edict_t *self, vec3_t start, vec3_t dir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

	vec3_t	vec;
	float	dot;
	vec3_t	forward;


    // Figure out what we hit, if anything:

    VectorMA (start, 55, dir, end);  //calculates the range vector //50 is sword range
    tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figures out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {

				if (tr.ent->client && 
					tr.ent->client->pers.weapon &&
					tr.ent->client->pers.weapon->classnameb == WEAPON_SABRE)
				{
					AngleVectors (tr.ent->s.angles, forward, NULL, NULL);
					VectorSubtract (self->s.origin, tr.ent->s.origin, vec);
					VectorNormalize (vec);
					dot = DotProduct (vec, forward);
					//safe_bprintf (PRINT_HIGH, "dot = %f \n",dot);
					if (dot > 0.6  &&
						tr.ent->client->weaponstate != WEAPON_FIRING)
					{
						if (random() < .5)	
							gi.sound (self, CHAN_AUTO, gi.soundindex("pol/sabre/hit1.wav") , 1, ATTN_NORM, 0);
						else
							gi.sound (self, CHAN_AUTO, gi.soundindex("pol/sabre/hit2.wav") , 1, ATTN_NORM, 0);
					}

					else
					{
						T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
					  gi.sound (self, CHAN_AUTO, gi.soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
					}

			
				}
				else
				{
					T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
	                gi.sound (self, CHAN_AUTO, gi.soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
				}

            }        
            else        
            {                
                gi.WriteByte (svc_temp_entity);    
                gi.WriteByte (TE_SPARKS);
                gi.WritePosition (tr.endpos);    
                gi.WriteDir (tr.plane.normal);
                gi.multicast (tr.endpos, MULTICAST_PVS);

				if (random() < .5)	
					gi.sound (self, CHAN_AUTO, gi.soundindex("pol/sabre/hit1.wav") , 1, ATTN_NORM, 0);
				else
					gi.sound (self, CHAN_AUTO, gi.soundindex("pol/sabre/hit2.wav") , 1, ATTN_NORM, 0);

            }    
			return true;
        }
    }
    return false;
} 
 


void Weapon_Sabre_Fire (edict_t *ent)
{
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;

	vec3_t direction;

	ent->client->ps.gunframe++;//faf


	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

	VectorAdd (forward, right, direction);
	VectorAdd (direction, forward, direction);
	VectorNormalize (direction);
 
	if (!(fire_sabre (ent, start, direction, 50, 0)))
	{
		VectorAdd (direction, forward, direction);
		VectorSubtract (forward, right, direction);
		VectorNormalize (direction);
		if (!(fire_sabre (ent, start, direction, 50, 0)))
		{
			if (!(fire_sabre (ent, start, forward, 50, 0)))
			{
				VectorAdd (direction, forward, direction);
				VectorSubtract (forward, right, direction);
				VectorNormalize (direction);
				if (!(fire_sabre (ent, start, direction, 50, 0)))
				{}
			}

		}

	}



	
	ent->client->ps.gunframe++;

	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}


//copy of bren
void Weapon_Breda_Fire (edict_t *ent)
{
	vec3_t		startc;
	vec3_t		forwardc, rightc;
	vec3_t		offsetc;
	vec3_t		up;
	trace_t tr; //faf
    vec3_t endc; //faf
	vec3_t g_offset; //faf
	
	vec3_t		forward, right,start;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;

	if (ent->client->next_fire_frame > level.framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].hmg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
		return;
	}


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	if (!ent->ai &&
		((ent->stanceflags == STANCE_STAND) || (!ent->client->aim)))
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;

	if ((!ent->stanceflags == STANCE_STAND) && (ent->client->aim))
		ent->client->machinegun_shots = 0;

//	}

	if (ent->client->pers.weapon->position == LOC_H_MACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

	


	if (ent->client->aim)
	{
		if (ent->stanceflags == STANCE_STAND)
		{
		
			VectorCopy (vec3_origin,g_offset);

			AngleVectors (ent->client->v_angle, forwardc, rightc, NULL);
			VectorSet(offsetc, 24, 8, ent->viewheight-25);
			VectorAdd (offsetc, g_offset, offsetc);
			P_ProjectSource (ent->client, ent->s.origin, offsetc, forwardc, rightc, startc);
			VectorScale (forwardc, -2, ent->client->kick_origin);
			
			VectorMA (startc, 10, forwardc, endc);  //calculates the range vector  //faf: 10 = range
			tr = gi.trace (ent->s.origin, NULL, NULL, endc, ent, MASK_SHOT);// figures out what in front of the player up till "end"

			if (tr.fraction >= 1.0 ||
				ent->client->v_angle[0] > 40)
			{
				int i;
				for (i=0 ; i<3 ; i++)
				{
					//rezmoth - changed for new firing system
					ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
					ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
				}
				//rezmoth - changed for new firing system
				ent->client->kick_origin[0] = crandom() * 0.35;
				ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
				// Raise HMG faster
				ent->client->machinegun_shots += 2;
				VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
				AngleVectors (angles, forward, right, up);				
			}

		}
	}
	else
	{
		int i;
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
			ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
		}
		//rezmoth - changed for new firing system
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
		// Raise HMG faster
		ent->client->machinegun_shots += 2;
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	fire_gun(ent, start, forward, damage, kick, HMG_SPREAD, HMG_SPREAD, mod, false);

	if(ent->client->mags[mag_index].hmg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//faf	Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}


//faf	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);// | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].hmg_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;
}


void Weapon_Panzerfaust_Fire (edict_t *ent)
{

	vec3_t	offset, start;
	vec3_t	forward, right;

	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	radius_damage = guninfo->damage_radius; //The *damage* within the radius
	int	damage		= guninfo->damage_direct;

	float	damage_radius; // The *radius* of the damage

	// Nick 28/11/2002 - flag for incorrect firing.
	short int		firewrong;
	firewrong = 0;



	if (ent->client->next_fire_frame > level.framenum)
		return;


		// Add the new ones.
		if (gi.pointcontents(ent->s.origin) & MASK_WATER) {
			if (ent->client->machinegun_shots == 0)
				safe_centerprintf(ent, "Get out the water to fire!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;
				}

		else if (!ent->client->aim) {

//			if (ent->client->machinegun_shots == 0)
//				safe_centerprintf(ent, "You have gotta AIM it, Tommy Atkins!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		else if ((ent->stanceflags == STANCE_STAND) || (ent->stanceflags == STANCE_CRAWL)) {

			if (ent->client->machinegun_shots == 0)
				safe_centerprintf(ent, "You have to kneel to fire!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		//else if (ent->stanceflags == STANCE_CRAWL) {
		//	if (ent->client->machinegun_shots == 0)
		//		safe_centerprintf(ent, "Get up on one knee, Tommy Atkins!!\n");
		//	firewrong = 1;
		//	ent->client->machinegun_shots = 1;
		//		}

		if (firewrong == 1) {
		//gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		//ent->pain_debounce_time = level.time + 1;
		//ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
		//													guninfo->LastFire + 1;

		// Nick - 25/11/2002 Had to add this to allow the new 'empty' idle frames to work here.

			if (ent->client->mags[mag_index].antitank_rnd == 0) {
			ent->client->ps.gunframe = 46;
			} else {
			// Original code follows.
			ent->client->ps.gunframe= (ent->client->aim)?72:
															guninfo->LastFire + 1;
			}
		// End Nick

 		ent->client->weapon_sound = 0;
		ent->client->weaponstate=WEAPON_READY;
		return;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastAFire+1;
		else
			ent->client->ps.gunframe = guninfo->LastFire+1;
		return;
	}




	ent->s.modelindex2 = gi.modelindex ("players/ita/w_panzerfmt.md2");






	// pbowens: rasied rocket dmg from 175 to 225
	damage_radius = 175;//faf 225;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -1;

//	VectorSet(offset, 8, 8, ent->viewheight-8);			//z,x,y
	VectorSet(offset, 0, 0, ent->viewheight-2);			//z,x,y
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (!ent->client->mags[mag_index].antitank_rnd)
	 {

		ent->client->ps.gunframe = (ent->client->aim) ? guninfo->LastAFire+1 : guninfo->LastFire+1;
		 if (level.time >= ent->pain_debounce_time)
		 {
			 gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = level.time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);

		return;
	}
/* 
	if (ent->client->mags[mag_index].antitank_rnd == 1) { // last round fire sounds

		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	} */
/*
	if (ent->client->pers.inventory[ent->client->ammo_index] == 1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}

	if (!ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = guninfo->LastFire;
		
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		//Make the user change weapons MANUALLY!

		if(auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);

		return;
	}
	*/


	ent->client->last_fire_time = level.time;//faf



	fire_rocket_panzerfaust (ent, start, forward, damage, 1600, damage_radius, radius_damage, .7);  
	// rezmoth - cosmetic recoil
	ent->client->kick_angles[0] -= 7;
	ent->client->kick_origin[2] -= 5;

//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	ent->client->ps.gunframe++;


	//ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->mags[mag_index].antitank_rnd--;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;


}


