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
#include "bot_procs.h"
#include "m_player.h"
#include "aj_weaponbalancing.h"

void TossClientWeapon (edict_t *self);
qboolean IsFemale (edict_t *ent);
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);

void bot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	float	rnd;

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
	}

	if (gamerules && gamerules->value)	//ScarFace if we're in a dm game, alert the game
	{
		if (DMGame.PlayerDeath)
			DMGame.PlayerDeath(self, inflictor, attacker);
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->client->quadfire_framenum = 0;
	self->client->double_framenum = 0;

	self->s.effects = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

/*
	ClientObituary(self, inflictor, attacker);

//ZOID
	CTFFragBonuses(self, inflictor, attacker);
//ZOID
	TossClientWeapon (self);
//ZOID
	CTFPlayerResetGrapple(self);
	CTFDeadDropFlag(self);
	CTFDeadDropTech(self);
//ZOID

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked weapon model
	self->s.effects = 0;
	self->client->quad_framenum = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;
*/

// check for gib
	if (self->health <= player_gib_health->value)
	{
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

		self->timestamp = level.time;
	}
	else if (!self->deadflag)
	{
		rnd = random() * 3;

		if (self->viewheight < 0)
			self->s.frame = FRAME_crdeath1;
		else if (rnd <= 1)
			self->s.frame = FRAME_death101;
		else if (rnd <= 2)
			self->s.frame = FRAME_death201;
		else
			self->s.frame = FRAME_death301;

		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
	}

	// regular death
	self->deadflag = DEAD_DEAD;

	gi.linkentity(self);
}
