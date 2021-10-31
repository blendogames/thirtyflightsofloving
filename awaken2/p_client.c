// p_client.c

#include "g_local.h"
#include "m_player.h"


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
//CW++
	self->svflags |= SVF_NOCLIENT;
	self->style = ENT_ID_PLAYER_SPAWN;
//CW--
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
//CW++
	self->svflags |= SVF_NOCLIENT;
	self->style = ENT_ID_PLAYER_SPAWN;
//CW--
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
We don't use these for Awakening II.
*/
void SP_info_player_coop(edict_t *self)
{
	G_FreeEdict(self);
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *self)
{
//CW++
	self->svflags |= SVF_NOCLIENT;
	self->style = ENT_ID_INTERMISSION;
//CW--
}


//CW++
/*QUAKED info_player_attack (1 0.3 0.3) (-16 -16 -24) (16 16 32) NoModel
  ASSAULT: Potential spawning position for a player on the attacking team.
*/
void SP_info_player_attack(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)
	{
		G_FreeEdict(self);
		return;
	}

	if (self->count < 0)
		self->count = 0;

	self->svflags |= SVF_NOCLIENT;
	self->style = ENT_ID_PLAYER_SPAWN;
}

/*QUAKED info_player_defend (0.3 1 0.3) (-16 -16 -24) (16 16 32) NoModel
  ASSAULT: Potential spawning position for a player on the defending team.
*/
void SP_info_player_defend(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)
	{
		G_FreeEdict(self);
		return;
	}

	if (self->count < 0)
		self->count = 0;

	self->svflags |= SVF_NOCLIENT;
	self->style = ENT_ID_PLAYER_SPAWN;
}
//CW--

//=======================================================================

void player_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}

qboolean IsFemale(edict_t *ent)
{
	char *info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey(ent->client->pers.userinfo, "skin");
	if ((info[0] == 'f') || (info[0] == 'F'))
		return true;

	return false;
}

