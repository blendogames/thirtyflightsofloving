// p_view.c

#include "g_local.h"
#include "m_player.h"

static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward;
static	vec3_t	right;
static	vec3_t	up;

float	xyspeed;
float	bobmove;
float	bobfracsin;		// sin(bobfrac*M_PI)
int		bobcycle;		// odd cycles are right foot going forward

qboolean PlayerOnFloor (edict_t *player);

/*
===============
SV_CalcRoll

===============
*/
float SV_CalcRoll(vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct(velocity, right);
	sign = (side < 0.0)?-1.0:1.0;
	side = fabs(side);
	
	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;
	
	return (side * sign);
	
}


/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void P_DamageFeedback(edict_t *player)
{
	gclient_t	*client;
	vec3_t		v;
	float		side;
	float		realcount;
	float		count;
	float		kick;
	int			r;
	int			l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (client->anim_priority < ANIM_PAIN && player->s.modelindex == (MAX_MODELS-1)) // was 255
	{
		static int i;

		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i + 1) % 3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}

	realcount = count;
	if (count < 10)
		count = 10;	// allways make a visible effect

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{
//CW++
		player->pain_debounce_time = level.time + 0.7;
		if (current_player->burning)
		{
			if (random() < 0.5)
				gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
//CW--
			r = 1 + (rand()&1);
			//player->pain_debounce_time = level.time + 0.7;	//CW: moved outside if statement
			if (player->health < 25)
				l = 25;
			else if (player->health < 50)
				l = 50;
			else if (player->health < 75)
				l = 75;
			else
				l = 100;
			gi.sound(player, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
		}
	}

	// the total alpha of the blend is allways proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0.0;
	client->damage_alpha += count * 0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed by different armors
	VectorClear(v);
	if (client->damage_parmor)
		VectorMA(v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA(v, (float)client->damage_armor/realcount,  acolor, v);
	if (client->damage_blood)
		VectorMA(v, (float)client->damage_blood/realcount,  bcolor, v);
	VectorCopy(v, client->damage_blend);

	// calculate view angle kicks
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)			// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count * 0.5)
			kick = count * 0.5;
		if (kick > 50)
			kick = 50.0;

		VectorSubtract(client->damage_from, player->s.origin, v);
		VectorNormalize(v);
		
		side = DotProduct(v, right);
		client->v_dmg_roll = kick * side * 0.3;
		
		side = -DotProduct(v, forward);
		client->v_dmg_pitch = kick * side * 0.3;

		client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}


/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001
===============
*/
void SV_CalcViewOffset(edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v;

//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear(angles);

		ent->client->ps.viewangles[ROLL] = 40.0;
		ent->client->ps.viewangles[PITCH] = -15.0;
		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		// add angles based on weapon kick

		VectorCopy(ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0.0)
		{
			ratio = 0.0;
			ent->client->v_dmg_pitch = 0.0;
			ent->client->v_dmg_roll = 0.0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6.0;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6.0;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin
	VectorClear(v);

	// add view height
	v[2] += ent->viewheight;

	// add fall height
	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0.0)
		ratio = 0.0;
	v[2] -= ratio * ent->client->fall_value * 0.4;

	// add bob height
	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6.0)
		bob = 6.0;
	v[2] += bob;

	// add kick offset
	VectorAdd(v, ent->client->kick_origin, v);

	// absolutely bound offsets so the view can never be outside the player box

//DH++
	if (ent->client->spycam)
	{
		VectorSet(v, 0.0, 0.0, 0.0);
        VectorCopy(ent->client->spycam->s.angles, ent->client->ps.viewangles);
		if (ent->client->spycam->svflags & SVF_MONSTER)
			ent->client->ps.viewangles[PITCH] = ent->client->spycam->move_angles[PITCH];
	}
	else
	{
//DH--
		if (v[0] < -14.0)
			v[0] = -14.0;
		else if (v[0] > 14.0)
			v[0] = 14.0;
		if (v[1] < -14.0)
			v[1] = -14.0;
		else if (v[1] > 14.0)
			v[1] = 14.0;
		if (v[2] < -22.0)
			v[2] = -22.0;
		else if (v[2] > 30.0)
			v[2] = 30.0;
	}

	VectorCopy(v, ent->client->ps.viewoffset);
}

