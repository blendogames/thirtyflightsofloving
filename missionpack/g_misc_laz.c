// g_misc_laz.c
// misc entities for the Lazarus mod

#include "g_local.h"

#define START_OFF	1

/*
=============================================================

Lazarus new entities

=============================================================
*/

void misc_light_think (edict_t *self)
{
	if (self->spawnflags & START_OFF)
		return;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLASHLIGHT);
	gi.WritePosition (self->s.origin);
	gi.WriteShort (self - g_edicts);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	self->nextthink = level.time + FRAMETIME;
}

void misc_light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		self->spawnflags &= ~START_OFF;
		self->nextthink = level.time + FRAMETIME;
	}
	else
		self->spawnflags |= START_OFF;
}

void SP_misc_light (edict_t *self)
{
	self->class_id = ENTITY_MISC_LIGHT;

	self->use = misc_light_use;
	if (self->movewith)
		self->movetype = MOVETYPE_PUSH;
	else
		self->movetype = MOVETYPE_NONE;
	self->think = misc_light_think;
	if (!(self->spawnflags & START_OFF))
		self->nextthink = level.time + 2*FRAMETIME;
}

/*
=============================================================

TARGET_PRECIPITATION

=============================================================
*/

#define SF_WEATHER_STARTON        1
#define SF_WEATHER_SPLASH         2
#define SF_WEATHER_GRAVITY_BOUNCE 4
#define SF_WEATHER_FIRE_ONCE      8
#define SF_WEATHER_START_FADE     16
#define STYLE_WEATHER_RAIN        0
#define STYLE_WEATHER_BIGRAIN     1
#define STYLE_WEATHER_SNOW        2
#define STYLE_WEATHER_LEAF        3
#define STYLE_WEATHER_USER        4

void drop_add_to_chain(edict_t *drop)
{
	edict_t	*owner = drop->owner;
	edict_t *parent;

	if (!owner || !owner->inuse || !(owner->spawnflags & SF_WEATHER_STARTON))
	{
		G_FreeEdict(drop);
		return;
	}
	parent = owner;
	while (parent->child)
		parent = parent->child;
	parent->child = drop;
	drop->child = NULL;
	drop->svflags |= SVF_NOCLIENT;
	drop->s.effects &= ~EF_SPHERETRANS;
	drop->s.renderfx &= ~RF_TRANSLUCENT;
	VectorClear(drop->velocity);
	VectorClear(drop->avelocity);
	gi.linkentity(drop);
}

void drop_splash(edict_t *drop)
{
	vec3_t	up = {0,0,1};
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LASER_SPARKS);
	gi.WriteByte (drop->owner->mass2);
	gi.WritePosition (drop->s.origin);
	gi.WriteDir (up);
	gi.WriteByte (drop->owner->sounds);
	gi.multicast (drop->s.origin, MULTICAST_PVS);
	drop_add_to_chain(drop);
}

void leaf_fade2(edict_t *ent)
{
	ent->count++;
	if (ent->count == 1)
	{
		ent->s.effects |= EF_SPHERETRANS;
		ent->nextthink=level.time+0.5;
		gi.linkentity(ent);
	}
	else
		drop_add_to_chain(ent);
}

void leaf_fade (edict_t *ent)
{
	ent->s.renderfx   = RF_TRANSLUCENT;
	ent->think        = leaf_fade2;
	ent->nextthink    = level.time+0.5;
	ent->count        = 0;
	gi.linkentity(ent);
}

void drop_touch(edict_t *drop, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (drop->owner->spawnflags & SF_WEATHER_START_FADE)
		return;
	else if (drop->fadeout > 0)
	{
		if ( (drop->spawnflags & SF_WEATHER_GRAVITY_BOUNCE) && (drop->owner->gravity > 0))
		{
			drop->movetype = MOVETYPE_DEBRIS;
			drop->gravity  = drop->owner->gravity;
		}
		drop->think     = leaf_fade;
		drop->nextthink = level.time + drop->fadeout;
	}
	else if (drop->spawnflags & SF_WEATHER_SPLASH)
		drop_splash(drop);
	else
		drop_add_to_chain(drop);
}

