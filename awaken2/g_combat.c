// g_combat.c

#include "g_local.h"

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage(edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest;
	trace_t	trace;

//	bmodels need special checking because their origin is {0,0,0}.

	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd(targ->absmin, targ->absmax, dest);
		VectorScale(dest, 0.5, dest);
		trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;

		if (trace.ent == targ)
			return true;

		return false;
	}
	
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	return false;
}


/*
============
Killed
============
*/
void Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;

	if ((targ->movetype == MOVETYPE_PUSH) || (targ->movetype == MOVETYPE_STOP) || (targ->movetype == MOVETYPE_NONE))
	{	// doors, triggers, etc
		targ->die(targ, inflictor, attacker, damage, point);
		return;
	}

	targ->die(targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(origin);
	gi.WriteDir(normal);
	gi.multicast(origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/
static int CheckPowerArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			power_armor_type;
	int			index = 0;																			//CW
	int			damagePerCell;
	int			pa_te_type;
	int			power = 0;																			//CW
	int			power_used;

	if (!damage)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	client = ent->client;																			//CW

//CW++
	if (!client)
		return 0;
//CW--

	power_armor_type = PowerArmorType(ent);
	if (power_armor_type != POWER_ARMOR_NONE)
	{
		index = ITEM_INDEX(FindItem("Cells"));
		power = client->pers.inventory[index];
	}

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;

	if (!power)
		return 0;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_t	vec;
		vec3_t	forward;
		float	dot;

		// only works if damage point is in front
		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorSubtract(point, ent->s.origin, vec);
		VectorNormalize(vec);
		dot = DotProduct(vec, forward);
		if (dot <= 0.3)
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = (int)(0.333 * damage);																//CW
	}
	else
	{
		if ((int)sv_gametype->value > G_FFA)														//CW
			damagePerCell = 1;		 // power armor is weaker in CTF, TDM & Assault
		else
			damagePerCell = 2;

		pa_te_type = TE_SHIELD_SPARKS;
		damage = (int)(0.667 * damage);																//CW
	}

	save = power * damagePerCell;
	if (!save)
		return 0;

	if (save > damage)
		save = damage;

	SpawnDamage(pa_te_type, point, normal, save);
	ent->powerarmor_time = level.time + 0.2;

	power_used = (int)(save / damagePerCell);														//CW
	if (client)
		client->pers.inventory[index] -= power_used;
	else
		ent->monsterinfo.power_armor_power -= power_used;

	return save;
}

static int CheckArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	gclient_t	*client;
	gitem_t		*armor;
	int			save;
	int			index;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	index = ArmorIndex(ent);
	if (!index)
		return 0;

	armor = GetItemByIndex(index);
	if (dflags & DAMAGE_ENERGY)
		save = ceil(((gitem_armor_t *)armor->info)->energy_protection * damage);
	else
		save = ceil(((gitem_armor_t *)armor->info)->normal_protection * damage);
	if (save >= client->pers.inventory[index])
		save = client->pers.inventory[index];

	if (!save)
		return 0;

	client->pers.inventory[index] -= save;
	SpawnDamage(te_sparks, point, normal, save);

	return save;
}


qboolean CheckTeamDamage(edict_t *targ, edict_t *attacker)
{
//ZOID++
	if (((int)sv_gametype->value > G_FFA) && targ->client && attacker->client)						//CW
	{
		if ((targ->client->resp.ctf_team == attacker->client->resp.ctf_team) && (targ != attacker))
			return true;
	}
//ZOID--

	return false;
}

//CW++
qboolean CheckTurretTeamDamage(edict_t *targ, edict_t *attacker)
{
	edict_t	*player;
	int		i;

	if (((int)sv_gametype->value > G_FFA) && targ->client && attacker)
	{
		for (i = 0, player = g_edicts + 1; i < (int)maxclients->value; ++i, ++player)
		{
			if (player->client && (player->client->spycam == attacker))
			{
				if ((targ->client->resp.ctf_team == player->client->resp.ctf_team) && (targ != attacker))
					return true;
			}
		}
	}

	return false;
}
//CW--