/*
==============
SV_CalcGunOffset
==============
*/
void SV_CalcGunOffset (edict_t *ent)
{
	float	delta;
	int		i;

	// gun angles from bobbing
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i = 0; i < 3; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180.0)
			delta -= 360.0;
		if (delta < -180.0)
			delta += 360.0;
		if (delta > 45.0)
			delta = 45.0;
		if (delta < -45.0)
			delta = -45.0;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1 * delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear(ent->client->ps.gunoffset);

	// gun_x / gun_y / gun_z are development tools
	for (i = 0; i < 3; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i] * (gun_y->value);
		ent->client->ps.gunoffset[i] += right[i] * gun_x->value;
		ent->client->ps.gunoffset[i] += up[i] * (-gun_z->value);
	}
}


/*
=============
SV_AddBlend
=============
*/
void SV_AddBlend(float r, float g, float b, float a, float *v_blend)
{
	float	a2;
	float	a3;

	if (a <= 0)
		return;

	a2 = v_blend[3] + ((1 - v_blend[3]) * a);		// new total alpha
	a3 = v_blend[3] / a2;							// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


/*
=============
SV_CalcBlend
=============
*/
void SV_CalcBlend(edict_t *ent)
{
	vec3_t	vieworg;
	int		contents;
	int		remaining;

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER))
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend(1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend(0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend(0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);

	// add for powerups and weapon effects
//CW++
	if (((int)sv_gametype->value > G_FFA) && (ent->client->invincible_framenum < level.framenum))
		ent->s.modelindex4 = 0;

	if (ent->client->frozen_framenum > level.framenum)
	{
		remaining = ent->client->frozen_framenum - level.framenum;
		if (remaining)
			SV_AddBlend(0.5, 0.5, 0.7, 0.2, ent->client->ps.blend);
	}
	else if (ent->client->held_by_agm)
		SV_AddBlend(0.5, 0.5, 1, 0.08, ent->client->ps.blend);
//CW--
	else if (ent->client->invincible_framenum > level.framenum)
	{
//CW++
		if (((int)sv_gametype->value > G_FFA) && !ent->s.modelindex4)
			ent->s.modelindex4 = gi.modelindex("sprites/s_invuln.sp2");
//CW--
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
//CW++
		if ((remaining == 1) && ((int)sv_gametype->value > G_FFA))
			ent->s.modelindex4 = 0;
//CW--
	}
	else if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
//CW++
	else if (ent->client->needle_framenum > level.framenum)
	{
		remaining = ent->client->needle_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(1, 1, 0, 0.04, ent->client->ps.blend);
	}
	else if (ent->client->siphon_framenum > level.framenum)
	{
		remaining = ent->client->siphon_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/siphon2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(1, 0, 1, 0.08, ent->client->ps.blend);
	}
//CW--
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}
//CW++
	else if (ent->client->haste_framenum > level.framenum)
	{
		remaining = ent->client->haste_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/haste2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(0.5, 0.5, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->antibeam_framenum > level.framenum)
	{
		remaining = ent->client->antibeam_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend(0.1, 0.5, 0.1, 0.04, ent->client->ps.blend);
	}
	else if (ent->burning && (ent->health > 0))
		SV_AddBlend(1, 0.6, 0, 0.2, ent->client->ps.blend);
//CW--

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend(ent->client->damage_blend[0], ent->client->damage_blend[1], 
		ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend(0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;
}


/*
=================
P_FallingDamage
=================
*/
void P_FallingDamage(edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;

//CW++
	trace_t	tr;
	vec3_t	bmins;
	vec3_t	bmaxs;
	int		mod = 0;
	float	xyvel;
	float	xyvel_old;
//CW--

	if (ent->s.modelindex != (MAX_MODELS-1)) // was 255
		return;			// not in the player model

 	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((ent->client->oldvelocity[2] < 0) && (ent->velocity[2] > ent->client->oldvelocity[2]) && !ent->groundentity)
		delta = ent->client->oldvelocity[2];
	else
	{
		if (!ent->groundentity)
//CW++
			goto wallcheck;
//CW--

		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}

//CW++
	delta *= delta;
	delta *= (ent->isabot && (ent->client->thrown_by_agm || ent->client->flung_by_agm))?0.0005:0.0001;
//CW--

//ZOID++
	// never take damage if just release grapple or on grapple
	if ((level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2.0) ||
		(ent->client->ctf_grapple && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)))
//CW++
	{
		ent->client->thrown_by_agm = false;
		ent->client->flung_by_agm = false;

		if (!ent->client->held_by_agm)
			ent->client->agm_enemy = NULL;
//CW--
		return;
	}
//ZOID--

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1.0)
//CW++
	{
		if (!ent->waterlevel)
		{
			ent->client->thrown_by_agm = false;
			ent->client->flung_by_agm = false;

			if (!ent->client->held_by_agm)
				ent->client->agm_enemy = NULL;
		}
//CW--
		return;
	}

	// Lazarus: Changed here to NOT play footstep sounds if ent isn't on the ground.
	//          So player will no longer play footstep sounds when descending a ladder.
	if (delta < 7) // Knightmare- was 15, changed to 7
	{
		if (!(ent->watertype & CONTENTS_MUD) && (ent->groundentity || PlayerOnFloor(ent)) )
			ent->s.event = EV_FOOTSTEP; // Knightmare- move Lazarus footsteps client-side
		return;
	}

	if (delta < 15.0)
	{
//CW++
		ent->client->thrown_by_agm = false;
		ent->client->flung_by_agm = false;
		
		if (!ent->client->held_by_agm)
		{
			ent->client->agm_enemy = NULL;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
			// Knightmare- loud footstep for softer landing
			if (delta > 7)
				ent->s.event = EV_LOUDSTEP; 
			else
				ent->s.event = EV_FOOTSTEP;
		}
		return;
	}

	ent->client->fall_time = level.time + FALL_TIME;
	ent->client->fall_value = delta * 0.5;
	if (ent->client->fall_value > 40.0)
		ent->client->fall_value = 40.0;

	if (delta > 30.0)
	{
		if (ent->health > 0)
		{
//CW++
			if (!ent->client->held_by_agm)
			{
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
				if (delta >= 55)
					ent->s.event = EV_FALLFAR;
				else
					ent->s.event = EV_FALL;
			}
		}
		ent->pain_debounce_time = level.time;			// no normal pain sound
		damage = (int)(0.5 * (delta - 30.0));														//CW
		if (damage < 1)
			damage = 1;

		VectorSet(dir, 0.0, 0.0, 1.0);

		if (!((int)dmflags->value & DF_NO_FALLING))													//CW
//CW++
		{
			if (ent->client->flung_by_agm || ent->client->held_by_agm)
			{
				if (ent->groundentity && (ent->client->oldvelocity[2] < 0.0))
				{
					if (ent->client->flung_by_agm)
					{
						mod = MOD_AGM_FLING;
						if ((ent->client->agm_enemy != NULL) && (ent->client->agm_enemy->client->quad_framenum > level.framenum))
							damage *= (int)sv_quad_factor->value;
					}
					else
						mod = MOD_AGM_SMASH;
				}
			}
			else
			{
				if (ent->client->agm_enemy != NULL)
				{
					if (ent->client->oldvelocity[2] < 0.0)
					{
						mod = MOD_AGM_DROP;
						if (damage > 100)
							damage = 100 + (int)(0.2 * (damage - 100));
					}
				}
				else
					mod = MOD_FALLING;
			}

			if (mod)
			{
				if (mod == MOD_FALLING)
					T_Damage(ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
				else
					T_Damage(ent, world, ent->client->agm_enemy, dir, ent->s.origin, vec3_origin, damage, 0, 0, mod);

				ent->client->thrown_by_agm = false;
				ent->client->flung_by_agm = false;

				if (!ent->client->held_by_agm)
					ent->client->agm_enemy = NULL;
			}
			return;
//CW--
		}
	}
	else
	{
//CW++
		ent->client->thrown_by_agm = false;
		ent->client->flung_by_agm = false;

		if (!ent->client->held_by_agm)
		{
			ent->client->agm_enemy = NULL;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
			ent->s.event = EV_FALLSHORT;
		}
		return;
	}

//CW++
wallcheck:

//	We're only interested in impact damage caused by an AGM wielder.

	if (ent->client->agm_enemy == NULL)
		return;

//	The victim can't get damaged if they're grappling.

	if ((level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2.0) ||
		(ent->client->ctf_grapple && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)))
		return;

//	Check to see if we're very close to a hard surface. If so, there's a chance we've been
//	smacked into it hard enough to cause damage.

	VectorSubtract(ent->mins, vec3_border, bmins);
	VectorAdd(ent->maxs, vec3_border, bmaxs);
	tr = gi.trace(ent->s.origin, bmins, bmaxs, ent->s.origin, ent, MASK_PLAYERSOLID);
	if ((tr.fraction < 1.0) && !(tr.surface->flags & SURF_SKY))
	{
		if (!ent->client->agm_enemy->client->agm_pull)
		{

//			Determine previous and current XY-plane velocities.

			xyvel = sqrt((ent->velocity[0] * ent->velocity[0]) + (ent->velocity[1] * ent->velocity[1]));
			xyvel_old = sqrt((ent->client->oldvelocity[0] * ent->client->oldvelocity[0]) +
							 (ent->client->oldvelocity[1] * ent->client->oldvelocity[1]));
			delta = xyvel_old - xyvel;

//			If the XY-plane velocity change is insufficient to cause damage, check for a ceiling hit.

			if (delta < 30.0)
			{
				if ((ent->client->oldvelocity[2] > 0) && (ent->velocity[2] < ent->client->oldvelocity[2]))
					delta = ent->client->oldvelocity[2] - ent->velocity[2];
			}

//			If the change in velocity is large enough, apply damage.

			if (delta > 30.0)
			{	
				damage = (int)((ent->isabot)?(0.15 * (delta - 30.0)):(0.03 * (delta - 30.0)));		//CW

				if (ent->client->agm_enemy->client->quad_framenum > level.framenum)
					damage *= (int)sv_quad_factor->value;

				if (damage < 1)
					damage = 1;

				if (tr.ent->client)
				{
					if (!CheckTeamDamage(tr.ent, ent->client->agm_enemy))
						T_Damage(tr.ent, ent, ent->client->agm_enemy, ent->client->oldvelocity, tr.ent->s.origin, vec3_origin, damage, damage, 0, MOD_AGM_HIT);

					T_Damage(ent, tr.ent, ent->client->agm_enemy, vec3_origin, ent->s.origin, vec3_origin, damage, 0, 0, MOD_AGM_HIT);
				}
				else
				{
					if (ent->client->flung_by_agm)
						mod = MOD_AGM_FLING;
					else if (ent->client->held_by_agm)
						mod = MOD_AGM_SMASH;
					else
						mod = MOD_AGM_THROW;
					T_Damage(ent, world, ent->client->agm_enemy, vec3_origin, ent->s.origin, vec3_origin, damage, 0, 0, mod);
				}
			}
		}
	}
//CW--
}


/*
=============
P_WorldEffects
=============
*/
void P_WorldEffects(void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel;
	int			old_waterlevel;

//CW++
	int			damage;
	int			mod;
//CW--

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12.0;	// don't need air
		return;
	}

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

//CW++
//	Extinguish flame if on fire and in deep enough water.

	if (current_player->burning)
	{
		if (waterlevel > 1)
		{
			current_player->burning = false;
			if (current_player->flame)
			{
				current_player->flame->touch = NULL;
				current_player->flame->think = Flame_Expire;
				current_player->flame->nextthink = level.time + FRAMETIME;
			}
		}
	}
//CW--

	// if just entered a water volume, play a sound
	if (!old_waterlevel && waterlevel)
	{
		if (current_player->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_SLIME)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);

		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);							//CW
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1.0;
	}

	// if just completely exited a water volume, play a sound
	if (old_waterlevel && !waterlevel)
	{
		gi.sound(current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
		current_player->flags &= ~FL_INWATER;
//CW++
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);

		if (current_player->client->agm_enemy != NULL)
		{
			current_player->client->thrown_by_agm = false;
			current_player->client->flung_by_agm = false;
			if (!current_player->client->held_by_agm)
				current_player->client->agm_enemy = NULL;
		}
//CW--
	}

	// check for head just going under water
	if ((old_waterlevel != 3) && (waterlevel == 3))
		gi.sound(current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);

	// check for head just coming out of water
	if ((old_waterlevel == 3) && (waterlevel != 3))
	{
		if (current_player->air_finished < level.time)						// gasp for air
		{
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);						//CW
		}
		else if (current_player->air_finished < level.time + 11.0)			// just break surface
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
	}

	// check for drowning
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10.0;

			if (((current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (!current_client->breather_sound)
					gi.sound(current_player, CHAN_AUTO, gi.soundindex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(current_player, CHAN_AUTO, gi.soundindex("player/u_breath2.wav"), 1, ATTN_NORM, 0);

				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				current_client->breather_sound ^= 1;
			}
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if ((current_player->client->next_drown_time < level.time) && (current_player->health > 0))
			{
				current_player->client->next_drown_time = level.time + 1.0;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
				else if (rand()&1)
					gi.sound(current_player, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(current_player, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time;

//CW++
				if ((current_player->client->agm_enemy != NULL) && current_player->client->held_by_agm)
					T_Damage(current_player, world, current_player->client->agm_enemy, vec3_origin, current_player->s.origin, 
					         vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_AGM_WATER_HELD);
				else
//CW--
					T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 
					         current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12.0;
		current_player->dmg = 2;
	}

	// check for sizzle damage
	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)))
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if ((current_player->health > 0) && (current_player->pain_debounce_time <= level.time) && (current_client->invincible_framenum < level.framenum))
			{
				current_player->pain_debounce_time = level.time + 1.0;
				if (rand() & 1)
					gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
			}
//CW++
			if (current_player->client->agm_enemy != NULL)
			{
				if (current_player->client->held_by_agm)
					mod = MOD_AGM_LAVA_HELD;
				else
					mod = MOD_AGM_LAVA_DROP;
			}
			else
				mod = MOD_LAVA;

			damage = (envirosuit) ? waterlevel : 3*waterlevel;
			if (mod == MOD_LAVA)
				T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, damage, 0, 0, mod);
			else
				T_Damage(current_player, world, current_player->client->agm_enemy, vec3_origin, current_player->s.origin, vec3_origin, damage, 0, 0, mod);
//CW--
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)	// no damage from slime with envirosuit
			{
//CW++
				if (current_player->client->agm_enemy != NULL)
				{
					if (current_player->client->held_by_agm)
						mod = MOD_AGM_SLIME_HELD;
					else
						mod = MOD_AGM_SLIME_DROP;
				}
				else
					mod = MOD_SLIME;

				if (mod == MOD_SLIME)
					T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, waterlevel, 0, 0, mod);
				else
					T_Damage(current_player, world, current_player->client->agm_enemy, vec3_origin, current_player->s.origin, vec3_origin, waterlevel, 0, 0, mod);