void spawn_precipitation(edict_t *self, vec3_t org, vec3_t dir, float speed)
{
	edict_t *drop;

	if (self->child)
	{
		// Then we already have a currently unused, invisible drop available
		drop = self->child;
		self->child = drop->child;
		drop->child = NULL;
		drop->svflags &= ~SVF_NOCLIENT;
		drop->groundentity = NULL;
	}
	else
	{
		drop = G_Spawn();
		if (self->style == STYLE_WEATHER_BIGRAIN)
			drop->s.modelindex = gi.modelindex ("models/objects/drop/heavy.md2");
		else if (self->style == STYLE_WEATHER_SNOW)
			drop->s.modelindex = gi.modelindex ("models/objects/snow/tris.md2");
		else if (self->style == STYLE_WEATHER_LEAF)
		{
			float	r=random();
			if (r < 0.33)
				drop->s.modelindex = gi.modelindex ("models/objects/leaf1/tris.md2");
			else if (r < 0.66)
				drop->s.modelindex = gi.modelindex ("models/objects/leaf2/tris.md2");
			else
				drop->s.modelindex = gi.modelindex ("models/objects/leaf3/tris.md2");
			VectorSet(drop->mins,-1,-1,-1);
			VectorSet(drop->maxs, 1, 1, 1);
		}
		else if (self->style == STYLE_WEATHER_USER)
			drop->s.modelindex = gi.modelindex(self->usermodel);
		else
			drop->s.modelindex = gi.modelindex ("models/objects/drop/tris.md2");
		drop->classname = "rain drop";
	}
	if (self->gravity > 0.0f || self->attenuation > 0 )
		drop->movetype = MOVETYPE_DEBRIS;
	else
		drop->movetype = MOVETYPE_RAIN;

	drop->touch    = drop_touch;
	if (self->style == STYLE_WEATHER_USER)
		drop->clipmask = MASK_MONSTERSOLID;
	else if ((self->fadeout > 0) && (self->gravity == 0.0f))
		drop->clipmask = MASK_SOLID | CONTENTS_WATER;
	else
		drop->clipmask = MASK_MONSTERSOLID | CONTENTS_WATER;
	drop->solid    = SOLID_BBOX;
	drop->svflags  = SVF_DEADMONSTER;
	VectorSet(drop->mins, -1, -1, -1);
	VectorSet(drop->maxs, 1, 1, 1);

	if (self->spawnflags & SF_WEATHER_GRAVITY_BOUNCE)
		drop->gravity = self->gravity;
	else
		drop->gravity = 0.0f;
	drop->attenuation = self->attenuation;
	drop->mass        = self->mass;
	drop->spawnflags  = self->spawnflags;
	drop->fadeout     = self->fadeout;
	drop->owner       = self;

	VectorCopy (org, drop->s.origin);
	vectoangles(dir, drop->s.angles);
	drop->s.angles[PITCH] -= 90;
	VectorScale (dir, speed, drop->velocity);
	if (self->style == STYLE_WEATHER_LEAF)
	{
		drop->avelocity[PITCH] = crandom() * 360;
		drop->avelocity[YAW] = crandom() * 360;
		drop->avelocity[ROLL] = crandom() * 360;
	}
	else if (self->style == STYLE_WEATHER_USER)
	{
		drop->s.effects = self->effects;
		drop->s.renderfx = self->renderfx;
		drop->avelocity[PITCH] = crandom() * self->pitch_speed;
		drop->avelocity[YAW]   = crandom() * self->yaw_speed;
		drop->avelocity[ROLL]  = crandom() * self->roll_speed;
	}
	else
	{
		drop->s.effects |= EF_SPHERETRANS;
		drop->avelocity[YAW] = self->yaw_speed;
	}
	if (self->spawnflags & SF_WEATHER_START_FADE)
	{
		drop->think     = leaf_fade;
		drop->nextthink = level.time + self->fadeout;
	}
	gi.linkentity(drop);
}

