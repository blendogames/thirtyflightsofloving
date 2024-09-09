#include "g_local.h"
#include "m_player.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "aj_statusbars.h"
#include "aj_banning.h"
#include "e_grapple.h"
#include "aj_weaponbalancing.h"

#include "p_trail.h"

#include "bot_procs.h"
#include "e_hook.h"

#include <sys/timeb.h>
#include <time.h>

#define	OPTIMIZE_INTERVAL	0.1

int	num_clients	= 0;

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ( (!self->targetname) || (Q_stricmp(self->targetname, spot->targetname) != 0) )
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if (Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}

void SP_misc_teleporter_dest (edict_t *ent);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if (Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
edict_t *dm_spots[64];
int		num_dm_spots;

void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);

// ADJ - fix for LMCTF's invisible dmstarts
	if (invisible_dmstarts->value)
		self->svflags |= SVF_NOCLIENT;
// end ADJ

	dm_spots[num_dm_spots++] = self;
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/
void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ( (Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0) )
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_coop_lava (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games on rmine2 where lava level
needs to be checked
*/
void SP_info_player_coop_lava(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	//ScarFace- crack for Crackwhore
	else if (strstr(info, "crakhor"))
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	else if (deathmatch->value && teamplay->value && attacker->client && SameTeam(self, attacker))
	{
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	}

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
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
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		// RAFAEL
		case MOD_GEKK:
		case MOD_BRAINTENTACLE:
			message = "that's gotta hurt";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			case MOD_PROX:
				if (IsNeutral(self))
					message = "got too close to its own proximity mine";
				else if (IsFemale(self))
					message = "got too close to her own proximity mine";
				else
					message = "got too close to his own proximity mine";
				break;
		// ROGUE
			case MOD_ETF_SPLASH:
				message = "shouldn't have used the ETF Rifle at point blank";
				break;
			case MOD_TRAP:
				if (IsFemale(self))
			 		message = "was sucked into her own trap";
				else
			 		message = "was sucked into his own trap";
				break;
			case MOD_DOPPLE_EXPLODE:
				if (IsFemale(self))
					message = "got caught in her own trap";
				else
					message = "got caught in his own trap";
				break;
		// ROGUE
		// SKWiD MOD
			case MOD_PLASMA:
				message = "gets shocked";
				break;
 			case MOD_SHOCK_SPHERE:
			case MOD_SHOCK_SPLASH:
				if (IsFemale(self))
					message = "was scrambled by her own shock sphere";
				else
					message = "was scrambled by his own shock sphere";
				break;
#ifdef USE_AM_ROCKETS
			case MOD_AM_ROCKET:
				if (IsFemale(self))
					message = "blew herself to giblets";
				else
					message = "blew himself to giblets";
				break;
#endif	// USE_AM_ROCKETS
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
			my_bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			{
				self->client->resp.score--;
				if (self->client->team)
					self->client->team->score--;
			}
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID
			// RAFAEL 14-APR-98
			case MOD_RIPPER:
				message = "was ripped to shreds by";
				message2 = "'s ripper gun";
				break;
			case MOD_PHALANX:
				message = "was evaporated by";
				break;
			case MOD_TRAP:
				message = "was sucked into";
				message2 = "'s trap";
				break;
			// END 14-APR-98
//===============
//ROGUE
			case MOD_CHAINFIST:
				message = "was shredded by";
				message2 = "'s ripsaw";
				break;
			case MOD_DISINTEGRATOR:
				message = "lost his grip courtesy of";
				message2 = "'s disintegrator";
				break;
			case MOD_ETF_RIFLE:
				message = "was perforated by";
				break;
			case MOD_ETF_SPLASH:
				message = "was torn up by";
				message2 = "'s explosive flechettes";
				break;
			case MOD_HEATBEAM:
				message = "was scorched by";
				message2 = "'s plasma beam";
				break;
			case MOD_TESLA:
				message = "was enlightened by";
				message2 = "'s tesla mine";
				break;
			case MOD_PROX:
				message = "got too close to";
				message2 = "'s proximity mine";
				break;
			case MOD_NUKE:
				message = "was nuked by";
				message2 = "'s antimatter bomb";
				break;
			case MOD_VENGEANCE_SPHERE:
				message = "was purged by";
				message2 = "'s vengeance sphere";
				break;
			case MOD_DEFENDER_SPHERE:
				message = "was blasted by";
				message2 = "'s defender sphere";
				break;
			case MOD_HUNTER_SPHERE:
				message = "was killed like a dog by";
				message2 = "'s hunter sphere";
				break;
			case MOD_TRACKER:
				message = "was annihilated by";
				message2 = "'s disruptor";
				break;
			case MOD_DOPPLE_EXPLODE:
				message = "was blown up by";
				message2 = "'s doppleganger";
				break;
			case MOD_DOPPLE_VENGEANCE:
				message = "was purged by";
				message2 = "'s doppleganger";
				break;
			case MOD_DOPPLE_HUNTER:
				message = "was hunted down by";
				message2 = "'s doppleganger";
				break;
//ROGUE
//===============
		// SKWiD MOD
			case MOD_PLASMA:
				message = "got an infusion of plasma from";
				message2 = "'s plasma rifle";
				break;
 			case MOD_SHOCK_SPHERE:
			case MOD_SHOCK_SPLASH:
				message = "was scrambled by";
				message2 = "'s shock sphere";
				break;
#ifdef USE_AM_ROCKETS
			case MOD_AM_ROCKET:
				message = "was blown to giblets by";
				message2 = "'s anti-matter rocket";
				break;
#endif	// USE_AM_ROCKETS
			}
			if (message)
			{
				my_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
//ROGUE
				if (gamerules && gamerules->value)
				{
					if(DMGame.Score)
					{
						if(ff)
							DMGame.Score(attacker, self, -1);
						else
							DMGame.Score(attacker, self, 1);
					}
					return;
				}
//ROGUE

				if (deathmatch->value)
				{
					if (ff)
					{
						attacker->client->resp.score--;
						if (attacker->client->team)
							attacker->client->team->score--;
					}
					//elimate scoring for telefragging in CTF
					else if ((ttctf->value) && (mod == MOD_TELEFRAG))
						return;
					else
					{
						attacker->client->resp.score++;
						if (attacker->client->team)
							attacker->client->team->score++;
					}
				}
				return;
			}
		}
	}

	my_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