//CW--
			}
		}
	}
}


/*
===============
G_SetClientEffects
===============
*/
void G_SetClientEffects(edict_t *ent)
{
	int		pa_type;
	int		remaining;

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if ((ent->health <= 0) || level.intermissiontime)
		return;

//ZOID++
	CTFEffects(ent);																				//CW
//ZOID--

	// show cheaters
	if (ent->flags & FL_GODMODE)																	//CW
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
//CW++
		return;
//CW--
	}

	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType(ent);
		if (pa_type == POWER_ARMOR_SCREEN)
			ent->s.effects |= EF_POWERSCREEN;
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
//CW++
		return;			// makes it clear to other players
//CW--
	}

//	Powerup/weapon effect colour shells.

	if (ent->client->invincible_framenum > level.framenum)											//CW
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4))
			CTFSetPowerUpEffect(ent, EF_PENT);
	}
//CW++
	else if (ent->client->held_by_agm)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_HALF_DAM | RF_SHELL_BLUE;
	}

	else if (ent->client->frozen_framenum > level.framenum)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_BLUE;
	}
//CW--
	else if (ent->client->quad_framenum > level.framenum)											//CW
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			CTFSetPowerUpEffect(ent, EF_QUAD);
	}
//CW++
	else if (ent->client->needle_framenum > level.framenum)
	{
		remaining = ent->client->needle_framenum - level.framenum;
		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_HALF_DAM | RF_SHELL_RED;
		}
	}
	else if (ent->client->siphon_framenum > level.framenum)
	{
		remaining = ent->client->siphon_framenum - level.framenum;
		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
		}
	}
	else if (ent->client->haste_framenum > level.framenum)
	{
		remaining = ent->client->haste_framenum - level.framenum;
		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_GREEN;
		}
	}
	else if (ent->client->antibeam_framenum > level.framenum)
	{
		remaining = ent->client->antibeam_framenum - level.framenum;
		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_HALF_DAM | RF_SHELL_GREEN;
		}
	}

	if (ent->burning)
		ent->s.effects |= EF_ROCKET;