void target_precipitation_think (edict_t *self)
{
	vec3_t	center;
	vec3_t	org;
	int		r, i;
	float	u, v, z;
	float	temp;
	qboolean	can_see_me;

	self->nextthink = level.time + FRAMETIME;

	// Don't start raining until player is in the game. The following 
	// takes care of both initial map load conditions and restored saved games.
	// This is a gross abuse of groundentity_linkcount. Sue me.
	if (g_edicts[1].linkcount == self->groundentity_linkcount)
		return;
	else
		self->groundentity_linkcount = g_edicts[1].linkcount;
	// Don't spawn drops if player can't see us. This SEEMS like an obvious
	// thing to do, but can cause visual problems if mapper isn't careful.
	// For example, placing target_precipitation where it isn't in the PVS
	// of the player's current position, but the result (rain) IS in the
	// PVS. In any case, this step is necessary to prevent overflows when
	// player suddenly encounters rain.
	can_see_me = false;
	for (i=1; i<=game.maxclients && !can_see_me; i++)
	{
		if (!g_edicts[i].inuse) continue;
		if (gi.inPVS(g_edicts[i].s.origin,self->s.origin))
			can_see_me = true;
	}
	if (!can_see_me) return;

	// Count is models/second. We accumulate a probability of a model
	// falling this frame in ->density. Yeah its a misnomer but density isn't 
	// used for anything else so it works fine.

	temp = 0.1*(self->density + crandom()*self->random);
	r = (int)(temp);
	if (r > 0)
		self->density = self->count + (temp-(float)r)*10;
	else
		self->density += (temp*10);
	if (r < 1) return;

	VectorAdd(self->bleft,self->tright,center);
	VectorMA(self->s.origin,0.5,center,center);

	for (i=0; i<r; i++)
	{
		u = crandom() * (self->tright[0] - self->bleft[0])/2;
		v = crandom() * (self->tright[1] - self->bleft[1])/2;
		z = crandom() * (self->tright[2] - self->bleft[2])/2;
		
		VectorCopy(center, org);
		
		org[0] += u;
		org[1] += v;
		org[2] += z;

		spawn_precipitation(self, org, self->movedir, self->speed);
	}
}

void target_precipitation_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->spawnflags & SF_WEATHER_STARTON)
	{
		// already on; turn it off
		ent->nextthink = 0;
		ent->spawnflags &= ~SF_WEATHER_STARTON;
		if (ent->child)
		{
			edict_t	*child, *parent;
			child = ent->child;
			ent->child = NULL;
			while (child)
			{
				parent = child;
				child = parent->child;
				G_FreeEdict(parent);
			}
		}
	}
	else
	{
		ent->density = ent->count;
		ent->think = target_precipitation_think;
		ent->spawnflags |= SF_WEATHER_STARTON;
		ent->think(ent);
	}
}