//ROGUE
	{
		if (gamerules && gamerules->value)
		{
			if(DMGame.Score)
			{
				DMGame.Score(self, self, -1);
			}
			return;
		}
		else
		{
			self->client->resp.score--;
			if (self->client->team)
				self->client->team->score--;
		}
	}
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	qboolean	doubled;
	// RAFAEL
	qboolean	quadfire;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP)) {
		quad = false;
		doubled = false;
	}
	else {
		quad = (self->client->quad_framenum > (level.framenum + 10));
		doubled = (self->client->double_framenum > (level.framenum + 10));
	}
	// RAFAEL
	if (!((int)(dmflags->value) & DF_QUADFIRE_DROP))
		quadfire = false;
	else
		quadfire = (self->client->quadfire_framenum > (level.framenum + 10));


	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
	if (doubled) // ScarFace
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_double"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->double_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
	// RAFAEL
	if (quadfire)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quadfire"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quadfire_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;


// AJ
	// Expert: Release hook if needed
	if ( (sk_hook_style->value == 0) && self->client->hook )
		Release_Grapple(self->client->hook);
// end AJ

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
//ZOID
		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		TossClientWeapon (self);
//ZOID
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
//ZOID
		// Knightmare added- drop ammogen backpack
		if (!OnSameTeam(self, attacker))
			CTFApplyAmmogen (attacker, self);

		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores
	}

	if (!self->bot_client && gamerules && gamerules->value)	// if we're in a dm game, alert the game
	{
		if(DMGame.PlayerDeath)
			DMGame.PlayerDeath(self, inflictor, attacker);
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->client->double_framenum = 0;				// PMM
	// RAFAEL
	self->client->quadfire_framenum = 0;

//==============
// ROGUE stuff
	self->client->double_framenum = 0;

	// turn off alt-fire mode if on
	self->client->pers.fire_mode = 0;
	self->client->pers.plasma_mode = 0;  // SKWiD- plasma rifle fire mode

	self->s.effects &= ~EF_HALF_DAMAGE; // ScarFace- remove safety mode shell

	// if there's a sphere around, let it know the player died.
	// vengeance and hunter will die if they're not attacking,
	// defender should always die
	if (self->client->owned_sphere)
	{
		edict_t *sphere;

		sphere = self->client->owned_sphere;
		sphere->die(sphere, self, self, 0, vec3_origin);
	}

	// if we've been killed by the tracker, GIB!
	if ((meansOfDeath & ~MOD_FRIENDLY_FIRE) == MOD_TRACKER)
	{
		self->health = (player_gib_health->value - 100);
		damage = 400;
	}

	// make sure no trackers are still hurting us.
	if (self->client->tracker_pain_framenum)
	{
		RemoveAttackingPainDaemons (self);
	}

	// if we got obliterated by the nuke, don't gib
	if ((self->health < -80) && (meansOfDeath == MOD_NUKE))
		self->flags |= FL_NOGIB;

// ROGUE
//==============

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (self->health < player_gib_health->value)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		if (mega_gibs->value)
		{
			ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		}
		ThrowClientHead (self, damage);

//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
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
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

/* AJ removed - replaced by code in Putclientinserver
	item = FindItem("Grapple");

	if (grapple->value || ctf->value)
	{
//ZOID
		client->pers.inventory[ITEM_INDEX(item)] = 1;
//ZOID
	}
	else
	{
		client->pers.inventory[ITEM_INDEX(item)] = 0;
	}
*/

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;
	// RAFAEL
	client->pers.max_magslug	= 50;
	client->pers.max_trap		= 5;

//ROGUE
	// FIXME - give these real numbers....
	client->pers.max_prox		= 50;
	client->pers.max_tesla		= 50;
	client->pers.max_flechettes = 200;
#ifndef KILL_DISRUPTOR
	client->pers.max_rounds     = 100;
#endif
//ROGUE

	client->pers.fire_mode			= 0;  // Lazarus alternate fire mode
	client->pers.plasma_mode		= 0;  // SKWiD- plasma rifle fire mode

// start AJ
	lithium_initclient(client);
// end AJ

	client->pers.connected = true;

	// Knightmare- custom client colors
	Vector4Set (client->pers.color1, 255, 255, 255, 0);
	Vector4Set (client->pers.color2, 255, 255, 255, 0);
}