void ClientObituary(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	char		*message = NULL;																	//Maj
	char		*message2 = "";																		//Maj
	int			mod;
	qboolean	ff;

//CW++
	edict_t		*player;
	char		killer[256];
	int			i;
	float		r;
//CW--

	ff = meansOfDeath & MOD_FRIENDLY_FIRE;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	message = NULL;
	message2 = "";

	switch (mod)																					//CW
	{
		case MOD_SUICIDE:
			message = "suicides";
			break;

		case MOD_FALLING:
			message = (random()<0.5)?"cratered":"did a terminal face-plant";						//CW
			break;

		case MOD_CRUSH:
			message = (random()<0.5)?"was squished":"was turned into a pancake";					//CW
			break;

		case MOD_WATER:
			message = (random()<0.5)?"sank like a rock":"went to swim with the fishes";				//CW
			break;

		case MOD_SLIME:
			message = (random()<0.5)?"melted":"drank too much green goo";							//CW
			break;

		case MOD_LAVA:
			message = (random()<0.5)?"does a back flip into the lava":"went to visit the volcano god";//CW
			break;

		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = (random()<0.5)?"blew up":"tried to cuddle a large explosion";					//CW
			break;

		case MOD_EXIT:
			message = "found a way out";
			break;

		case MOD_TARGET_LASER:
			message = "saw the light";
			break;

		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;

		case MOD_BOMB:
//CW++
			message = "caught a bomb";
			break;

		case MOD_FORBIDDEN:
			message = "tried to enter a forbidden zone";
			break;
//CW--
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
	}

	if (attacker == self)
	{
		switch (mod)
		{
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
//CW++
			case MOD_SPIKE_SPLASH:
				message = "played with explosive sharp objects";
				break;

			case MOD_FLAMETHROWER:
				message = (random()<0.5)?"had a meltdown":"really shouldn't play with fire";
				break;

			case MOD_FLAME:
				message = (random()<0.5)?"became toast":"became a greasy charcoal lump";
				break;

			case MOD_FIREBOMB:
				message = "really shouldn't play with fire";
				break;

			case MOD_FIREBOMB_SPLASH:
				if (IsFemale(self))
					message = "burned herself out";
				else
					message = "burned himself out";
				break;

			case MOD_SR_DISINT_WAVE:
				message = (random()<0.5)?"went surfing":"should have used a smaller gun";
				break;

			case MOD_SR_HOMING:
				if (IsFemale(self))
					message = "ate her own plasma";
				else
					message = "ate his own plasma";
				break;

			case MOD_C4:
				if (IsFemale(self))
					message = "demolished herself";
				else
					message = "demolished himself";
				break;

			case MOD_C4_HELD:
				if (IsFemale(self))
					message = "forgot to throw her C4";
				else
					message = "forgot to throw his C4";
				break;

			case MOD_TRAP:
				if (IsFemale(self))
					message = "was disembowelled by her own trap";
				else
					message = "was disembowelled by his own trap";
				break;

			case MOD_TRAP_HELD:
				if (IsFemale(self))
					message = "was too attached to her trap";
				else
					message = "was too attached to his trap";
				break;

			case MOD_D89:
				message = "overdosed on D89";
				break;

			case MOD_DISC:
				if (IsFemale(self))
					message = "was sliced open by her own disc";
				else
					message = "was sliced open by his own disc";
				break;
//CW--
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
		}
	}

	if (message)
	{
		gi_bprintf (PRINT_MEDIUM, "%s %s\n", self->client->pers.netname, message);
			self->client->resp.score--;
//CW++
		if (self->client->resp.ctf_team == CTF_TEAM1)
			--teamgame.frags1;
		if (self->client->resp.ctf_team == CTF_TEAM2)
			--teamgame.frags2;
//CW--
		self->enemy = NULL;
		return;
	}

	self->enemy = attacker;
	if (attacker && attacker->client)
	{
		switch (mod)
		{
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;

			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;

			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;

			case MOD_RAILGUN:
//CW++
				if (random() < 0.5)
				{
					message = "was railed by";
					message2 = " ";
				}
				else
				{
					message = "played slug-magnet with";
					message2 = "'s railgun";
				}
//CW--
				break;

			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
//ZOID++
			case MOD_GRAPPLE:
				message = (random()<0.5)?"was caught by":"ate the pointy end of";					//CW
				message2 = "'s grapple";
				break;
//ZOID--

//CW++
			case MOD_CHAINSAW:
				r = random();
				if (r < 0.333)
					message = "was turned into puree by";
				else if (r < 0.667)
					message = "was used as grease to lubricate";
				else
					message = "was beautified by";
				message2 = "'s chainsaw";
				break;

			case MOD_DESERTEAGLE:
				message = (random()<0.5)?"took a bullet from":"bit the bullet from";
				message2 = "'s Desert Eagle";
				break;

			case MOD_JACKHAMMER:
				r = random();
				if (r < 0.333)
					message = "was blown away by";
				else if (r < 0.667)
					message = "was shredded like tissue by";
				else
					message = "was torn apart by";
				message2 = "'s jackhammer";
				break;

			case MOD_MAC10:
				r = random();
				if (r < 0.333)
					message = "was mown down by";
				else if (r < 0.667)
					message = "was cut in half by";
				else
					message = "was gunned down by";
				message2 = (random()<0.95)?"'s Mac-10":"'s Sven-10...dat's a good one dere, ya!";
				break;

			case MOD_GAUSS_BLASTER:
				message = "was splattered by";
				message2 = "'s gauss pistol";
				break;

			case MOD_GAUSS_BEAM:
				r = random();
				if (r < 0.333)
				{
					if (IsFemale(self))
						message = "had her tent ruined by";
					else
						message = "had his tent ruined by";
				}
				else if (r < 0.667)
					message = "found you can't hide from";
				else
					message = "cursed the long reach of";
				message2 = "'s particle beam";
				break;

			case MOD_GAUSS_BEAM_REF:
				message = "'s particle beam was reflected off";
				message2 = (random()<0.8)?" ":" ... ha ha!";
				break;

			case MOD_TRAP:
				if (random() < 0.5)
				{
					message = "was disembowelled by";
					message2 = "'s trap";
				}
				else
				{
					message = "could not get free from";
					message2 = "'s diabolical trap";
				}
				break;

			case MOD_C4:
			case MOD_C4_HELD:
				r = random();
				if (r < 0.333)
					message = "was demolished by";
				else if (r < 0.667)
					message = "was blown into tiny pieces by";
				else
					message = "was turned into Chinese fireworks by";
				message2 = "'s C4";	
				break;

			case MOD_C4_PROXIMITY:
				message = (random()<0.5)?"didn't notice the deadly outline of":"came too close to";
				message2 = "'s proximity mine";
				break;

			case MOD_C4_LIFETIME:
				message = "fell victim to";
				message2 = "'s short fuse";
				break;

			case MOD_SPIKE:
				r = random();
				if (r < 0.45)
				{
					message = "was spiked by";
					message2 = " ";
				}
				else if (r < 0.9)
				{
					message = "was impaled on";
					message2 = "'s spike";
				}
//SNX++
				else
				{
					message = "was mounted by";
					message2 = "'s spike";
				}
//SNX--
				break;

			case MOD_SPIKE_SPLASH:
				message = (random()<0.5)?"exploded everywhere thanks to":"was exploded by";
				message2 = "'s spike";
				break;

			case MOD_FLAMETHROWER:
				message = (random()<0.5)?"was melted by":"was terminally scorched by";
				message2 = "'s flamethrower";
				break;

			case MOD_FLAME:
				message = (random()<0.5)?"was cremated by":"was flame-grilled to perfection by";
				message2 = " ";
				break;

			case MOD_FIREBOMB:
				message = "was obliterated by";
				message2 = "'s firebomb";
				break;

			case MOD_FIREBOMB_SPLASH:
				message = "was fatally scorched by";
				message2 = "'s firebomb blast";
				break;

			case MOD_SR_DISINT:
				message = (random()<0.5)?"was disintegrated by":"was turned into a red mist by";
				message2 = "'s shockbolt";
				break;

			case MOD_SR_DISINT_WAVE:
				message = (random()<0.5)?"was rocked by":"was spread like paste by";
				message2 = "'s shockwave";
				break;

			case MOD_SR_HOMING:
				message = (random()<0.5)?"couldn't outrun":"ran but couldn't hide from";
				message2 = "'s homing plasma";
				break;

			case MOD_AGM_FEEDBACK:
				message = "crossed streams with";
				message2 = "'s AGM";
				break;

			case MOD_AGM_BEAM_REF:
				message = "'s AG Manipulator beam was reflected off";
				message2 = (random()<0.8)?" ":" ... bwahaha!";
				break;

			case MOD_AGM_HIT:
				message = "was used for bowling practice by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_FLING:
				message = "was smeared across the map by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SMASH:
				message = "was smashed into a hard surface by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_THROW:
				message = "was used to paint the walls by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DROP:
				message = (random()<0.5)?"was used to mop the floor by":"was spread across the floor by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_HELD:
				message = (random()<0.5)?"was dunked in lava like a donut by":"was forced to drink lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_DROP:
				message = "was tossed into the lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_HELD:
				message = (random()<0.5)?"was dunked in slime like a biscuit by":"was forced to drink slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_DROP:
				message = "was tossed into the slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_WATER_HELD:
				message = "was drowned like a rat by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TRIG_HURT:
				message = "was sent to the wrong place by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TARG_LASER:
				message = "was shown a very bright light by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DISRUPT:
				if (random() < 0.5)
				{
					if (IsFemale(self))
						message = "had her insides scrambled by";
					else
						message = "had his insides scrambled by";
				}
				else
					message = "was sucked dry by";
				message2 = "'s cellular disruptor";
				break;

			case MOD_DISC:
				message = (random()<0.5)?"was sliced-n-diced by":"was gutted by";
				message2 = "'s disc";
				break;
//CW--
		}

		if (message)
		{
			gi_bprintf(PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

//Maj++
			if ((int)sv_bots_taunt->value > 0)														//CW++
				TauntVictim(attacker, self);

			if (((int)sv_bots_insult->value > 0) && !attacker->client->pers.muted)					//CW++
				InsultVictim(attacker, self);
//Maj--

			if (ff)
			{
				attacker->client->resp.score--;
//CW++
				if (self->client->resp.ctf_team == CTF_TEAM1)
					--teamgame.frags1;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					--teamgame.frags2;
			}
			else if ((mod == MOD_GAUSS_BEAM_REF) || (mod == MOD_AGM_FEEDBACK))
			{
				self->client->resp.score--;
				if (self->client->resp.ctf_team == CTF_TEAM1)
					--teamgame.frags1;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					--teamgame.frags2;
			}
			else
			{
				if (self->client->resp.ctf_team == CTF_TEAM1)
					++teamgame.frags2;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					++teamgame.frags1;
//CW--
				attacker->client->resp.score++;
			}

			return;
		}
	}

//CW++
	else if (attacker && (!Q_stricmp(attacker->classname, "turret_breach") || !Q_stricmp(attacker->classname, "model_turret")))
	{
		switch (mod)
		{
			case MOD_ROCKET:				// turret type 1
				message = "ate the rocket from";
				message2 = "rocket turret";
				break;

			case MOD_R_SPLASH:				// turret type 1
				message = "almost dodged the rocket from";
				message2 = "rocket turret";
				break;

			case MOD_RAILGUN:				// turret type 2
				message = (random()<0.5)?"was railed by":"played slug-magnet with";
				message2 = "railgun turret";
				break;

			case MOD_MACHINEGUN:			// turret type 3
				message = "was mown down by";
				message2 = "machinegun turret";
				break;

			case MOD_SPIKE:					// turret type 4
				message = "was spiked by";
				message2 = "ESG turret";
				break;

			case MOD_SPIKE_SPLASH:			// turret type 4
				message = "was exploded by a spike from";
				message2 = "ESG turret";
				break;

			case MOD_SR_DISINT:				// turret type 5
				message = (random()<0.5)?"was disintegrated by":"was turned into a red mist by";
				message2 = "shockbolt turret";
				break;

			case MOD_SR_DISINT_WAVE:		// turret type 5
				message = (random()<0.5)?"was rocked by":"was spread like paste by";
				message2 = "shockbolt turret";
				break;

			case MOD_PLASMA:				// turret type 6
				message = (random()<0.5)?"was melted by":"was blasted into oblivion by";
				message2 = "plasma turret";
				break;
		}

		if (message)
		{
			Com_sprintf(killer, sizeof(killer), "an auto-tracking");
			if (self->noise_index)
				Com_sprintf(killer, sizeof(killer), "their very own");
			else
			{
				for (i = 0, player = g_edicts + 1; i < (int)maxclients->value; ++i, ++player)
				{
					if (player->client && (player->client->spycam == attacker))
					{
						Com_sprintf(killer, sizeof(killer), "%s's", player->client->pers.netname);
						if (ff)
							player->client->resp.score--;
						else if (!self->noise_index)
							player->client->resp.score++;

						break;
					}
				}
			}

			gi_bprintf(PRINT_MEDIUM,"%s %s %s %s\n", self->client->pers.netname, message, killer, message2);

			if (ff)
			{	// death by friendly fire
				if (self->client->resp.ctf_team == CTF_TEAM1)
					--teamgame.frags1;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					--teamgame.frags2;
			}
			else if (self->noise_index)
			{	// shot own self with our remote-controlled turret
				self->client->resp.score--;

				if (self->client->resp.ctf_team == CTF_TEAM1)
					--teamgame.frags1;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					--teamgame.frags2;
			}
			else
			{	// death by enemy turret
				if (self->client->resp.ctf_team == CTF_TEAM1)
					++teamgame.frags2;
				if (self->client->resp.ctf_team == CTF_TEAM2)
					++teamgame.frags1;
			}

			return;
		}
	}
//CW--...

	gi_bprintf(PRINT_MEDIUM,"%s died\n", self->client->pers.netname);
	self->client->resp.score--;

//CW++
	if (self->client->resp.ctf_team == CTF_TEAM1)
		--teamgame.frags1;
	if (self->client->resp.ctf_team == CTF_TEAM2)
		--teamgame.frags2;
//CW--
}

void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon(edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	item = self->client->pers.weapon;
	if (!self->client->pers.inventory[self->client->ammo_index])
		item = NULL;

//CW++
	if (item && ((item->weapmodel == WEAP_CHAINSAW) || (item->weapmodel == WEAP_DESERTEAGLE)))
		item = NULL;
//CW--

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = ((self->client->quad_framenum > (level.framenum + 10))) ? true : false;				//CW

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item(self, item); 
		self->client->v_angle[YAW] += spread;
//CW++
		if (drop != NULL)
//CW--
			drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
//CW++
		drop = Drop_Item(self, FindItem("Quad Damage"));
		if (drop != NULL)
		{
			drop->classname = "item_quad";
//CW--
			drop->spawnflags |= DROPPED_PLAYER_ITEM;

			drop->touch = Touch_Item;
			drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
			drop->think = G_FreeEdict;
		}
		self->client->v_angle[YAW] -= spread;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t dir;

//Maj++
	if (self->isabot)
		return;
//Maj--

	if (attacker && (attacker != world) && (attacker != self))
		VectorSubtract(attacker->s.origin, self->s.origin, dir);
	else if (inflictor && (inflictor != world) && (inflictor != self))
		VectorSubtract(inflictor->s.origin, self->s.origin, dir);
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = RAD2DEG(atan2(dir[1], dir[0]));
	else
	{
		self->client->killer_yaw = 0.0;
		if (dir[1] > 0.0)
			self->client->killer_yaw = 90.0;
		else if (dir[1] < 0.0)
			self->client->killer_yaw = -90.0;
	}

	if (self->client->killer_yaw < 0.0)
		self->client->killer_yaw += 360.0;
}

//CW++
/*
==================
FindLeader
==================
*/
edict_t *FindLeader(void)
{
	edict_t	*ent;
	edict_t	*ent_lead = NULL;
	int		max_score = -999999;
	int		n_clients = 0;
	int		i;

//	Search through client entities to determine who has the highest score.

	for (i = 1; i <= game.maxclients; ++i)
	{
		ent = &g_edicts[i];
		if (!ent->client)
			continue;
		if (!ent->inuse)
			continue;
		if (ent->health < 1)
			continue;

		++n_clients;
		if (n_clients == 1)
		{
			ent_lead = ent;
			max_score = ent->client->resp.score;
		}
		
		if (ent->client->resp.score > max_score)
		{
			ent_lead = ent;
			max_score = ent->client->resp.score;
		}
	}

//	In the case of there being "first equal" players, and one of them is the currently indicated
//	leader, make sure they retain the title (otherwise, the first player in the list is to get it).

	if ((ent_lead != NULL) && (level.leader != NULL))
	{
		if ((ent_lead->client->resp.score == level.lead_score) && (ent_lead != level.leader))
			ent_lead = level.leader;
	}

	return ent_lead;
}
//CW--

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//CW++
	edict_t	*ent;
	edict_t	*ent_lead = NULL;
//CW--

	int		n;

//DH++
	if (self->client->spycam)
		camera_off(self);

	if (self->turret)
		turret_disengage(self->turret);
//DH--

//CW++
	self->client->agm_charge = 0;
	self->client->agm_showcharge = false;
	self->client->agm_tripped = false;
	self->client->agm_on = false;
	self->client->agm_push = false;
	self->client->agm_pull = false;
	self->client->held_by_agm = false;
	self->client->flung_by_agm = false;
	self->client->thrown_by_agm = false;

	if (self->client->agm_target != NULL)
	{
		self->client->agm_target->client->held_by_agm = false;
		self->client->agm_target->client->flung_by_agm = false;
		self->client->agm_target->client->thrown_by_agm = true;
		self->client->agm_target = NULL;
	}
//CW--

	VectorClear(self->avelocity);
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked ctf flag								//ZOID++
	self->s.modelindex4 = 0;	// remove linked leader indicator						//CW++

	self->s.angles[0] = 0.0;
	self->s.angles[2] = 0.0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8.0;

//CW++
//	If we've been trap-tractored to death, destroy the trap that killed us and
//	any others that are currently trying to.

	if (self->tractored)
	{
		self->tractored = false;
		for (n = 0; n < globals.num_edicts; ++n)
		{
			ent = &g_edicts[n];
			if (!ent->inuse)
				continue;
			if (ent->client)
				continue;
			if (!ent->die)
				continue;

			if ((ent->die == Trap_DieFromDamage) && (ent->enemy == self))
			{
				ent->think = Trap_Die;
				ent->nextthink = level.time + FRAMETIME;
			}
		}
	}
//CW--

	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;

//Maj++
		if (self->isabot)
			self->client->respawn_time += rand() % 5;												//CW
//Maj--

		LookAtKiller(self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary(self, inflictor, attacker);

//ZOID++
		// if at start and same team, clear
		if (((int)sv_gametype->value > G_FFA) && (meansOfDeath == MOD_TELEFRAG) && (self->client->resp.ctf_state < 2) && //CW
			(self->client->resp.ctf_team == attacker->client->resp.ctf_team))
		{
//CW++
			if (self->client->resp.ctf_team == CTF_TEAM1)
				--teamgame.frags2;
			if (self->client->resp.ctf_team == CTF_TEAM2)
				--teamgame.frags1;
//CW--
			attacker->client->resp.score--;
			self->client->resp.ctf_state = 0;
		}

//CW++
//		Fix scores for team-change "suicide".

		if (((sv_gametype->value == G_TDM) || (sv_gametype->value == G_ASLT)) && self->client->mod_changeteam)
		{
			if (self->client->resp.ctf_team == CTF_TEAM1)
			{
				++teamgame.frags1;
				teamgame.frags2 -= (self->client->resp.score + 1);
			}
			else if (self->client->resp.ctf_team == CTF_TEAM2)
			{
				++teamgame.frags2;
				teamgame.frags1 -= (self->client->resp.score + 1);
			}
		}
//CW--

		CTFFragBonuses(self, inflictor, attacker);
//ZOID--

//CW++
//		Update the score leader status as necessary.

		if (sv_gametype->value == G_FFA)
		{
			if (level.leader == self)		// fragged by a player
			{
				level.lead_score = self->client->resp.score;
				if (attacker->client && (attacker != self))
				{
					if (attacker->client->resp.score > self->client->resp.score)
					{
						level.leader = attacker;
						level.lead_score = attacker->client->resp.score;
						if ((int)sv_show_leader->value)
							attacker->s.modelindex4 = gi.modelindex("models/halo/tris.md2");
					}
				}
				else						// fragged by environment or suicide
				{
					if ((ent_lead = FindLeader()) != NULL)
					{
						level.leader = ent_lead;
						level.lead_score = ent_lead->client->resp.score;
						if (ent_lead != self)
						{
							if ((int)sv_show_leader->value)
								ent_lead->s.modelindex4 = gi.modelindex("models/halo/tris.md2");
						}
					}
				}
			}
			else
			{
				if (attacker->client && (attacker->client->resp.score > level.lead_score))
				{
					level.leader->s.modelindex4 = 0;
					if ((int)sv_show_leader->value)
						attacker->s.modelindex4 = gi.modelindex("models/halo/tris.md2");

					level.leader = attacker;
					level.lead_score = attacker->client->resp.score;
				}
			}
		}
//CW--

		TossClientWeapon(self);

//ZOID++
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
//ZOID--

		if (!self->client->showscores)																//CW
			Cmd_Score_f(self);		// show scores													//
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~(FL_POWER_SHIELD|FL_POWER_SCREEN);

//CW++
	self->client->show_gausscharge = false;
	self->client->show_gausstarget = 0;
	self->client->gauss_dmg = 0;
	self->client->gauss_framenum = 0;

	self->client->antibeam_framenum = 0;
	self->client->frozen_framenum = 0;
	self->client->siphon_framenum = 0;
	self->client->needle_framenum = 0;
	self->client->haste_framenum = 0;
	self->client->mod_changeteam = false;

//	Force gibbing for death by chainsaw and disc-launcher ... MUAHAHAHA!

	if (attacker->client && (attacker != self) && (attacker->health > 0))
	{
		int	attack_weapon = attacker->client->pers.weapon->weapmodel;

		if (attack_weapon == WEAP_CHAINSAW)
		{
			float r;

			self->health = -41;
			r = random();
			if (r < 0.8)
				gi.sound(self, CHAN_VOICE, gi.soundindex("voice/s_humil.wav"), 1, ATTN_NORM, 0);
			else if (r < 0.9)
				gi.sound(self, CHAN_VOICE, gi.soundindex("voice/s_bleed.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(self, CHAN_VOICE, gi.soundindex("voice/s_wound.wav"), 1, ATTN_NORM, 0);
		}
		else if (attack_weapon == WEAP_DISCLAUNCHER)
			self->health = -41;
	}
//CW--

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (self->health < -40)
	{	// gib
		gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);

//CW++
//		For disintegrations, do a fountain of blood; otherwise, throw gibs.

		if (self->burning)
		{
			self->burning = false;
			if (self->flame)				// sanity check
			{
				self->flame->touch = NULL;
				self->flame->think = Flame_Expire;
				self->flame->nextthink = level.time + FRAMETIME;
			}
		}

		if (self->disintegrated)
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_MOREBLOOD);
			gi.WritePosition(self->s.origin);
			gi.WriteDir(vec3_up);
			gi.multicast(self->s.origin, MULTICAST_PVS);
		}
		else
//CW--
		{
			for (n = 0; n < 4; ++n)
				ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC, 0.0);
		}

		ThrowClientHead(self, damage);

//ZOID++
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID--

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i + 1) % 3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
				case 0:
					self->s.frame = FRAME_death101-1;
					self->client->anim_end = FRAME_death106;
					break;

				case 1:
					self->s.frame = FRAME_death201-1;
					self->client->anim_end = FRAME_death206;
					break;

				case 2:
					self->s.frame = FRAME_death301-1;
					self->client->anim_end = FRAME_death308;
					break;
			}
			gi.sound(self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

//Pon++
//	Routing last index move.

	if ((int)chedit->value && (self == &g_edicts[1]))
		Move_LastRouteIndex();
//Pon--

//CW++
//	Clear any noise entities we own.

	if (self->mynoise)
	{
		G_FreeEdict(self->mynoise);
		self->mynoise = NULL;
	}

	if (self->mynoise2)
	{
		G_FreeEdict(self->mynoise2);
		self->mynoise2 = NULL;
	}
//CW--

	gi.linkentity(self);
}