void target_precipitation_delayed_use (edict_t *self)
{
	// Since target_precipitation tends to be a processor hog,
	// for START_ON we wait until the player has spawned into the
	// game to ease the startup burden somewhat
	if (g_edicts[1].linkcount)
	{
		self->think = target_precipitation_think;
		self->think(self);
	}
	else
		self->nextthink = level.time + FRAMETIME;
}
void SP_target_precipitation (edict_t *ent)
{
	if (deathmatch->value || coop->value)
	{
		G_FreeEdict(ent);
		return;
	}

	ent->class_id = ENTITY_TARGET_PRECIPITATION;

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if (ent->spawnflags & SF_WEATHER_STARTON)
	{
		ent->think = target_precipitation_delayed_use;
		ent->nextthink = level.time + 1;
	}

	if (ent->style == STYLE_WEATHER_USER)
	{
		char	*buffer;
		size_t	bufSize;

		if (!ent->usermodel)
		{
			gi.dprintf("target_precipitation style=user\nwith no usermodel.\n");
			G_FreeEdict(ent);
			return;
		}

		// Knightmare- check for "models/" or "sprites/" already in path
		if ( strncmp(ent->usermodel, "models/", 7) && strncmp(ent->usermodel, "sprites/", 8) )
		{
			bufSize = strlen(ent->usermodel)+10;
			buffer = gi.TagMalloc(bufSize, TAG_LEVEL);
			if (strstr(ent->usermodel,".sp2"))
				Com_sprintf(buffer, bufSize, "sprites/%s", ent->usermodel);
			else
				Com_sprintf(buffer, bufSize, "models/%s", ent->usermodel);
			ent->usermodel = buffer;
		}

		if (st.gravity)
			ent->gravity = atof(st.gravity);
		else
			ent->gravity = 0.0f;
	}
	else
	{
		ent->gravity = 0.0f;
		ent->attenuation = 0.0f;
	}

	// If not rain or "user", turn off splash. Yeah I know goofy mapper
	// might WANT splash, but we're enforcing good taste here :)
	if (ent->style > STYLE_WEATHER_BIGRAIN && ent->style != STYLE_WEATHER_USER)
		ent->spawnflags &= ~SF_WEATHER_SPLASH;

	ent->use = target_precipitation_use;
	
	if (!ent->count)
		ent->count = 1;

	if (!ent->sounds)
		ent->sounds = 2;	// blue splash

	if (!ent->mass2)
		ent->mass2 = 8;		// 8 particles in splash

	if ((ent->style < STYLE_WEATHER_RAIN) || (ent->style > STYLE_WEATHER_USER))
		ent->style = STYLE_WEATHER_RAIN;		// single rain drop model

	if (ent->speed <= 0)
	{
		switch (ent->style)
		{
		case STYLE_WEATHER_SNOW: ent->speed = 50; break;
		case STYLE_WEATHER_LEAF: ent->speed = 50; break;
		default: ent->speed = 300;
		}
	}

	if ((VectorLength(ent->bleft) == 0.) && (VectorLength(ent->tright) == 0.))
	{
		// Default distribution places raindrops vertically for
		// full coverage, to help avoid "lumps"
		VectorSet(ent->bleft,-512,-512, -ent->speed*0.05);
		VectorSet(ent->tright,512, 512,  ent->speed*0.05);
	}

	if (VectorLength(ent->s.angles) > 0)
		G_SetMovedir(ent->s.angles,ent->movedir);
	else
		VectorSet(ent->movedir,0,0,-1);

	ent->density = ent->count;

	gi.linkentity (ent);
}