//CW--
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent(edict_t *ent)
{
	if (ent->s.event)
		return;

//	if (ent->groundentity && (xyspeed > 225.0))
	if ( ent->groundentity || PlayerOnFloor(ent) )
	{
		if ( !ent->waterlevel && (xyspeed > 225) )
		{
			if ((int)(current_client->bobtime + bobmove) != bobcycle)
			{
				ent->s.event = EV_FOOTSTEP;
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);											//CW
			}
		}
	/*	else if (ent->in_mud && (ent->waterlevel == 1) && (xyspeed > 40))
		{
			if ( (level.framenum % 10) == 0 )
			{
				ent->s.event = EV_WADE_MUD; // Knightmare- move this client-side
			}
		}*/
		else if ( ((ent->waterlevel == 1) || (ent->waterlevel == 2)) && (xyspeed > 100) /*&& !(ent->in_mud)*/ )
		{
			if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			{
				if (ent->waterlevel == 1)
					ent->s.event = EV_SLOSH;	 // Knightmare- move Lazarus footsteps client-side
				else if (ent->waterlevel == 2)
					ent->s.event = EV_WADE;	 // Knightmare- move Lazarus footsteps client-side
			}
		}
	}
	// Knightmare- swimming sounds
	else if ((ent->waterlevel == 2) && (xyspeed > 60) /*&& !(ent->in_mud)*/ && (world->effects & FX_WORLDSPAWN_STEPSOUNDS))
	{
		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			ent->s.event = EV_WADE;	 // Knightmare- move Lazarus footsteps client-side
	}
	// Ladder sounds
	else if ( (level.framenum % 4) == 0)
	{
		if (!ent->waterlevel && (ent->movetype != MOVETYPE_NOCLIP) && (fabs(ent->velocity[2]) > 50))
		{
			vec3_t	end, forward;
			trace_t	tr;
			AngleVectors(ent->s.angles,forward,NULL,NULL);
			VectorMA(ent->s.origin,2,forward,end);
			tr = gi.trace(ent->s.origin,ent->mins,ent->maxs,end,ent,CONTENTS_LADDER);
			if (tr.fraction < 1.0)
				ent->s.event = EV_CLIMB_LADDER;	 // Knightmare- move Lazarus footsteps client-side
		}
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound(edict_t *ent)
{
	int	weap;																						//CW

	if (ent->client->resp.game_helpchanged != game.helpchanged)
	{
		ent->client->resp.game_helpchanged = game.helpchanged;
		ent->client->resp.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->resp.helpchanged && (ent->client->resp.helpchanged <= 3) && !(level.framenum & 63))
	{
		ent->client->resp.helpchanged++;
		gi.sound (ent, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}

//CW++
	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->weapmodel;
	else
		weap = 0;
//CW--

	if (ent->waterlevel && (ent->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)))
		ent->s.sound = snd_fry;
	else if (weap == WEAP_RAILGUN)																	//CW
		ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
//CW++
	else if (weap == WEAP_SHOCKRIFLE)
		ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
	else if ((weap == WEAP_AGM) && (ent->client->agm_target != NULL))
		ent->s.sound = gi.soundindex("world/amb15.wav");
	else if (weap == WEAP_DISCLAUNCHER)
		ent->s.sound = gi.soundindex("weapons/disc/idle.wav");
	else if (weap == WEAP_CHAINSAW)
		ent->s.sound = gi.soundindex("weapons/chainsaw/idle.wav");
//CW--
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

//#define MAX_STEP_FRACTION 0.80
qboolean PlayerOnFloor (edict_t *player)
{
	trace_t		tr;
	vec3_t		end = {0, 0, -2};

	if (!player->client)
		return false;

	VectorMA (player->s.origin, 50, end, end);
    tr = gi.trace (player->s.origin, NULL, NULL, end, player, MASK_ALL);
    //Com_Printf("%f\n", tr.fraction);
	if (tr.fraction >= sv_step_fraction->value)
		return false;
	else if (player->client->oldvelocity[2] > 0 || player->velocity[2] > 0)
		return false;

	return true;
}


/*
===============
G_SetClientFrame
===============
*/
void G_SetClientFrame(edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck;
	qboolean	run;
	qboolean	floor;

	if (ent->s.modelindex != (MAX_MODELS-1)) // was 255
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	// Knightmare- do the check here, to be sure not to skip over the frame increment
	floor = PlayerOnFloor(ent);

	// check for stand/duck and stop/go transitions
	if ((duck != client->anim_duck) && (client->anim_priority < ANIM_DEATH))
		goto newanim;
	if ((run != client->anim_run) && (client->anim_priority == ANIM_BASIC))
		goto newanim;
	if (!ent->groundentity && (client->anim_priority <= ANIM_WAVE))
		goto newanim;
	// Knightmare- only skip increment if greater than step or swimming
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE && (!floor || ent->waterlevel > 2))
		goto newanim;

	if (client->anim_priority == ANIM_REVERSE)
	{
		if (ent->s.frame > client->anim_end)
		{
			ent->s.frame--;
			return;
		}
	}
	else if (ent->s.frame < client->anim_end)
	{	// continue an animation
		ent->s.frame++;
		return;
	}

	if (client->anim_priority == ANIM_DEATH)
		return;			// stay there

	if (client->anim_priority == ANIM_JUMP)
	{
		if (!ent->groundentity)
			return;		// stay there

		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->client->anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity && (!floor || ent->waterlevel > 2)) // CDawg modify this
	{
//ZOID++
//		If on grapple, don't go into jump frame, go into standing frame.

		if (client->ctf_grapple)
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
		else
		{
//ZOID--
			client->anim_priority = ANIM_JUMP;
			if (ent->s.frame != FRAME_jump2)
				ent->s.frame = FRAME_jump1;
			client->anim_end = FRAME_jump2;
		}
	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else
		{
			// CDawg - add here!
			if (client->backpedaling)
			{
				client->anim_priority = ANIM_REVERSE;
				ent->s.frame = FRAME_run6;
				client->anim_end = FRAME_run1; 
			}
			else
			{
				ent->s.frame = FRAME_run1;
				client->anim_end = FRAME_run6;
			} // CDawg end here!
		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}

/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame(edict_t *ent)
{
//CW++
	edict_t *e;
	edict_t	*check;
	int		inLOS = 0;
	int		inPVS = 0;
//CW--
	float	bobtime;
	int		i;

//CW++
//	Display developer info on entities if flagged to do so (NB: 'show_hostile' abused for this!).

	if (ent->show_hostile)
	{
		for (i = 0; i < globals.num_edicts; ++i)
		{
			check = &g_edicts[i];
			if (!check->inuse)
				continue;
			if (check->svflags & SVF_NOCLIENT)
				continue;

			if (visible(ent, check))
				++inLOS;

			if (gi.inPVS(ent->s.origin, check->s.origin))
				++inPVS;
		}
		gi_cprintf(ent, PRINT_HIGH, "%4d inLOS   %4d inPVS\n", inLOS, inPVS);
	}
//CW--

	current_player = ent;
	current_client = ent->client;

//	If the origin or velocity have changed since ClientThink(), update the pmove values.
//	This will happen when the client is pushed by a bmodel or kicked by an explosion.
//	If it wasn't updated here, the view position would lag a frame behind the body position
//	when pushed - eg. causing "sinking into plats".

	for (i = 0; i < 3; ++i)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i] * 8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i] * 8.0;
	}

//	If the end of unit layout is displayed, don't give the player any normal movement attributes.

	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0.0;
		current_client->ps.fov = 90.0;
		G_SetStats(ent);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, right, up);

