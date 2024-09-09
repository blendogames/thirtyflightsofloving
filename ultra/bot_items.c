/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin

 *****************************************************************/

#include "g_local.h"
#include "g_items.h"
#include "p_trail.h"
#include "bot_procs.h"
#include "aj_startmax.h" // AJ

extern gitem_t *titems[4];

void botSetWant(edict_t	*self, int dist_divide)
{
	if (dist_divide <= 1)
		self->movetarget_want = WANT_KINDA;
	else if (dist_divide <= 3)
		self->movetarget_want = WANT_YEH_OK;
	else
		self->movetarget_want = WANT_SHITYEAH;
}

/*
=============
RoamFindBestItem

  Searches for the best reachable item, in list_head
  set check_paths to enable reaching items that aren't visible
  ** be careful not to call this too often, it will slow things down
=============
*/
int	RoamFindBestItem(edict_t	*self, edict_t	*list_head, int	check_paths)
{
	float	closest_dist=999999, this_dist;
	edict_t	*best = NULL, *node = NULL, *trav = NULL;
	int		si;		// node closest to self
	int		dist_divide = 1;		// divide distance by this, simulates wieghts for better targets
	int		best_divide = 0;

	if (!list_head)	// nothing to look for
		return -1;

	si = ClosestNodeToEnt(self, false, false);

	for (trav = list_head; trav; trav = trav->node_target)
	{
		if	(trav->solid != SOLID_TRIGGER)
			continue;

		this_dist = 0;

		//ScarFace- crash protection
		if (!trav || !trav->inuse || !trav->item || !trav->item->pickup)
			continue;

		if (((trav->item->pickup != CTFPickup_Flag) && (trav->item->pickup != Pickup_Weapon) && (trav->item->pickup != Tag_PickupToken))
			&& (this_dist = entdist(self, trav)) > 2000) // too far away
			continue;

		// CTF, if guarding base, don't go too far away
		if (self->target_ent && self->target_ent->item && (self->target_ent->item->pickup == CTFPickup_Flag)
			&& (this_dist > BOT_GUARDING_RANGE))
			continue;
		// CTF

		if (trav->ignore_time >= level.time)
			continue;

		if ((list_head == ammo_head) &&		// make sure we can pickup the ammo
			(!((dist_divide = (2*botHasWeaponForAmmo(self->client, trav->item) + (this_dist < 256))) &&
			   (dist_divide *= 2*botCanPickupAmmo(self->client, trav->item)) &&
			   (dist_divide *= 3*(self->bot_fire == botBlaster)))))	// no weapon to use this ammo, or ammo full
		{
			continue;
		}
		else if (list_head == bonus_head)
		{
			if (trav->item->pickup == Pickup_Armor)
			{
				if (!(dist_divide = botCanPickupArmor(self, trav)))
				{	// can't pickup this armor item (already have better armor)
					dist_divide = 1;
					continue;
				}
			}
			else if (trav->item->pickup == CTFPickup_Flag)
			{
				dist_divide = 6;

				if (self->bot_fire != botBlaster && self->bot_fire != botShotgun)
					dist_divide += 100;

				if (	(	(self->client->resp.ctf_team == CTF_TEAM1)
						 &&	(	(self->client->pers.inventory[ITEM_INDEX(flag2_item)])
							 || (self->client->pers.inventory[ITEM_INDEX(flag3_item)]))
						 &&	(trav == flag1_ent))
					||	(	(self->client->resp.ctf_team == CTF_TEAM2)
						 &&	(	(self->client->pers.inventory[ITEM_INDEX(flag1_item)])
							 || (self->client->pers.inventory[ITEM_INDEX(flag3_item)]))
						 &&	(trav == flag2_ent))
					||	(	(self->client->resp.ctf_team == CTF_TEAM3)
						 &&	(	(self->client->pers.inventory[ITEM_INDEX(flag1_item)])
							 || (self->client->pers.inventory[ITEM_INDEX(flag2_item)]))
						 &&	(trav == flag3_ent)))
				{	// we have their flag, so HEAD FOR OUR FLAG!
					dist_divide += 9999;
				}
				// AJ - added a second equals to the last clause... surely not assignments...
				else if (	((self->client->resp.ctf_team == CTF_TEAM1) && (trav == flag1_ent) && (flag1_ent->solid == SOLID_TRIGGER))
						 || ((self->client->resp.ctf_team == CTF_TEAM2) && (trav == flag2_ent) && (flag2_ent->solid == SOLID_TRIGGER))
						 || ((self->client->resp.ctf_team == CTF_TEAM3) && (trav == flag3_ent) && (flag3_ent->solid == SOLID_TRIGGER)))
				{	// flag is sitting at home, don't try and get it

					continue;
				}
			}
			else if (trav->item->pickup == Tag_PickupToken) //ScarFace- DM tag support
			{
				dist_divide = 10;
			}
			else if (trav->item->pickup == CTFPickup_Tech)
			{
				qboolean has_tech = false;

				if (	(self->client->pers.inventory[ITEM_INDEX(item_tech1)])
					||	(self->client->pers.inventory[ITEM_INDEX(item_tech2)])
					||	(self->client->pers.inventory[ITEM_INDEX(item_tech3)])
					||	(self->client->pers.inventory[ITEM_INDEX(item_tech4)])
					||	(self->client->pers.inventory[ITEM_INDEX(item_tech5)]) // AJ
					||	(self->client->pers.inventory[ITEM_INDEX(item_tech6)])) // Knightmare added
				{
					has_tech = true;
				}

				if (has_tech)	// already have a tech, so ignore
					continue;

				if (!self->client->ctf_has_tech)		
					dist_divide = 10; // AJ - changed from 3... techs are GOOD.
			}
			else // item is a powerup
			{
				
				if (trav->item->use == Use_Invulnerability) //ScarFace
				{
					dist_divide = 5;
				}
				else if (trav->item->use == Use_Quad)
				{
					dist_divide = 4;

					if (skill->value > 1)
						dist_divide += 4 * (skill->value - 1);

					if (self->bot_stats->quad_freak)
						dist_divide *= 2;		// REALLY REALLY WANT THIS SUCKER!!
				}
				else if (trav->item->use == Use_Double || trav->item->use == Use_QuadFire) //ScarFace
				{
					dist_divide = 3;

					if (skill->value > 1)
						dist_divide += 4 * (skill->value - 1);

					if (self->bot_stats->quad_freak)
						dist_divide *= 2;		// REALLY WANT THIS SUCKER!!
				}
				else if (trav->item->use == Use_Defender || trav->item->use == Use_Hunter || trav->item->use == Use_Vengeance) //ScarFace
					dist_divide = 4;
				else if (trav->item->use == Use_Doppleganger) //ScarFace
					dist_divide = 3;
			}

		}
		else if (list_head == weapons_head)	// for weapons, apply weights for good weapons
		{
			// don't go for a (non-droppped) weapon we already have, if in "weapons stay" mode
			if (	((int)(dmflags->value) & DF_WEAPONS_STAY)
				&&	(!(trav->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ))
				&&	(self->client->pers.inventory[ITEM_INDEX(trav->item)]))
			{
				continue;
			}

			else if ((trav->item == item_rocketlauncher) || 
					 (trav->item == item_chaingun) || 
					 (trav->item == item_railgun))
				dist_divide = 4;
			else if (trav->item == item_bfg10k)
			{
				dist_divide = 3;

				if (num_players > 4)	// BFG rocks with lots of people
					dist_divide += 3;
			}

			if (trav->item == self->bot_stats->fav_weapon)
				dist_divide *= 2; // AJ changed from +3 to *2

			if (self->bot_fire == botBlaster)	// we really need a weapon
				dist_divide += 4;
		}
		else if (list_head == health_head)
		{
			if (trav->count == 100)
				dist_divide = 4;
			else if (self->health > (sk_max_health->value - 10))	// don't need it     // AJ - changed health relative to cvar
				continue;
			else if ((self->health > (sk_max_health->value /2)) && check_paths)	// only check routes for health when really low
				continue;
		}

		if ((this_dist < 384) && visible_box(self, trav) && CanReach(self, trav))	// go for it!
		{
			if (trav != self->save_movetarget)
				self->goalentity = trav;
			else
				self->goalentity = self->save_goalentity;

			self->movetarget = trav;

			botSetWant(self, dist_divide);

			this_dist = this_dist/dist_divide;

//			this_dist = this_dist/256;		// always grab something close by

			return this_dist;
		}

		if (!check_paths)
			continue;

		if (si == -1)		// can't see a node, so don't bother checking routes
			continue;

		// don't look for a path to shards
		if (list_head == health_head)
		{
			if (trav->count < 10)
				continue;
		}
		else if (list_head == bonus_head)
		{
			if (trav->item->tag == ARMOR_SHARD)
				continue;
		}

		// see if any of our visible nodes, has a route to one of the
		// item's visible nodes
		if (((this_dist = PathToEnt(trail[si], trav, false, false)) > -1) && ((this_dist / dist_divide) < closest_dist))
		{
			this_dist = this_dist / dist_divide;
			closest_dist = this_dist;
			best = trav;
			node = trail[si];	// go for the nearest node first
			best_divide = dist_divide;

			if (this_dist < 128)	// OPTIMIZE: go for this one!
			{
				if (node != self->save_movetarget)
					self->goalentity = node;
				else
					self->goalentity = self->save_goalentity;

				self->movetarget = best;

				botSetWant(self, dist_divide);

				return this_dist;
			}
		}

//botDebugPrint("Found item %s: %i away\n", trav->classname, (int) this_dist);
	}

	if (best)
	{
//botDebugPrint("Best item %s: %i away\n", best->classname, (int) closest_dist);
		if (node != self->save_movetarget)
			self->goalentity = node;
		else
			self->goalentity = self->save_goalentity;

		self->movetarget = best;

		botSetWant (self, best_divide);

		return closest_dist;
	}
	else
	{
		return -1;
	}
}

// FIXME: these are in g_items.c also!!
extern gitem_armor_t jacketarmor_info;
extern gitem_armor_t combatarmor_info;
extern gitem_armor_t bodyarmor_info;

// returns 0 if this armour is of no use to the bot, a higher number is returned for more useful armour
int botCanPickupArmor (edict_t *self, edict_t *ent)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;
	gclient_t	*client;

	if (self->client)
		client = self->client;
	else
		return false;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (self);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		return 1;
	}
	// if bot has no armor, just use it
	else if (!old_armor_index)
	{
		return 4;	// any armour is FUCKING GOOD armour!
	}
	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			return (int) (((newcount - client->pers.inventory[old_armor_index]) / 50)*3 + 1);
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// armour is useful, return 4 if VERY useful :)
			return (int) (((newcount - client->pers.inventory[old_armor_index]) / 50)*3 + 1);
		}
	}

	return false;
}