//=============================================================================
// TARGET_FOUNTAIN is identical to TARGET_PRECIPITATION, with these exceptions:
// ALL styles are "user-defined" (no predefined rain, snow, etc.)
// Models are spawned from a point source, and bleft/tright form a box within
// which the target point is found.
//=============================================================================
void target_fountain_think (edict_t *self)
{
	vec3_t	center;
	vec3_t	org;
	vec3_t	dir;
	int		r, i;
	float	u, v, z;
	float	temp;
	qboolean	can_see_me;

	if (!(self->spawnflags & SF_WEATHER_FIRE_ONCE))
		self->nextthink = level.time + FRAMETIME;

	// Don't start raining until player is in the game. The following 
	// takes care of both initial map load conditions and restored saved games.
	// This is a gross abuse of groundentity_linkcount. Sue me.
	if (g_edicts[1].linkcount == self->groundentity_linkcount)
		return;
	else
		self->groundentity_linkcount = g_edicts[1].linkcount;
	// Don't spawn drops if player can't see us. This SEEMS like an obvious
	// thing to do, but can cause visual problems if mapper isn't careful.
	// For example, placing target_precipitation where it isn't in the PVS
	// of the player's current position, but the result (rain) IS in the
	// PVS. In any case, this step is necessary to prevent overflows when
	// player suddenly encounters rain.
	can_see_me = false;
	for (i=1; i<=game.maxclients && !can_see_me; i++)
	{
		if (!g_edicts[i].inuse) continue;
		if (gi.inPVS(g_edicts[i].s.origin,self->s.origin))
			can_see_me = true;
	}
	if (!can_see_me) return;

	// Count is models/second. We accumulate a probability of a model
	// falling this frame in ->density. Yeah its a misnomer but density isn't 
	// used for anything else so it works fine.

	temp = 0.1*(self->density + crandom()*self->random);
	r = (int)(temp);
	if (r > 0)
		self->density = self->count;
	else
		self->density += (temp*10);
	if (r < 1) return;

	VectorAdd(self->bleft,self->tright,center);
	VectorMA(self->s.origin,0.5,center,center);

	for (i=0; i<r; i++)
	{
		u = crandom() * (self->tright[0] - self->bleft[0])/2;
		v = crandom() * (self->tright[1] - self->bleft[1])/2;
		z = crandom() * (self->tright[2] - self->bleft[2])/2;
		
		VectorCopy(center, org);
		
		org[0] += u;
		org[1] += v;
		org[2] += z;
		VectorSubtract(org,self->s.origin,dir);
		VectorNormalize(dir);

		spawn_precipitation(self, self->s.origin, dir, self->speed);
	}
}

void target_fountain_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	if ((ent->spawnflags & SF_WEATHER_STARTON) && !(ent->spawnflags & SF_WEATHER_FIRE_ONCE))
	{
		// already on; turn it off
		ent->nextthink = 0;
		ent->spawnflags &= ~SF_WEATHER_STARTON;
		if (ent->child)
		{
			edict_t	*child, *parent;
			child = ent->child;
			ent->child = NULL;
			while (child)
			{
				parent = child;
				child = parent->child;
				G_FreeEdict(parent);
			}
		}
	}
	else
	{
		ent->density = ent->count;
		ent->think = target_fountain_think;
		ent->spawnflags |= SF_WEATHER_STARTON;
		ent->think(ent);
	}
}

void target_fountain_delayed_use (edict_t *self)
{
	// Since target_fountain tends to be a processor hog,
	// for START_ON we wait until the player has spawned into the
	// game to ease the startup burden somewhat
	if (g_edicts[1].linkcount)
	{
		self->think = target_fountain_think;
		self->think(self);
	}
	else
		self->nextthink = level.time + FRAMETIME;
}

void SP_target_fountain (edict_t *ent)
{
	char	*buffer;
	size_t	bufSize;

	if (deathmatch->value || coop->value)
	{
		G_FreeEdict(ent);
		return;
	}

	ent->class_id = ENTITY_TARGET_FOUNTAIN;

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if (ent->spawnflags & SF_WEATHER_STARTON)
	{
		ent->think = target_fountain_delayed_use;
		ent->nextthink = level.time + 1;
	}

	ent->style = STYLE_WEATHER_USER;
	if (!ent->usermodel)
	{
		gi.dprintf("target_fountain with no usermodel.\n");
		G_FreeEdict(ent);
		return;
	}

	// Knightmare- check for "models/" or "sprites/" already in path
	if ( strncmp(ent->usermodel, "models/", 7) && strncmp(ent->usermodel, "sprites/", 8) )
	{
		bufSize = strlen(ent->usermodel)+10;
		buffer = gi.TagMalloc(bufSize, TAG_LEVEL);
		if (strstr(ent->usermodel,".sp2"))
			Com_sprintf(buffer, bufSize, "sprites/%s", ent->usermodel);
		else
			Com_sprintf(buffer, bufSize, "models/%s", ent->usermodel);
		ent->usermodel = buffer;
	}

	if (st.gravity)
		ent->gravity = atof(st.gravity);
	else
		ent->gravity = 0.0f;

	ent->use = target_fountain_use;
	
	if (!ent->count)
		ent->count = 1;

	if (!ent->sounds)
		ent->sounds = 2;	// blue splash

	if (!ent->mass2)
		ent->mass2 = 8;		// 8 particles in splash

	if (ent->speed <= 0)
		ent->speed = 300;

	if ((VectorLength(ent->bleft) == 0.) && (VectorLength(ent->tright) == 0.))
	{
		// Default distribution places raindrops vertically for
		// full coverage, to help avoid "lumps"
		VectorSet(ent->bleft,-32, -32, 64);
		VectorSet(ent->tright,32,  32,128);
	}

	ent->density = ent->count;

	gi.linkentity (ent);
}