//	Add burns from lava, slime, etc.

	P_WorldEffects();

//	Set the model angles from view angles, so other things in the world can tell in which direction
//	you are looking.

//Maj++
	if (!ent->isabot)
	{
//Maj--
		if (ent->client->v_angle[PITCH] > 180.0)
			ent->s.angles[PITCH] = (-360.0 + ent->client->v_angle[PITCH]) * 0.333;					//CW
		else
			ent->s.angles[PITCH] = ent->client->v_angle[PITCH] * 0.333;								//CW

		ent->s.angles[YAW] = ent->client->v_angle[YAW];
		ent->s.angles[ROLL] = 0.0;
		ent->s.angles[ROLL] = SV_CalcRoll(ent->s.angles, ent->velocity) * 4.0;
	}

//	Calculate speed and cycle to be used for all cyclic walking effects.

	xyspeed = sqrt((ent->velocity[0] * ent->velocity[0]) + (ent->velocity[1] * ent->velocity[1]));

//Maj++
	SetBotXYSpeed(ent, &xyspeed);
//Maj--

	if (xyspeed < 5.0)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)			// so bobbing only cycles when on ground
	{
		if (xyspeed > 210.0)
			bobmove = 0.25;
		else if (xyspeed > 100.0)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);
	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4.0;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime * M_PI));