//						  /--- CW: was *targ
void T_Damage(edict_t *in_targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			psave;
	int			te_sparks;

//DH++
	edict_t		*targ;

	targ = in_targ;
	if (!targ || !targ->inuse)
		return;
//DH--

	if (!targ->takedamage)
		return;

//Maj++
	CheckBotCrushed(targ, inflictor, mod);
//Maj--

//CW++
	if ((mod == MOD_GAUSS_BEAM) && targ->client && (targ->client->antibeam_framenum > level.framenum))
	{
		gi.sound(targ, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	targ->noise_index = 0;
//CW--

//DH++
//	If targ is a fake player for the real player viewing camera, get that player
//	out of the camera and do the damage to him (CW: 'style' abused with a hardcoded
//	value so we don't have to to a slooow classname strcmp).

//CW++
	if (targ->style == ENT_ID_FAKER)
//CW--
	{
		if (targ->target_ent && targ->target_ent->client && targ->target_ent->client->spycam)
		{
//CW++
			if (targ->target_ent->client->spycam == attacker)	// shot self with turret
				targ->target_ent->noise_index = 1;				// abuse var so edict_t doesn't have to be extended
//CW--
			if (attacker->enemy == targ)
			{
				attacker->enemy = targ->target_ent;
				attacker->goalentity = NULL;
				attacker->movetarget = NULL;
			}
			targ = targ->target_ent;
			camera_off(targ);
		}
		else
			return;		// don't damage target_monitor camplayer
	}
//DH--

//Maj++	
	if (targ != attacker)																			//CW...
	{
		if (CheckTeamDamage(targ, attacker))
			damage = 0;
		else
		{
			if (targ->client && !targ->isabot && attacker->client)
				targ->client->current_enemy = attacker;
		}
	}
//Maj--


	// friendly fire avoidance: if enabled you can't hurt teammates (but you can hurt yourself);
	// knockback still occurs
	meansOfDeath = mod;

	client = targ->client;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

//ZOID++
	// strength tech
	damage = CTFApplyStrength(attacker, damage);
//ZOID--

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if (knockback && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
		{
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50.0)
				mass = 50.0;
			else
				mass = (float)targ->mass;

			if (client && (attacker == targ) && (inflictor->die != Trap_DieFromDamage))				//CW
//CW++
			{
				if (inflictor->die == C4_DieFromDamage)
					VectorScale(dir, 2250.0 * ((float)knockback / mass), kvel);		// the C4 jump hack
				else
//CW--
					VectorScale(dir, 1600.0 * ((float)knockback / mass), kvel);		// the rocket jump hack
			}
			else
//CW++
			{
				if ((inflictor->touch == Rocket_Touch) || (inflictor->die == C4_DieFromDamage))
					VectorScale(dir, sv_rocket_kick_scale->value * ((float)knockback / mass), kvel);
				else
//CW--
					VectorScale(dir, 500.0 * ((float)knockback / mass), kvel);
			}

			VectorAdd(targ->velocity, kvel, targ->velocity);
		}
	}

	take = damage;
	save = 0;

	// check for godmode
	if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		take = 0;
		save = damage;
		SpawnDamage(te_sparks, point, normal, save);
	}

	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2.0;
		}
		take = 0;
		save = damage;
	}

//ZOID++
	//team armor protect
	if (((int)sv_gametype->value > G_FFA) && targ->client && attacker->client &&					//CW
		(targ->client->resp.ctf_team == attacker->client->resp.ctf_team) &&
		(targ != attacker) && ((int)dmflags->value & DF_ARMOR_PROTECT))
	{
		psave = asave = 0;
	}
	else if (CheckTurretTeamDamage(targ, attacker) && ((int)dmflags->value & DF_ARMOR_PROTECT))
	{
		psave = asave = 0;
	}
	else
	{
//ZOID--
		psave = CheckPowerArmor(targ, point, normal, take, dflags);
		take -= psave;
	
		asave = CheckArmor(targ, point, normal, take, te_sparks, dflags);
		take -= asave;
	}

	//treat cheat/powerup savings the same as armor
	asave += save;

//ZOID++
	//resistance tech
	take = CTFApplyResistance(targ, take);
//ZOID--

	// team damage avoidance
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
		return;

//CW++
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTurretTeamDamage(targ, attacker))
		return;
//CW--

//ZOID++
	CTFCheckHurtCarrier(targ, attacker);
//ZOID--