//
/*=============================================================================

MISC_DEADSOLDIER MODEL PATCH

==============================================================================*/

#define NUM_SKINS		16
#define MAX_SKINNAME	64
#define DEADSOLDIER_MODEL "models/deadbods/dude/tris.md2"

#include "pak.h"

int PatchDeadSoldier (void)
{
	cvar_t		*gamedir;
	char		skins[NUM_SKINS][MAX_SKINNAME];	// skin entries
	char		infilename[MAX_OSPATH];
	char		outfilename[MAX_OSPATH];
	char		tempname[MAX_OSPATH];
	int			j;
//	char		*p;
	FILE		*infile;
	FILE		*outfile;
	dmdl_t		model;				// model header
	byte		*data;				// model data
	int			datasize;			// model data size (bytes)
	int			newoffset;			// model data offset (after skins)

	// get game (moddir) name
	gamedir = gi.cvar("game", "", 0);
	if (!*gamedir->string)
		return 0;	// we're in baseq2

//	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string,DEADSOLDIER_MODEL);
	Com_sprintf (tempname, sizeof(tempname), DEADSOLDIER_MODEL);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	if (outfile = fopen (outfilename, "rb"))
	{
		// output file already exists, move along
		fclose (outfile);
	//	gi.dprintf ("PatchDeadSoldier: Could not save %s, file already exists\n", outfilename);
		return 0;
	}

	for (j = 0; j < NUM_SKINS; j++)
		memset (skins[j], 0, MAX_SKINNAME);

	Com_sprintf (skins[0],  sizeof(skins[0]), "models/deadbods/dude/dead1.pcx");
	Com_sprintf (skins[1],	sizeof(skins[1]), "players/male/cipher.pcx");
	Com_sprintf (skins[2],	sizeof(skins[2]), "players/male/claymore.pcx");
	Com_sprintf (skins[3],	sizeof(skins[3]), "players/male/flak.pcx");
	Com_sprintf (skins[4],	sizeof(skins[4]), "players/male/grunt.pcx");
	Com_sprintf (skins[5],	sizeof(skins[5]), "players/male/howitzer.pcx");
	Com_sprintf (skins[6],	sizeof(skins[6]), "players/male/major.pcx");
	Com_sprintf (skins[7],	sizeof(skins[7]), "players/male/nightops.pcx");
	Com_sprintf (skins[8],	sizeof(skins[8]), "players/male/pointman.pcx");
	Com_sprintf (skins[9],	sizeof(skins[9]), "players/male/psycho.pcx");
	Com_sprintf (skins[10],	sizeof(skins[10]), "players/male/rampage.pcx");
	Com_sprintf (skins[11], sizeof(skins[11]), "players/male/razor.pcx");
	Com_sprintf (skins[12], sizeof(skins[12]), "players/male/recon.pcx");
	Com_sprintf (skins[13], sizeof(skins[13]), "players/male/scout.pcx");
	Com_sprintf (skins[14], sizeof(skins[14]), "players/male/sniper.pcx");
	Com_sprintf (skins[15], sizeof(skins[15]), "players/male/viper.pcx");


	// load original model
	Com_sprintf (infilename, sizeof(infilename), "baseq2/%s", DEADSOLDIER_MODEL);
	if ( !(infile = fopen (infilename, "rb")) )
	{
		// If file doesn't exist on user's hard disk, it must be in 
		// pak0.pak

		pak_header_t	pakheader;
		pak_item_t		pakitem;
		FILE			*fpak;
		int				k, numitems;

		fpak = fopen("baseq2/pak0.pak","rb");
		if (!fpak)
		{
			cvar_t	*cddir;
			char	pakfile[MAX_OSPATH];

			cddir = gi.cvar("cddir", "", 0);
			Com_sprintf(pakfile, sizeof(pakfile), "%s/baseq2/pak0.pak",cddir->string);
			fpak = fopen(pakfile,"rb");
			if (!fpak)
			{
				gi.dprintf("PatchDeadSoldier: Cannot find pak0.pak\n");
				return 0;
			}
		}
		fread(&pakheader,1,sizeof(pak_header_t),fpak);
		numitems = pakheader.dsize/sizeof(pak_item_t);
		fseek(fpak,pakheader.dstart,SEEK_SET);
		data = NULL;
		for (k=0; k<numitems && !data; k++)
		{
			fread(&pakitem,1,sizeof(pak_item_t),fpak);
			if (!Q_stricmp(pakitem.name,DEADSOLDIER_MODEL))
			{
				fseek(fpak,pakitem.start,SEEK_SET);
				fread(&model, sizeof(dmdl_t), 1, fpak);
				datasize = model.ofs_end - model.ofs_skins;
				if ( !(data = malloc (datasize)) )	// make sure freed locally
				{
					fclose(fpak);
					gi.dprintf ("PatchDeadSoldier: Could not allocate memory for model\n");
					return 0;
				}
				fread (data, sizeof (byte), datasize, fpak);
			}
		}
		fclose(fpak);
		if (!data)
		{
			gi.dprintf("PatchDeadSoldier: Could not find %s in baseq2/pak0.pak\n",DEADSOLDIER_MODEL);
			return 0;
		}
	}
	else
	{
		fread (&model, sizeof (dmdl_t), 1, infile);
	
		datasize = model.ofs_end - model.ofs_skins;
		if ( !(data = malloc (datasize)) )	// make sure freed locally
		{
			gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
			return 0;
		}
		fread (data, sizeof (byte), datasize, infile);
	
		fclose (infile);
	}
	
	// update model info
	model.num_skins = NUM_SKINS;
	
	// Already had 1 skin, so new offset doesn't include that one
//	newoffset = (model.num_skins-1) * MAX_SKINNAME;
	newoffset = model.num_skins * MAX_SKINNAME;
	model.ofs_st     += newoffset;
	model.ofs_tris   += newoffset;
	model.ofs_frames += newoffset;
	model.ofs_glcmds += newoffset;
	model.ofs_end    += newoffset;
	
	// save new model
/*	Com_sprintf (outfilename, sizeof(outfilename), "%s/models", gamedir->string);	// make some dirs if needed
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/deadbods");
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/dude");
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, DEADSOLDIER_MODEL);
	p = strstr(outfilename,"/tris.md2");
	*p = 0;
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, DEADSOLDIER_MODEL);
*/	
	Com_sprintf (tempname, sizeof(tempname), DEADSOLDIER_MODEL);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	CreatePath (outfilename);

	if ( !(outfile = fopen (outfilename, "wb")) )
	{
		// file couldn't be created for some other reason
		gi.dprintf ("PatchDeadSoldier: Could not save %s\n", outfilename);
		free (data);
		return 0;
	}
	
	fwrite (&model, sizeof (dmdl_t), 1, outfile);
//	fwrite (skins, sizeof (char), model.num_skins*MAX_SKINNAME, outfile);
	fwrite (skins, sizeof (char), newoffset, outfile);
//	data += MAX_SKINNAME;
	fwrite (data, sizeof (byte), datasize, outfile);
	
	fclose (outfile);
	gi.dprintf ("PatchDeadSoldier: Saved %s\n", outfilename);
	free (data);	// crashes here
	return 1;
}