//=======================================================================

//CW++
/*
==================
GiveClientItem
==================
*/
gitem_t *GiveClientItem(gclient_t *client, char *pickup_name, int num)
{
	gitem_t *item;

	item = FindItem(pickup_name);
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = num;

	return item;
}

/*
==================
SetClientWeapon
==================
*/
gitem_t *SetClientWeapon(gclient_t *client, char *pickup_name)
{
	gitem_t *weapon;

	weapon = FindItem(pickup_name);
	client->pers.weapon = weapon;
	client->pers.lastweapon = weapon;

	return weapon;
}
//CW--

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant(gclient_t *client)
{
	gitem_t		*item;
//CW++
	char		ip[22];
	qboolean	op_status;
	qboolean	muted;
	qboolean	have_weap = false;
	qboolean	use_weap = false;
	int			get_weap = WINV_DESERTEAGLE;
	int			atype;
	int			weap_note;
	int			botindex;

	Com_sprintf(ip, sizeof(ip), "%s", client->pers.ip);
	op_status = client->pers.op_status;
	muted = client->pers.muted;
	weap_note = client->pers.weap_note;
	botindex = client->pers.botindex;
//CW--

	memset(&client->pers, 0, sizeof(client->pers));

//CW++
	GiveClientItem(client, "Chainsaw", 1);

	if ((int)sv_have_traps->value)
	{
		item = GiveClientItem(client, "Traps", (int)sv_initial_traps->value);
		have_weap = true;
		get_weap = WINV_TRAPS;

		if ((int)sv_initial_weapon->value == WINV_TRAPS)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_deserteagle->value)
	{
		item = GiveClientItem(client, "Desert Eagle", 1);
		have_weap = true;
		get_weap = WINV_DESERTEAGLE;

		if ((int)sv_initial_weapon->value == WINV_DESERTEAGLE)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_agm->value)
	{
		item = GiveClientItem(client, "AG Manipulator", 1);
		have_weap = true;
		get_weap = WINV_AGM;

		if ((int)sv_initial_weapon->value == WINV_AGM)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_c4->value)
	{
		item = GiveClientItem(client, "C4", (int)sv_initial_c4->value);
		have_weap = true;
		get_weap = WINV_C4;

		if ((int)sv_initial_weapon->value == WINV_C4)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_flamethrower->value)
	{
		item = GiveClientItem(client, "Flamethrower", 1);
		have_weap = true;
		get_weap = WINV_FLAMETHROWER;

		if ((int)sv_initial_weapon->value == WINV_FLAMETHROWER)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_gausspistol->value)
	{
		item = GiveClientItem(client, "Gauss Pistol", 1);
		have_weap = true;
		get_weap = WINV_GAUSSPISTOL;

		if ((int)sv_initial_weapon->value == WINV_GAUSSPISTOL)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_mac10->value)
	{
		item = GiveClientItem(client, "Mac-10", 1);
		have_weap = true;
		get_weap = WINV_MAC10;

		if ((int)sv_initial_weapon->value == WINV_MAC10)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_jackhammer->value)
	{
		item = GiveClientItem(client, "Jackhammer", 1);
		have_weap = true;
		get_weap = WINV_JACKHAMMER;

		if ((int)sv_initial_weapon->value == WINV_JACKHAMMER)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_disclauncher->value)
	{
		item = GiveClientItem(client, "Disc Launcher", 1);
		have_weap = true;
		get_weap = WINV_DISCLAUNCHER;

		if ((int)sv_initial_weapon->value == WINV_DISCLAUNCHER)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_spikegun->value)
	{
		item = GiveClientItem(client, "E.S.G.", 1);
		have_weap = true;
		get_weap = WINV_ESG;

		if ((int)sv_initial_weapon->value == WINV_ESG)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_rocketlauncher->value)
	{
		item = GiveClientItem(client, "Rocket Launcher", 1);
		have_weap = true;
		get_weap = WINV_ROCKETLAUNCHER;

		if ((int)sv_initial_weapon->value == WINV_ROCKETLAUNCHER)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_railgun->value)
	{
		item = GiveClientItem(client, "Railgun", 1);
		have_weap = true;
		get_weap = WINV_RAILGUN;

		if ((int)sv_initial_weapon->value == WINV_RAILGUN)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if ((int)sv_have_shockrifle->value)
	{
		item = GiveClientItem(client, "Shock Rifle", 1);
		have_weap = true;
		get_weap = WINV_SHOCKRIFLE;

		if ((int)sv_initial_weapon->value == WINV_SHOCKRIFLE)
		{
			client->pers.weapon = item;
			client->pers.lastweapon = item;
			use_weap = true;
		}
	}

	if (!have_weap)
	{
		gi.cvar_forceset("initial_weapon", va("%d", WINV_CHAINSAW));
		SetClientWeapon(client, "Chainsaw");
		get_weap = WINV_CHAINSAW;
		use_weap = true;
	}

	if (!use_weap)
	{
		SetClientWeapon(client, "Chainsaw");

		switch (get_weap)
		{
			case WINV_DESERTEAGLE:
				if ((int)sv_initial_bullets->value)
					SetClientWeapon(client, "Desert Eagle");

				break;

			case WINV_GAUSSPISTOL:
				if ((int)sv_initial_cells->value)
					SetClientWeapon(client, "Gauss Pistol");

				break;

			case WINV_MAC10:
				if ((int)sv_initial_bullets->value)
					SetClientWeapon(client, "Mac-10");

				break;

			case WINV_JACKHAMMER:
				if ((int)sv_initial_shells->value)
					SetClientWeapon(client, "Jackhammer");

				break;

			case WINV_C4:
				if ((int)sv_initial_c4->value)
					SetClientWeapon(client, "C4");

				break;

			case WINV_TRAPS:
				if ((int)sv_initial_traps->value)
					SetClientWeapon(client, "Traps");

				break;

			case WINV_ESG:
				if ((int)sv_initial_rockets->value)
					SetClientWeapon(client, "E.S.G.");

				break;

			case WINV_ROCKETLAUNCHER:
				if ((int)sv_initial_rockets->value)
					SetClientWeapon(client, "Rocket Launcher");

				break;

			case WINV_FLAMETHROWER:
				if ((int)sv_initial_cells->value)
					 SetClientWeapon(client, "Flamethrower");

				break;

			case WINV_RAILGUN:
				if ((int)sv_initial_slugs->value)
					SetClientWeapon(client, "Railgun");

				break;

			case WINV_SHOCKRIFLE:
				if ((int)sv_initial_cells->value)
					SetClientWeapon(client, "Shock Rifle");

				break;

			case WINV_AGM:
				if ((int)sv_initial_cells->value)
					SetClientWeapon(client, "AG Manipulator");

				break;

			case WINV_DISCLAUNCHER:
				if ((int)sv_initial_rockets->value)
					SetClientWeapon(client, "Disc Launcher");

				break;

			case WINV_CHAINSAW:
			default:
				SetClientWeapon(client, "Chainsaw");
				break;
		}
	}
//CW--

//ZOID++
	if ((int)sv_allow_hook->value && !level.nohook)													//CW
	{
		item = FindItem("Grapple");

//CW++
//		Setup off-hand grapple.

		if ((int)sv_hook_offhand->value)
		{
			item->use = NULL;
			item->view_model = '\0';
			item->icon = '\0';
			client->hookstate = WEAPON_READY;
		}
//CW--

		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
//ZOID--

//CW++
//	Default starting ammo.

	if ((int)sv_initial_bullets->value)
	{
		item = FindItem("bullets");
		client->pers.inventory[ITEM_INDEX(item)] = (int)sv_initial_bullets->value;
	}

	if ((int)sv_initial_shells->value)
	{
		item = FindItem("shells");
		client->pers.inventory[ITEM_INDEX(item)] = (int)sv_initial_shells->value;
	}

	if ((int)sv_initial_rockets->value)
	{
		item = FindItem("rockets");
		client->pers.inventory[ITEM_INDEX(item)] = (int)sv_initial_rockets->value;
	}

	if ((int)sv_initial_cells->value)
	{
		item = FindItem("cells");
		client->pers.inventory[ITEM_INDEX(item)] = (int)sv_initial_cells->value;
	}

	if ((int)sv_initial_slugs->value)
	{
		item = FindItem("slugs");
		client->pers.inventory[ITEM_INDEX(item)] = (int)sv_initial_slugs->value;
	}
//CW--

	client->pers.health			= (int)sv_health_initial->value;									//CW...
	client->pers.max_health		= (int)sv_health_max->value;

	client->pers.max_bullets	= (int)sv_max_bullets->value;										//CW...
	client->pers.max_shells		= (int)sv_max_shells->value;
	client->pers.max_rockets	= (int)sv_max_rockets->value;
	client->pers.max_c4			= (int)sv_max_c4->value;
	client->pers.max_cells		= (int)sv_max_cells->value;
	client->pers.max_slugs		= (int)sv_max_slugs->value;

//CW++
	client->pers.max_traps		= (int)sv_max_traps->value;

	client->pers.op_status = op_status;
	client->pers.muted = muted;
	Com_sprintf(client->pers.ip, sizeof(client->pers.ip), "%s", ip);
	client->pers.weap_note = weap_note;
	client->pers.botindex = botindex;
//CW--

//CW++
//	Default starting armour.

	if ((int)sv_initial_armor->value > 0)
	{
		if ((int)sv_initial_armortype->value == 0)
			atype = jacket_armor_index;
		else if ((int)sv_initial_armortype->value == 1)
			atype = combat_armor_index;
		else
			atype = body_armor_index;

		client->pers.inventory[atype] = (int)sv_initial_armor->value;
	}
//CW--

	client->pers.connected = true;

	// Knightmare- custom client colors
	Vector4Set (client->pers.color1, 255, 255, 255, 0);
	Vector4Set (client->pers.color2, 255, 255, 255, 0);

//DH++
	client->spycam = NULL;
//DH--
}


void InitClientResp(gclient_t *client)
{
//ZOID++
	int			ctf_team = client->resp.ctf_team;
	qboolean	id_state = client->resp.id_state;
//ZOID--

	qboolean	id_trap = client->resp.id_trap;														//CW++

	memset(&client->resp, 0, sizeof(client->resp));
	
//ZOID++
	client->resp.ctf_team = ctf_team;
	client->resp.id_state = id_state;
//ZOID--

	client->resp.id_trap = id_trap;																	//CW++
	client->resp.enterframe = level.framenum;														//CW

//ZOID++
	if (client->resp.ctf_team < CTF_TEAM1)															//CW
		CTFAssignTeam(client);
//ZOID--
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData(void)
{
	int		i;
	edict_t	*ent;

	for (i = 0; i < (int)game.maxclients; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;

		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_SHIELD|FL_POWER_SCREEN));	//CW
	}
}

void FetchClientEntData(edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;														//CW
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float PlayersRangeFromSpot(edict_t *spot)
{
	edict_t	*player;
	vec3_t	v;
	float	bestplayerdistance;
	float	playerdistance;
	int		n;

	bestplayerdistance = 9999999;

	for (n = 1; n <= (int)maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (player->health < 1)
			continue;

		VectorSubtract(spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength(v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

//CW++
/*
================
TeamPlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
who is on the same team as the specified team.
================
*/
float TeamPlayersRangeFromSpot(edict_t *spot, ctfteam_t team)
{
	edict_t	*player;
	vec3_t	v;
	float	bestplayerdistance;
	float	playerdistance;
	int		n;

	bestplayerdistance = 9999999;

	for (n = 1; n <= (int)maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (player->health < 1)
			continue;
		if (player->client->resp.ctf_team != team)
			continue;

		VectorSubtract(spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength(v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}
//CW--

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot;
	edict_t	*spot1;
	edict_t	*spot2;
	float	range;
	float	range1;
	float	range2;
	int		count = 0;
	int		selection;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");
		if ((spot == spot1) || (spot == spot2))
			selection++;
	} while (selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint(void)
{
	edict_t	*bestspot;
	edict_t	*spot;
	float	bestdistance;
	float	bestplayerdistance;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot(spot);
		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
		return bestspot;

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint(void)
{
	if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint();
	else
		return SelectRandomDeathmatchSpawnPoint();
}


/*
===========
SelectSpawnPoint

Chooses a player start position.
============
*/
void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

//ZOID++
	if (sv_gametype->value == G_CTF)
		spot = SelectCTFSpawnPoint(ent, false);														//CW
//ZOID--

//CW++
	else if (sv_gametype->value == G_ASLT)
		spot = SelectASLTSpawnPoint(ent);
//CW--
	else
		spot = SelectDeathmatchSpawnPoint();

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find(spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy(spot->s.origin, origin);
	origin[2] += 9.0;

//Maj++
	if (ent->isabot)
		origin[2] += 21.0; 
//Maj--

	VectorCopy(spot->s.angles, angles);
}

//======================================================================

void InitBodyQue(void)
{
	edict_t	*ent;
	int		i;

	level.body_que = 0;
	for (i = 0; i < BODY_QUEUE_SIZE; ++i)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound(self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 4; ++n)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC, 0.0);

		self->s.origin[2] -= 48.0;
		ThrowClientHead(self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue(edict_t *ent)
{
	edict_t *body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	gi.unlinkentity(ent);

	gi.unlinkentity(body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

//CW++
	body->s.event = EV_OTHER_TELEPORT;
//CW--

	body->svflags = ent->svflags;
	VectorCopy(ent->mins, body->mins);
	VectorCopy(ent->maxs, body->maxs);
	VectorCopy(ent->absmin, body->absmin);
	VectorCopy(ent->absmax, body->absmax);
	VectorCopy(ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

//CW++
	body->s.modelindex4 = 0;
//CW--

	gi.linkentity(body);
}


void Respawn(edict_t *self)
{
	if (self->movetype != MOVETYPE_NOCLIP)
		CopyToBodyQue(self);

	self->svflags &= ~SVF_NOCLIENT;

//Maj++
	if (self->isabot)
		return;
//Maj--

	PutClientInServer(self);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;
}

//==============================================================

//CW++
void player_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*agm_enemy;
	float		speed;
	int			damage;

//	Sanity checks.

	if (!other->client)
		return;
	if (!other->inuse)
		return;
	if (other->client->agm_enemy == NULL)
		return;

	agm_enemy = other->client->agm_enemy;
	if (agm_enemy == self)
		return;
	if (CheckTeamDamage(self, agm_enemy))
		return;

//	Check for collision with other players when flung/thrown/smashed by an AGM.

	speed = VectorLength(other->client->oldvelocity);
	if (speed > 400.0)
	{
		damage = (int)(0.1 * (speed - 400.0));
		if (damage < 1)
			damage = 1;
		if (agm_enemy->client->quad_framenum > level.framenum)
			damage *= (int)sv_quad_factor->value;

		T_Damage(self, other, agm_enemy, other->client->oldvelocity, self->s.origin, vec3_origin, damage, damage, 0, MOD_AGM_HIT);
		T_Damage(other, self, agm_enemy, vec3_origin, other->s.origin, vec3_origin, damage, 0, 0, MOD_AGM_HIT);
	}
}
//CW--

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer(edict_t *ent)
{
	client_persistant_t	saved;
	client_respawn_t	resp;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	vec3_t		mins = {-16.0, -16.0, -24.0};
	vec3_t		maxs = {16.0, 16.0, 32.0};
	vec3_t		spawn_origin;
	vec3_t		spawn_angles;
	int			index;
	int			i;

//	Find a spawn point. Do it before setting health back up, so the farthest
//	ranging doesn't count this client.

	SelectSpawnPoint(ent, spawn_origin, spawn_angles);

	index = ent - g_edicts - 1;
	client = ent->client;

//	Most client data is wiped every spawn.
	
	resp = client->resp;																			//CW
	memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant (client);
	ClientUserinfoChanged (ent, userinfo);

//	Clear everything but the persistant data.

	saved = client->pers;
	memset(client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant (client);

	client->resp = resp;

//	Copy some data from the client to the entity.

	FetchClientEntData(ent);

//	Clear entity values.

	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = (ent->isabot)?"AwakenBot":"player";											//CW
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12.0;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

//CW++
	ent->svflags &= ~SVF_NOCLIENT;
	ent->touch = player_touch;
	ent->client->chase_target = NULL;

	ent->burning = false;
	ent->disintegrated = false;
	ent->gravity = 1.0;
//CW--

//DH++
	ent->client->spycam = NULL;
	ent->client->camplayer = NULL;
//DH--

	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);
	VectorClear(ent->velocity);

//	Clear playerstate values.

	memset(&ent->client->ps, 0, sizeof(client->ps));
	client->ps.pmove.origin[0] = spawn_origin[0] * 8;
	client->ps.pmove.origin[1] = spawn_origin[1] * 8;
	client->ps.pmove.origin[2] = spawn_origin[2] * 8;

//ZOID++
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID--

	if ((int)dmflags->value & DF_FIXED_FOV)															//CW
		client->ps.fov = 90.0;
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1.0)
			client->ps.fov = 90.0;
		else if (client->ps.fov > 160.0)
			client->ps.fov = 160.0;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

//	Clear entity state values.

	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = (MAX_MODELS-1);				// will use the skin specified model	// was 255
	ent->s.modelindex2 = (MAX_MODELS-1);			// custom gun model						// was 255
	ent->s.skinnum = ent - g_edicts - 1;	// sknum is player num and weapon number
											// weapon number will be added in changeweapon

	ent->s.frame = 0;
	VectorCopy(spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1.0;				// make sure off ground
	VectorCopy(ent->s.origin, ent->s.old_origin);

//	Set the delta angle.

	for (i = 0; i < 3; ++i)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0.0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0.0;
	VectorCopy(ent->s.angles, client->ps.viewangles);
	VectorCopy(ent->s.angles, client->v_angle);

//Maj++
	SetBotThink(ent);
//Maj--

//ZOID++
	if (CTFStartClient(ent))
		return;
//ZOID--

	if (!KillBox(ent))
	{	// could't spawn in?
	}

	gi.linkentity(ent);

//CW++
//	Force-set client aliases for off-hand grapple and AGM commands.
//	(Moved after CTFStartClient() call to prevent "Info string..." errors).

	if (!ent->isabot && !ent->client->resp.aliases_set)
	{
		StuffCmd(ent, "alias +hook cmd hook\n");
		StuffCmd(ent, "alias -hook cmd unhook\n");

		StuffCmd(ent, "alias +push cmd push\n");
		StuffCmd(ent, "alias -push cmd unpush\n");
		StuffCmd(ent, "alias +pull cmd pull\n");
		StuffCmd(ent, "alias -pull cmd unpull\n");

		ent->client->resp.aliases_set = true;
	}

//	Set respawn invulnerability, if appropriate.

	client->invincible_framenum = level.framenum + (int)(10.0 * sv_respawn_invuln_time->value);

//	In the game, so not a spectator.
	
	client->spectator = false;
//CW--

//	Force the current weapon up.

	client->newweapon = client->pers.weapon;
	ChangeWeapon(ent);

//CW++	Determine if we should have the leader indicator.

	if (sv_gametype->value == G_FFA)
	{
		if ((level.leader == NULL) || (level.leader == ent))
		{
			level.leader = ent;
			level.lead_score = ent->client->resp.score;
			if ((int)sv_show_leader->value)
				ent->s.modelindex4 = gi.modelindex("models/halo/tris.md2");
		}
		else if (ent->client->resp.score > level.lead_score)	// just joined, and everyone else has -ve frags
		{
			level.leader->s.modelindex4 = 0;
			level.leader = ent;
			level.lead_score = ent->client->resp.score;
			if ((int)sv_show_leader->value)
				ent->s.modelindex4 = gi.modelindex("models/halo/tris.md2");
		}
	}
//CW--
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch(edict_t *ent)
{
	G_InitEdict(ent);
	InitClientResp(ent->client);

	// locate ent at a spawn point
	 PutClientInServer(ent);

	if (level.intermissiontime)
		MoveClientToIntermission(ent);
	else
	{
		// send effect
//CW++
		if (!ent->client->spectator)
		{
//CW--
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(ent-g_edicts);
			gi.WriteByte(MZ_LOGIN);
			gi.multicast(ent->s.origin, MULTICAST_PVS);
		}
	}

	gi_bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame(ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin(edict_t *ent)
{
//CW++
	edict_t	*timer;
//CW--

	ent->client = game.clients + (ent - g_edicts - 1);
	ClientBeginDeathmatch(ent);																		//CW

//CW++
	if (ent->isabot)
		return;

	timer = G_Spawn();
	timer->svflags |= SVF_NOCLIENT;
	timer->target_ent = ent;
	timer->message = "exec autoawaken.cfg\n";
	timer->think = StuffCmd_Ent;
	timer->nextthink = level.time + 1.0;
//CW--
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		Com_strcpy(userinfo, MAX_INFO_STRING, "\\name\\badinfo\\skin\\male/grunt");	// userinfo is always length of MAX_INFO_STRING 
	}

	// set name
	s = Info_ValueForKey(userinfo, "name");
	strncpy(ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey(userinfo, "skin");
	playernum = ent - g_edicts - 1;

	// combine name and skin into a configstring
//ZOID++
	if ((int)sv_gametype->value > G_FFA)															//CW
		CTFAssignSkin(ent, s);
	else
//ZOID--
	{
//CW++
		// prevent potential configstring overflow (netname+s)
		if (strlen(ent->client->pers.netname) + strlen(s) + 4 > MAX_SKINLEN - 1)					//r1,CW
		{
			gi_cprintf(ent, PRINT_HIGH, "Skin name is too long.\n");
			s = "male/grunt";
		}
//CW--
		gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s));
	}

//ZOID++
	// set player name field (used in id_state view)
	gi.configstring(CS_GENERAL+playernum, ent->client->pers.netname);
//ZOID--

	// fov
	if ((int)dmflags->value & DF_FIXED_FOV)															//CW
		ent->client->ps.fov = 90.0;
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1.0)
			ent->client->ps.fov = 90.0;
		else if (ent->client->ps.fov > 160.0)
			ent->client->ps.fov = 160.0;
	}

	// handedness
	s = Info_ValueForKey(userinfo, "hand");
	if (strlen(s))
		ent->client->pers.hand = atoi(s);

	// Knightmare- custom colors
	s = Info_ValueForKey (userinfo, "color1");
	if (strlen(s) >= 6) {
		if ( Com_ParseColorString (s, ent->client->pers.color1) )
			ent->client->pers.color1[3] = 255;	// mark as set
	}

	s = Info_ValueForKey (userinfo, "color2");
	if (strlen(s) >= 6) {
		if ( Com_ParseColorString (s, ent->client->pers.color2) )
			ent->client->pers.color2[3] = 255;	// mark as set
	}

//CW++
	if (strlen(ent->client->pers.old_name) == 0)
		strncpy(ent->client->pers.old_name, ent->client->pers.netname, sizeof(ent->client->pers.netname));
//CW--

	// save off the userinfo in case we want to check something later
	strncpy(ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect(edict_t *ent, char *userinfo)
{
	char		*value;

//CW++
	char		*tname;
	qboolean	cl_rsv = false;

	if (Info_Validate(userinfo))
		tname = Info_ValueForKey(userinfo, "name");													//r1: buffer overflow fix (tname)
	else
		tname =  "(Unknown)";

//	If the 'isabot' flag is set, the client is trying to connect into a slot that's occupied
//	by an AwakenBot, ie. there are no free slots left.

	if (ent->isabot)
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "All player slots are taken.");
		gi.dprintf("** %s : connection refused (no free slots).\n", tname);
		return false;
	}
//CW--

//	Check to see if they are on the banned IP list.

//SNX++ better way of getting IP
	value = strstr(userinfo,"\\ip\\");

	if (value == NULL)
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart Quake 2.");
		return false;
	}
	//ip will always be the final thing in the userinfo string
	value += 4;
//SNX--

	if (SV_FilterPacket(value))
	{
//CW++
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "You have been banned.");
		gi.dprintf("** %s [%s]: connection refused (banned).\n", tname, ent->client->pers.ip);
//CW--
		return false;
	}

//CW++
//	Reject clients with blank IP addresses if flagged to do so.

	if ((int)sv_reject_blank_ip->value && (strlen(value) == 0))
	{
//CW++
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "You have a blank IP address (forbidden).");
		gi.dprintf("** %s : connection refused (blank IP).\n", tname);
//CW--
		return false;
	}

//	Need to store their IP separately, because if the client's userinfo changes later (eg. they 
//	change their name), then their IP value is blanked.

	strncpy(ent->client->pers.ip, value, sizeof(ent->client->pers.ip));
//CW--

//	Check for a password, or a slot reserved password.

	value = Info_ValueForKey(userinfo, "password");
	if (*password->string && strcmp(password->string, "none"))										//CW
	{																								//
		if (strcmp(password->string, value))														//
		{
//CW++
			userinfo[0] = '\0';
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			gi.dprintf("** %s [%s]: connection refused (wrong password).\n", tname, ent->client->pers.ip);
			
			// prevent "ghost client" crashes
			ent->solid = SOLID_NOT;
			ent->inuse = false;
			ent->s.effects = 0;
			ent->s.event = 0;
			ent->s.modelindex = 0;
			ent->s.modelindex2 = 0;
			ent->s.modelindex3 = 0;
			ent->s.modelindex4 = 0;
			ent->s.sound = 0;
//CW--
			return false;
		}
	}

//CW++
//	If a server password isn't set, check to see if the "reserved slots" feature is being flagged 
//	for use by the server. If the Operator password is set, check to see if the player's password 
//	is set to it. If so, try to assign them to a reserved slot first (to keep the public ones free). 
//	If there are no free reserved slots, assign them to a public slot. Note that if all the public 
//	slots are full at this point, they won't even get passed into this function - they'll get a 
//	"server is full" message automatically (from their Q2 EXE).
//  NOTE: array setup = [<--sv_reserved-->|<--(maxclients - sv_reserved)-->]

	else if (((int)sv_reserved->value > 0) && *sv_rsv_password->string)
	{
		if (!strcmp(sv_rsv_password->string, value))
		{	// reserve password is correct (so try to get them into a reserved slot)
			if (g_reserve_used < (int)sv_reserved->value)
			{	// reserved slot
				g_slots[g_reserve_used] = ent;
				++g_reserve_used;
				cl_rsv = true;
			}
			else
			{
				if (g_public_used < (int)maxclients->value - (int)sv_reserved->value)
				{	// public slot
					g_slots[(int)sv_reserved->value + g_public_used] = ent;
					++g_public_used;
				}
				else
				{	// no free slots
					userinfo[0] = '\0';
					Info_SetValueForKey(userinfo, "rejmsg", "All player slots are taken.");
					gi.dprintf("** %s : connection refused (no free slots).\n", tname);
					return false;
				}
			}
		}
		else
		{	// can use public slots only
			if (g_public_used < (int)maxclients->value - (int)sv_reserved->value)
			{	// public slot
				g_slots[(int)sv_reserved->value + g_public_used] = ent;
				++g_public_used;
			}
			else
			{	// no free slots
				userinfo[0] = '\0';
				Info_SetValueForKey(userinfo, "rejmsg", "All player slots are taken.");
				gi.dprintf("** %s : connection refused (no free slots).\n", tname);
				return false;
			}
		}
	}
	else
	{
		// public slots only
		if (g_public_used < (int)maxclients->value)
		{	// public slot
			g_slots[g_public_used] = ent;
			++g_public_used;
		}
		else
		{	// no free slots
			userinfo[0] = '\0';
			Info_SetValueForKey(userinfo, "rejmsg", "All player slots are taken.");
			gi.dprintf("** %s : connection refused (no free slots).\n", tname);
			return false;
		}
	}
//CW--

//	They can connect.

	ent->client = game.clients + (ent - g_edicts - 1);

//	If there is already a body waiting for us (a loadgame), just take it,
//	otherwise spawn one from scratch.

	if (ent->inuse == false)
	{

//		Clear the respawning variables.

//ZOID++
//		Force team join.

		ent->client->resp.ctf_team = -1;
		ent->client->resp.id_state = false;															//CW
//ZOID--
		InitClientResp(ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant(ent->client);
	}

//CW++
//	We don't need to do the full ClientUserinfoChanged() function here, as this sends
//	config strings unnecessarily.

	if (!Info_Validate(userinfo)) {
		Com_strcpy(userinfo, MAX_INFO_STRING, "\\name\\badinfo\\skin\\male/grunt");	// userinfo is always length of MAX_INFO_STRING 
	}

	strncpy(ent->client->pers.netname, Info_ValueForKey(userinfo, "name"), sizeof(ent->client->pers.netname)-1);
	strncpy(ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
	if (strlen(ent->client->pers.old_name) == 0)
		strncpy(ent->client->pers.old_name, ent->client->pers.netname, sizeof(ent->client->pers.netname));
//CW--

	if (game.maxclients > 1)
		gi.dprintf("%s connected %s\n", ent->client->pers.netname, (cl_rsv)?"--> reserved slot":"");

	ent->client->pers.connected = true;

//CW++
	gi.dprintf(" (IP = %s)\n", ent->client->pers.ip);

	ent->client->resp.id_trap = true;		// default = on
	ent->client->pers.weap_note = 1;		// default = text only
//CW--

	return true;
}

//CW++
/*
===========
ClearSlot

Removes the specified player from the global
player slots array, and reorders the array.

array setup = [<--sv_reserved-->|<--(maxclients - sv_reserved)-->]
============
*/
void ClearSlot(edict_t *ent)
{
	int i;
	int n;

	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		if (ent == g_slots[i])
		{
			if (((int)sv_reserved->value > 0) && *sv_rsv_password->string)
			{	// reserved slots feature is in use
				if (i < (int)sv_reserved->value)
				{	// client has a reserved slot
					if (g_reserve_used > 1)
					{
						if (i == (int)sv_reserved->value - 1)
							g_slots[i] = NULL;
						else
						{
							for (n = i; n < g_reserve_used - 1; ++n)
								g_slots[n] = g_slots[n+1];

							g_slots[n] = NULL;
						}
					}
					else if (g_reserve_used == 1)
						g_slots[0] = NULL;
					else
					{	// should never happen
						gi.dprintf("BUG: g_reserve_used <= 0 in ClearSlot()\nPlease contact musashi@planetquake.com\n");
						return;
					}
					--g_reserve_used;
					break;
				}
				else
				{	//client has a public slot
					if (g_public_used > 1)
					{
						if (i == (int)maxclients->value - 1)
							g_slots[i] = NULL;
						else
						{
							for (n = i; n < (int)sv_reserved->value + g_public_used - 1; ++n)
								g_slots[n] = g_slots[n+1];

							g_slots[n] = NULL;
						}
					}
					else if (g_public_used == 1)
						g_slots[(int)sv_reserved->value] = NULL;
					else
					{	// should never happen
						gi.dprintf("BUG: g_public_used <= 0 in ClearSlot()\nPlease contact musashi@planetquake.com\n");
						return;
					}

					--g_public_used;
					break;
				}
			}
			else
			{	// public slots only (so client must have a public slot)
				if (g_public_used > 1)
				{
					if (i == (int)maxclients->value - 1)
						g_slots[i] = NULL;
					else
					{
						for (n = i; n < (int)sv_reserved->value + g_public_used - 1; ++n)
							g_slots[n] = g_slots[n+1];

						g_slots[n] = NULL;
					}
				}
				else if (g_public_used == 1)
					g_slots[(int)sv_reserved->value] = NULL;
				else
				{	// should never happen
					gi.dprintf("BUG: g_public_used <= 0 in ClearSlot()\nPlease contact musashi@planetquake.com\n");
					return;
				}

				--g_public_used;
				break;
			}
		}
	}
}
//CW--

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect(edict_t *ent)
{
//CW++
	edict_t		*index;
	edict_t		*check;
	qboolean	finished = false;
	int			i;
//CW--
	int			playernum;

	if (!ent->client)
		return;

//CW++
	if (ent->isabot)
		gi_bprintf(PRINT_HIGH, "%s was removed\n", ent->client->pers.netname);
	else
//CW--
		gi_bprintf(PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//CW++
	ent->s.sound = 0;
	ent->s.effects = 0;
	ent->s.event = 0;

	ClearSlot(ent);

//	Destroy any traps that are locked on to us with their tractor-beam.

	if (ent->tractored)
	{
		ent->tractored = false;
		for (i = 0; i < globals.num_edicts; ++i)
		{
			index = &g_edicts[i];
			if (!index->inuse)
				continue;
			if (index->client)
				continue;
			if (!index->die)
				continue;

			if ((index->die == Trap_DieFromDamage) && (index->enemy == ent))
			{
				index->think = Trap_Die;
				index->nextthink = level.time + FRAMETIME;
			}
		}
	}

	ent->disintegrated = false;
	ent->show_hostile = false;
	ent->client->resp.score = -999999;
	ent->client->resp.ctf_team = CTF_NOTEAM;

//	Snuff out flame if player is on fire.

	if (ent->burning)
	{
		ent->burning = false;
		if (ent->flame)					// sanity check
		{
			ent->flame->touch = NULL;
			Flame_Expire(ent->flame);
		}
	}

//	Search through the player's linked list of Trap and C4 entities (if any), and pop them.

	if (ent->next_node)
	{
		index = ent->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (check->die == C4_DieFromDamage)
				C4_Die(check);
			else if (check->die == Trap_DieFromDamage)
				Trap_Die(check);
			else
				gi.dprintf("BUG: Invalid next_node pointer in ClientDisconnect().\nPlease contact musashi@planetquake.com\n");
		}
	}
//CW--

//CW++
	ent->client->agm_on = false;
	ent->client->agm_pull = false;
	ent->client->agm_push = false;

	if (ent->client->agm_target != NULL)
	{
		ent->client->agm_target->client->held_by_agm = false;
		ent->client->agm_target->client->flung_by_agm = false;
		ent->client->agm_target = NULL;
	}
//CW--

//ZOID++
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID--

	// send effect
//CW++
	if (!ent->client->spectator)
	{
//CW--
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent-g_edicts);
		gi.WriteByte(MZ_LOGOUT);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	gi.unlinkentity(ent);
	ent->s.modelindex = 0;

//CW++
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;

	ent->client->pers.op_status = false;
	ent->client->pers.muted = false;

	ent->s.solid = 0;
//CW--

	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

//DH++
	if (ent->client->spycam)
		camera_off(ent);
//DH--

//CW++
	if (sv_gametype->value == G_FFA)
	{
		if (level.leader == ent)
		{
			level.leader = NULL;
			if ((check = FindLeader()) != NULL)
			{
				if ((int)sv_show_leader->value)
					check->s.modelindex4 = gi.modelindex("models/halo/tris.md2");

				level.leader = check;
				level.lead_score = check->client->resp.score;
			}
			else
				level.lead_score = 0;
		}
	}

	if (ent->client->chase_target)
	{
		ent->client->chase_target = NULL;
		ent->client->update_chase = false;
	}

//	If we're being watched by someone, deactivate their chase-cam.

	for (i = 0; i < game.maxclients; ++i)
	{
		check = g_edicts + 1 + i;
		if (!check->client)
			continue;
		if (!check->inuse)
			continue;
		if (check == ent)
			continue;

		if (check->client->chase_target && (check->client->chase_target == ent))
			ToggleChaseCam(check, NULL);
	}

	ent->svflags = 0;
	ent->flags = 0;
//CW--

	playernum = ent - g_edicts - 1;
	gi.configstring(CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace(start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace(start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock(void *b, int c)
{
	int	v;
	int	i;

	v = 0;
	for (i = 0; i < c; ++i)
		v += ((byte *)b)[i];

	return v;
}

void PrintPmove(pmove_t *pm)
{
	unsigned c1;
	unsigned c2;

	c1 = CheckBlock(&pm->s, sizeof(pm->s));
	c2 = CheckBlock(&pm->cmd, sizeof(pm->cmd));
	Com_Printf("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

//DH++
void ClientSpycam(edict_t *ent)
{
	gclient_t	*client = ent->client;
	edict_t		*camera = ent->client->spycam;
	pmove_t		pm;
	trace_t		tr;
	vec3_t		forward;
	vec3_t		left;
	vec3_t		up;
	vec3_t		dir;
	vec3_t		start;
	float		dist;
	int			i;

	memset(&pm, 0, sizeof(pm));
	if (client->ucmd.sidemove && (level.time > ent->last_move_time + 1.0))
	{
		if (camera->viewer == ent)
			camera->viewer = NULL;

		if (client->ucmd.sidemove > 0)
			camera = G_FindNextCamera(camera, client->monitor);
		else
			camera = G_FindPrevCamera(camera, client->monitor);
		
		if (camera)
		{
			if (!camera->viewer)
				camera->viewer = ent;

			client->spycam = camera;
			VectorAdd(camera->s.origin, camera->move_origin, ent->s.origin);
			if (camera->viewmessage)
				gi_centerprintf(ent,camera->viewmessage);

			ent->last_move_time = level.time;
		}
		else
			camera = client->spycam;
	}

	if (camera->enemy && (camera->enemy->deadflag || !camera->enemy->inuse))
		camera->enemy = NULL;

	AngleVectors(camera->s.angles, forward, left, up);
	client->ps.pmove.pm_type = PM_FREEZE;
	if (camera->viewer == ent)
	{
		if ((client->ucmd.buttons & BUTTON_ATTACK) && (camera->sounds > -1))						//CW
		{
			if (level.time + 0.001 >= camera->monsterinfo.attack_finished)							//CW: account for fp error
			{
				client->latched_buttons &= ~BUTTON_ATTACK;
				if (!Q_stricmp(camera->classname, "turret_breach")
				    || !Q_stricmp(camera->classname, "model_turret"))								//CW
				{
					if (camera->sounds == 3)														//CW
						camera->monsterinfo.attack_finished = level.time + FRAMETIME;				//CW
					else
						camera->monsterinfo.attack_finished = level.time + camera->wait;			//CW

					turret_breach_fire(camera);
//CW++
					if (ent->client->camplayer == NULL)		// player has shot themselves with the turret (well done, llama!)
						return;
//CW--
				}
			}
		}
	}

	VectorMA(camera->s.origin, camera->move_origin[0],forward,start);
	VectorMA(start,           -camera->move_origin[1],left,   start);
	VectorMA(start,            camera->move_origin[2],up,     start);
	
	tr = gi.trace(camera->s.origin, NULL, NULL, start, camera, MASK_SOLID);
	if (tr.fraction < 1.0)
	{
		VectorSubtract(tr.endpos, camera->s.origin, dir);
		dist = VectorNormalize(dir) - 2.0;
		if (dist < 0.0)
			dist = 0.0;
		VectorMA(camera->s.origin, dist, dir, start);
	}
	VectorCopy(start, ent->s.origin);
	VectorCopy(camera->velocity, ent->velocity);
	
	client->resp.cmd_angles[0] = SHORT2ANGLE(client->ucmd.angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(client->ucmd.angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(client->ucmd.angles[2]);
	
	memset(&pm, 0, sizeof(pm));
	pm.s = client->ps.pmove;
	for (i = 0; i < 3; i++)
	{
		pm.s.origin[i] = ent->s.origin[i] * 8;
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(client->ps.viewangles[i] - client->resp.cmd_angles[i]);
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		pm.snapinitial = true;

	pm.cmd = client->ucmd;
	pm.trace = PM_trace;					// adds default parms
	pm.pointcontents = gi.pointcontents;
	gi.Pmove(&pm);
	
	gi.linkentity(ent);
	G_TouchTriggers(ent); // we'll only allow touching trigger_look with "Cam Owner" SF
	
}
//DH--

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink(edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t		*other;
	pmove_t		pm;
	int			i;
	int			j;

//CW++
	qboolean	cam_user = false;
//CW--

//Pon++
	static	edict_t		*old_ground;
	static	qboolean	wasground;

	if ((int)chedit->value && (CurrentIndex < MAXNODES) && !ent->deadflag && (ent == &g_edicts[1]))
	{
		trace_t	rs_trace;
		vec3_t	min;
		vec3_t	max;
		vec3_t	v;
		vec3_t	vv;
		float	x;
		int		oldwaterstate;
		int		k;
		int		l;

		oldwaterstate = ent->client->waterstate;
		Get_WaterState(ent);
		i = false;
		l = GRS_NORMAL;

		if (CurrentIndex > 0)
			Get_RouteOrigin(CurrentIndex - 1, v);

		if (!Route[CurrentIndex].index)
		{
			VectorCopy(ent->s.origin, v);
			old_ground = ent->groundentity;
			if (ent->groundentity)
				i = true;
		}
		else if (!TraceX(ent, v))
		{
			VectorCopy(ent->s.old_origin, v);
			i = 3;
		}
		else if (ent->client->waterstate != oldwaterstate)
		{
			i = true;
			if (ent->groundentity)
			{
				if ((ent->groundentity->blocked == train_blocked) || (ent->groundentity->blocked == plat_blocked) || ((ent->groundentity->blocked == door_blocked) && (ent->groundentity->style != ENT_ID_DOOR_ROTATING)))	//CW
					i = false;
			}

			if (ent->client->waterstate > oldwaterstate)
				VectorCopy(ent->s.origin, v);
			else
				VectorCopy(ent->s.old_origin, v);
		}
		else if (fabs(v[2] - ent->s.origin[2]) > 20)
		{
			if (ent->groundentity && (ent->waterlevel < 2))
			{
				i = true;
				k = true;
				VectorCopy(ent->s.origin, v);
			}

		}
		else if(((!ent->groundentity && (wasground == true)) || (ent->groundentity && (wasground == false))) && (Route[CurrentIndex - 1].state <= GRS_ITEMS))
		{
			j = false;
			k = true;
			VectorCopy(ent->s.old_origin, v);
			v[2] -= 2;

			rs_trace = gi.trace(ent->s.old_origin, ent->mins, ent->maxs, v, ent, MASK_PLAYERSOLID);
			if (rs_trace.fraction != 1.0)
				j = true;

			if (old_ground)
			{
				if ((old_ground->blocked == train_blocked) || (old_ground->blocked == plat_blocked) || ((old_ground->blocked == door_blocked) && (old_ground->style != ENT_ID_DOOR_ROTATING)))	//CW
					k = false;
			}

			if (!ent->groundentity && (wasground == true) && k)
			{
				VectorCopy(ent->s.old_origin, v);
				i = true;				
			}			
			else if (ent->groundentity && (wasground == false) && k)
			{
				VectorCopy(ent->s.origin, v);
				i = true;					
			}
		}
		else if (Route[CurrentIndex-1].index > 1)
		{
			k = true;
			Get_RouteOrigin(CurrentIndex - 1, min);
			Get_RouteOrigin(CurrentIndex - 2, max);
			VectorSubtract(min, max, v);
			x = Get_yaw(v);
			VectorSubtract(ent->s.origin, ent->s.old_origin, v);
			if ((VectorLength(v) > 0) && (Get_vec_yaw(v, x) > 45) && k)
			{
				VectorCopy(ent->s.old_origin, v);
				i = true;							
			}		
		}
		
		if (ent->groundentity)
		{
			if (ent->groundentity != old_ground)
			{
				other = old_ground;
				old_ground = ent->groundentity;
				if (old_ground->blocked == plat_blocked)											//CW
				{
					if (old_ground->union_ent)
					{
						if (old_ground->union_ent->inuse && (old_ground->union_ent->classname[0] == 'R'))
						{
							VectorCopy(old_ground->monsterinfo.last_sighting, v);
							l = GRS_ONPLAT;
							i = 2;
						}
					}
				}
				else if (old_ground->blocked == train_blocked)										//CW
				{
					if (old_ground->union_ent)
					{
						if (old_ground->union_ent->inuse && (old_ground->union_ent->classname[0] == 'R'))
						{
							VectorCopy(old_ground->monsterinfo.last_sighting, v);
							l = GRS_ONTRAIN;
							i = 2;
						}
					}
				}
				//else if (!Q_stricmp(old_ground->classname, "func_door"))
				else if ((old_ground->blocked == door_blocked) && (old_ground->style != ENT_ID_DOOR_ROTATING))	//CW
				{
					k = false;
					if (old_ground->targetname && old_ground->union_ent)
					{
						if (TraceX(ent, old_ground->union_ent->s.origin) && (fabs(ent->s.origin[2] - old_ground->union_ent->s.origin[2]) < JumpMax))
						{
							VectorCopy(old_ground->monsterinfo.last_sighting, v);
							l = GRS_ONDOOR;
							i = 2;
						}
						else
							k = true;
					}
					else
						k = true;

					if (k && i)
					{
						i = 2;
						old_ground = other;
					}
				}
			}
		}

		if (old_ground)
		{
			if ((old_ground->classname[0] == 'f') && (i != 2))
			{
				if ((old_ground->blocked == train_blocked) || (old_ground->blocked == plat_blocked) || ((old_ground->blocked == door_blocked) && (old_ground->style != ENT_ID_DOOR_ROTATING)))	//CW
					i = false;
			}
		}

		if ((Route[CurrentIndex-1].index > 0) && (i == true))
		{
			Get_RouteOrigin(CurrentIndex - 1, max);
			VectorSubtract(max, v, vv);
			if (VectorLength(vv) <= 32 )
				i = false;
		}

		if ((l == GRS_ONTRAIN) || (l == GRS_ONPLAT) || (l == GRS_ONDOOR))
		{
			if (Route[CurrentIndex - 1].ent == old_ground)
				i = false;
		}

		if (i)
		{
			if ((l == GRS_NORMAL) && ent->groundentity)
			{
				if (!Q_stricmp(old_ground->classname, "func_rotating"))
					l = GRS_ONROTATE;
			}

			VectorCopy(v, Route[CurrentIndex].Pt);
			Route[CurrentIndex].state = l;
			if ((l > GRS_ITEMS) && (l <= GRS_ONTRAIN))
				Route[CurrentIndex].ent = old_ground;
			else if (l == GRS_ONDOOR)
				Route[CurrentIndex].ent = old_ground;
			
			if ((l == GRS_ONTRAIN) && old_ground->trainteam && old_ground->target_ent)
			{
				if (old_ground->target_ent->touch == path_corner_touch)								//CW
					VectorCopy(old_ground->target_ent->s.origin, Route[CurrentIndex].Tcorner);		
			}

			//when normal or items
			if (++CurrentIndex < MAXNODES)
			{
				gi.bprintf(PRINT_HIGH, "Last %i pod(s).\n", MAXNODES - CurrentIndex);
				memset(&Route[CurrentIndex], 0, sizeof(route_t));		//initialize
				Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1;
			}
		}

		if (ent->groundentity != NULL)
			wasground = true;
		else
			wasground = false;
	}
//Pon--

	level.current_entity = ent;
	client = ent->client;

//DH++
	client->ucmd = *ucmd;	// copy latest usercmd for use in other routines

	if (ent->turret || client->spectator)															//CW
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	else
//CW++
	{
		if (!ent->client->ctf_grapple)
//CW--
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	}


//	Perform special actions taken when +use is pressed.

	if (!client->use && (ucmd->buttons & BUTTON_USE) && (ent->health > 0))							//CW
	{
		// use key was NOT pressed, but now is
		client->use = 1;
		if (client->spycam)
			camera_off(ent);
		else
		{
			edict_t *viewing;
			vec3_t	intersect;
			float	range;
//CW++
			if (!ent->tractored && !client->agm_enemy && (client->frozen_framenum <= level.framenum))
			{
//CW--
				viewing = LookingAt(ent, 0, intersect, &range);
				if (viewing && viewing->classname)
				{
					if ((viewing->use == use_camera) && (range <= 100.0))							//CW
					{
						use_camera(viewing, ent, ent);
						if (client->spycam && (client->spycam->viewer == ent))
						{
							client->old_owner_angles[0] = ucmd->angles[0];
							client->old_owner_angles[1] = ucmd->angles[1];
						}
//CW++
						cam_user = true;
//CW--
					}
				}
			}
		}

//CW++
		if (!cam_user)
		{
			if (client->pers.inventory[ITEM_INDEX(item_teleporter)])
				Use_Teleporter(ent, item_teleporter);
		}
//CW--
	}

	if (ucmd->buttons & BUTTON_USE)
		client->use = 1;
	else
		client->use = 0;

	if (ent->turret && (ucmd->upmove > 10))
		turret_disengage(ent->turret);
//DH--

	if (level.intermissiontime)
	{
//DH++
		if (client->spycam)
			camera_off(ent);
//DH--
		client->ps.pmove.pm_type = PM_FREEZE;

		// can exit intermission after five seconds
		if ((level.time > level.intermissiontime + 5.0)	&& (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;

		return;
	}

//DH++
	if (client->spycam)
	{
		ClientSpycam(ent);		// no movement while in cam
		return;
    }
//DH--

	pm_passent = ent;

//CW++
	if (ent->client->spectator)
	{
		if (!client->buttons && (ucmd->buttons & BUTTON_ATTACK))
			SwitchModeChaseCam(ent);
	}
//CW--

//ZOID++
	if (ent->client->chase_target)
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

//CW++
		client->oldbuttons = client->buttons;
		client->buttons = ucmd->buttons;
		client->latched_buttons |= client->buttons & ~client->oldbuttons;
//CW--

		return;
	}
//ZOID--

	// set up for pmove
	memset(&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != (MAX_MODELS-1)) // was 255
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

//CW++
	if (client->frozen_framenum > level.framenum)
	{
		for (i = 0; i < 3; ++i)
		{
			ent->s.old_origin[i] = ent->s.origin[i];
			ent->velocity[i] = 0.0;
		}
	}
	else
	{
//CW--
		for (i = 0; i < 3; ++i)
		{
			pm.s.origin[i] = ent->s.origin[i] * 8;
			pm.s.velocity[i] = ent->velocity[i] * 8;
		}
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		pm.snapinitial = true;

	pm.cmd = *ucmd;

	// adds default parms
	pm.trace = PM_trace;	
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove(&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

//CW++
	if (client->frozen_framenum > level.framenum)
	{
		for (i = 0; i < 3; ++i)
		{
			ent->s.origin[i] = ent->s.old_origin[i];
			ent->velocity[i] = 0.0;
		}
	}
	else
	{
//CW--
		for (i = 0; i < 3; ++i)
		{
			ent->s.origin[i] = pm.s.origin[i] * 0.125;
			ent->velocity[i] = pm.s.velocity[i] * 0.125;
		}
	}

	VectorCopy(pm.mins, ent->mins);
	VectorCopy(pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);												//CW

//CW++
//		Jumping has a small chance of extinguishing flames.

		if (ent->burning)
		{
			if (random() < FLAME_EXPIRE_PROB)
			{
				ent->burning = false;
				if (ent->flame != NULL)		// sanity check
				{
					ent->flame->touch = NULL;
					ent->flame->think = Flame_Expire;
					ent->flame->nextthink = level.time + FRAMETIME;
				}
				else
					gi.dprintf("BUG: ClientThink() => %s is burning with a null flame\n", ent->client->pers.netname);
			}
		}
//CW--
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40.0;
		client->ps.viewangles[PITCH] = -15.0;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy(pm.viewangles, client->v_angle);
		VectorCopy(pm.viewangles, client->ps.viewangles);
	}

//ZOID++
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID--

	gi.linkentity(ent);
	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers(ent);

	// touch other objects
	for (i = 0; i < pm.numtouch; ++i)
	{
		other = pm.touchents[i];
		for (j = 0; j < i; ++j)
		{
			if (pm.touchents[j] == other)
				break;
		}

		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;

		other->touch(other, ent, NULL, NULL);
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// CDawg - add here!
	if (ucmd->forwardmove < -1)
		ent->client->backpedaling = true;
	else
		ent->client->backpedaling = false;
	// CDawg end here! 

	// fire weapon from final position if needed
	if ((client->latched_buttons & BUTTON_ATTACK) && (ent->movetype != MOVETYPE_NOCLIP))			//ZOID
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon(ent);
		}
	}

//CW++
	if ((int)sv_hook_offhand->value && !level.nohook)
		CTFWeapon_Grapple_OffHand(ent);
//CW--

//ZOID++
	CTFApplyRegeneration(ent);

	for (i = 1; i <= (int)maxclients->value; ++i)
	{
		other = g_edicts + i;
		if (other->inuse && (other->client->chase_target == ent))
			UpdateChaseCam(other);
	}

	if (client->menudirty && (client->menutime <= level.time))
	{
		PMenu_Do_Update(ent);
		gi.unicast(ent, true);
		client->menutime = level.time;
		client->menudirty = false;
	}
//ZOID--
}

/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame(edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

//DH++
	if (client->spycam)
		client = client->camplayer->client;
//DH--

//Maj++
	if (ent->isabot)
		SV_Physics_Step(ent);
//Maj--

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && (ent->movetype != MOVETYPE_NOCLIP))								//ZOID
		Think_Weapon(ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if (level.time > client->respawn_time)
		{
			// only wait for attack button
			buttonMask = BUTTON_ATTACK;																//CW

			if ((client->latched_buttons & buttonMask) || ((int)dmflags->value & DF_FORCE_RESPAWN) || CTFMatchOn())	//CW
			{
				Respawn(ent);
				client->latched_buttons = 0;
			}
		}
//CW++
		if (ent->isabot && ((ent->movetype == MOVETYPE_TOSS) || (ent->movetype == MOVETYPE_BOUNCE)))
		{
			if (gi.pointcontents(ent->s.origin) & MASK_WATER)	
				ent->velocity[2] -= 0.1 * (ent->gravity * sv_gravity->value * FRAMETIME);
			else
				ent->velocity[2] -= 0.5 * (ent->gravity * sv_gravity->value * FRAMETIME);
		}
//CW--
		return;
	}

	client->latched_buttons = 0;
}