qboolean is_bot = false;
void InitClientResp (gclient_t *client)
{
//ZOID
	int ctf_team = client->resp.ctf_team;
//ZOID

	memset (&client->resp, 0, sizeof(client->resp));
//ZOID
	client->resp.ctf_team = ctf_team;
//ZOID
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

//ZOID
	if (ctf->value && (client->resp.ctf_team < CTF_TEAM1))
		CTFAssignTeam(client, is_bot);
//ZOID
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
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
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
float PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 0; n < num_players; n++)
	{
//		player = &g_edicts[n];
		player = players[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0, spot_num;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

//	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	while (count < num_dm_spots)
	{
		spot = dm_spots[count];

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
	spot_num = 0;
	do
	{
		spot = dm_spots[spot_num++];
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	int		spot_num;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	spot_num = 0;
	while (spot_num < num_dm_spots)
	{
		spot = dm_spots[spot_num++];

		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = dm_spots[0];

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
//ZOID
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
//ZOID
		spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
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
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void Body_droptofloor(edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-24);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	ent->s.origin[2] += 32;

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

	VectorCopy(tr.endpos, ent->s.origin);

	gi.linkentity(ent);

	if (tr.ent)
		ent->nextthink = level.time + 0.1;
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < player_gib_health->value)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		if (mega_gibs->value) //mega gibs
		{
			ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		}
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);

//	body->nextthink = level.time + 0.2;
//	body->think = Body_droptofloor;
}


void respawn (edict_t *self)
{

	if (deathmatch->value || coop->value)
	{
		CopyToBodyQue (self);
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

// AJ
		// Expert: prevent grappling on immediate respawn
		if (sk_hook_style->value == 0)
			self->client->hook_frame = level.framenum + 1;
// end AJ

		self->client->respawn_time = level.time;
// AJ - update the HUD
		lithium_updatestats(self);
// end AJ

	}

	// restart the entire server
	//ScarFace- disabled this, it's annoying
	if (!deathmatch->value && !coop->value)
		gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void ShowGun(edict_t *ent);

void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	bot_team_t	*team;
	lag_trail_t	*lag_trail;
	lag_trail_t	*lag_angles;
	float	latency;

	if (!the_client && !ent->bot_client && (num_players <= 1))
		the_client = ent;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if(gamerules && gamerules->value && DMGame.SelectSpawnPoint)		// PGM
		DMGame.SelectSpawnPoint (ent, spawn_origin, spawn_angles);		// PGM
	else																// PGM
		SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	team = client->team;
	lag_trail = client->lag_trail;
	lag_angles = client->lag_angles;
	latency = client->latency;

	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->resp = resp;
	client->team = team;
	client->lag_trail = lag_trail;
	client->lag_angles = lag_angles;
	client->latency = latency;
	if (client->pers.health <= 0)
		InitClientPersistant(client);

// This code was in initclientpersistent but needs ent-> for bot detection
// AJ - change grapple use from just CTF to dependent on use_hook
// special case it so that bots still get CTF grapple when hook_offhand set
	if (use_hook->value)
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;
	else
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;
		gi.cvar_forceset("grapple", "0");
	}

	if (sk_hook_offhand->value && ent->bot_client)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;
		gi.cvar_forceset("grapple", "1");
	}
// end AJ

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	if (!ent->bot_client)
		ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";

	if (!ent->bot_client)
	{
		ent->pain = player_pain;
		ent->die = player_die;
	}

	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (!ent->bot_client)
	{
		if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
		{
			client->ps.fov = 90;
		}
		else
		{
			client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
			if (client->ps.fov < 1)
				client->ps.fov = 90;
			else if (client->ps.fov > 160)
				client->ps.fov = 160;
		}

		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	}

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = MAX_MODELS-1;		// will use the skin specified model, was 255
#ifdef USE_ITEMTABLE_VWEP
	ent->s.modelindex2 = MAX_MODELS-1;		// custom gun model, was 255
#endif	// USE_ITEMTABLE_VWEP
	ShowGun(ent);

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;			// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID

// AJ - switch on safety mode
	if (use_safety->value && !ent->client->ps.stats[STAT_LITHIUM_MODE])
	{
		//ent->takedamage = DAMAGE_NO;
		ent->client->safety_mode = TRUE;
		ent->client->safety_time = level.time + safety_time->value;

//		if (!ent->bot_client)
//			gi.centerprintf(ent, "Safety mode.");
		ent->client->ps.stats[STAT_LITHIUM_MODE] = CS_SAFETY;
	}
// end AJ

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	if (!ent->map)
		ent->map = G_CopyString(ent->client->pers.netname);

// AJ - force change to best weapon
	lithium_choosestartweap(ent);
	lithium_updatestats(ent); // AJ TEMP
// end AJ

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	if (!ent->bot_client)
		ChangeWeapon (ent);

	ent->last_max_z = 32;
	ent->jump_ent = NULL;
	ent->duck_ent = NULL;
	ent->last_trail_dropped = NULL;

	if (!ent->bot_client)
	{
		if (ent->client->lag_trail)
		{
			// init the lag_trail
			for (i=0; i<10; i++)
			{
				VectorCopy(ent->s.origin, (*ent->client->lag_trail)[i]);
				VectorCopy(ent->client->v_angle, (*ent->client->lag_angles)[i]);
			}
		}

		if ((bot_calc_nodes->value) && (trail[0]->timestamp))
		{	// find a trail node to start checking from
			if ((i = ClosestNodeToEnt(ent, false, true)) > -1)
			{
				ent->last_trail_dropped = trail[i];
			}
		}
	}
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	if (ctf->value)
		ent->client->team = NULL;

	InitClientResp (ent->client);

	//PGM
	if(gamerules && gamerules->value && DMGame.ClientBegin)
	{
		DMGame.ClientBegin (ent);
	}
	//PGM

	// locate ent at a spawn point
	PutClientInServer (ent);

// AJ enable observer mode
	if (!ctf->value && use_observer->value && !ent->bot_client)
		lithium_observer (ent, NULL);
	else ent->client->observer_mode = FALSE;

	if ( (int)def_hud->value >= 3 )	// set up ammo HUD
	{
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = ent->client->pers.inventory[ITEM_INDEX(item_shells)];
		ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = ent->client->pers.inventory[ITEM_INDEX(item_bullets)];
		ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = ent->client->pers.inventory[ITEM_INDEX(item_grenades)];
		ent->client->ps.stats[STAT_CTF_TECH] = ent->client->pers.inventory[ITEM_INDEX(item_rockets)];
		ent->client->ps.stats[STAT_LITHIUM_PLAYERS] = ent->client->pers.inventory[ITEM_INDEX(item_cells)];
		ent->client->ps.stats[STAT_LITHIUM_PLACE] = ent->client->pers.inventory[ITEM_INDEX(item_slugs)];
	}
// end AJ
// AJ zbot stuff
	ent->client->resp.bot_start = level.time + 5 + (rand() % 5);  // between 5 and 9 seconds later.
	ent->client->resp.bot_retry = (int) zk_retry->value;
// end AJ

    ///Q2 Camera Begin
    EntityListAdd(ent);
    ///Q2 Camera End

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	//gi.WriteByte (EV_PLAYER_TELEPORT); //ScarFace- get rid of grenade launcher sound
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	players[num_players++] = ent;
	num_clients++;

// AJ - force the +hook command...
	if (!ent->bot_client)
	{
//		gi.AddCommandString("alias +hook +use ; alias -hook -use\n");
		stuffcmd(ent, "alias +hook +use ; alias -hook -use\n");
		stuffcmd(ent, "set hook_color red u\n");
	}

// end AJ

// AJ - change grapple use from just CTF to dependent on use_hook
// special case it so that bots still get CTF grapple when hook_offhand set
	if (use_hook->value)
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;
	else ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;

	if (sk_hook_offhand->value && ent->bot_client)
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;
// end AJ

	if (deathmatch->value)
	{
// AJ - no need for this, as can go in the MOTD.txt...
/*		if (bot_show_connect_info->value)
			gi.centerprintf(ent, "\n\n=====================================\nThe Eraser Bot v%1.3f\nby Ryan Feltrin (aka Ridah)\n\nRead the readme.txt file\nlocated in the Eraser directory!\n\nVisit http://impact.frag.com/\nfor Eraser news\n\n-------------------------------------\n", ERASER_VERSION, maxclients->value);

		if (teamplay->value && !ctf->value)
			gi.cprintf(ent, PRINT_HIGH, "\n\n=====================================\nServer has enabled TEAMPLAY!\n\nType: \"cmd teams\" to see the list of teams\nType: \"cmd join <teamname>\" to join a team\n\n");
*/
		lithium_motd(ent);
// end AJ
		ClientBeginDeathmatch (ent);

		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
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
	if (!Info_Validate(userinfo))
	{
	//	strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
		Com_strcpy (userinfo, MAX_INFO_STRING, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	if (!ent->client->team || (strlen(ent->client->team->default_skin) == 0))
	{
		// set skin
		s = Info_ValueForKey (userinfo, "skin");

		playernum = ent-g_edicts-1;

		// combine name and skin into a configstring
//ZOID
		if (ctf->value)
			CTFAssignSkin(ent, s);
		else
//ZOID
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
	}
	else	// only update the name
	{
		s = G_CopyString(ent->client->team->default_skin);

		playernum = ent-g_edicts-1;

		// combine name and skin into a configstring
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

		gi.TagFree(s);
	}

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

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
	// end Knightmare

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
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
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

// AJ - add IP banning test
	if (check_ip(value))
		return false;
// end AJ

	// check for a password
	if (!ent->bot_client)
	{
		value = Info_ValueForKey (userinfo, "password");
		if (strcmp(passwd->string, value) != 0)
			return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
//ZOID
		is_bot = ent->bot_client;			// make sure bot's join a team
		InitClientResp (ent->client);
		is_bot = false;

		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	// do real client specific stuff
	if (!ent->bot_client)
	{
		int i;

		ent->client->team = NULL;

		ent->client->lag_trail = gi.TagMalloc(sizeof(lag_trail_t), TAG_GAME);
		ent->client->lag_angles = gi.TagMalloc(sizeof(lag_trail_t), TAG_GAME);

		// init the lag_trail
		for (i=0; i<10; i++)
		{
			VectorCopy(ent->s.origin, (*ent->client->lag_trail)[i]);
			VectorCopy(ent->client->v_angle, (*ent->client->lag_angles)[i]);
		}
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void botRemovePlayer(edict_t *self);

void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

    ///Q2 Camera Begin
    EntityListRemove(ent);
    ///Q2 Camera End

	botRemovePlayer(ent);

	my_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//============
//ROGUE
	// make sure no trackers are still hurting us.
	if(ent->client->tracker_pain_framenum)
		RemoveAttackingPainDaemons (ent);

	if (ent->client->owned_sphere)
	{
		if(ent->client->owned_sphere->inuse)
			G_FreeEdict (ent->client->owned_sphere);
		ent->client->owned_sphere = NULL;
	}

	if (gamerules && gamerules->value)
	{
		if(DMGame.PlayerDisconnect)
			DMGame.PlayerDisconnect(ent);
	}
//ROGUE
//============

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
int	last_bot=0;
#define		BOT_THINK_TIME	0.03	// never do bot thinks for more than this time

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	int count=0, start;
	clock_t	start_time, now;

// AJ - zkick
	if (ucmd->impulse) // this is a bot!
	{
		if (zk_logonly->value)
			gi.dprintf ("[ZKick]: client#%d (%s) @ %s is a bot (impulse=%d)\n",
						(ent-g_edicts)-1, ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo,"ip"), ucmd->impulse);
		else
		{
			gi.dprintf ("[ZKick]: client#%d (%s) @ %s was kicked (impulse=%d)\n",
						(ent-g_edicts)-1, ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo,"ip"), ucmd->impulse);
			stuffcmd (ent, "disconnect\n"); // terminate client's session
		}
	}
// end AJ - zkick

	if (paused)
	{
		gi.centerprintf(ent, "PAUSED\n\n(type \"botpause\" to resume)");
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		return;
	}

	level.current_entity = ent;
	client = ent->client;

	if ( client->hook_on && client->hook && (sk_hook_style->value == 1) )
		abandon_hook_service (client->hook);


	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
// AJ replaced constant 5.0 with intermission_time->value
		if (level.time > level.intermissiontime + intermission_time->value
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

// AJ
	// Expert: Check if player needs to be pulled by hook
	if ( (sk_hook_style->value == 0) && (client->on_hook == true) )
		Pull_Grapple (ent);
// end AJ

    // Q2 Camera Begin
    if (ent->client->bIsCamera)
    {
        CameraThink(ent,ucmd);
        return;
    }
    // Q2 Camera End

	if (ent->bot_client)
		return;

//	gi.dprintf("f: %i, s: %i, u: %i\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
//	gi.dprintf("%i, %i\n", ucmd->buttons, client->ps.pmove.pm_flags);

	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != (MAX_MODELS-1)) //was 255
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

// AJ
	// Expert: If the hook is in use, just ignore gravity
	if ( (sk_hook_style->value == 0) && client->on_hook )
		client->ps.pmove.gravity = 0;
	else
// end AJ
		client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}


//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

	if ((client->latency > 0) && !(client->buttons & BUTTONS_ATTACK) && (ucmd->buttons & BUTTONS_ATTACK))
	{
		if ((level.time - client->firing_delay) > 0.1)
			client->firing_delay = level.time + (client->latency/1000);

		if (client->firing_delay > level.time)		// turn it off
		{
			ucmd->buttons &= ~BUTTONS_ATTACK;
			if (ucmd->buttons >= BUTTON_ANY)
				ucmd->buttons -= BUTTON_ANY;
		}
	}

	if ((client->latency > 0) && !(ucmd->buttons & BUTTON_ATTACK) && (client->firing_delay > (level.time - 0.1)) && (client->firing_delay <= level.time))
	{	// remember the button after it was released when simulating lag
		ucmd->buttons |= BUTTON_ATTACK;
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTONS_ATTACK
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

// AJ abandon hope hook
	if ( (sk_hook_style->value == 1) && client->hook_on && (VectorLength(ent->velocity) < 1) )
		  client->ps.pmove.gravity = 0;


//ZOID
//regen tech
	CTFApplyRegeneration(ent);
//ZOID

//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
//ZOID

	// =========================================================
	// check for a bots to think
	if ((num_players > 2) && (num_clients == 1))
	{
		int	save=-1, loop_count=0;
		vec3_t	org;

		start = last_bot;
		start_time = clock();
		now = start_time;

		// go to the next bot
		last_bot++;
		if (last_bot >= num_players)
			last_bot = 0;

		VectorCopy(ent->s.origin, org);
		org[2] += ent->viewheight;

		while (	(count < 5) && (((double) (now - start_time)) < 50))
		{
			// don't do client's, or bot's in this client's view
			while	(	(players[last_bot]->last_think_time == level.time)
					 ||	(!players[last_bot]->bot_client)
					 ||	(	(gi.inPVS(org, players[last_bot]->s.origin))
						 &&	(	(players[last_bot]->nextthink >= level.time)
							 ||	(players[last_bot]->nextthink = level.time + 0.1)
							)
						) // this makes the bot think by itself (smooth movement)
					)
			{
				last_bot++;
				if (last_bot == num_players)
					last_bot = 0;
				loop_count++;

				if (players[last_bot]->bot_client && (last_bot == start))	// we've done all bots
					goto done_all_bots;

				if (loop_count > num_players)
					goto done_all_bots;
			}

			if (start == 0)
				start = last_bot;

			if ((level.time - players[last_bot]->last_think_time) >= 0.1)
			{
				players[last_bot]->think(players[last_bot]);
				players[last_bot]->nextthink = -1;			// don't think by themselves
				count++;

				save = last_bot;
			}

			if (last_bot == start)		// we've done all bots
				break;

			last_bot++;
			if (last_bot == num_players)
				last_bot = 0;
			loop_count++;
			if (loop_count > num_players)
				goto done_all_bots;

			now = clock();

		}

	}

done_all_bots:
/*
	// check for firing between server frames
	for (i=0; i<num_players; i++)
	{
		if (	(players[i]->bot_client)
			&&	(players[i]->dmg)
//			&&	(	(timebuffer.millitm < players[i]->lastattack_time.millitm)
//				 ||	((timebuffer.millitm - players[i]->lastattack_time.millitm) >= 50)
//				)
			)
		{
			bot_FireWeapon(players[i]);
			players[i]->dmg = 0;
		}
	}
*/
	if (!dedicated->value)
		OptimizeRouteCache();

	// =========================================================

// AJ added
	// Expert Hook : Grapple command code
	// Expert Pogo : Pogo command code
	// Check to see if player pressing the "use" key
	if (sk_hook_offhand->value && //|| expflags & EXPERT_POGO) &&
		ent->client->buttons & BUTTON_USE &&
	    !ent->deadflag &&
	    client->hook_frame <= level.framenum
		&& !ent->client->observer_mode)			// AJ added clause so observers cannot grapple
	{
		if (sk_hook_style->value == 1)
			abandon_hook_fire (ent, false);
		else
		{
			int i;
			vec3_t	oldorg, oldangles;

			if (ent->client->latency > 0)
			{	// simulate lag

				// find the best lag_trail location to use
				i = (int) (ent->client->latency/100);
				if (i > 9)
					i = 9;

				VectorCopy(ent->s.origin, oldorg);
				VectorCopy(ent->client->v_angle, oldangles);

				VectorCopy((*ent->client->lag_trail)[i], ent->s.origin);
				VectorCopy((*ent->client->lag_angles)[i], ent->client->v_angle);
				gi.linkentity(ent);
			}

			Throw_Grapple (ent);

			if (ent->client->latency > 0)
			{	// simulate lag
				VectorCopy(oldorg, ent->s.origin);
				VectorCopy(oldangles, ent->client->v_angle);

				gi.linkentity(ent);
			}

		}
	}

	// Expert: Check to see if the grapple key was released
	if (sk_hook_offhand->value && Ended_Grappling (client) &&
		!ent->deadflag && client->hook)
	{
		if (sk_hook_style->value == 1)
			abandon_hook_reset(ent->client->hook);
		else
			Release_Grapple (client->hook);
	}

// end AJ Added

}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/

void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void CTFFlagThink(edict_t *ent);

void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			i;
	int			buttonMask;

// AJ moved this above bot_client test so that can have safety mode in bots
	if (level.intermissiontime)
		return;


// AJ - detect end of safety mode
	if (ent->client->safety_mode && ent->client->safety_time < level.time && (!ent->client->observer_mode || ent->bot_client))
	{
		ent->takedamage = DAMAGE_YES;
		ent->client->safety_mode = FALSE;
//		if (!ent->bot_client)
//			gi.centerprintf(ent, "End of safety.");
		ent->client->respawn_time = level.time;
		ent->client->ps.stats[STAT_LITHIUM_MODE] = 0;
//		ent->s.effects &= 0xF7FFFFFF; // clear the yellow shell
//		ent->s.effects &= ~EF_COLOR_SHELL;
//		ent->s.renderfx &= ~RF_SHELL_GREEN;
		ent->s.effects &= EF_HALF_DAMAGE; //ScarFace- clear green shell
	}
// end AJ

	if (ent->bot_client)
		return;

    ///Q2 Camera Begin
    if (ent->client->bIsCamera)
    {
        return;
    }
    ///Q2 Camera End


// AJ - detect the end of observer mode
	if ( ent->client->observer_mode && (ent->client->buttons & BUTTON_ATTACK) )
	{
// reset the players time to time they actually join
// however, cannot do this as if they hop into observer mode and then exit it resets their join time
//		ent->client->resp.enterframe = level.framenum;
		ent->client->observer_mode = FALSE;
		ent->client->ps.stats[STAT_LITHIUM_MODE] = 0;
		PutClientInServer (ent);
		ent->svflags &= ~SVF_NOCLIENT;
	}
// end AJ

// AJ - detect zbot's
	if (ent->client->resp.bot_start>0 && ent->client->resp.bot_start <= level.time)
	{
		ent->client->resp.bot_start = 0;
		stuffcmd(ent, "!zbot\n");
		ent->client->resp.bot_end = level.time + 4;
	}
	if (ent->client->resp.bot_end>0 && ent->client->resp.bot_end <= level.time)
	{
		if (ent->client->resp.bot_retry > 0 || (ent->client->resp.bot_retry == 0
			&& zk_retry->value == 0 && zk_pinglimit->value > 0 && ent->client->ping > (int) zk_pinglimit->value))
		{
			ent->client->resp.bot_retry--;
			stuffcmd (ent, "!zbot\n");
			ent->client->resp.bot_end = level.time + 4;
			gi.dprintf ("[ZKick]: client#%d (%s) @ %s timed out (ping=%d)\n",
				        (ent-g_edicts)-1, ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo,"ip"), ent->client->ping);
		}
		else
		{
			ent->client->resp.bot_end = 0;
			if (zk_logonly->value)
				gi.dprintf ("[ZKick]: client#%d (%s) @ %s is a bot\n", (ent-g_edicts)-1, ent->client->pers.netname,
							Info_ValueForKey (ent->client->pers.userinfo,"ip"));
			else
			{
				gi.dprintf ("[ZKick]: client#%d (%s) @ %s was kicked\n", (ent-g_edicts)-1, ent->client->pers.netname,
							Info_ValueForKey (ent->client->pers.userinfo,"ip"));
				stuffcmd (ent, "disconnect\n");
			}
		}
	}
// end AJ

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (client->latency > 0)
	{
		// -- save the location in the lag_trail
		//    filter the trails down one position
		for (i=9; i>0; i--)
		{
			VectorCopy((*client->lag_trail)[i-1], (*client->lag_trail)[i]);
			VectorCopy((*client->lag_angles)[i-1], (*client->lag_angles)[i]);
		}

		VectorCopy(ent->s.origin, (*client->lag_trail)[0]);
		VectorCopy(ent->client->v_angle, (*client->lag_angles)[0]);
		// -- done.
	}

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTONS_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	client->latched_buttons = 0;

//	if (gi.pointcontents(ent->s.origin) & CONTENTS_LADDER)

	// check for new node(s)
	if (bot_calc_nodes->value)
	{
		CheckMoveForNodes(ent);
	}

//if (ent->groundentity)
//gi.dprintf("%s\n", ent->groundentity->classname);

	if (ent->flags & FL_SHOWPATH)
	{
		// set the target location
		if (!ent->goalentity)	// spawn it
		{
			ent->goalentity = G_Spawn();
			ent->goalentity->classname = "player goal";
			VectorCopy(ent->mins, ent->goalentity->mins);
			VectorCopy(ent->maxs, ent->goalentity->maxs);

			VectorCopy(ent->s.origin, ent->goalentity->s.origin);
			ent->goalentity->s.modelindex = MAX_MODELS-1; // was 255
			gi.linkentity(ent->goalentity);
		}

		if (ent->client->buttons & BUTTON_ATTACK)
		{
			VectorCopy(ent->s.origin, ent->goalentity->s.origin);
			ent->goalentity->s.modelindex = MAX_MODELS-1; // was 255
			gi.linkentity(ent->goalentity);
		}

		Debug_ShowPathToGoal(ent, ent->goalentity);
	}

//gi.dprintf("%i\n", ent->waterlevel);

	// HACK, send bots to us if we have the flag, and also summon some helper bots
	if (ctf->value && CarryingFlag(ent))// &&
//		(!ent->movetarget || !ent->movetarget->item || (ent->movetarget->item->pickup != CTFPickup_Flag)))
	{
		edict_t *flag, *enemy_flag, *enemy_flag2, *plyr, *self;
		int	i=0, count=0, ideal;
		static float	last_checkhelp=0;

		self = ent;

		if (self->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			enemy_flag2 = flag3_ent;

		}
		else if (self->client->resp.ctf_team == CTF_TEAM2)
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag3_ent;
		}
		else
		{
			flag = flag3_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag2_ent;
		}

		// look for some helpers
		if (last_checkhelp < (level.time - 0.5))
		{
			for (i=0; i<num_players; i++)
			{
				plyr = players[i];

				if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
				{
					if (	(plyr->enemy != self)
						&&	(!plyr->target_ent || (plyr->target_ent->think != CTFFlagThink) || (entdist(plyr, plyr->target_ent) > 1000))
						&&	(entdist(plyr, self) < 2000))
					{	// send this enemy to us
						plyr->enemy = self;
					}
//					continue;
				}

				if ((plyr != self) && (plyr->target_ent == self))
					count++;
			}

			ideal = ((int)ceil((1.0*(float)num_players)/4.0));

			if (count < ideal)
			{
				for (i=0; (i<num_players && count<ideal); i++)
				{
					plyr = players[i];

					if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
						continue;

					if (plyr->target_ent == self)
						continue;

					if (entdist(plyr, self) > 700)
						continue;

					if (!gi.inPVS(plyr->s.origin, self->s.origin))
						continue;

					plyr->target_ent = self;
					if (++count >= ideal)
						break;
				}
			}
			else if (count > ideal)	// release a defender
			{
				for (i=0; (i<num_players && count<ideal); i++)
				{
					plyr = players[i];

					if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
						continue;

					if (plyr->target_ent != self)
						continue;

					plyr->target_ent = NULL;
					break;
				}
			}

			last_checkhelp = level.time + random()*0.5;
		}

	}

	if (ent->flags & FL_SHOW_FLAGPATHS)
	{
		edict_t *trav;

		// show lines between alternate routes
		trav = NULL;
		while (trav = G_Find(trav, FOFS(classname), "flag_path_src"))
		{
			if (!trav->last_goal || !trav->target_ent)
				continue;	// not complete, don't save

			trav->s.modelindex = gi.modelindex ("models/objects/gibs/chest/tris.md2");

			if (trav->last_goal)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (trav->s.origin);
				gi.WritePosition (trav->last_goal->s.origin);
				gi.multicast (trav->s.origin, MULTICAST_PVS);
			}
			if (trav->target_ent)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (trav->s.origin);
				gi.WritePosition (trav->target_ent->s.origin);
				gi.multicast (trav->s.origin, MULTICAST_PHS);
			}

		}
	}

}

/*
==============
RemoveAttackingPainDaemons

This is called to clean up the pain daemons that the disruptor attaches
to clients to damage them.
==============
*/
void RemoveAttackingPainDaemons (edict_t *self)
{
	edict_t *tracker;

	tracker = G_Find (NULL, FOFS(classname), "pain daemon");
	while(tracker)
	{
		if(tracker->enemy == self)
			G_FreeEdict(tracker);
		tracker = G_Find (tracker, FOFS(classname), "pain daemon");
	}

	if(self->client)
		self->client->tracker_pain_framenum = 0;
}