//	Detect hitting the floor.

	P_FallingDamage(ent);

//	Apply all the damage taken this frame.

	P_DamageFeedback(ent);

//	Determine the view offsets.

	SV_CalcViewOffset(ent);

//	Determine the gun offsets.

	SV_CalcGunOffset(ent);

//	Determine the full screen color blend; must be done after viewoffset, so eye contents
//	can be accurately determined.
//	FIXME: with client prediction, the contents should be determined by the client

	SV_CalcBlend(ent);

//ZOID++
	if (!ent->client->chase_target)
//ZOID--
		G_SetStats(ent);

//ZOID++
//	Update chasecam follower stats.

	for (i = 1; i <= (int)maxclients->value; ++i)
	{
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->client->chase_target != ent)
			continue;

		memcpy(e->client->ps.stats, ent->client->ps.stats, sizeof(ent->client->ps.stats));
		e->client->ps.stats[STAT_LAYOUTS] = 1;
		break;
	}
//ZOID--

	G_SetClientEvent(ent);
	G_SetClientEffects(ent);
	G_SetClientSound(ent);
	G_SetClientFrame(ent);

	VectorCopy(ent->velocity, ent->client->oldvelocity);
	VectorCopy(ent->client->ps.viewangles, ent->client->oldviewangles);

//	Clear weapon kicks.

	VectorClear(ent->client->kick_origin);
	VectorClear(ent->client->kick_angles);

//	If the scoreboard is up, update it (CW: update every frame for Gauss Pistol targeting).

//Maj++
	// don't unicast() to bots!
	if (ent->isabot)
		return;
//Maj--

//CW++
	if (ent->client->showscores || ent->client->show_gausstarget)
//CW--
	{
//ZOID++
		if (ent->client->menu && !(level.framenum & 31))											//CW
		{
			PMenu_Do_Update(ent);
			ent->client->menudirty = false;
			ent->client->menutime = level.time;
		}
//ZOID--

//CW++
		else if (!ent->client->menu)
		{
			if ((ent->client->showscores && !(level.framenum & 31)) || (ent->client->show_gausstarget && !ent->client->showscores))
//CW--
				DeathmatchScoreboardMessage(ent, ent->enemy);
		}

		gi.unicast(ent, false);
	}
}