//	Do the damage.

	if (take)
	{
		if ((targ->svflags & SVF_MONSTER) || client)
		{
			SpawnDamage(TE_BLOOD, point, normal, take);
			Bot_CheckEnemy(client, attacker, targ, mod);							//Maj++
		}
		else
			SpawnDamage(te_sparks, point, normal, take);

		if (!CTFMatchSetup())
//CW++
		{			
//			D89 (needle) power-up: damage is halved.

			if (client && (client->needle_framenum > level.framenum))
				take = (int)(0.5 * take);
//CW--

			targ->health = targ->health - take;

//CW++
//			Awakening (siphon) power-up: attacker gets health equal to damage inflicted (unless it's us).

			if (client && attacker->client)
			{
				if (attacker->client->siphon_framenum > level.framenum)
				{
					if ((targ != attacker) && (attacker->health < (int)sv_health_max_siphon->value))
					{
						attacker->health += take + ((targ->health < 0)?targ->health:0);
						gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/siphon3.wav"), 1, ATTN_NORM, 0);
						if (attacker->health > (int)sv_health_max_siphon->value)
							attacker->health = (int)sv_health_max_siphon->value;
					}
				}

				if ((mod == MOD_AGM_DISRUPT) && (attacker->health < (int)sv_health_max_siphon->value))
				{
					attacker->health += (int)(sv_agm_disrupt_siphon->value * (take + ((targ->health < 0)?targ->health:0)));
					gi.sound(attacker, CHAN_ITEM, gi.soundindex("parasite/paratck3.wav"), 1, ATTN_NORM, 0);
					if (attacker->health > (int)sv_health_max_siphon->value)
						attacker->health = (int)sv_health_max_siphon->value;
				}
			}
		}
//CW--

		if (targ->health < 1)																		//CW
		{
			if ((targ->svflags & SVF_MONSTER) || client)
				targ->flags |= FL_NO_KNOCKBACK;

			Killed(targ, inflictor, attacker, take, point);
			return;
		}
	}

	if (client)
	{
		if (!(targ->flags & FL_GODMODE) && take && !CTFMatchSetup())
			targ->pain(targ, attacker, knockback, take);
	}
	else if (take)
	{
		if (targ->pain)
			targ->pain(targ, attacker, knockback, take);
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks at the end of the frame
	if (client)
	{
		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy(point, client->damage_from);
	}
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;
	float	points;

//CW++
	edict_t	*targ;
//CW--

	while ((ent = FindRadius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

//CW++
//		For detonating C4 bundles, don't apply radius damage to other C4 bundles belonging to the player.

		if (inflictor->die == C4_DieFromDamage)
		{
			if ((ent->die == C4_DieFromDamage) && (ent->owner == inflictor->owner))
				continue;
		}
//CW--
		VectorAdd(ent->mins, ent->maxs, v);
		VectorMA(ent->s.origin, 0.5, v, v);
		VectorSubtract(inflictor->s.origin, v, v);
		points = damage - (0.5 * VectorLength(v));
		if (ent == attacker)
			points = points * 0.5;

		if ((points > 0) && CanDamage(ent, inflictor))												//CW
		{
//CW++
			if (ent->client)
			{
				if (ent->target_ent && ent->target_ent->client && ent->target_ent->client->spycam)
					targ = ent->target_ent;															// ...ent -> targ for all below
				else
					targ = ent;

//				Apply freezing effect of Shock Rifle (disintegrator mode) to still-living players.

				if (mod == MOD_SR_DISINT_WAVE)
				{
					if (CheckTeamDamage(targ, attacker))
						continue;
					if (targ->client->invincible_framenum > level.framenum)
						continue;

					if ((ent->health > 0) && (targ->client->enviro_framenum <= level.framenum))
					{
						if (targ == attacker)
							gi_centerprintf(targ, "You've frozen yourself\n");
						else
//CW++
						{
							if (!attacker->client)
								gi_centerprintf(targ, "You've been frozen\n");
							else
//CW--
								gi_centerprintf(targ, "You've been frozen by %s\n", attacker->client->pers.netname);
						}

						if (ent->client->frozen_framenum > level.framenum)
							ent->client->frozen_framenum += (int)(10.0 * sv_shock_freeze_time->value);	//CW
						else
							ent->client->frozen_framenum = level.framenum + (int)(10.0 * sv_shock_freeze_time->value);	//CW
					}
				}

//				Ignite players who are caught in the blast radius of a firebomb.

				else if (mod == MOD_FIREBOMB_SPLASH)
				{
					if (targ->client->enviro_framenum > level.framenum)
						continue;
					if (CheckTeamDamage(targ, attacker))
						continue;
					if (targ->client->invincible_framenum > level.framenum)
						continue;

					if (!ent->burning)
					{
						ent->burning = true;
						Spawn_Flame(inflictor, ent, ent->s.origin, inflictor->radius_dmg);
					}
				}
			}

			VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
			T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
		}
	}
}
