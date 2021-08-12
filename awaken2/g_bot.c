// g_bot.c

//Maj/Pon++
#include "g_local.h"
#include "m_player.h"

// Special thanks to Ponpoko for his brilliant work upon
// which this work is based.

#define MaxOf(x,y) ((x)>(y)?(x):(y))

//CW++
vec3_t VEC_TMINS4 = {-4.0F, -4.0F, -4.0F};
vec3_t VEC_TMAXS4 = { 4.0F,  4.0F,  4.0F};
//CW--

qboolean Bot_Fall(edict_t *ent, vec3_t pos, float dist);

//=====================================================
//=====================================================

botinfo_t	Bot[MAXBOTS+1];
int			NumBotsInGame = 0;			// [1..MAXBOTS]

route_t		Route[MAXNODES];
int			TotalRouteNodes = 0;
int			CurrentIndex = 0;

qboolean	pickup_priority = false;
int			trace_priority = 0;
float		JumpMax = 0.0F;

vec3_t		zvec = {0, 0, 0};
float		myrandom = 0.5f;

int SkillLevel[10] = {
//skill 0
	FIRE_REFUGE  | FIRE_PRESTAYFIRE | FIRE_STAYFIRE,												//CW...
//skill 1
	FIRE_REFUGE  | FIRE_PRESTAYFIRE,
//skill 2
	FIRE_REFUGE,
//skill 3
	FIRE_REFUGE  | FIRE_IGNORE,
//skill 4
	FIRE_REFUGE  | FIRE_IGNORE | FIRE_AVOIDINVULN,
//skill 5
	FIRE_REFUGE  | FIRE_IGNORE | FIRE_AVOIDINVULN | FIRE_AVOIDEXPLO,
//skill 6
	FIRE_JUMPROC | FIRE_IGNORE | FIRE_AVOIDINVULN | FIRE_AVOIDEXPLO,
//skill 7
	FIRE_JUMPROC | FIRE_IGNORE | FIRE_AVOIDINVULN | FIRE_AVOIDEXPLO | FIRE_C4USE,
//skill 8
	FIRE_JUMPROC | FIRE_IGNORE | FIRE_AVOIDINVULN | FIRE_AVOIDEXPLO | FIRE_C4USE | FIRE_DODGE,
//skill 9
	FIRE_JUMPROC | FIRE_IGNORE | FIRE_AVOIDINVULN | FIRE_AVOIDEXPLO | FIRE_C4USE | FIRE_DODGE | FIRE_QUADUSE
};

typedef char	cfg[64];
typedef cfg		cfg_t[2];

cfg_t gbot[] = { // ENTRIES MUST == MAXBOTS, else kabooom!
{	"Tyr574[BOT]",		"cyborg/tyr574"		},
{	"Razor[BOT]",		"male/razor"		},
{	"Cobalt[BOT]",		"female/cobalt"		},
{	"Scout[BOT]",		"male/scout"		},
{	"PS9000[BOT]",		"cyborg/ps9000"		},
{	"Brianna[BOT]",		"female/brianna"	},
{	"Recon[BOT]",		"male/recon"		},
{	"Viper[BOT]",		"male/viper"		},
{	"Oni911[BOT]",		"cyborg/oni911"		},
{	"Flak[BOT]",		"male/flak"			},

{	"Venus[BOT]",		"female/venus"		},
{	"Pointman[BOT]",	"male/pointman"		},
{	"Stiletto[BOT]",	"female/stiletto"	},
{	"Claymore[BOT]",	"male/claymore"		},
{	"Jezebel[BOT]",		"female/jezebel"	},
{	"Cypher[BOT]",		"male/cypher"		},
{	"Athena[BOT]",		"female/athena"		},
{	"Major[BOT]",		"male/major"		},
{	"Jungle[BOT]",		"female/jungle"		},
{	"Howitzer[BOT]",	"male/howitzer"		},

{	"Ensign[BOT]",		"female/ensign"		},
{	"NightOps[BOT]",	"male/nightops"		},
{	"Psycho[BOT]",		"male/psycho"		},
{	"Voodoo[BOT]",		"female/voodoo"		},
{	"Rampage[BOT]",		"male/rampage"		},
{	"Brazen[BOT]",		"cyborg/tyr574"		},
{	"Zeroid[BOT]",		"male/razor"		},
{	"Lotus[BOT]",		"female/lotus"		},
{	"Grunt[BOT]",		"male/grunt"		},

//CW++
{	"Mu[BOT]",			"male/psycho"		},
{	"Wiz[BOT]",			"male/rampage"		},
{	"Monstra[BOT]",		"cyborg/ps9000"		},
{	"Sn33k[BOT]",		"female/jezebel"	},
{	"Sherm[BOT]",		"male/cypher"		},
{	"Bassy[BOT]",		"male/claymore"		},
{	"Mis[BOT]",			"female/lotus"		},
{	"QUIET![BOT]",		"male/major"		},
{	"C.G.[BOT]",		"cyborg/tyr574"		},
{	"Wyld[BOT]",		"male/howitzer"		},
{	"Webdude[BOT]",		"male/claymore"		},
{	"Leadhed[BOT]",		"male/pointman"		},
{	"Yestah[BOT]",		"male/viper"		},
{	"Panzi[BOT]",		"male/nightops"		},
{	"Buzzi[BOT]",		"male/claymore"		},
{	"Sarkastor[BOT]",	"male/viper"		},
{	"Deth[BOT]",		"male/grunt"		},
{	"Flashy[BOT]",		"male/flak"			},
{	"Ripley[BOT]",		"female/brianna"	},
{	"Kryten[BOT]",		"cyborg/ps9000"		},
{	"Zakalwe[BOT]",		"cyborg/oni911"		},	// Count = 50
//CW--
{	"Killer[BOT]",		"male/grunt"		},	// 1 extra for safety
};
//===================================
//===================================


//======================================================
//========== BASIC BOT UTILITY FUNCTIONS ===============
//======================================================

//======================================================
qboolean G_EntExists(edict_t *ent)
{
	return (ent && ent->client && ent->inuse);
}

//======================================================
qboolean G_ClientNotDead(edict_t *ent)
{
	qboolean b1 = (ent->client->ps.pmove.pm_type != PM_DEAD);
	qboolean b2 = (ent->deadflag == DEAD_NO);
	qboolean b3 = (ent->health > 0);

	return (b1 || b2 || b3);																		//CW
}

//======================================================
qboolean G_ClientInGame(edict_t *ent)
{
	if (!G_EntExists(ent))
		return false;

	if (!G_ClientNotDead(ent))
		return false;

	if (ent->client->spectator)																		//CW
		return false;

	return (ent->client->respawn_time + 5.0 < level.time);
}

//==============================================
float Get_yaw(vec3_t vec)
{
	vec3_t	out;
	double	yaw;

	VectorCopy(vec, out);
	out[2] = 0.0;
	VectorNormalize(out);
	yaw = (double)(RAD2DEG(acos((double)out[0])));													//CW

	if (asin((double)out[1]) < 0)
		yaw *= -1.0;

	return (float)yaw;
}

//==============================================
float Get_pitch(vec3_t vec)
{
	vec3_t	out;
	float	pitch;

	VectorCopy(vec, out);
	VectorNormalize(out);
	pitch = (float)(RAD2DEG(acos((double)out[2]))) - 90.0;											//CW

	return (float)((pitch < -180.0) ? (pitch + 360.0) : pitch);
}

//==============================================

float Get_vec_yaw(vec3_t vec, float yaw)
{
	float	vecsyaw;

	vecsyaw = Get_yaw(vec);
	if (vecsyaw > yaw)
		vecsyaw -= yaw;
	else
		vecsyaw = yaw - vecsyaw;

	if (vecsyaw > 180)
		vecsyaw = 360 - vecsyaw;

	return vecsyaw;
}

//======================================================
void AdjustAngle(edict_t *ent, vec3_t targaim, float aim, float angle_gap)							//CW
{
	VectorSet(ent->s.angles, (Get_pitch(targaim)), (Get_yaw(targaim)), 0.0F);

	ent->s.angles[YAW] += aim * angle_gap * (myrandom - 0.5);
	if (ent->s.angles[YAW] > 180.0)
		ent->s.angles[YAW] -= 360.0;
	else if (ent->s.angles[YAW] < -180.0)
		ent->s.angles[YAW] += 360.0;

	ent->s.angles[PITCH] += aim * angle_gap * (myrandom - 0.5);
	if (ent->s.angles[PITCH] > 90.0)
		ent->s.angles[PITCH] = 90.0;
	else if (ent->s.angles[PITCH] < -90.0)
		ent->s.angles[PITCH] = -90.0;
}

//=============================================
qboolean BankCheck(edict_t *ent, vec3_t pos)
{
	trace_t	tr;
	vec3_t	end;

	VectorCopy(pos, end);
	end[2] = -4096.0;																				//CW
	tr = gi.trace(pos, ent->mins, ent->maxs, end, ent, MASK_BOTSOLIDX);

	return !(tr.startsolid || tr.allsolid || (tr.plane.normal[2] < 0.8));
}

//=============================================
qboolean HazardCheck(edict_t *ent, vec3_t pos)
{
	trace_t	tr;
	vec3_t	end;
	int		contents;

	VectorCopy(pos, end);
	end[2] = -4096.0;																				//CW
	contents = (ent->client->enviro_framenum > level.framenum)?CONTENTS_LAVA:(CONTENTS_LAVA|CONTENTS_SLIME);
	tr = gi.trace(pos, ent->mins, ent->maxs, end, ent, MASK_OPAQUE);

	return !(tr.contents & contents);		// true = no hazard detected; false = hazard detected
}

//=============================================
//CW++
qboolean TriggerHurtCheck(edict_t *ent)
{
	edict_t		*trighurt = NULL;
	qboolean	danger_willrobinson = false;
	trace_t		tr;
	vec3_t		end;

	end[0] = ent->s.origin[0];
	end[1] = ent->s.origin[1];
	while ((trighurt = G_Find(trighurt, FOFS(classname), "trigger_hurt")) != NULL)
	{
		if ((ent->s.origin[0] > trighurt->mins[0]) && (ent->s.origin[0] < trighurt->maxs[0]) && 
			(ent->s.origin[1] > trighurt->mins[1]) && (ent->s.origin[1] < trighurt->maxs[1]) &&
			(ent->s.origin[2] - ent->mins[2] > trighurt->maxs[2]))
		{
			end[2] = trighurt->maxs[2];
			tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SOLID);
			if (tr.fraction == 1.0)
			{
				danger_willrobinson = true;
				break;
			}
		}
	}

	return danger_willrobinson;		// true = trigger_hurt detected below; false = no trigger_hurt detected
}
//CW--

//==============================================
void SetBotAnim(edict_t *ent)
{
	gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);													//CW++

	if (ent->client->anim_priority >= ANIM_JUMP)
		return;

	ent->s.frame = FRAME_jump1 - 1;
	ent->client->anim_end = FRAME_jump6;
}

//============================================================
qboolean Get_FlyingSpeed(float bottom, float block, float dist, float *speed)
{
	float	tdist;

	if (bottom >= 40)
	{
		if (block > 4) return false;
		tdist = (dist * block) * 0.250;
	}
	else if (bottom >= 35)
	{
		if (block > 5) return false;
		tdist = (dist * block) * 0.200;
	}
	else if (bottom >= 30)
	{
		if (block > 6) return false;
		tdist = (dist * block) * 0.167;
	}
	else if (bottom >= 20)
	{
		if (block > 7) return false;
		tdist = (dist * block) * 0.143;
	}
	else if (bottom >= -5)
	{
		if (block > 8) return false;
		tdist = (dist * block) * 0.125;
	}
	else if (bottom >= -20)
	{
		if (block > 9) return false;
		tdist = (dist * block) * 0.143;
	}
	else if (bottom >= -35)
	{
		if (block > 10) return false;
		tdist = (dist * block) * 0.167;
	}
	else if (bottom >= -52)
	{
		if (block > 11) return false;
		tdist = (dist * block) * 0.200;
	}
	else if (bottom >= -75)
	{
		if (block > 12) return false;
		tdist = (dist * block) * 0.250;
	}
	else if (bottom >= -95)
	{
		if (block > 13) return false;
		tdist = (dist * block) * 0.333;
	}
	else if (bottom >=-125)
	{
		if (block > 14) return false;
		tdist = (dist * block) * 0.500;
	}
	else
	{
		if (block > 15) return false;
		tdist = (dist * block) * 0.500;
	}

	*speed = tdist / 30.0;

	return true;
}

//==========================================
float SetBotXYSpeed(edict_t *ent, float *xyspeed)
{
	if (!ent->isabot)
		return *xyspeed;

//CW++
	if (ent->tractored)
		return *xyspeed;

	if (ent->client->held_by_agm || ent->client->flung_by_agm || ent->client->thrown_by_agm)
		return *xyspeed;
//CW--

	if (ent->groundentity && (ent->client->movestate & STS_WAITS))
	{
		*xyspeed = (VectorLength(ent->groundentity->velocity) < 1) ? 300 : 0;
		if (*xyspeed)
			ent->client->movestate |= STS_W_DONT;		// don't wait
	}
	else
		*xyspeed = (ent->client->camptime > level.time) ? 0 : 300;

	return *xyspeed;
}

//==========================================
void SetBotThink(edict_t *ent)
{
	if (!ent->isabot)
		return;

	ent->client->chattime = level.time + (10.0 * (rand() % 7));
	ent->client->ping = atoi(Info_ValueForKey(ent->client->pers.userinfo, "ping"));
	ent->think = Bot_Think;
	ent->nextthink = level.time + FRAMETIME;
}

//==========================================
void ForceRouteReset (edict_t *other)
{
	if (!other->isabot)
		return;

	if (!other->client->routetrace)
		return;

	if (other->client->pers.routeindex < TotalRouteNodes)
	{
		if (Route[other->client->pers.routeindex].state == GRS_TELEPORT)
			other->client->pers.routeindex++;

		if (other->client->pers.routeindex < TotalRouteNodes)
		{
			if (Route[other->client->pers.routeindex].state == GRS_GRAPRELEASE)
				other->client->pers.routeindex++;
		}
	}
}

//==========================================
void G_FindTrainTeam (void)																			//CW (various bugfixes)
{
	static edict_t	*teamlist[MAX_EDICTS + 1];	// Knightmare- made static due to stack size
	edict_t			*e;
	edict_t			*t;
	edict_t			*p;
	static char		*targethist[MAX_EDICTS];	// Knightmare- made static due to stack size
	char			*currtarget;
	char			*currtargetname;
	qboolean		findteam = false;
	int				loopindex;
	int				lc;
	int				i;
	int				j;
	int				k;

	e = &g_edicts[(int)maxclients->value+1];
	for (i = (int)maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (e->inuse && e->classname)
		{
			if ((e->touch == path_corner_touch) && e->targetname && e->target)
			{
				currtarget = e->target;
				currtargetname = e->targetname;

				memset(&teamlist, 0, sizeof(teamlist));
				memset(&targethist, 0, sizeof(targethist));
				targethist[0] = e->targetname;

				lc = 0;
				loopindex = 0;
				while (lc < MAX_EDICTS)
				{
					t = &g_edicts[(int)maxclients->value+1];
					for (j = (int)maxclients->value + 1; j < globals.num_edicts; j++, t++)
					{
						if (t->inuse && t->classname)
						{
							if ((t->use == train_use) && !Q_stricmp(t->target, currtargetname) && (t->trainteam == NULL))
							{
								for (k = 0; k < lc; k++)
								{
									if (teamlist[k] == t)
										break;
								}

								if (k == lc)
								{
									teamlist[lc] = t;
									lc++;
								}
							}
						}
					}

					p = G_PickTarget(currtarget);
					if (!p)
						break;

					currtarget = p->target;
					currtargetname = p->targetname;
					if (!p->target)
						break;

					for (k = 0; k < loopindex; k++)
					{
						if (!Q_stricmp(targethist[k], currtargetname))
							break;
					}

					if (k < loopindex)
					{
						findteam = true;
						break;
					}

					targethist[loopindex] = currtargetname;
					loopindex++;
				}

				if (findteam && (lc > 0))
				{
					gi.dprintf("%i train chainings found.\n", lc);
					for (k = 0; k < lc; k++)
					{
						if (teamlist[k+1] == NULL)
						{
							teamlist[k]->trainteam = teamlist[0];
							break;
						}

						teamlist[k]->trainteam = teamlist[k+1];
					}
				}
			}
		}
	}
}

//==============================================
void droptofloor2(edict_t *ent)
{
	trace_t	tr;
	vec3_t	trmin;
	vec3_t	trmax;
	vec3_t	min;
	vec3_t	mins;
	vec3_t	maxs;
	vec3_t	dest;
	vec3_t	v;
	float	i;
	float	j = 0;
	float	yaw;	

	VectorSet(ent->mins, -15, -15, -15);
	VectorSet(ent->maxs, 8, 8, 15);

	if (ent->union_ent && !(ent->item == item_navi2))												//CW
	{
		dest[0] = (ent->union_ent->s.origin[0] + ent->union_ent->mins[0] + ent->union_ent->s.origin[0] + ent->union_ent->maxs[0]) * 0.5;
		dest[1] = (ent->union_ent->s.origin[1] + ent->union_ent->mins[1] + ent->union_ent->s.origin[1] + ent->union_ent->maxs[1]) * 0.5;
		for (i = ent->union_ent->s.origin[2] + ent->union_ent->mins[2]; i <= ent->union_ent->s.origin[2] + ent->union_ent->maxs[2] + 16; i++)
		{
			dest[2] = i;
			tr = gi.trace(dest, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
			if (!tr.startsolid && !tr.allsolid && (j == 1))
			{
				j = 2;
				break;
			}
			else if ((tr.startsolid || tr.allsolid) && !j && (tr.ent == ent->union_ent))			//CW
				j = 1;
		}

		VectorCopy(dest, ent->s.origin);
		VectorSubtract(ent->s.origin, ent->union_ent->s.origin, ent->moveinfo.dir);
	}

	ent->s.modelindex = 0;
	ent->solid = (ent->item == item_navi3) ? SOLID_NOT : SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;
	ent->use = NULL;

	VectorSet(v, 0, 0, -128);
	VectorAdd(ent->s.origin, v, dest);

	tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid && (ent->classname[0] != 'R' && ent->classname[6] != '1'))					//CW
	{
		gi.dprintf("droptofloor2: %s startsolid at %s.\n", (ent->classname)?ent->classname:"unknown", vtosf(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}

	VectorCopy(tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & 2)		// NO_TOUCH
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & 1)		// TRIGGER_SPAWN
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity(ent);

	VectorCopy(ent->s.origin, min);
	VectorSet(mins, -15, -15, -15);
	VectorSet(maxs, 8, 8, 0);
	min[2] -= 128;

	for (i = 0; i < 8; i++)
	{
		if (i < 4)
		{
			yaw = DEG2RAD((90 * i) - 180);
			for (j = 32; j < 80; j += 2)
			{
				trmin[0] = ent->s.origin[0] + cos(yaw) * j;
				trmin[1] = ent->s.origin[1] + sin(yaw) * j;
				trmin[2] = ent->s.origin[2];
				VectorCopy(trmin, trmax);
				trmax[2] -= 128;
				tr = gi.trace(trmin, mins, maxs, trmax, ent, MASK_PLAYERSOLID);
				if ((tr.endpos[2] < ent->s.origin[2] - 16) && (tr.endpos[2] > min[2]))
				{
					if (!tr.allsolid && !tr.startsolid)
					{
						min[2] = tr.endpos[2];
						min[0] = ent->s.origin[0] + cos(yaw) * (j + 16);
						min[1] = ent->s.origin[1] + sin(yaw) * (j + 16);
						break;
					}
				}
			}
		}
		else
		{
			yaw = DEG2RAD((90 * (i - 4)) - 135);
			for (j = 32; j < 80; j += 2)
			{
				trmin[0] = ent->s.origin[0] + cos(yaw) * 46;
				trmin[1] = ent->s.origin[1] + sin(yaw) * 46;
				trmin[2] = ent->s.origin[2];
				VectorCopy(trmin, trmax);
				trmax[2] -= 128;
				tr = gi.trace(trmin, NULL, NULL, trmax, ent, MASK_PLAYERSOLID);
				if ((tr.endpos[2] < ent->s.origin[2] - 16) && (tr.endpos[2] > min[2]))
				{
					if (!tr.allsolid && !tr.startsolid)
					{
						VectorCopy(tr.endpos, min);
						break;
					}
				}
			}
		}
	}

	VectorCopy(min, ent->moveinfo.start_origin);
}

//==============================================
void TraceAllSolid(edict_t *ent, vec3_t point, trace_t tr)
{
	if (tr.allsolid)
	{
		trace_t	tracep;
		vec3_t	stp;
		vec3_t	v1;
		vec3_t	v2;		

		VectorSet(v1, -16, -16, -24);
		VectorSet(v2, 16, 16, 4);
		VectorCopy(ent->s.origin, stp);
		stp[2] += 24;
		tracep = gi.trace(stp, v1, v2, point, ent, MASK_BOTSOLID);

		if (tracep.ent && !tracep.allsolid)
		{
			if (tracep.ent->classname[0] == 'f')
			{
				VectorCopy(tracep.endpos, ent->s.origin);
				ent->groundentity = tracep.ent;
				ent->groundentity_linkcount = tracep.ent->linkcount;
				gi.linkentity(ent);
				return;
			}
		}
	}
	else
	{
		if (ent->client)
		{
			ent->client->ground_contents = tr.contents;
			ent->client->ground_slope = tr.plane.normal[2];
		}

		VectorCopy(tr.endpos, ent->s.origin);
		ent->groundentity = tr.ent;
		ent->groundentity_linkcount = tr.ent->linkcount;
	}

	gi.linkentity(ent);
}

//==============================================
void ResetGroundSlope(edict_t *ent)
{
	if (!ent->isabot)
		return;

	ent->client->ground_slope = 1.0;
}

//==============================================
void SpawnItem3(edict_t *it_ent, gitem_t *item)
{
	it_ent->item = item;
	it_ent->s.effects = 0;
	it_ent->s.renderfx = 0;
	it_ent->s.modelindex = 0;
	it_ent->nextthink = level.time + 0.2;
	it_ent->think = droptofloor2;
}

//===============================================
void bFuncTrain(edict_t *self)
{
	gitem_t	*it = item_navi1;
	edict_t	*it_ent = G_Spawn();

	VectorAdd(self->s.origin, self->mins, self->monsterinfo.last_sighting);
	it_ent->classname = it->classname;
	it_ent->union_ent = self;
	self->union_ent = it_ent;
	SpawnItem3(it_ent, it);
}

//===============================================
void bFuncDoor(edict_t *ent)
{
	VectorAdd(ent->s.origin, ent->mins, ent->monsterinfo.last_sighting);
	if (fabs(ent->moveinfo.start_origin[2] - ent->moveinfo.end_origin[2]) > 20.0)
	{
		gitem_t	*it = item_navi3;
		edict_t	*it_ent = G_Spawn();

		it_ent->classname = it->classname;
		it_ent->union_ent = ent;
		ent->union_ent = it_ent;
		SpawnItem3(it_ent, it);
	}
}

//===============================================
void bDoorBlocked(edict_t *self)
{
	edict_t	*ent;																					//CW
	int		i;

	for (i = 1; i <= maxclients->value; i++)
	{
		ent = &g_edicts[i];																			//CW
		if (!ent->isabot)
			continue;

		if (ent->client->waiting_obj != self)
			continue;

		if (!ent->client->movestate)
			continue;

		ent->client->movestate |= STS_W_DONT;
	}
}

//===============================================
void bFuncButton(edict_t *ent)
{
	edict_t	*it_ent = G_Spawn();
	gitem_t	*it = item_navi2;
	vec3_t	tdir;
	vec3_t	tdir2;
	vec3_t	abs_movedir;
	float	dist = 1.0;

	it_ent->classname = it->classname;
	VectorAdd(ent->s.origin, ent->mins, ent->monsterinfo.last_sighting);
	VectorCopy(ent->s.origin, it_ent->s.origin);
	it_ent->s.origin[0] = (ent->absmin[0] + ent->absmax[0]) * 0.5;
	it_ent->s.origin[1] = (ent->absmin[1] + ent->absmax[1]) * 0.5;
	it_ent->s.origin[2] = (ent->absmin[2] + ent->absmax[2]) * 0.5;

	it_ent->union_ent = ent;
	ent->union_ent = it_ent;
	VectorSubtract(ent->moveinfo.start_origin, ent->moveinfo.end_origin, abs_movedir);
	VectorNormalize(abs_movedir);

	while (dist < 500.0)
	{
		VectorScale(abs_movedir, dist, tdir);
		VectorAdd(it_ent->s.origin, tdir, tdir2);
		if (!(gi.pointcontents(tdir2) & CONTENTS_SOLID))
			break;

		dist++;
	}

	VectorScale(abs_movedir, (dist + 20), tdir);
	VectorAdd(it_ent->s.origin, tdir, tdir2);
	VectorCopy(tdir2,it_ent->s.origin);

	it_ent->item = it;
	it_ent->s.effects = 0;
	it_ent->s.renderfx = 0;
	it_ent->s.modelindex = 0;
	it_ent->solid = SOLID_TRIGGER;
	it_ent->movetype = MOVETYPE_NONE;
	it_ent->touch = Touch_Item;
	gi.linkentity(it_ent);
}

//===============================================
void bFuncPlat(edict_t *ent)
{
	edict_t	*it_ent = G_Spawn();
	gitem_t	*it = item_navi1;

	VectorAdd(ent->s.origin, ent->mins, ent->monsterinfo.last_sighting);
	it_ent->classname = it->classname;
	it_ent->union_ent = ent;
	ent->union_ent = it_ent;
	SpawnItem3(it_ent, it);
}

//===============================================
void CheckBotCrushed(edict_t *targ,edict_t *inflictor, int mod)
{
	if (!targ->isabot || (mod != MOD_CRUSH))
		return;

	if (((targ->client->waiting_obj == inflictor) && targ->client->movestate) || (targ->groundentity == inflictor))
		targ->client->movestate |= STS_W_DONT;
}

//===============================================
void CheckPrimaryWeapon(edict_t *ent, edict_t *other)
{
	if (!other->isabot)
		return;

	if (ent->item->use)
	{
		// switch weapon if picked up primary
		int wepnum = GetKindWeapon(ent->item);

		if (wepnum != MOD_CHAINSAW)																	//CW
		{
			if (Bot[other->client->pers.botindex].skill[PRIMARYWEAP] == wepnum)						//CW
				ent->item->use(other, ent->item);
		}
	}
}

//===============================================
void Bot_CheckEnemy(gclient_t *client, edict_t *attacker, edict_t *targ, int mod)
{
	if (client && targ->isabot && attacker)
	{
		if (client->battlemode & FIRE_CHICKEN)														//CW
			client->battlemode &= ~FIRE_CHICKEN;

//CW++
//		If tractored, target the trap.

		if (mod == MOD_TRAP)
		{
			edict_t	*echeck;
			int		i;

			for (i = 0; i < globals.num_edicts; ++i)
			{
				echeck = &g_edicts[i];
				if (!echeck->inuse)
					continue;
				if (echeck->client)
					continue;
				if (!echeck->die)
					continue;

				if ((echeck->die == Trap_DieFromDamage) && (echeck->enemy == targ))
				{
					client->current_enemy = echeck;
					break;
				}
			}
		}

//		If in lava/slime/hurt zone and have a Personal Teleporter, use it.

		else if ((mod == MOD_LAVA) || (mod == MOD_SLIME) || (mod == MOD_TRIGGER_HURT))
		{
			if (targ->client->pers.inventory[ITEM_INDEX(item_teleporter)])
				Use_Teleporter(targ, item_teleporter);
		}
		else if (attacker->client && !(attacker->flags & FL_NOTARGET))
		{
			qboolean switch_enemy;

			switch_enemy = ((mod == MOD_SR_HOMING)		|| (mod == MOD_SR_DISINT_WAVE)	|| (mod == MOD_RAILGUN)			|| (mod == MOD_ROCKET)			||
							(mod == MOD_GAUSS_BLASTER)	|| (mod == MOD_GAUSS_BEAM)		|| (mod == MOD_FLAMETHROWER)	|| (mod == MOD_CHAINSAW)		|| 
							(mod == MOD_AGM_DISRUPT)	|| (mod == MOD_AGM_LAVA_HELD)	|| (mod == MOD_AGM_SLIME_HELD)	|| (mod == MOD_AGM_WATER_HELD)	|| 
							(mod == MOD_AGM_TRIG_HURT)	|| (mod == MOD_AGM_TARG_LASER));
			switch_enemy = switch_enemy && (client->current_enemy != attacker);

			if (!client->current_enemy || (client->current_enemy && switch_enemy && (client->current_enemy->health > 25)))
			{
				if (!CheckTeamDamage(targ, attacker))
					client->current_enemy = attacker;
			}
		}
//CW--
	}
}

//===============================================
void CheckCampSite(edict_t *ent, edict_t *other)
{
	if (!other->isabot)
		return;

//CW++
	if (!(int)sv_bots_camp->value)
		return;

	if (!Bot[other->client->pers.botindex].camper)
		return;

	if (other->client->camping)
		return;
//CW--

	if ((ent->item != item_health_mega) && (ent->item != item_railgun) && (ent->item != item_bodyarmor) && (ent->item != item_powershield))	//CW
		return;

	if (other->client->quad_framenum > level.framenum)
		return;

	if (other->client->camptime >= level.time)
		return;

	if (random() > BOT_CAMP_PROB)																	//CW
		return;

	other->client->camptime = level.time + 20.0 + (rand() % 11);	// 20..30
	other->client->chattime = level.time + (rand() % 16);			// 0..15
	other->client->taunttime = other->client->camptime + 10.0;		// turn taunting off whilst camping
	
	VectorCopy(ent->s.origin, other->client->lastorigin);
	other->client->lastorigin[2] += 16.0;
	other->client->campitem = ent->item;							//camping near this item
	other->client->camping = true;																	//CW++
}


//======================================================
//======== ROUTE FILE AND BOT CONFIGURATION ============
//======================================================

void LoadBotConfig(void)
{
	FILE		*filestream;
	char		name[512];
	char		skin[512];
	int			fnum;
	int			num_bots = 0;
	int			combat_skill = 0;
	int			accuracy = 0;
	int			aggression = 0;
	int			weapon = 1;
	int			h_view = 90;
	int			v_view = 90;
	int			camper = 1;
	qboolean	finished = false;

	if ((filestream = OpenBotConfigFile(true, true)) != NULL)
	{
		memset(&name, 0, sizeof(name));
		memset(&skin, 0, sizeof(skin));

		while (!finished && ((fnum = fscanf(filestream, "%s %s %d %d %d %d %d %d %d", name, skin, &combat_skill, &accuracy, &aggression, &weapon, &h_view, &v_view, &camper)) != EOF))
		{
			if (strlen(name) >= MAX_NAMELEN)
			{
				gi.dprintf("** Bot name in line %d is too long.\n", num_bots + 1);
				name[MAX_NAMELEN-1] = 0;
			}

			if (strlen(skin) >= MAX_SKINLEN)
			{
				gi.dprintf("** Bot skin in line %d is too long.\n", num_bots + 1);
				skin[MAX_SKINLEN-1] = 0;
			}

			memset(&Bot[num_bots], 0, sizeof(botinfo_t));
			strncpy(Bot[num_bots].netname, name, MAX_NAMELEN);										//r1,CW
			strncpy(Bot[num_bots].skin, skin, MAX_SKINLEN);											//r1,CW

			//combat skill [0..9]
			if (combat_skill < 0)
				combat_skill = 0;
			else if (combat_skill > 9)
				combat_skill = 9;
			Bot[num_bots].skill[COMBATSKILL] = combat_skill;

			//aiming accuracy [0..9]
			if (accuracy < 0)
				accuracy = 0;
			else if (accuracy > 9)
				accuracy = 9;
			Bot[num_bots].skill[AIMACCURACY] = accuracy;

			//aggression [0..9]
			if (aggression < 0)
				aggression = 0;
			else if (aggression > 9)
				aggression = 9;
			Bot[num_bots].skill[AGGRESSION] = aggression;

			//primary weapon [1..14]
			if (weapon < 1)
				weapon = 1;
			else if (weapon > 14)
				weapon = 14;
			Bot[num_bots].skill[PRIMARYWEAP] = weapon;

			//horizontal view range [0..180]
			if (h_view < 0)
				h_view = 0;
			else if (h_view > 180)
				h_view = 180;
			Bot[num_bots].skill[HRANGEVIEW] = h_view;

			//vertical view range [0..180]
			if (v_view < 0)
				v_view = 0;
			else if (v_view > 180)
				v_view = 180;
			Bot[num_bots].skill[VRANGEVIEW] = v_view;

			//camping tendency [0 or 1]
			if (camper < 0)
				camper = 0;
			else if (camper > 1)
				camper = 1;
			Bot[num_bots].camper = (qboolean)camper;

			gi.dprintf("Loaded bot: %s\n", Bot[num_bots].netname);
			if (++num_bots == MAXBOTS)
			{
				gi.dprintf("** Maximum number of bots loaded!\n");
				finished = true;
			}

			memset(&name, 0, sizeof(name));
			memset(&skin, 0, sizeof(skin));
		}
		fclose(filestream);

		if (num_bots == 0)
			gi.dprintf("** Bot config file has no valid entries\n");

		gi.dprintf("\n");
	}
}

//===============================================
void RandomizeParameters(int i)
{
//CW++	(rewritten)
	int	primary_weapon;

	Bot[i].skill[AIMACCURACY] = rand() % 10;				// [0..9]
	Bot[i].skill[AGGRESSION]  = rand() % 10;				// [0..9]
	Bot[i].skill[COMBATSKILL] = rand() % 10;				// [0..9]
	Bot[i].skill[VRANGEVIEW]  = 60 + (10 * (rand() % 7));	// [60..120]
	Bot[i].skill[HRANGEVIEW]  = 60 + (10 * (rand() % 7));	// [60..120]
	Bot[i].camper = rand() % 2;								// [0 or 1]

	do
	{
		primary_weapon = 2 + (rand() % 13);					// [2..14]
	}
	while (primary_weapon == WEAP_TRAP);					// traps should never be a primary weapon (unless you're Bass[MaN] ;-)
	Bot[i].skill[PRIMARYWEAP] = primary_weapon;
}
//CW--

//==============================================
void LoadBotNames(void) 
{
	int i;

	for (i = 0; i < MAXBOTS; i++)
	{
		memset(&Bot[i], 0, sizeof(botinfo_t));
		Com_sprintf(Bot[i].netname, sizeof(Bot[i].netname), "%s", gbot[i][0]);
		Com_sprintf(Bot[i].skin, sizeof(Bot[i].skin), "%s", gbot[i][1]);
		RandomizeParameters(i);
	}
}

//CW++
void LoadBots(void)
{
	LoadBotNames();
	if ((int)sv_bots_use_file->value)
		LoadBotConfig();
}
//CW--


//==================================================
qboolean RTJump_Chk(vec3_t apos, vec3_t tpos)
{
	float	x;
	float	l;
	float	vel;
	float	yori;
	vec3_t	v;
	vec3_t	vv;
	int		mf = 0;

	vel = VEL_BOT_JUMP;
	yori = apos[2];
	VectorSubtract(tpos, apos, v);

	for (x = 1; x <= BOT_FALLCHK_LOOPMAX * 2; ++x)
	{
		vel -= sv_gravity->value * FRAMETIME;
		yori += vel * 0.1;
		if (vel > 0)
		{
			if (mf == 0)
			{
				if (tpos[2] < yori)
					mf = 2;
			}
		}
		else if (x > 1)
		{
			if (mf == 0)
			{
				if (tpos[2] < yori)
					mf = 2;
			}
			else if (mf == 2)
			{
				if (tpos[2] >= yori)
				{
					mf = 1;
					break;
				}
			}
		}
	}

	VectorCopy(v, vv);
	vv[2] = 0;
	l = VectorLength(vv);
	if (x > 1)
		l /= (x - 1);

	return ((l < MOVE_SPD_RUN) && (mf == 1));
}

//==============================================
void G_FindRouteLink(edict_t *ent)
{
	trace_t		rs_trace;
	vec3_t		v;
	float		x;
	qboolean	tpbool;
	int			i;
	int			j;
	int			k;
	int			l;
	int			total = 0;

	gi.dprintf("Linking routes...");

	// get JumpMax
	if (JumpMax == 0)
	{
		x = VEL_BOT_JUMP - (ent->gravity * sv_gravity->value * FRAMETIME);
		JumpMax = 0;
		while (1)
		{
			JumpMax += x * FRAMETIME;
			x -= ent->gravity * sv_gravity->value * FRAMETIME;
			if (x < 0)
				break;
		}
	}

	// search
	for (i = 0; i < CurrentIndex; i++)
	{
		if (Route[i].state == GRS_NORMAL)
		{
			for (j = 0; j < CurrentIndex; j++)
			{
				if ((abs(i - j) <= 50) || (j == i) || (Route[j].state != GRS_NORMAL))
					continue;

				VectorSubtract(Route[j].Pt, Route[i].Pt, v);
				if ((v[2] > JumpMax) || (v[2] < -500))
					continue;

				v[2] = 0;
				if (VectorLength(v) > 200)
					continue;

				if ((fabs(v[2]) > 20) || (VectorLength(v) > 64))
				{
					if (!RTJump_Chk(Route[i].Pt, Route[j].Pt))
						continue;
				}

				tpbool = false;
				for (l = -5; l < 6; l++)
				{
					if ((i + l < 0) || (i + l >= CurrentIndex))
						continue;

					for (k = 0; k < MAXLINKPOD; k++)												//CW
					{	//search blanked index
						if (!Route[i + l].linkpod[k])
							break;

						if (abs(Route[i + l].linkpod[k] - j) < 50)
						{
							tpbool = true;
							break;
						}
					}

					if (tpbool)
						break;
				}

				if (tpbool)
					continue;

				rs_trace = gi.trace(Route[j].Pt, NULL, NULL, Route[i].Pt, ent, MASK_SOLID);

				// found!
				if (!rs_trace.startsolid && !rs_trace.allsolid && (rs_trace.fraction == 1.0))
				{
					for (k = 0; k < MAXLINKPOD; k++)												//CW
					{	//search blanked index
						if (!Route[i].linkpod[k])
						{
							Route[i].linkpod[k] = j;
							total++;
							break;
						}
					}
				}
			}
		}
	}

	gi.dprintf("done!\n");																			//CW
	G_FreeEdict(ent);
}

//==================================================
void ReadRouteFile(void)
{
	edict_t	*e;
	FILE	*fp;
	char	name[MAX_OSPATH];																		//CW
	vec3_t	v;
	int		i;
	int		j;

//CW++
	cvar_t	*game;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(name, sizeof(name), "%s/botroutes/%s.chn", GAMEVERSION, level.mapname);
	else
		Com_sprintf(name, sizeof(name), "%s/botroutes/%s.chn", game->string, level.mapname);
//CW--

	TotalRouteNodes = 0;

	if ((fp = fopen(name, "rb")) != NULL)															//CW
	{
		char		code[8];
		unsigned	int size;

		CurrentIndex = 0;
		memset(Route, 0, sizeof(Route));															//CW
		memset(code, 0, 8);

		fread(code, sizeof(char), 8, fp);
		fread(&CurrentIndex, sizeof(int), 1, fp);
		size = (unsigned int)CurrentIndex * sizeof(route_t);
		fread(Route, size, 1, fp);
		fclose(fp);

		TotalRouteNodes = CurrentIndex;
		gi.dprintf("AwakenBots: %d route nodes for map\n", TotalRouteNodes);
	}

	if (TotalRouteNodes == 0)
	{
		gi.dprintf("AwakenBots: No route file loaded\n");											//CW
		return;
	}

	for (i = 0; i < TotalRouteNodes; i++)
	{
		if (((Route[i].state > GRS_TELEPORT) && (Route[i].state <= GRS_PUSHBUTTON)) || (Route[i].state == GRS_REDFLAG) || (Route[i].state ==GRS_BLUEFLAG))
		{
			edict_t *other = &g_edicts[(int)maxclients->value+1];

			for (j = maxclients->value+1; j < globals.num_edicts; j++, other++)
			{
				if (other && other->inuse)
				{
					if ((Route[i].state == GRS_ONPLAT) || (Route[i].state == GRS_ONTRAIN) || (Route[i].state == GRS_ONDOOR) || (Route[i].state == GRS_PUSHBUTTON))
					{
						VectorAdd(other->s.origin, other->mins, v);
						if (VectorCompare(Route[i].Pt, v))
						{
							if ((Route[i].state == GRS_ONPLAT) && (!Q_stricmp(other->classname, "func_plat") || !Q_stricmp(other->classname, "func_plat2")) )
							{
								Route[i].ent = other;
								break;
							}
							else if ((Route[i].state == GRS_ONTRAIN) && !Q_stricmp(other->classname, "func_train"))
							{
								Route[i].ent = other;
								break;
							}
							else if ((Route[i].state == GRS_ONDOOR) && !Q_stricmp(other->classname, "func_door"))
							{
								Route[i].ent = other;
								break;
							}
							else if ((Route[i].state == GRS_PUSHBUTTON) && !Q_stricmp(other->classname, "func_button"))
							{
								Route[i].ent = other;
								break;
							}
						}
					}
					else if ((Route[i].state == GRS_ITEMS) || (Route[i].state == GRS_REDFLAG) || (Route[i].state == GRS_BLUEFLAG))
					{
						if (VectorCompare(Route[i].Pt, other->monsterinfo.last_sighting))
						{
							Route[i].ent = other;
							break;
						}
					}
				}
			}

			if (j >= globals.num_edicts)
				Route[i].state = GRS_NORMAL;
		}
	}

	e = G_Spawn();
	e->think = G_FindRouteLink;
	e->nextthink = level.time + (FRAMETIME * 2.0);
	e->svflags |= SVF_NOCLIENT;																		//CW
}

void Move_LastRouteIndex(void)
{
	int	i;

	for (i = CurrentIndex - 1; i >= 0; i--)
	{
		if (Route[i].state)
			break;
		else if (!Route[i].index)
			break;
	}

	if (!CurrentIndex || !Route[i].index)
		CurrentIndex = i;
	else
		CurrentIndex = i + 1;

	if (CurrentIndex < MAXNODES)
	{
		memset(&Route[CurrentIndex], 0, sizeof(route_t));
		if (CurrentIndex > 0)
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1; 
	}
}


//======================================================
//============= SPAWNING BOTS INTO THE GAME ============
//======================================================

//======================================================
char *Random_IP(void)
{
	static	char ipstr[16];
	int		ip1;

	do
	{
		ip1 = 128 + (rand() % 128);
	} while ((ip1 == 192) || (ip1 == 172));

	Com_sprintf(ipstr, sizeof(ipstr), "%d.%d.%d.%d", ip1, (int)(rand()%256), (int)(rand()%256), (int)(rand()%256));

	return ipstr;
}

//=============================================
int GetFreeEdict(void)
{
//CW++
	edict_t	*ent;
//CW--

	int		i;

	for (i = (int)(game.maxclients-1); i >= 0; i--)
	{
		ent = g_edicts + i + 1; 																	//CW
		if (!ent->inuse)
		{
			G_InitEdict(ent);
			return i;
		}
	}

	return -1; // refuse connection
}

//======================================================
void G_MuzzleFlash(short rec_no, vec3_t origin, int flashnum)
{
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(rec_no);
	gi.WriteByte(flashnum);
	gi.multicast(origin, MULTICAST_PVS);
}

//=============================================
qboolean SpawnBot(int botindex)																		//CW...
{
	edict_t	*ent;
	char	userinfo[512];
	int		clientnum;

	if ((botindex < 0) || (botindex > MAXBOTS - 1))
		return false;

//CW++
//	Add bot to a public slot.

	if ((int)sv_reserved->value > 0)
	{
		if (g_public_used < (int)maxclients->value - (int)sv_reserved->value)
		{	// public slot
			clientnum = GetFreeEdict();
			if (clientnum < 0)
			{
				gi.dprintf("AddBots: server is full.\n");
				return false;
			}

			ent = g_edicts + clientnum + 1;
			ent->client = &game.clients[clientnum];
			ent->isabot = true;
			g_slots[(int)sv_reserved->value + g_public_used] = ent;
			++g_public_used;
		}
		else
		{	// no free slots
			gi.dprintf("AddBots: all player slots are taken.\n");
			return false;
		}
	}
	else
	{
		if (g_public_used < (int)maxclients->value)
		{	// public slot
			clientnum = GetFreeEdict();
			if (clientnum < 0)
			{
				gi.dprintf("AddBots: server is full.\n");
				return false;
			}

			ent = g_edicts + clientnum + 1;
			ent->client = &game.clients[clientnum];
			ent->isabot = true;
			g_slots[g_public_used] = ent;
			++g_public_used;
		}
		else
		{	// no free slots
			gi.dprintf("AddBots: all player slots are taken.\n");
			return false;
		}
	}
//CW--

	InitClientResp(ent->client);
	InitClientPersistant(ent->client);

	ent->client->pers.botindex = botindex;
	ent->client->pers.routeindex = 0;

//CW++
	ent->client->pers.connected = true;
	ent->client->normal_rockets = true;
	ent->client->agm_disrupt = true;

	if (teamgame.match > MATCH_NONE)
		ent->client->resp.ready = true;
//CW--

	Com_sprintf(userinfo, sizeof(userinfo), "\\name\\%s\\skin\\%s\\fov\\90\\hand\\2\\ip\\%s\\ping\\%3d", Bot[botindex].netname, Bot[botindex].skin, Random_IP(), (int)(100+(rand()%128)));	// Maj++ - We store ping in userinfo string. hehe
	ClientUserinfoChanged(ent, userinfo);															//CW++

	PutClientInServer(ent);
	gi_bprintf(PRINT_HIGH, "%s was added to the game\n", Bot[botindex].netname);					//CW
	if (!(int)chedit->value)
		G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, (int)(MZ_LOGIN));

	ClientEndServerFrame(ent);

	return true;
}

//=============================================
// Temp edict to re-insert active bots into game 
//=============================================
void PutNextBotInGame(edict_t *ent)
{
	if (ent->count >= MAXBOTS)
	{
		G_FreeEdict(ent);
		return;
	}

	while (ent->count < MAXBOTS)
	{
		if (Bot[ent->count].ingame)
		{
			SpawnBot(ent->count++);
			break;
		}

		ent->count++;
	}

	ent->nextthink = level.time + 0.5;
}

void RespawnAllBots(void)
{
	edict_t *ent;

	ent = G_Spawn();
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->count = 0;
	ent->nextthink = level.time + 1.0;
	ent->think = PutNextBotInGame;
	ent->classname = "bot_respawner";																//CW++
}


//CW++
//==================================================
qboolean RemoveBot(edict_t *ent)
{
	if (!ent || !ent->isabot)
		return false;

	Bot[ent->client->pers.botindex].ingame = 0;
	ClientDisconnect(ent);
	ent->isabot = false;
	--NumBotsInGame;

	return true;
}

//=============================================
// Temp edict to remove bots from the game 
// without causing overflows.
//=============================================
void BotRemover_Think(edict_t *self)
{
	edict_t	*ent;
	int		i;

	if (self->count < 1)
	{
		G_FreeEdict(self);
		return;
	}

	if (NumBotsInGame == 0)
	{
		G_FreeEdict(self);
		return;
	}

	for (i = 1; i <= game.maxclients; i++)
	{
		ent = &g_edicts[i];
		if (RemoveBot(ent))
			break;
	}

	--self->count;
	self->nextthink = level.time + FRAMETIME;
}

void RemoveNumBots_Safe(int numbots)
{
	edict_t *ent;
	edict_t *current_remover = NULL;

	if (numbots < 1)
		return;

	if (NumBotsInGame <= 0)
	{
		gi.cprintf(NULL, PRINT_HIGH, "There are no bots spawned!\n");
		return;
	}

	if ((current_remover = G_Find(NULL, FOFS(classname), "bot_remover")) == NULL)
	{
		ent = G_Spawn();
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->classname = "bot_remover";
		ent->count = numbots;

		ent->think = BotRemover_Think;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
	{
		current_remover->count += numbots;
		if (current_remover->count > MAXBOTS)
			current_remover->count = MAXBOTS;
	}
}


//=============================================
// Temp edict to spawn bots into the game 
// without causing overflows.
//=============================================
void BotSpawner_Think(edict_t *self)
{
	int	n;

	if (self->count < 1)
	{
		G_FreeEdict(self);
		return;
	}

	if (NumBotsInGame >= MAXBOTS)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Maximum bots spawned!\n");
		G_FreeEdict(self);
		return;
	}

	if ((int)sv_bots_random->value)
	{
		do
		{
			n = (int)(rand() % MAXBOTS);
		} while (Bot[n].ingame);
	}
	else
		n = NumBotsInGame;

	if (!SpawnBot(n))
	{
		G_FreeEdict(self);
		return;
	}

	Bot[n].ingame = 1;
	NumBotsInGame++;

	--self->count;
	self->nextthink = level.time + FRAMETIME;
}

void SpawnNumBots_Safe(int numbots)
{
	edict_t *ent;
	edict_t *current_spawner = NULL;

	if (numbots < 1)
		return;

	if (NumBotsInGame >= MAXBOTS)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Maximum bots spawned!\n");
		return;
	}

	if ((current_spawner = G_Find(NULL, FOFS(classname), "bot_spawner")) == NULL)
	{
		ent = G_Spawn();
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->classname = "bot_spawner";
		ent->count = numbots;

		ent->think = BotSpawner_Think;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
	{
		current_spawner->count += numbots;
		if (current_spawner->count > MAXBOTS)
			current_spawner->count = MAXBOTS;
	}
}

 
//=======================================================
//============ TAUNTING/CHATTING/INSULTING ==============
//=======================================================

//==============================================
void InsultVictim(edict_t *ent, edict_t *victim)
{
	if (!ent->isabot)
		return;

	if (!victim || !victim->client)
		return;

	if (ent->client->insulttime > level.time)
		return;

	if (victim == ent)		// bot killed self
	{
		if (myrandom < 0.2)
		{
			switch (rand() % 8)
			{
				case 0: gi_bprintf(3, "%s: OUCH!!!\n", ent->client->pers.netname); break;
				case 1: gi_bprintf(3, "%s: I hate that!\n", ent->client->pers.netname); break;
				case 2: gi_bprintf(3, "%s: Shit!\n", ent->client->pers.netname); break;
				case 3: gi_bprintf(3, "%s: Not again!\n", ent->client->pers.netname); break;
				case 4: gi_bprintf(3, "%s: Ugghhhh!\n", ent->client->pers.netname); break;
				case 5: gi_bprintf(3, "%s: WTF!\n", ent->client->pers.netname); break;
				case 6: gi_bprintf(3, "%s: Ohhhhh!\n", ent->client->pers.netname); break;
				case 7: gi_bprintf(3, "%s: DOH!\n", ent->client->pers.netname); break;				//CW++
			}
		}
	} 
	else					// insults to other players
	{
		if (myrandom < 0.75)
		{
			switch (rand() % 20)
			{
				case 0:  gi_bprintf(3, "%s: You REALLY suck!\n", ent->client->pers.netname); break;
				case 1:  gi_bprintf(3, "%s: YOU SUCK!\n", ent->client->pers.netname); break;
				case 2:  gi_bprintf(3, "%s: Suck THIS!\n", ent->client->pers.netname); break;
				case 3:  gi_bprintf(3, "%s: This sucks!\n", ent->client->pers.netname); break;
				case 4:  gi_bprintf(3, "%s: Eat Me!\n", ent->client->pers.netname); break;
				case 5:  gi_bprintf(3, "%s: You ALL suck!\n", ent->client->pers.netname); break;
				case 6:  gi_bprintf(3, "%s: Suck THAT!\n", ent->client->pers.netname); break;
				case 7:  gi_bprintf(3, "%s: Muhhhhaahhhaaa\n", ent->client->pers.netname); break;
				case 8:  gi_bprintf(3, "%s: Muhaaaaaaaaa!!\n", ent->client->pers.netname); break;
				case 9:  gi_bprintf(3, "%s: Huuuhhhaaaaaa!!\n", ent->client->pers.netname); break;
				case 10: gi_bprintf(3, "%s: Muhhhhhhaaaaa!!\n", ent->client->pers.netname); break;
				case 11: gi_bprintf(3, "%s: Whoooooaaaaa!!\n", ent->client->pers.netname); break;
				case 12: gi_bprintf(3, "%s: Your sister!!\n", ent->client->pers.netname); break;
				case 13: gi_bprintf(3, "%s: Your daughter!!\n", ent->client->pers.netname); break;
				case 14: gi_bprintf(3, "%s: Your mama!!\n", ent->client->pers.netname); break;
				case 15: gi_bprintf(3, "%s: Arggggghhhh!\n", ent->client->pers.netname); break;
				case 16: gi_bprintf(3, "%s: Your daddy!!\n", ent->client->pers.netname); break;
				case 17: gi_bprintf(3, "%s: Bite Me!\n", ent->client->pers.netname); break;
				case 18: gi_bprintf(3, "%s: Haaa Haaaaa!\n", ent->client->pers.netname); break;
				case 19: gi_bprintf(3, "%s: Just testing!\n", ent->client->pers.netname); break;	//CW++
			}
		}
	}

	ent->client->insulttime = level.time + 60.0 + (10.0 * (rand() % 7));
}

//==============================================
void TauntVictim(edict_t *ent, edict_t *victim)
{
	vec3_t	vtmp;

	if (!ent->isabot || (ent == victim))
		return;

	if (!victim || !victim->client)
		return;

	if (ent->client->taunttime > level.time)
		return;

//	Only taunt if near the victim (don't reset timer).

	VectorSubtract(ent->s.origin, victim->s.origin, vtmp);
	if (VectorLength(vtmp) > BOT_MAX_TAUNT_DIST)													//CW
		return;

	switch (rand() % 3)
	{
		case 0:
			ent->s.frame = FRAME_flip01 - 1;
			ent->client->anim_end = FRAME_flip12;
			break;

		case 1:
			ent->s.frame = FRAME_salute01 - 1;
			ent->client->anim_end = FRAME_salute11;
			break;

		case 2:
			ent->s.frame = FRAME_taunt01 - 1;
			ent->client->anim_end = FRAME_taunt17;
			break;
	}

	ent->client->taunttime = level.time + 30.0 + (10.0 * (rand() % 7));
}

//==============================================
void RandomChat(edict_t *ent)
{
	if (ent->client->chattime > level.time)
		return;

	if (ent->client->camptime > level.time)
	{
		if (random() < 0.50)		// camp and chat 50% of the time
		{
			if (ent->client->campitem == item_railgun)
			{
				switch (rand() % 6)
				{
					case 0: gi_bprintf(3, "%s: Bring firewood next time!\n", ent->client->pers.netname); break;
					case 1: gi_bprintf(3, "%s: Want a roasted weener?\n", ent->client->pers.netname); break;
					case 2: gi_bprintf(3, "%s: Want a beer with that?\n", ent->client->pers.netname); break;
					case 3: gi_bprintf(3, "%s: Don't ya just love it?\n", ent->client->pers.netname); break;
					case 4: gi_bprintf(3, "%s: Get the camper at the Railgun!\n", ent->client->pers.netname); break;
					case 5: gi_bprintf(3, "%s: There's a camper at the Railgun!\n", ent->client->pers.netname); break;
				}
			}
			else if (ent->client->campitem == item_health_mega)
			{
				switch (rand() % 6)
				{
					case 0: gi_bprintf(3, "%s: Kill the MegaHealth camper!\n", ent->client->pers.netname); break;
					case 1: gi_bprintf(3, "%s: Want marshmallows?\n", ent->client->pers.netname); break;
					case 2: gi_bprintf(3, "%s: Got hotdogs?\n", ent->client->pers.netname); break;
					case 3: gi_bprintf(3, "%s: Damn campers!\n", ent->client->pers.netname); break;
					case 4: gi_bprintf(3, "%s: Get the camper by the MegaHealth!\n", ent->client->pers.netname); break;
					case 5: gi_bprintf(3, "%s: Camper at the MegaHealth!\n", ent->client->pers.netname); break;
				}
			}
//CW++
			else if (ent->client->campitem == item_bodyarmor)
			{
				switch (rand() % 6)
				{
					case 0: gi_bprintf(3, "%s: Kill the RA camper!\n", ent->client->pers.netname); break;
					case 1: gi_bprintf(3, "%s: Want marshmallows?\n", ent->client->pers.netname); break;
					case 2: gi_bprintf(3, "%s: Got hotdogs?\n", ent->client->pers.netname); break;
					case 3: gi_bprintf(3, "%s: Damn campers!\n", ent->client->pers.netname); break;
					case 4: gi_bprintf(3, "%s: Get the camper by RA!\n", ent->client->pers.netname); break;
					case 5: gi_bprintf(3, "%s: Camper at RA!\n", ent->client->pers.netname); break;
				}
			}
			else if (ent->client->campitem == item_powershield)
			{
				switch (rand() % 6)
				{
					case 0: gi_bprintf(3, "%s: Kill the PowerShield camper!\n", ent->client->pers.netname); break;
					case 1: gi_bprintf(3, "%s: Want marshmallows?\n", ent->client->pers.netname); break;
					case 2: gi_bprintf(3, "%s: Got hotdogs?\n", ent->client->pers.netname); break;
					case 3: gi_bprintf(3, "%s: Damn campers!\n", ent->client->pers.netname); break;
					case 4: gi_bprintf(3, "%s: Get the camper by the PowerShield!\n", ent->client->pers.netname); break;
					case 5: gi_bprintf(3, "%s: Camper at the PowerShield!\n", ent->client->pers.netname); break;
				}
			}
//CW--
		}

		ent->client->chattime = level.time + 5.0 + (rand() % 6);
	}
	else
	{
		if (random() < 0.25)
		{
			switch (rand() % 6)
			{
				case 0: gi_bprintf(3, "%s: Bunch of chickenshits!\n", ent->client->pers.netname); break;
				case 1: gi_bprintf(3, "%s: Come and get it!\n", ent->client->pers.netname); break;
				case 2: gi_bprintf(3, "%s: Who wants a piece of me?\n", ent->client->pers.netname); break;
				case 3: gi_bprintf(3, "%s: Where'd everybody go?\n", ent->client->pers.netname); break;
				case 4: gi_bprintf(3, "%s: This server sucks!\n", ent->client->pers.netname); break;
				case 5: gi_bprintf(3, "%s: Only pussies on this server!\n", ent->client->pers.netname); break;
			}
		}

		ent->client->chattime = level.time + 60.0 + (10.0 * (rand() % 7));
	}
}


//=====================================================
//=========== BASIC TRACING ALGORITHMS ================
//=====================================================

//==============================================
qboolean InSight(edict_t *ent, edict_t *other)
{
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;

	if (other->client && !G_ClientInGame(other))
		return false;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight - 8.0;

	VectorCopy(other->s.origin, end);
	end[2] += other->viewheight - 8.0;

	if (gi.pointcontents(start) & CONTENTS_WATER)													//CW
	{
		if (!other->waterlevel)
		{
			tr = gi.trace(end, NULL, NULL, start, ent, CONTENTS_WINDOW | MASK_OPAQUE | CONTENTS_WATER);
			if (tr.surface && (tr.surface->flags & SURF_WARP))
				return false;

			tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_WINDOW | MASK_OPAQUE);
			return (tr.fraction == 1.0);
		}
		else
		{
			VectorCopy(other->s.origin, end);
			end[2] -= 16.0;
			tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_SOLID | CONTENTS_WINDOW);
			return (tr.fraction == 1.0);
		}
	}

	if (other->waterlevel)
	{
		VectorCopy(other->s.origin, end);
		end[2] += 32.0;
		tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_WATER);
		if (tr.surface && (tr.surface->flags & SURF_WARP))
			return false;
	}

	return (gi.trace(start, NULL, NULL, end, ent, CONTENTS_WINDOW | MASK_OPAQUE).fraction == 1.0);
}

//==============================================
qboolean Bot_trace(edict_t *ent, edict_t *other)
{
	trace_t	tr;
	vec3_t	ttx;
	vec3_t	tty;

//CW++
	if (!other)
		return false;
//CW--

	VectorCopy(ent->s.origin, ttx);
	VectorCopy(other->s.origin, tty);
	if (ent->maxs[2] >= 32)
	{
		if (tty[2] > ttx[2])
			tty[2] += 16;

		ttx[2] += 30;
	}
	else
		ttx[2] -= 12;

	tr = gi.trace(ttx, NULL, NULL, tty, ent, CONTENTS_WINDOW | MASK_OPAQUE);
	if ((tr.fraction == 1.0) && !tr.allsolid && !tr.startsolid)
		return true;

	if (ent->maxs[2] < 32)
		return false;

	if (tr.ent && (tr.ent->use == door_use))
	{
		if (!tr.ent->targetname)
			return true;
	}

	if ((ent->s.origin[2] < other->s.origin[2]) || (ent->s.origin[2] - 24 > other->s.origin[2]))
		return false;

	ttx[2] -= 36;
	tr = gi.trace(ttx, NULL, NULL, other->s.origin, ent, CONTENTS_WINDOW | MASK_OPAQUE);
	return ((tr.fraction == 1.0) && !tr.allsolid && !tr.startsolid);
}

//==============================================
qboolean Bot_trace2(edict_t *ent, vec3_t ttz)
{
	vec3_t	ttx;

	VectorCopy(ent->s.origin, ttx);
	ttx[2] += (ent->maxs[2] >= 32) ? 24 : -12;

	return (gi.trace(ttx, NULL, NULL, ttz ,ent, MASK_OPAQUE).fraction == 1.0);
}

//==================================================
qboolean TraceX(edict_t *ent, vec3_t p2)
{
	trace_t	tr;
	vec3_t	v1;
	vec3_t	v2;
	int		contents = (CONTENTS_SOLID | CONTENTS_WINDOW);

	if (ent->svflags & ~SVF_MONSTER)
	{
		if (ent->client->waterstate)
		{
			VectorCopy(ent->mins, v1);
			VectorCopy(ent->maxs, v2);
		}
		else
		{
			if (ent->client->ps.pmove.pm_flags & ~PMF_DUCKED)
			{
				VectorSet(v1, -16, -16, -4);
				VectorSet(v2, 16, 16, 32);
			}
			else
			{
				VectorSet(v1, -4, -4, -4);
				VectorSet(v2, 4, 4, 4);
			}
		}
	}
	else
	{
		VectorClear(v1);
		VectorClear(v2);
		contents |= (CONTENTS_LAVA | CONTENTS_SLIME);
	}

	tr = gi.trace(ent->s.origin, v1, v2, p2, ent, contents);
	if ((tr.fraction == 1.0) && !tr.allsolid && !tr.startsolid)
		return true;

	if (ent->client->routetrace)
	{
		if (ent->svflags & SVF_MONSTER)
		{
			if (tr.ent && (tr.ent->use == door_use))
				return (tr.ent->moveinfo.state == PSTATE_UP);
		}
	}

	return false;
}

//============================================================
void Get_RouteOrigin(int index, vec3_t pos)
{
	edict_t	*e;

	// when normal or items
	if ((Route[index].state <= GRS_ITEMS) || (Route[index].state >= GRS_GRAPSHOT))
	{
		if (Route[index].state == GRS_ITEMS)
		{
			VectorCopy(Route[index].ent->s.origin, pos);
			pos[2] += 8;
		}
		else
			VectorCopy(Route[index].Pt, pos);
	}

	switch (Route[index].state)
	{	// when plat
		case GRS_ONPLAT:
			VectorCopy(Route[index].ent->union_ent->s.origin, pos);
			pos[2] += 8;
			return;

		// when train
		case GRS_ONTRAIN:
			if (!Route[index].ent->trainteam)
			{
				VectorCopy(Route[index].ent->union_ent->s.origin,pos);
				pos[2] += 8;
				return;
			}

			if (Route[index].ent->target_ent)
			{
				if (VectorCompare(Route[index].Tcorner, Route[index].ent->target_ent->s.origin))
				{
					VectorCopy(Route[index].ent->union_ent->s.origin, pos);
					pos[2] += 8;
					return;
				}
			}

			e = Route[index].ent->trainteam;
			while (1)
			{
				if (e == Route[index].ent)
					break;

				if (e->target_ent)
				{
					if (VectorCompare(Route[index].Tcorner, e->target_ent->s.origin))
					{
						VectorCopy(e->union_ent->s.origin, pos);
						pos[2] += 8;
						Route[index].ent = e;
						return;
					}
				}
				e = e->trainteam;
			}

			VectorCopy(Route[index].ent->union_ent->s.origin,pos);
			pos[2] += 8;
			return;

		case GRS_ONDOOR:
			if (Route[index].ent->union_ent)
			{
				VectorCopy(Route[index].ent->union_ent->s.origin, pos);
				pos[2] += 8;
			}
			else if (index + 1 < TotalRouteNodes)
			{
				if (Route[index+1].state <= GRS_ITEMS)
				{
					VectorCopy(Route[index+1].Pt, pos);
					pos[2] += 8;
				}
				else if (Route[index+1].state <= GRS_ONTRAIN)
				{	//when plat or train
					VectorCopy(Route[index+1].ent->union_ent->s.origin, pos);
					pos[2] += 8;
				}
				else if (Route[index+1].state == GRS_PUSHBUTTON)
				{
					VectorCopy(Route[index+1].ent->union_ent->s.origin, pos);
					pos[2] += 8;
				}
				else
					VectorCopy(Route[index+1].Pt,pos);
			}
			else
			{
				pos[0] = (Route[index].ent->absmin[0] + Route[index].ent->absmax[0]) * 0.5;
				pos[1] = (Route[index].ent->absmin[1] + Route[index].ent->absmax[1]) * 0.5;
				pos[2] =  Route[index].ent->absmax[2];
			}
			return;

		case GRS_PUSHBUTTON:
			VectorCopy(Route[index].ent->union_ent->s.origin, pos);
	}
}

//==============================================
void GetAimAngle(edict_t *ent, float aim, float dist)
{
	vec3_t	targaim;
	trace_t	tr;
	int		weapon;

	weapon = GetKindWeapon(ent->client->pers.weapon);

	switch (weapon)
	{
		case WEAP_CHAINSAW:
		case WEAP_DESERTEAGLE:																		//CW...
		case WEAP_GAUSSPISTOL:
		case WEAP_JACKHAMMER:
		case WEAP_MAC10:
		case WEAP_RAILGUN:
		case WEAP_AGM:
			if (ent->client->current_enemy != ent->client->prev_enemy)
			{
				if (ent->client->current_enemy->isabot)
					VectorSubtract(ent->client->current_enemy->s.old_origin, ent->client->current_enemy->s.origin, targaim);
				else
				{
					VectorCopy(ent->client->current_enemy->velocity, targaim);
					VectorInverse(targaim);
				}

				VectorNormalize(targaim);
				VectorMA(ent->client->current_enemy->s.origin, AIM_SFPOS * aim * myrandom, targaim, targaim);
			}
			else
			{
				VectorSubtract(ent->client->targ_old_origin, ent->client->current_enemy->s.origin, targaim);
				VectorMA(ent->client->current_enemy->s.origin, aim * myrandom, targaim, targaim);
			}

			VectorSubtract(targaim, ent->s.origin, targaim);
			AdjustAngle(ent, targaim, aim, AIM_SFANG_HITSCAN);
			break;

		case WEAP_C4:																				//CW...
		case WEAP_SHOCKRIFLE:
		case WEAP_ROCKETLAUNCHER:
		case WEAP_DISCLAUNCHER:
		case WEAP_ESG:
		case WEAP_FLAMETHROWER:
			if (ent->client->current_enemy != ent->client->prev_enemy)
			{
				if (ent->client->current_enemy->isabot)
					VectorSubtract(ent->client->current_enemy->s.origin, ent->client->current_enemy->s.old_origin, targaim);
				else
				{
					VectorCopy(ent->client->current_enemy->velocity, targaim);
					VectorScale(targaim, 32.0, targaim);
				}

				VectorNormalize(targaim);
				VectorMA(ent->client->current_enemy->s.origin, (11 - aim) * (dist / 25), targaim, targaim);
			}
			else
			{
				VectorSubtract(ent->client->current_enemy->s.origin, ent->client->targ_old_origin, targaim);
				targaim[2] *= 0.5;
				VectorMA(ent->client->current_enemy->s.origin, (-aim * myrandom) + (dist / 75), targaim, targaim);
			}

			tr = gi.trace(ent->client->current_enemy->s.origin, NULL, NULL, targaim, ent->client->current_enemy, MASK_SHOT);
			VectorCopy(tr.endpos, targaim);

			if (weapon == WEAP_ROCKETLAUNCHER)														//CW
			{
				if (targaim[2] < (ent->s.origin[2] + JumpMax))
				{
					vec3_t	vtmp;

					targaim[2] -= 24;
					VectorCopy(ent->s.origin, vtmp);
					vtmp[2] += ent->viewheight - 8;
					tr = gi.trace(vtmp, NULL, NULL, targaim, ent, MASK_SHOT);
					if (tr.fraction != 1.0)
						targaim[2] += 24;
				}
				else
				{
					if (targaim[2] > ent->s.origin[2] + JumpMax)
						targaim[2] += 5;
				}
			}

			VectorSubtract(targaim, ent->s.origin, targaim);
			AdjustAngle(ent, targaim, aim, AIM_SFANG_PROJ);
			break;

		case WEAP_TRAP:																				//CW
			VectorCopy(ent->client->vtemp, targaim);
			VectorSubtract(targaim, ent->s.origin, targaim);
			VectorSet(ent->s.angles, (Get_pitch(targaim)), (Get_yaw(targaim)), 0.0F);
			break;

		default:
			break;
	}
}

//=================================================
qboolean HasAmmoForWeapon(edict_t *self, gitem_t *weapon)
{
	if (weapon == item_chainsaw)																	//CW
		return true;

	if ((int)dmflags->value & DF_INFINITE_AMMO)
		return true;
	
	return (self->client->pers.inventory[ITEM_INDEX(FindItem(weapon->ammo))] >= weapon->quantity);
}

//========================================================
gitem_t *GetWeaponType(int weapnum)
{
	switch (weapnum)
	{
		case WEAP_CHAINSAW:			return item_chainsaw;											//CW...
		case WEAP_DESERTEAGLE:		return item_deserteagle;
		case WEAP_GAUSSPISTOL:		return item_gausspistol;
		case WEAP_JACKHAMMER:		return item_jackhammer;
		case WEAP_MAC10:			return item_mac10;
		case WEAP_ESG:				return item_esg;
		case WEAP_C4:				return item_c4;
		case WEAP_TRAP:				return item_trap;
		case WEAP_ROCKETLAUNCHER:	return item_rocketlauncher;
		case WEAP_FLAMETHROWER:		return item_flamethrower;
		case WEAP_RAILGUN:			return item_railgun;
		case WEAP_SHOCKRIFLE:		return item_shockrifle;
		case WEAP_DISCLAUNCHER:		return item_disclauncher;
		case WEAP_AGM:				return item_agm;
		case WEAP_GRAPPLE:			return item_grapple;
	}

	return item_chainsaw;
}

//==============================================
qboolean CanUseWeapon(edict_t *ent, int weapnum)
{
	gitem_t *weapon;

//CW++
	if ((weapnum == WEAP_FLAMETHROWER) && (ent->waterlevel > 1))
		return false;
//CW--

	weapon = GetWeaponType(weapnum);
	if (ent->client->pers.inventory[ITEM_INDEX(weapon)])
	{
		if (HasAmmoForWeapon(ent, weapon))
		{
			if ((ent->client->weaponstate == WEAPON_READY) || (ent->client->weaponstate == WEAPON_FIRING))	//CW
			{
				if (ent->client->pers.weapon != weapon)
				{
					ent->client->newweapon = weapon;
					ChangeWeapon(ent);
				}

				return true;		// true whether switched or not
			}
		}
	}

	return false;
}

//CW++
void UsePrimaryWeapon(edict_t *ent)
{
	int	my_weapon = GetKindWeapon(ent->client->pers.weapon);
	int	primary_weapon = Bot[ent->client->pers.botindex].skill[PRIMARYWEAP];

	if (my_weapon != primary_weapon)
		CanUseWeapon(ent, primary_weapon);
}
//CW--

//==============================================
qboolean Pickup_Navi(edict_t *ent, edict_t *other)
{
	int i;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		if (ent->item->quantity)
			SetRespawn(ent, ent->item->quantity);
	}

	// on door (up & down)
	if ((ent->item == item_navi3) && ent->union_ent)
	{
		qboolean	flg = false;
		int			j;
		int			k;

		if (ent->target_ent == other)
		{
			other->client->movestate &= ~STS_WAITS;
			other->client->waiting_obj = ent->union_ent;
			if (ent->union_ent->spawnflags & PDOOR_TOGGLE)
			{
				if ((ent->union_ent->moveinfo.state == PSTATE_DOWN) || (ent->union_ent->moveinfo.state == PSTATE_BOTTOM))
					other->client->movestate |= STS_W_ONDOORDWN;
				else
					other->client->movestate |= STS_W_ONDOORUP;
			}
			else
			{
				if ((ent->union_ent->moveinfo.state == PSTATE_DOWN) || (ent->union_ent->moveinfo.state == PSTATE_TOP))
					other->client->movestate |= STS_W_ONDOORDWN;
				else if ((ent->union_ent->moveinfo.state == PSTATE_UP) || (ent->union_ent->moveinfo.state == PSTATE_BOTTOM))
					other->client->movestate |= STS_W_ONDOORUP;
			}

			for (i =- MAX_DOORSEARCH; i < MAX_DOORSEARCH; i++)
			{
				if (i <= 0)
					j = other->client->pers.routeindex - (MAX_DOORSEARCH - i);
				else
					j = other->client->pers.routeindex+i;

				if (j < 0)
					continue;
				if (j >= TotalRouteNodes)
					continue;

				if (((Route[j].state == GRS_ONDOOR) && (Route[j].ent == ent->union_ent)) || (Route[j].state == GRS_PUSHBUTTON))
				{
					vec3_t	v;

					k = 1;
					flg = false;

					while (1)
					{
						if (j + k >= TotalRouteNodes)		// overflow
							break;

						if (j + k >= other->client->pers.routeindex)
						{
							Get_RouteOrigin(j+k, v);
							if (fabs(v[2] - other->s.origin[2]) > JumpMax)
							{
								flg = true;
								break;
							}
						}

						k++;
					}

					if ((j + k < TotalRouteNodes) && flg)
					{
						other->client->pers.routeindex = j + k;		// set!
						break;
					}
				}
			}

			if (!flg)
				other->client->movestate |= STS_W_DONT;		// failed

			ent->target_ent = NULL;
		}

		// not target
		SetRespawn(ent, 1000000);
		ent->solid = SOLID_NOT;
	}
	else if (ent->item == item_navi2)
	{
		for (i = 0; i < 10; i++)
		{
			if ((other->client->pers.routeindex+i) >= TotalRouteNodes)
				break;

			if (!Route[other->client->pers.routeindex+i].index)
				break;

			if (Route[other->client->pers.routeindex+i].state != GRS_PUSHBUTTON)
				continue;

			if (Route[other->client->pers.routeindex+i].ent == ent->union_ent)
			{
				other->client->pers.routeindex += i + 1;
				break;
			}
		}
	}

	return true;
}

//==============================================
qboolean B_UseWeapon(edict_t *ent, float aim, float distance, int my_weapon)						//CW...
{																									//(heavily modifed)
	qboolean	see_enemy = false;
	edict_t		*target = ent->client->current_enemy;
	vec3_t		vdir;
	int			enemy_weapon = 0;
	int			skill;

	if (!CanUseWeapon(ent, my_weapon))
		return false;

	GetAimAngle(ent, aim, distance);

	if (trace_priority < TRP_ANGLEKEEP)
		trace_priority = TRP_ANGLEKEEP;

	if (target->client)
		enemy_weapon = GetKindWeapon(target->client->pers.weapon);

	skill = Bot[ent->client->pers.botindex].skill[COMBATSKILL];

//	Special weapon handling code: Rocket Launcher.

	if (my_weapon == WEAP_ROCKETLAUNCHER)
	{
		see_enemy = InSight(ent, target);

		if (SkillLevel[skill] & FIRE_PRESTAYFIRE)
		{
			if (((distance > 500) && (random() < 0.25)) || (fabs(ent->s.angles[PITCH]) > 45))
			{
				if ((enemy_weapon < WEAP_ROCKETLAUNCHER) && (ent->groundentity || ent->client->waterstate) && see_enemy)
				{
					ent->client->battlemode |= FIRE_PRESTAYFIRE;
					ent->client->battlecount = 2 + (int)(6 * random());
					trace_priority = TRP_ALLKEEP;

					return true;
				}
			}
		}

//		Check for jumping-when-firing-rockets and explosion-avoidance skill use.

		if ((SkillLevel[skill] & FIRE_JUMPROC) && (random() < 0.3) && ((target->s.origin[2] - ent->s.origin[2]) < JumpMax) && !(ent->client->ps.pmove.pm_flags && PMF_DUCKED))
		{
			if (ent->groundentity && (ent->waterlevel < 2))
			{
				if (ent->client->routetrace)
				{
					if (Bot_Fall(ent, ent->s.origin, 0))
					{
						trace_priority = TRP_ALLKEEP;
						if (see_enemy)
							ent->client->buttons |= BUTTON_ATTACK;

						return true;
					}
				}
				else
				{
					ent->moveinfo.speed = 0;

					ent->velocity[2] = VEL_BOT_JUMP;
					SetBotAnim(ent);
					trace_priority = TRP_ALLKEEP;
					if (see_enemy)
						ent->client->buttons |= BUTTON_ATTACK;

					return true;
				}
			}
		}
		else if ((SkillLevel[skill] & FIRE_AVOIDEXPLO) && see_enemy && (distance < 120) && (random() < 0.5))
		{
			if (ent->groundentity || ent->client->waterstate)
			{
				ent->client->battlemode |= FIRE_AVOIDEXPLO;
				ent->client->battlecount = 4 + (int)(6 * random());
				trace_priority = TRP_ALLKEEP;

				return true;
			}
		}

		if (see_enemy)
			ent->client->buttons |= BUTTON_ATTACK;

		return true;
	}

//	Special weapon handling code: Shock Rifle and Disc Launcher.

	else if ((my_weapon == WEAP_SHOCKRIFLE) || (my_weapon == WEAP_DISCLAUNCHER))
	{
		see_enemy = InSight(ent, target);
		if (see_enemy)
			VectorCopy(target->s.origin, ent->client->vtemp);

		if (SkillLevel[skill] & FIRE_STAYFIRE)
		{
			if (see_enemy)
			{
				ent->client->battlemode |= FIRE_STAYFIRE;
				ent->client->battlecount = 8 + (int)(10 * random());
				trace_priority = TRP_ALLKEEP;

				return true;
			}
		}

//		Check for explosion avoidance (SR).

		else if (my_weapon == WEAP_SHOCKRIFLE)
		{
			if ((SkillLevel[skill] & FIRE_AVOIDEXPLO) && see_enemy && (distance < 150))
			{
				if (ent->groundentity || ent->client->waterstate)
				{
					ent->client->battlemode |= FIRE_AVOIDEXPLO;
					ent->client->battlecount = 6 + (int)(6 * random());
					trace_priority = TRP_ALLKEEP;

					return true;
				}
			}
		}
	}

//	Special weapon handling code: C4.

	else if (my_weapon == WEAP_C4)
	{
		ent->client->battlemode |= FIRE_C4;
		ent->client->battlecount = 4 + (int)(8 * random());
		trace_priority = TRP_ALLKEEP;
		if (ent->client->weaponstate == WEAPON_READY)
			ent->client->buttons |= BUTTON_ATTACK;

		return true;
	}

//	Special weapon handling code: Traps.

	else if (my_weapon == WEAP_TRAP)
	{
		see_enemy = InSight(ent, target);
		if (see_enemy)
			VectorCopy(target->s.origin, ent->client->vtemp);
		else
		{
			AngleVectors(ent->client->v_angle, vdir, NULL, NULL);
			VectorMA(ent->s.origin, 500.0, vdir, ent->client->vtemp);
		}
	}

	ent->client->buttons |= BUTTON_ATTACK;
	return true;
}

//======================================================
//=========== BOT FIGHTING/COMBAT FUNCTIONS ============
//======================================================

//==============================================
void Combat_LevelX(edict_t *ent, float distance)
{
	vec3_t		vdir;
	qboolean	use_weapon = false;																	//CW++

	if (ent->client->battlemode & FIRE_ESTIMATE)
	{
		float	aim = 10.0 - Bot[ent->client->pers.botindex].skill[AIMACCURACY];

//CW++
		//only use speculative fire for certain weapons
		switch (GetKindWeapon(ent->client->pers.weapon))
		{
			case WEAP_DESERTEAGLE:
				if ((distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_DESERTEAGLE))
					use_weapon = true;

				break;

			case WEAP_JACKHAMMER:
				if ((distance < 700) && B_UseWeapon(ent, aim, distance, WEAP_JACKHAMMER))
					use_weapon = true;

				break;

			case WEAP_MAC10:
				if ((distance < 400) && B_UseWeapon(ent, aim, distance, WEAP_MAC10))
					use_weapon = true;

				break;

			case WEAP_ESG:
				if ((distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_ESG))
					use_weapon = true;

				break;

			case WEAP_C4:
				if ((distance < 800) && B_UseWeapon(ent, aim, distance, WEAP_C4))
					use_weapon = true;

				break;

			case WEAP_TRAP:
				if ((distance < 800) && B_UseWeapon(ent, aim, distance, WEAP_TRAP))
					use_weapon = true;

				break;

			case WEAP_ROCKETLAUNCHER:
				if ((distance > 100) && (distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_ROCKETLAUNCHER))
					use_weapon = true;

				break;

			case WEAP_FLAMETHROWER:
				if ((distance > 100) && (distance < 1000) && B_UseWeapon(ent, aim, distance, WEAP_FLAMETHROWER))
				{
					use_weapon = true;
					ent->client->ft_firebomb = true;
				}
				break;

			case WEAP_SHOCKRIFLE:
				if ((distance > 100) && (distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_SHOCKRIFLE))
				{
					use_weapon = true;
					ent->client->homing_plasma = true;
				}
				break;

			case WEAP_DISCLAUNCHER:
				if ((distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_DISCLAUNCHER))
					use_weapon = true;

				break;
		}
//CW--

		VectorSubtract(ent->client->vtemp, ent->s.origin, vdir);
		ent->s.angles[YAW] = Get_yaw(vdir);
		ent->s.angles[PITCH] = Get_pitch(vdir);
		trace_priority = (use_weapon) ? TRP_ALLKEEP : TRP_ANGLEKEEP;								//CW

		return;
	}

	VectorSubtract(ent->client->current_enemy->s.origin, ent->s.origin, vdir);
	ent->s.angles[YAW] = Get_yaw(vdir);
	ent->s.angles[PITCH] = Get_pitch(vdir);
	trace_priority = TRP_ANGLEKEEP;			// use this angle										//CW
}

//==============================================
void Combat_Normal(edict_t *ent, float distance)
{
	edict_t		*target;
	trace_t		tr;
	vec3_t		v;
	vec3_t		vv;
	float		aim;
	float		f;
	qboolean	danger;
	qboolean	shift;
	int			i;

//CW++
	vec3_t		vdir;
	qboolean	quad_weapon = false;
	int			my_weapon;
	int			enemy_weapon = 0;
	int			skill;
//CW--

//	Set-up useful variables.

	aim = 10.0 - Bot[ent->client->pers.botindex].skill[AIMACCURACY];
	target = ent->client->current_enemy;
	my_weapon = GetKindWeapon(ent->client->pers.weapon);											//CW

//CW++
	if (target->client)
		enemy_weapon = GetKindWeapon(target->client->pers.weapon);

	skill = Bot[ent->client->pers.botindex].skill[COMBATSKILL];
//CW--

//	Modify aiming for chicken-shooting.

	if (ent->client->battlemode == FIRE_CHICKEN)
		aim *= BOT_CHICKENAIM_FACTOR;																//CW

//	Battlemode handling: strafing movement.

	if (ent->client->battlemode & FIRE_SHIFT)														//CW...
	{
		GetAimAngle(ent, aim, distance);
		if (--ent->client->battlesubcnt > 0)
		{
			if (ent->groundentity)
			{
				if (ent->client->battlemode & FIRE_SHIFT_R)
				{
					ent->client->moveyaw = ent->s.angles[YAW] + 90;
					if (ent->client->moveyaw > 180)
						ent->client->moveyaw -= 360;
				}
				else
				{
					ent->client->moveyaw = ent->s.angles[YAW] - 90;
					if (ent->client->moveyaw < -180)
						ent->client->moveyaw += 360;
				}

				trace_priority = TRP_MOVEKEEP;
			}
		}
		else
			ent->client->battlemode &= ~FIRE_SHIFT;
	}

//	Try to dodge (duck or jump) if we have that combat skill.

	if (SkillLevel[skill] & FIRE_DODGE)																	//CW
	{
		if (ent->groundentity && !ent->waterlevel && target->client)
		{
			AngleVectors(target->client->v_angle, v, NULL, NULL);
			VectorScale(v, BOT_DODGE_ENEMYRANGE, v);													//CW
			VectorSet(vv, 0, 0, target->viewheight - 8);
			VectorAdd(target->s.origin, vv, vv);
			VectorAdd(vv, v, v);

			tr = gi.trace(vv, VEC_TMINS4, VEC_TMAXS4, v, target, MASK_SHOT);							//CW
			if (tr.ent == ent)
			{
				if (tr.endpos[2] > ent->s.origin[2] + 4)
				{
					if (random() < 0.4)
					{
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
						ent->client->battleduckcnt = 2 + (8 * random());
					}
				}
				else if (tr.endpos[2] < ent->s.origin[2] + JumpMax - 24)
				{
					if (ent->client->routetrace)
					{
						if (Bot_Fall(ent, ent->s.origin, 0))
							trace_priority = TRP_MOVEKEEP;
					}
					else
					{
						ent->moveinfo.speed = 0.5;
						ent->velocity[2] = VEL_BOT_JUMP;
						ent->client->anim_priority = ANIM_JUMP;
//CW++
						gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
						PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
					}
				}
			}
		}
	}

//	Battlemode handling: ignore current enemy.

	if (ent->client->battlemode & FIRE_IGNORE)														//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			if (ent->client->current_enemy == ent->client->prev_enemy)
				return;
		}

		ent->client->battlemode = FIRE_NULL;
	}

//	Battlemode handling: pre hold-and-fire state.

	if (ent->client->battlemode & FIRE_PRESTAYFIRE)													//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			GetAimAngle(ent, aim, distance);
			if (ent->groundentity)
			{
				if (target->client && (target->client->weaponstate == WEAPON_FIRING))
				{
					if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
			}

			trace_priority = TRP_ALLKEEP;
			return;
		}

		if (!(ent->client->battlemode & FIRE_SHIFT))
			ent->client->battlemode = FIRE_STAYFIRE;

		ent->client->battlecount = 5 + (int)(20 * random());
	}

//	Battlemode handling: hold-and-fire state.

	if (ent->client->battlemode & FIRE_STAYFIRE)													//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			aim *= 0.95;
			GetAimAngle(ent, aim, distance);

			if (ent->groundentity && target->client && (target->client->weaponstate == WEAPON_FIRING))
			{
				if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
					ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			}

			if (!(ent->client->battlemode & FIRE_SHIFT))
				trace_priority = TRP_ALLKEEP;

			my_weapon = GetKindWeapon(ent->client->pers.weapon);
			if (my_weapon == WEAP_SHOCKRIFLE)
				ent->client->homing_plasma = (distance > 600) ? true : false;

			ent->client->buttons |= BUTTON_ATTACK;
			return;
		}

		ent->client->battlemode = FIRE_NULL;
	}

//	Battlemode handling: charge the enemy (we'll be Quadded).

	if (ent->client->battlemode & FIRE_RUSH)														//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			aim *= 0.95;
			GetAimAngle(ent, aim, distance);
			my_weapon = GetKindWeapon(ent->client->pers.weapon);

			if (ent->groundentity && target->client && (target->client->weaponstate == WEAPON_FIRING))
			{
				if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
					ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			}

			trace_priority = TRP_MOVEKEEP;
			ent->client->moveyaw = ent->s.angles[YAW];
			
			if ((my_weapon == WEAP_DISCLAUNCHER) || InSight(ent, target))
				ent->client->buttons |= BUTTON_ATTACK;

			return;
		}

		ent->client->battlemode = FIRE_NULL;
	}

//	Battlemode handling: avoid our own weapon explosions.
	
	if (ent->client->battlemode & FIRE_AVOIDEXPLO)													//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			GetAimAngle(ent, aim, distance);
			my_weapon = GetKindWeapon(ent->client->pers.weapon);

			if (ent->groundentity && (target->client->weaponstate == WEAPON_FIRING))
			{
				if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
					ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			}

			trace_priority = TRP_MOVEKEEP;
			ent->client->moveyaw = ent->s.angles[YAW] + 180;
			if (ent->client->moveyaw > 180)
				ent->client->moveyaw -= 360;

			if (InSight(ent, target))
				ent->client->buttons |= BUTTON_ATTACK;

			return;
		}

		ent->client->battlemode = FIRE_NULL;
	}

//CW++
//	Battlemode handling: throwing C4.

	if (ent->client->battlemode & FIRE_C4)
	{
		if (--ent->client->battlecount > 0) 
		{
			GetAimAngle(ent, aim, distance);

			if (ent->groundentity && target->client && (target->client->weaponstate == WEAPON_FIRING))
			{
				if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
					ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			}

			trace_priority = TRP_ANGLEKEEP;
			ent->client->buttons |= BUTTON_ATTACK;

			return;
		}

		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->battlemode = FIRE_NULL;
	}
//CW--

//	Battlemode handling: back-tracking with defensive fire.

	if (ent->client->battlemode & FIRE_REFUGE)														//CW...
	{
		if (--ent->client->battlecount > 0)
		{
			aim *= 0.95;
			GetAimAngle(ent, aim, distance);
			my_weapon = GetKindWeapon(ent->client->pers.weapon);

			if (ent->groundentity && target->client && (target->client->weaponstate == WEAPON_FIRING))
			{
				if ((enemy_weapon != WEAP_DESERTEAGLE) && (enemy_weapon != WEAP_RAILGUN) && (enemy_weapon != WEAP_CHAINSAW))
					ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			}

			trace_priority = TRP_ANGLEKEEP;
			if ((my_weapon == WEAP_DISCLAUNCHER) || InSight(ent, target))
				ent->client->buttons |= BUTTON_ATTACK;

			return;
		}

		ent->client->battlemode = FIRE_NULL;
		ent->client->pers.routeindex -= 2;
	}

//	We can see the enemy far below us, and we're on a moving door/plat/train, so fire splash-damage weapons.

	if (!(ent->client->combatstate & CTS_ENEM_NSEE) && (ent->client->movestate & STS_WAITSMASK2))	//CW...
	{
		if (target->s.origin[2] - ent->s.origin[2] < -300)											//CW
		{
//CW++
			//SR
			if (CanUseWeapon(ent, WEAP_SHOCKRIFLE))
			{
				GetAimAngle(ent, aim, distance);
				ent->client->homing_plasma = false;
				trace_priority = TRP_ANGLEKEEP;
				ent->client->buttons |= BUTTON_ATTACK;
				
				return;
			}

			//RL
			if (CanUseWeapon(ent, WEAP_ROCKETLAUNCHER))
			{
				GetAimAngle(ent, aim, distance);
				trace_priority = TRP_ANGLEKEEP;
				ent->client->buttons |= BUTTON_ATTACK;
				
				return;
			}

			//FT
			if (CanUseWeapon(ent, WEAP_FLAMETHROWER))
			{
				GetAimAngle(ent, aim, distance);
				ent->client->ft_firebomb = true;
				trace_priority = TRP_ANGLEKEEP;
				ent->client->buttons |= BUTTON_ATTACK;
				
				return;
			}

			//C4
			if (CanUseWeapon(ent, WEAP_C4))
			{
				GetAimAngle(ent, aim, distance);
				ent->client->battlemode |= FIRE_C4;
				ent->client->battlecount = 4 + (int)(8 * random());
				trace_priority = TRP_ANGLEKEEP;
				if (ent->client->weaponstate == WEAPON_READY)
					ent->client->buttons |= BUTTON_ATTACK;
				
				return;
			}
//CW--
		}
	}

//	Initiate further dodging (left/right strafing) if we have that combat skill.

	if (!(ent->client->battlemode & FIRE_SHIFT) && (SkillLevel[skill] & FIRE_DODGE))				//CW...
	{
		shift = true;
		if (ent->client->routetrace && (enemy_weapon != WEAP_RAILGUN))
		{
			for (i = ent->client->pers.routeindex; i < (ent->client->pers.routeindex + 10); i++)
			{
				if (i >= TotalRouteNodes)
					break;

				if (Route[i].state == GRS_ITEMS)
				{
					if (Route[i].ent->solid == SOLID_TRIGGER)
					{
						shift = false;
						break;
					}
				}
			}
		}

		if (shift)
		{
			GetAimAngle(ent, aim, distance);
			f = target->s.angles[YAW] - ent->s.angles[YAW];
			if (f > 180)
				f = -(360 - f);
			else if (f < -180)
				f = -(f + 360);

			if (f <= -160)
			{
				ent->client->battlemode |= FIRE_SHIFT_L;
				ent->client->battlesubcnt = 5 + (int)(15 * random());
			}
			else if (f >= 160)
			{
				ent->client->battlemode |= FIRE_SHIFT_R;
				ent->client->battlesubcnt = 5 + (int)(15 * random());
			}
		}
	}

	//================================================

//	Avoid an invincible enemy if we have that combat skill.

	if (target->client && (target->client->invincible_framenum > level.framenum))					//CW...
	{
		if (SkillLevel[skill] & FIRE_AVOIDINVULN)
		{
			GetAimAngle(ent, aim, distance);
			trace_priority = TRP_MOVEKEEP;
			ent->client->moveyaw = ent->s.angles[YAW] + 180;	// turn around!
			if (ent->client->moveyaw > 180)
				ent->client->moveyaw -= 360;

			return;
		}
	}

	//================================================

//	Choose a good weapon to use with the Quad if we have that combat skill.

	if ((ent->client->quad_framenum > level.framenum) && (SkillLevel[skill] & FIRE_QUADUSE))		//CW...
	{
//CW++
		if (CanUseWeapon(ent, WEAP_AGM))
		{
			quad_weapon = true;
			ent->client->agm_disrupt = true;
			my_weapon = WEAP_AGM;
		}
		else if (CanUseWeapon(ent, WEAP_MAC10))
		{
			quad_weapon = true;
			my_weapon = WEAP_MAC10;
		}
		else if (CanUseWeapon(ent, WEAP_JACKHAMMER))
		{
			quad_weapon = true;
			my_weapon = WEAP_JACKHAMMER;
		}
		else if (CanUseWeapon(ent, WEAP_SHOCKRIFLE))
		{
			quad_weapon = true;
			ent->client->homing_plasma = (distance < BOT_RUSH_ENEMYRANGE) ? false : true;
			my_weapon = WEAP_SHOCKRIFLE;
		}
		else if (CanUseWeapon(ent, WEAP_FLAMETHROWER))
		{
			quad_weapon = true;
			ent->client->ft_firebomb = (distance < BOT_RUSH_ENEMYRANGE) ? false : true;
			my_weapon = WEAP_FLAMETHROWER;
		}
		else if (CanUseWeapon(ent, WEAP_DESERTEAGLE))
		{
			quad_weapon = true;
			my_weapon = WEAP_DESERTEAGLE;
		}
//CW--

		if (quad_weapon)
		{
			GetAimAngle(ent, aim, distance);
			ent->client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;

//			If close to the enemy and they're carrying a weaker weapon, rush them!

			if ((distance < BOT_RUSH_ENEMYRANGE) && ((enemy_weapon == WEAP_CHAINSAW) || (enemy_weapon == WEAP_C4) || (enemy_weapon == WEAP_TRAP)))
			{
//CW++
//				Don't rush in if we know to avoid our own weapon explosions.

				if (SkillLevel[skill] & FIRE_AVOIDEXPLO)
				{
					if ((my_weapon == WEAP_ROCKETLAUNCHER) || (my_weapon == WEAP_SHOCKRIFLE))
						return;
				}
//CW--
				ent->client->battlemode |= FIRE_RUSH;
				ent->client->battlecount = 10 + (int)(10 * random());
			}

			return;
		}
	}

	//================================================

//	Back-track with defensive fire if we have that skill.

	if (SkillLevel[skill] & FIRE_REFUGE)															//CW...
	{
		if ((ent->client->battlemode == FIRE_NULL) && ent->client->routetrace && (ent->client->pers.routeindex > 1))
		{
			danger = ((enemy_weapon != WEAP_CHAINSAW) && (enemy_weapon != WEAP_C4) && (enemy_weapon != WEAP_TRAP) && (enemy_weapon != WEAP_GRAPPLE) && (enemy_weapon != WEAP_DESERTEAGLE)) ? 1 : 0;
			Get_RouteOrigin(ent->client->pers.routeindex - 2, v);
			if ((fabs(v[2] - ent->s.origin[2]) < JumpMax) && danger)
			{
				my_weapon = GetKindWeapon(ent->client->pers.weapon);
				if ((my_weapon == WEAP_ROCKETLAUNCHER) || (my_weapon == WEAP_SHOCKRIFLE))
				{
					ent->client->battlemode |= FIRE_REFUGE;
					ent->client->battlecount = 10 + (int)(10 * random());
					trace_priority = TRP_ALLKEEP;

					return;
				}
			}
		}
	}

	if (!ent->client->routetrace && (distance < 100))
	{
		ent->client->battlecount = 4 + (int)(8 * random());
		trace_priority = TRP_ALLKEEP;
	}

//CW++
//	If we're being held by a trap, the sensible weapon priority list for destroying it is different.

	if (ent->client->current_enemy && ent->client->current_enemy->die && (ent->client->current_enemy->die == Trap_DieFromDamage))
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_MAC10))
			goto FIRED;

		if (B_UseWeapon(ent, aim, distance, WEAP_JACKHAMMER))
			goto FIRED;

		if ((ent->waterlevel < 2) && B_UseWeapon(ent, aim, distance, WEAP_FLAMETHROWER))
		{
			ent->client->ft_firebomb = false;
			goto FIRED;
		}

		if (B_UseWeapon(ent, aim, distance, WEAP_DESERTEAGLE))
			goto FIRED;

		if (B_UseWeapon(ent, aim, distance, WEAP_CHAINSAW))
			goto FIRED;
	}

//	Try to select our primary weapon first.

	switch (Bot[ent->client->pers.botindex].skill[PRIMARYWEAP])
	{
		case WEAP_CHAINSAW:
			if ((distance < 300) && B_UseWeapon(ent, aim, distance, WEAP_CHAINSAW))
				goto FIRED;

			break;

		case WEAP_DESERTEAGLE:
			if ((distance < 1000) && B_UseWeapon(ent, aim, distance, WEAP_DESERTEAGLE))
				goto FIRED;

			break;

		case WEAP_GAUSSPISTOL:
			if ((distance < 1500) && B_UseWeapon(ent, aim, distance, WEAP_GAUSSPISTOL))
				goto FIRED;

			break;

		case WEAP_JACKHAMMER:
			if ((distance < 700) && B_UseWeapon(ent, aim, distance, WEAP_JACKHAMMER))
				goto FIRED;

			break;

		case WEAP_MAC10:
			if ((distance < 500) && B_UseWeapon(ent, aim, distance, WEAP_MAC10))
				goto FIRED;

			break;

		case WEAP_ESG:
			if ((distance < 1000) && B_UseWeapon(ent, aim, distance, WEAP_ESG))
				goto FIRED;

			break;

		case WEAP_C4:
			if ((distance < 600) && B_UseWeapon(ent, aim, distance, WEAP_C4))
				goto FIRED;

			break;

		case WEAP_ROCKETLAUNCHER:
			if ((distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_ROCKETLAUNCHER))
			{
				ent->client->normal_rockets = true;
				goto FIRED;
			}
			break;

		case WEAP_FLAMETHROWER:
			if ((ent->waterlevel < 2) && (distance < 1000) && B_UseWeapon(ent, aim, distance, WEAP_FLAMETHROWER))
			{
				ent->client->ft_firebomb = (distance > 500) ? true : false;
				goto FIRED;
			}
			break;

		case WEAP_RAILGUN:
			if ((distance < 1500) && B_UseWeapon(ent, aim, distance, WEAP_RAILGUN))
				goto FIRED;

			break;

		case WEAP_SHOCKRIFLE:
			if ((distance < 1600) && B_UseWeapon(ent, aim, distance, WEAP_SHOCKRIFLE))
			{
				ent->client->homing_plasma = (distance > 700) ? true : false;
				goto FIRED;
			}
			break;

		case WEAP_DISCLAUNCHER:
			if ((distance < 1000) && B_UseWeapon(ent, aim, distance, WEAP_DISCLAUNCHER))
				goto FIRED;

			break;

		case WEAP_AGM:
			if ((distance < 1200) && B_UseWeapon(ent, aim, distance, WEAP_AGM))
			{
				ent->client->agm_disrupt = true;
				goto FIRED;
			}
			break;
	}

//	If we don't have our primary weapon, select the best one from our inventory.

	//Mac-10
	if (distance < 500)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_MAC10))
			goto FIRED;
	}
	
	//Shock Rifle
	if (distance < 1600)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_SHOCKRIFLE))
		{
			ent->client->homing_plasma = (distance > 700) ? true : false;
			goto FIRED;
		}
	}

	//Jackhammer
	if (distance < 700)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_JACKHAMMER))
			goto FIRED;
	}

	//Flamethrower - normal
	if ((distance < 500) && (ent->waterlevel < 2))
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_FLAMETHROWER))
		{
			ent->client->ft_firebomb = false;
			goto FIRED;
		}
	}

	//Gauss Pistol
	if (distance < 1500)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_GAUSSPISTOL))
			goto FIRED;
	}

	//Railgun
	if (distance < 1500)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_RAILGUN))
			goto FIRED;
	}

	// AGM - Cellular Disruptor
	if (distance < 1200)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_AGM))
		{
			ent->client->agm_disrupt = true;
			goto FIRED;
		}
	}

	//Rocket Launcher
	if (distance < 1200)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_ROCKETLAUNCHER))
		{
			ent->client->normal_rockets = true;
			goto FIRED;
		}
	}

	//E.S.G.
	if (distance < 1000)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_ESG))
			goto FIRED;
	}

	//Disc Launcher
	if (distance < 1000)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_DISCLAUNCHER))
			goto FIRED;
	}

	//Flamethrower - firebomb
	if ((distance < 1000) && (ent->waterlevel < 2))
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_FLAMETHROWER))
		{
			ent->client->ft_firebomb = true;
			goto FIRED;
		}
	}

	//C4
	if (distance < 600)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_C4))
			goto FIRED;
	}

	//Desert Eagle
	if (distance < 1000)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_DESERTEAGLE))
			goto FIRED;
	}

	// Should be firing?
	if (ent->groundentity && (distance > 400) && (SkillLevel[skill] & FIRE_IGNORE))					//CW
	{
		if (!(ent->client->movestate & STS_WAITSMASK))
		{
			ent->client->battlemode = FIRE_IGNORE;
			ent->client->battlecount = 5 + (int)(10 * random());
		}
	}

	//Traps
	if (distance > 150)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_TRAP))
			goto FIRED;
	}

	//Chainsaw
	if (distance < 300)
	{
		if (B_UseWeapon(ent, aim, distance, WEAP_CHAINSAW))
			goto FIRED;
	}

	VectorSubtract(ent->client->vtemp, ent->s.origin, vdir);
	ent->s.angles[YAW] = Get_yaw(vdir);
	ent->s.angles[PITCH] = Get_pitch(vdir);
	trace_priority = TRP_ANGLEKEEP;	
//CW--

	return;

FIRED:	// shoot the weapon

//CW++
	my_weapon = GetKindWeapon(ent->client->pers.weapon);

//	If we're using the AGM and it's charging, release the fire button.

	if ((my_weapon == WEAP_AGM) && ent->client->agm_tripped)
		ent->client->buttons &= ~BUTTON_ATTACK;
//CW--

	if (ent->client->battlemode == FIRE_CHICKEN)
	{
		if ((--ent->client->battlesubcnt > 0) && ent->groundentity && (ent->waterlevel < 2))
		{
			f = target->s.angles[YAW] - ent->s.angles[YAW];
			if (f > 180)
				f = -(360 - f);

			if (f < -180)
				f = -(f + 360);

			if (fabs(f) >= 150)
				ent->client->battlemode = FIRE_NULL;
			else
			{
				trace_priority = TRP_ALLKEEP;
				if ((ent->client->weaponstate != WEAPON_READY) && (target->s.origin[2] < ent->s.origin[2]))
				{
					if ((my_weapon == WEAP_ROCKETLAUNCHER) || (my_weapon == WEAP_RAILGUN) || (my_weapon == WEAP_ESG))
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					else if (skill >= 7)															//CW
					{
						if ((my_weapon == WEAP_GAUSSPISTOL) || (my_weapon == WEAP_DESERTEAGLE))		//CW
							ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					}
				}
			}

			return;
		}
		else
			ent->client->battlemode = FIRE_NULL;
	}
	else if ((ent->client->battlemode == FIRE_NULL) && (distance > 200) && ent->groundentity && (ent->waterlevel < 2))
	{
		if (9 * random() > Bot[ent->client->pers.botindex].skill[AGGRESSION])
		{
			if ((my_weapon > WEAP_CHAINSAW) && (my_weapon != WEAP_GRAPPLE) && target->client && (target->client->current_enemy != ent))	//CW
			{
				f = target->s.angles[YAW] - ent->s.angles[YAW];
				if (f > 180)
					f = -(360 - f);

				if (f < -180)
					f = -(f + 360);

				if (fabs(f) < 150)
				{
					ent->client->battlemode = FIRE_CHICKEN;
					ent->client->battlesubcnt = 5 + (int)(random() * 8);
					trace_priority = TRP_ALLKEEP;
				}
			}
		}
	}
}

//============================================================
void Set_Combatstate(edict_t *ent) 
{
	edict_t	*enemy;																					//CW++
	vec3_t	vtmp;
	float	distance;

	if (ent->client->movestate & STS_LADDERUP)
		return;

//CW++
	enemy = ent->client->current_enemy;

	if (!(enemy && enemy->die && (enemy->die == Trap_DieFromDamage)))
	{
		if (!enemy)
		{
			ent->client->combatstate &= ~CTS_COMBS;			//clear status
			return;
		}

		//target is dead
		if (!enemy->inuse || enemy->deadflag || (enemy->solid != SOLID_BBOX))
		{
			ent->client->battleduckcnt = 0;
			ent->client->current_enemy = NULL;
			ent->client->combatstate &= ~CTS_COMBS;			// clear status

			if ((9 * random()) < Bot[ent->client->pers.botindex].skill[COMBATSKILL])
				UsePrimaryWeapon(ent);

			return;
		}
	}

	if (!enemy)
	{
		ent->client->combatstate &= ~CTS_COMBS;				//clear status
		return;
	}
//CW--

	if (!Bot_trace(ent, ent->client->current_enemy)) 
	{
		if (ent->client->targetlock <= level.time)
		{
			ent->client->current_enemy = NULL;
			return;
		}

		ent->client->combatstate |= CTS_ENEM_NSEE;			//can't see enemy
	}
	else
	{
		ent->client->combatstate &= ~CTS_ENEM_NSEE;			//can see enemy
		ent->client->targetlock = level.time + 1.2;
		ent->client->battlemode &= ~FIRE_ESTIMATE;
	}

	VectorSubtract(ent->client->current_enemy->s.origin, ent->s.origin, vtmp);
	distance = VectorLength(vtmp);

	if (!(ent->client->combatstate & CTS_ENEM_NSEE))
		Combat_Normal(ent, distance);
	else if (ent->client->battlemode & FIRE_REFUGE)													//CW: was combatstate
		Combat_Normal(ent, distance);
	else
		Combat_LevelX(ent, distance);

	if (ent->client->current_enemy)
	{
		ent->client->prev_enemy = ent->client->current_enemy;
		VectorCopy(ent->client->current_enemy->s.origin, ent->client->targ_old_origin);
	}
}

//====================================================
//============ BOT ENEMY SEARCHING ROUTINES ==========
//====================================================

//====================================================
void Bot_SearchEnemy(edict_t *ent)
{
	qboolean	tmpflg = false;
	edict_t		*target = NULL;
	edict_t		*trent;
	vec3_t		vdir;
	vec3_t		end;																				//CW++
	int			i;
	int			j;

	if (ent->client->current_enemy)
	{
		if (Bot_trace(ent, ent->client->current_enemy))
			tmpflg = true;
	}

	j = (random() < 0.5) ? 0 : -1;

	for (i = 1; (i <= maxclients->value) && (target == NULL); i++)									//CW
	{
		if (j)
			trent = &g_edicts[i];
		else
			trent = &g_edicts[(int)(maxclients->value)-i+1];

		if (!trent->inuse || (ent == trent) || trent->deadflag)
			continue;
		if (ent->client->current_enemy == trent)
			continue;
		if (ent->client->current_enemy && (ent->client->current_enemy->health < 1))
			continue; // raven - added deadflag check
//CW++
		if (CheckTeamDamage(trent, ent))		// don't target team members in TDM
			continue;

		if (trent->flags & FL_NOTARGET)
			continue;
//CW--
		if (trent->movetype != MOVETYPE_NOCLIP)
		{
			if (InSight(ent, trent))
			{
				VectorSubtract(trent->s.origin, ent->s.origin, vdir);

				if (!tmpflg && (target == NULL))
				{
					float vr = (float)Bot[ent->client->pers.botindex].skill[VRANGEVIEW];
					float hr = (float)Bot[ent->client->pers.botindex].skill[HRANGEVIEW];
					float pitch = fabs(Get_pitch(vdir) - ent->s.angles[PITCH]);

					if (pitch > 180)
						pitch = 360 - pitch;

					if (pitch <= vr)
					{
						float yaw = Get_yaw(vdir);

						yaw = fabs(yaw - ent->s.angles[YAW]);
						if (yaw > 180)
							yaw = 360 - yaw;

						if ((yaw <= hr) || (ent->client->movestate & STS_WAITS))
							target = trent;
					}
				}

				if (!tmpflg && (target == NULL) && trent->mynoise && trent->mynoise2)
				{
					if (trent->mynoise->teleport_time >= level.time - FRAMETIME)
					{
						VectorSubtract(trent->mynoise->s.origin, ent->s.origin, vdir);
						if (VectorLength(vdir) < BOT_PNOISE_SELF_DIST)								//CW
							target = trent;
					}
					
					if ((target == NULL) && (trent->mynoise2->teleport_time >= level.time - FRAMETIME))
					{
						VectorSubtract(trent->mynoise2->s.origin, ent->s.origin, vdir);				//CW
						if (VectorLength(vdir) < BOT_PNOISE_IMPACT_DIST)							//CW
							target = trent;
					}
				}
			}
			else if (!tmpflg && trent->mynoise && trent->mynoise2)									//CW...
			{
				if ((target == NULL) && (trent->mynoise->teleport_time >= level.time - FRAMETIME))
				{
					trace_t tr;
					
					AngleVectors(trent->client->v_angle, vdir, NULL, NULL);
					VectorScale(vdir, BOT_PNOISE_RADIUS, vdir);
					VectorAdd(trent->s.origin, vdir, end);
					tr = gi.trace(trent->s.origin, NULL, NULL, end, trent, MASK_SHOT);
					VectorSubtract(ent->s.origin, tr.endpos, vdir);
					if (VectorLength(vdir) < BOT_PNOISE_SELF_DIST)
					{
						VectorCopy(tr.endpos, end);
						tr = gi.trace(ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
						if (tr.fraction == 1.0)
						{
							target = trent;
							ent->client->battlemode |= FIRE_ESTIMATE;
							VectorCopy(end, ent->client->vtemp);
						}
					}
				}
//CW++
				if ((target == NULL) && (trent->mynoise2->teleport_time >= level.time - FRAMETIME))
				{
					VectorSubtract(trent->mynoise2->s.origin, ent->s.origin, vdir);
					if (VectorLength(vdir) < BOT_PNOISE_IMPACT_DIST)
					{
						target = trent;
						ent->client->battlemode |= FIRE_ESTIMATE;
						VectorCopy(trent->mynoise2->s.origin, ent->client->vtemp);
					}
				}
//CW--
			}
		}
	}

	if (target && !tmpflg)
	{
		ent->client->current_enemy = target;
		ent->client->targetlock = level.time + 2.0;													//CW++
	}
	else if (target && ent->client->current_enemy)
	{
		if (target->client && GetKindWeapon(target->client->pers.weapon) > GetKindWeapon(ent->client->current_enemy->client->pers.weapon))	//CW
		{
			ent->client->current_enemy = target;
			ent->client->targetlock = level.time + 2.0;												//CW++
		} 
	}
}


//==============================================
//============ ITEM HANDLING ROUTINES ==========
//==============================================

//==============================================
void InitAllItems(void)
{
	// set IT_ARMOR lookups
	item_jacketarmor = FindItem("Jacket Armor");
	item_combatarmor = FindItem("Combat Armor");
	item_bodyarmor	 = FindItem("Body Armor");
	item_armorshard	 = FindItem("Armor Shard");
	item_powerscreen = FindItem("Power Screen");
	item_powershield = FindItem("Power Shield");

	// set IT_AMMO lookups
	item_shells   = FindItem("shells");
	item_cells    = FindItem("cells");
	item_bullets  = FindItem("bullets");
	item_rockets  = FindItem("rockets");
	item_slugs    = FindItem("slugs");

//CW++
	// set IT_WEAPON lookups
	item_chainsaw       = FindItem("chainsaw");
	item_deserteagle    = FindItem("desert eagle");
	item_gausspistol    = FindItem("gauss pistol");
	item_jackhammer     = FindItem("jackhammer");
	item_c4             = FindItem("c4");
	item_mac10          = FindItem("mac-10");
	item_esg            = FindItem("e.s.g.");
	item_trap           = FindItem("traps");
	item_rocketlauncher = FindItem("rocket launcher");
	item_railgun        = FindItem("railgun");
	item_flamethrower   = FindItem("flamethrower");
	item_shockrifle     = FindItem("shock rifle");
	item_disclauncher   = FindItem("disc launcher");
	item_agm            = FindItem("ag manipulator");
	item_grapple        = FindItem("grapple");
//CW--

	// set IT_HEALTH lookups
	item_adrenaline   = FindItem("Adrenaline");
	item_health       = FindItem("Health");
	item_stimpak      = FindItem("Health");
	item_health_large = FindItem("Health");
	item_health_mega  = FindItem("Health");

	// set IT_POWERUP lookups
	item_quad = FindItem("Quad Damage");
	item_invulnerability = FindItem("Invulnerability");
	item_silencer = FindItem("Silencer");
	item_breather = FindItem("Rebreather");
	item_enviro = FindItem("Environment Suit");

//CW++
	item_teleporter = FindItem("Teleporter");
//CW--

	// set IT_PACK lookups
	item_pack = FindItem("Ammo Pack");
	item_bandolier = FindItem("Bandolier");

	// set IT_NODE lookups
	item_navi1 = FindItem("Roam Navi1");
	item_navi2 = FindItem("Roam Navi2");
	item_navi3 = FindItem("Roam Navi3");
}

//==============================================
int GetKindWeapon(gitem_t *it)
{
//CW++
	if (!it)
		return 0;
//CW--

	if (it->weaponthink == Weapon_DesertEagle)		return WEAP_DESERTEAGLE;						//CW...
	if (it->weaponthink == Weapon_GaussPistol)		return WEAP_GAUSSPISTOL;
	if (it->weaponthink == Weapon_Jackhammer)		return WEAP_JACKHAMMER;
	if (it->weaponthink == Weapon_Mac10)			return WEAP_MAC10;
	if (it->weaponthink == Weapon_ESG)				return WEAP_ESG;
	if (it->weaponthink == Weapon_C4)				return WEAP_C4;
	if (it->weaponthink == Weapon_Trap)				return WEAP_TRAP;
	if (it->weaponthink == Weapon_RocketLauncher)	return WEAP_ROCKETLAUNCHER;
	if (it->weaponthink == Weapon_Flamethrower)		return WEAP_FLAMETHROWER;
	if (it->weaponthink == Weapon_Railgun)			return WEAP_RAILGUN;
	if (it->weaponthink == Weapon_ShockRifle)		return WEAP_SHOCKRIFLE;
	if (it->weaponthink == Weapon_DiscLauncher)		return WEAP_DISCLAUNCHER;
	if (it->weaponthink == Weapon_AGM)				return WEAP_AGM;
	if (it->weaponthink == CTFWeapon_Grapple)		return WEAP_GRAPPLE;

	return WEAP_CHAINSAW;
}


//====================================================
//============ BOT MOVEMENT TESTING ROUTINES =========
//====================================================

//============================================================
int Bot_TestMove(edict_t *ent, float ryaw, vec3_t pos, float dist, float *bottom)
{
	vec3_t	trstart;
	vec3_t	trend;
	vec3_t	trmin;
	vec3_t	trmax;
	vec3_t	v;
	vec3_t	vv;
	trace_t	tr;
	float	tracelimit;
	float	yaw;
	int		contents;

	if (ent->waterlevel >= 1)
		tracelimit = 75;
	else
	{
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			tracelimit = 26;
		else
			tracelimit = JumpMax + 5;
	}

	VectorSet(trmin, -16, -16, -24);
	VectorSet(trmax, 16, 16, 3);

	if (ent->client->routetrace)
		VectorSet(vv, 16, 16, 0);
	else
		VectorSet(vv, 16, 16, 3);

	if (ent->client->routetrace)
	{
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			if (ent->waterlevel < 2)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				if ((v[2] - ent->s.origin[2]) > 20)
					trmax[2] = 31;
			}
		}
	}

	yaw = DEG2RAD(ryaw);
	trend[0] = cos(yaw) * dist;
	trend[1] = sin(yaw) * dist;
	trend[2] = 0;
	VectorAdd(trend, ent->s.origin, trstart);
	VectorCopy(trstart, trend);
	trend[2] += 1;
	tr = gi.trace(trstart, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
	trmax[2] += 1;

	if (tr.allsolid || tr.startsolid || (tr.fraction != 1.0))
	{
		float		i;
		qboolean	moveok = false;

		VectorCopy(trstart, trend);
		for (i = 4; i < tracelimit + 4; i += 4)
		{
			trstart[2] = ent->s.origin[2] + i;
			tr = gi.trace(trstart, trmin, vv, trend, ent, MASK_BOTSOLIDX);
			if (!tr.allsolid && !tr.startsolid && (tr.fraction > 0.0))
			{
				moveok = true;
				break;
			}
		}

		if (!moveok)
			return 0;

		*bottom = tr.endpos[2] - ent->s.origin[2];
		if (!ent->client->routetrace)
		{
			if ((tr.plane.normal[2] < 0.7) && !ent->client->waterstate && ent->groundentity)
				return 0;
		}
		else
		{
			Get_RouteOrigin(ent->client->pers.routeindex, v);
			if ((tr.plane.normal[2] < 0.7) && (v[2] < ent->s.origin[2]))
				return 0;
		}

		if (*bottom > tracelimit - 5)
			return 0;

		VectorCopy(tr.endpos, pos);

		if (trmax[2] == 32)
			return 1;

		VectorCopy(pos, trend);
		trend[2] += 28;
		tr = gi.trace(pos, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
		return (!tr.allsolid && !tr.startsolid && (tr.fraction == 1.0))?1:2;
	}
	else
	{
		VectorCopy(trstart, pos);
		VectorCopy(trstart, trend);
		trstart[2] = trend[2] - 8190;
		tr = gi.trace(trend, trmin, trmax, trstart, ent, MASK_BOTSOLIDX|MASK_OPAQUE);
		*bottom = tr.endpos[2] - ent->s.origin[2];
		
		contents = 0;
		if (!ent->waterlevel)
		{
			if (ent->client->enviro_framenum > level.framenum)
				contents = CONTENTS_LAVA;
			else
				contents = (CONTENTS_LAVA | CONTENTS_SLIME);
		}

		if (tr.contents & contents)
			*bottom = -9999;
		else if (tr.surface->flags & SURF_SKY)
			*bottom = -9999;

		if (!ent->waterlevel && !ent->groundentity)
		{
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				if ((ent->velocity[2] > 10) && (trmax[2] == 4))
					return 2;
			}
		}

		if (trmax[2] == 32)
			return 1;

		VectorCopy(pos, trend);
		trend[2] += 28;
		tr = gi.trace(pos, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
		return (!tr.allsolid && !tr.startsolid && (tr.fraction == 1.0))?1:2;
	}
}

//============================================================
qboolean Bot_Watermove(edict_t *ent, vec3_t pos, float dist, float upd)
{
	trace_t	tr;
	vec3_t	trmin;
	vec3_t	trmax;
	vec3_t	touchmin;
	float	max;
	float	vec;
	float	i;
	float	j;

	VectorCopy(ent->s.origin, trmax);
	trmax[2] += upd;
	tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, trmax, ent, MASK_BOTSOLIDX);
	if (!tr.allsolid && !tr.startsolid && (tr.fraction > 0))
	{
		VectorCopy(tr.endpos, pos);
		return true;
	}

	VectorCopy(ent->s.origin, trmin);
	trmin[2] += upd;
	vec = -1;
	max = 0;

	for (i = 0; i < 360; i += 10)
	{
		if (i && (upd > -13) && (upd < 0))
			break;

		if ((i > 60) && (i < 300))
			continue;

		j = ent->client->moveyaw + i;
		if (j > 180)
			j -= 360;
		else if (j < -180)
			j += 360;
		else
			j = i;

		touchmin[0] = cos(j) * 24;
		touchmin[1] = sin(j) * 24;
		touchmin[2] = 0;
		VectorAdd(trmin, touchmin, trmax);
		tr = gi.trace(trmax, ent->mins, ent->maxs, trmin, ent, MASK_BOTSOLIDX);
		if (!tr.allsolid && !tr.startsolid)
		{
			VectorAdd(tr.endpos, touchmin, trmax);
			tr = gi.trace(trmax, ent->mins, ent->maxs, trmax, ent, MASK_BOTSOLIDX);
			if (!tr.allsolid && !tr.startsolid)
			{
				vec = i;
				break;
			}
		}
	}

	if (vec == -1)
		return false;

	VectorCopy(trmax, pos);
	if (upd < 0)
		ent->velocity[2] = 0;

	return true;
}

//============================================================
qboolean Bot_moveW(edict_t *ent, float ryaw, vec3_t pos, float dist, float *bottom)
{
	vec3_t	trstart;
	vec3_t	trend;
	trace_t	tr;
	float	yaw;
	int		contents;

	if (ent->client->enviro_framenum > level.framenum)
		contents = CONTENTS_LAVA;
	else
		contents = (CONTENTS_LAVA|CONTENTS_SLIME);

	yaw = DEG2RAD(ryaw);
	trend[0] = cos(yaw) * dist;
	trend[1] = sin(yaw) * dist;
	trend[2] = 0;
	VectorAdd(trend, ent->s.origin, trstart);
	VectorCopy(trstart, pos);
	VectorCopy(trstart, trend);
	trstart[2] = trend[2] - 8190;

	tr = gi.trace(trend, ent->mins, ent->maxs, trstart, ent, MASK_BOTSOLIDX|CONTENTS_WATER);
	if ((trend[2] - tr.endpos[2]) >= 95)
		return false;

	if (tr.contents & contents)
		return false;

	if (!(tr.contents & CONTENTS_WATER))
		return false;

	*bottom = tr.endpos[2] - ent->s.origin[2];

	return true;
}

//============================================================
qboolean Bot_Jump(edict_t *ent, vec3_t pos, float dist)
{
	vec3_t	temppos;
	float	yaw;
	float	tdist;
	float	bottom;
	float	speed;
	float	x;

	yaw = ent->client->moveyaw;

	Bot_TestMove(ent, yaw, temppos, dist, &bottom);
	if (bottom > -JumpMax)
		return false;

	for (x = 2; x <= 16; x++)
	{
		tdist = dist * x;
		if (Bot_TestMove(ent, yaw, temppos, tdist, &bottom))
		{
			if ((bottom <= JumpMax) && (bottom > -JumpMax))
			{
				if (Get_FlyingSpeed(bottom, x, dist, &speed))
				{
					speed *= 1.5;
					if (speed > 1.2)
						speed = 1.2;

					ent->moveinfo.speed = speed;
					ent->velocity[2] = VEL_BOT_JUMP;
					SetBotAnim(ent);
					return true;
				}
			}

			continue;
		}
		else
			return false;
	}

	return false;
}

//============================================================
qboolean Bot_Fall(edict_t *ent,	vec3_t pos,	float dist)
{
	vec3_t	vdir;
	vec3_t	vv;
	float	speed;
	float	vel;
	float	z_org;
	float	ypos;
	float	x;
	float	n;
	int		mf = 0;
	int		mode = 0;

	if (ent->client->routetrace)
	{
		mode = 2;
		Get_RouteOrigin(ent->client->pers.routeindex, vv);
		ypos = vv[2];
		if (!HazardCheck(ent, vv))
		{
			if (++ent->client->pers.routeindex >= TotalRouteNodes)
				ent->client->pers.routeindex = 0;

			return false;
		}

		z_org = pos[2];
		VectorSubtract(vv, pos, vdir);
		if (vdir[2] >= 0)
			goto JUMPCATCH;

		n = 1.0;
		vel = ent->velocity[2];

		for (x = 1; x <= BOT_FALLCHK_LOOPMAX; ++x, n += x)
		{
			vel -= (ent->gravity * sv_gravity->value * FRAMETIME);
			z_org += vel * FRAMETIME;
			if (ypos >= z_org)
			{
				mf = 1;
				break;
			}
		}

		VectorCopy(vdir, vv);
		vv[2] = 0;
		if (Route[ent->client->pers.routeindex].state == GRS_ONTRAIN)
		{
			vv[0] += FRAMETIME * Route[ent->client->pers.routeindex].ent->velocity[0] * x;
			vv[1] += FRAMETIME * Route[ent->client->pers.routeindex].ent->velocity[1] * x;
		}

		speed = VectorLength(vv) / x;
		if ((speed <= MOVE_SPD_RUN) && mf)															//CW...
		{
			ent->moveinfo.speed = speed / MOVE_SPD_RUN;
			VectorCopy(pos, ent->s.origin);
			return true;
		}

		goto JUMPCATCH;
	}

	goto JMPCHK;

JUMPCATCH:
	vel = VEL_BOT_JUMP;
	z_org = pos[2];
	mf = 0;

	for (x = 1; x <= BOT_FALLCHK_LOOPMAX; ++x)
	{
		vel -= (ent->gravity * sv_gravity->value * FRAMETIME);
		z_org += vel * FRAMETIME;
		if (vel > 0)
		{
			if (mf == 0)
			{
				if (ypos < z_org)
					mf = 2;
			}
		}
		else if (x > 1)
		{
			if (mf == 0)
			{
				if (ypos < z_org)
					mf = 2;
			}
			else if (mf == 2)
			{
				if (ypos >= z_org)
				{
					mf = 1;
					break;
				}
			}
		}
	}

	VectorCopy(vdir, vv); 
	vv[2] = 0;
	if (mode == 2)
	{
		if (Route[ent->client->pers.routeindex].state == GRS_ONTRAIN)
		{
			vv[0] += FRAMETIME * Route[ent->client->pers.routeindex].ent->velocity[0] * x;
			vv[1] += FRAMETIME * Route[ent->client->pers.routeindex].ent->velocity[1] * x;
		}
	}

	n = VectorLength(vv);
	if (x > 1)
		n /= (x - 1);

	if ((n < MOVE_SPD_RUN) && (mf == 1))															//CW...
	{
		ent->moveinfo.speed = n / MOVE_SPD_RUN;
		VectorCopy(pos, ent->s.origin);
		ent->velocity[2] = VEL_BOT_JUMP;
		SetBotAnim(ent);
		return true;
	}

JMPCHK:
	if (Bot_Jump(ent, pos, dist))
		return true;

	return false;
}

//============================================================
qboolean TargetJump(edict_t *ent, vec3_t tpos)
{
	vec3_t	vdir;
	float	vel;
	float	z_org;
	float	x;
	float	n;
	int		mf = 0;

	vel = VEL_BOT_JUMP;
	z_org = ent->s.origin[2];

	//if on hazard object, cause error
	if (!HazardCheck(ent, tpos))
		return false;

	VectorSubtract(tpos, ent->s.origin, vdir);

	for (x = 1; x <= BOT_FALLCHK_LOOPMAX * 2; ++x)
	{
		vel -= (ent->gravity * sv_gravity->value * FRAMETIME);
		z_org += vel * FRAMETIME;

		if (vel > 0)
		{
			if (mf == 0)
			{
				if (tpos[2] < z_org)
					mf = 2;
			}
		}
		else if (x > 1)
		{
			if (mf == 0)
			{
				if (tpos[2] < z_org)
					mf = 2;
			}
			else if (mf == 2)
			{
				if (tpos[2] >= z_org)
				{
					mf = 1;
					break;
				}
			}
		}
	}

	vdir[2] = 0;
	n = VectorLength(vdir);

	if (x > 1)
		n /= (x - 1);

	if ((n < MOVE_SPD_RUN) && (mf == 1))															//CW
	{
		ent->moveinfo.speed = n / MOVE_SPD_RUN;
		ent->velocity[2] = VEL_BOT_JUMP;															//CW
		SetBotAnim(ent);
		return true;
	}

	return false;
}

//============================================================
qboolean TargetJump_Chk(edict_t *ent, vec3_t tpos, float defvel)
{
	vec3_t	vdir;
	float	vel;
	float	z_org;
	float	x;
	float	n;
	int		mf = 0;

	vel = defvel + VEL_BOT_JUMP;
	z_org = ent->s.origin[2];

	//if on hazard object, cause error
	if (!HazardCheck(ent, tpos))
		return false;

	VectorSubtract(tpos, ent->s.origin, vdir);

	for (x = 1; x <= BOT_FALLCHK_LOOPMAX * 2; ++x)
	{
		vel -= (ent->gravity * sv_gravity->value * FRAMETIME);
		z_org += vel * FRAMETIME;

		if (vel > 0)
		{
			if (mf == 0)
			{
				if (tpos[2] < z_org)
					mf = 2;
			}
		}
		else if (x > 1)
		{
			if (mf == 0)
			{
				if (tpos[2] < z_org)
					mf = 2;
			}
			else if (mf == 2)
			{
				if (tpos[2] >= z_org)
				{
					mf = 1;
					break;
				}
			}
		}
	}

	vdir[2] = 0;
	n = VectorLength(vdir);
	if (x > 1)
		n /= (x - 1);

	return ((n < MOVE_SPD_RUN) && (mf == 1));														//CW
}

//============================================================
void Get_WaterState(edict_t *ent)
{
//CW++
//	Previous waterlevel code meant bots would never drown or be extinguished if on fire.

	M_CatagorizePosition(ent);
//CW--

	if (ent->waterlevel)
	{
		trace_t	tr;
		vec3_t	trmin;
		vec3_t	trmax;
		float	x;

		VectorCopy(ent->s.origin, trmax);
		VectorCopy(ent->s.origin, trmin);
		trmax[2] -= 24;
		trmin[2] += 8;
		tr = gi.trace(trmin, NULL, NULL, trmax, ent, MASK_WATER);
		x = trmin[2] - tr.endpos[2];
		if (tr.allsolid || tr.startsolid || (x < 4.0))
			ent->client->waterstate = WAS_IN;
		else
			ent->client->waterstate = ((x >= 4.0) && (x <= 12.0))?WAS_FLOAT:WAS_NONE;
	}
	else
		ent->client->waterstate = WAS_NONE;
}

//============================================================
void Search_NearbyPod(edict_t *ent)
{
	if (Route[ent->client->pers.routeindex].state >= 3)
		return;

	if ((ent->client->pers.routeindex+1) < TotalRouteNodes)
	{
		if (Route[ent->client->pers.routeindex+1].state < 3)
		{
			vec3_t	v;

			Get_RouteOrigin(ent->client->pers.routeindex+1, v);
			if (TraceX(ent, v))
			{
				vec3_t	v1;
				vec3_t	v2;
				float	x;

				VectorSubtract(v, ent->s.origin, v1);
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				VectorSubtract(v, ent->s.origin, v2);
				x = fabs(v1[2]);
				if ((VectorLength(v1) < VectorLength(v2)) && (x <= JumpMax) && (Route[ent->client->pers.routeindex].state <= 1))
					ent->client->pers.routeindex++;
				else
				{
					if (ent->client->waterstate == WAS_NONE)
					{
						if ((v2[2] > JumpMax) && (fabs(v1[2]) < JumpMax))
							ent->client->pers.routeindex++;
					}
				}
			}
		}
	}
}

//========================================================
void Bot_AI(edict_t *ent)
{
	gitem_t		*it;
	edict_t		*it_ent = NULL;																		//CW
	edict_t		*touch[1024];
	edict_t		*trent;
	edict_t		*front;
	edict_t		*left;
	edict_t		*right;
	edict_t		*e;
	trace_t		tr;
	cplane_t	plane;
	char		*str;
	vec3_t		touchmin;
	vec3_t		touchmax;
	vec3_t		v;
	vec3_t		vv;
	vec3_t		temppos;
	vec3_t		trmin;
	vec3_t		trmax;
	vec3_t		tmp_org;																			//CW
	vec3_t		tmp_vel;																			//CW
	float		tmp_yaw;																			//CW
	float		dist;
	float		x;
	float		yaw = 0.0F;																			//CW
	float		iyaw = 0.0F;																		//CW
	float		f1;
	float		f2;
	float		f3;
	float		bottom;
	int			tempflag;
	int			contents;																			//CW++
	int			i;
	int			j = 0;																				//CW
	int			k;	
	qboolean	ladderdrop;
	qboolean	canrocj;
	qboolean	waterjumped;

	myrandom = random();

	trace_priority = TRP_NORMAL;
	ent->client->objshot = false;
	ent->client->buttons &= ~BUTTON_ATTACK;

//CW++
	if (ent->client->agm_enemy)
		ent->client->movestate |= STS_AGMMOVE;
	else
		ent->client->movestate &= ~STS_AGMMOVE;

//	Detonate C4 if one is near our current enemy.

	if (SkillLevel[Bot[ent->client->pers.botindex].skill[COMBATSKILL]] & FIRE_C4USE)
	{
		if (ent->client->current_enemy && ent->next_node)
		{
			edict_t		*check;
			edict_t		*index;
			qboolean	finished = false;

			index = ent->next_node;
			while (index && !finished)
			{
				check = index;
				if (index->next_node)
					index = index->next_node;
				else
					finished = true;

				if (VecRange(ent->client->current_enemy->s.origin, check->s.origin) < 100)
				{
					Cmd_Detonate_C4_f(ent);
					break;
				}
			}
		}
	}

//	Solid checks.

	contents = gi.pointcontents(ent->s.origin);
	if (contents & CONTENTS_SOLID)
		T_Damage(ent, ent, ent, ent->s.origin, ent->s.origin, ent->s.origin, 100, 1, 0, MOD_CRUSH);
//CW--

	if (VectorCompare(ent->s.origin, ent->s.old_origin))
	{
		if (!ent->groundentity && !ent->waterlevel)
		{
			VectorCopy(ent->s.origin, v);
			v[2] -= 1.0;
			tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, v, ent, MASK_BOTSOLIDX);
			if (!tr.allsolid && !tr.startsolid)
				ent->groundentity = tr.ent;
		}
	}

//CW++
//	Frozen bots can't move!

	if (ent->client->frozen_framenum > level.framenum)
	{
		int	n;

		for (n = 0; n < 3; ++n)
		{
			ent->s.old_origin[n] = ent->s.origin[n];
			ent->velocity[n] = 0.0;
		}

		return;
	}
//CW--

	VectorCopy(ent->s.origin, tmp_org);
	VectorCopy(ent->velocity, tmp_vel);
	tmp_yaw = ent->s.angles[YAW];

//	Check chaining mode.

	if ((int)chedit->value)
	{
		qboolean tracefail = false;

		if (!ent->client->routetrace)
			tracefail = true;				// route off
		else if (ent->client->routeindex >= CurrentIndex)
			tracefail = true;				// index overflow
		else if ((Route[ent->client->routeindex].index == 0) && (ent->client->routeindex > 0))
			tracefail = true;				// index end

		if (tracefail)
		{
			gi.dprintf("Tracing failed for %s.\n", ent->client->pers.netname);
			RemoveBot(ent);
			return;
		}
	}

//	Get JumpMax.

	if (JumpMax == 0)
	{
		x = VEL_BOT_JUMP - (ent->gravity * sv_gravity->value * FRAMETIME);
		while (1)
		{
			JumpMax += x * FRAMETIME;
			x -= ent->gravity * sv_gravity->value * FRAMETIME;
			if (x < 0)
				break;
		}
	}

//	Set target.

	if (!ent->client->havetarget && ent->client->routetrace)
	{
		int	it_num;																					//CW++

		j = Bot[ent->client->pers.botindex].skill[PRIMARYWEAP];
		it_num = ITEM_INDEX(GetWeaponType(j));														//CW++
		if (j && !ent->client->pers.inventory[it_num])												//CW
		{
			it = &itemlist[it_num];																	//CW
			if ((ent->client->enemy_routeindex < ent->client->pers.routeindex) || (ent->client->enemy_routeindex >= TotalRouteNodes))
				ent->client->enemy_routeindex = ent->client->pers.routeindex;

			for (i = ent->client->enemy_routeindex + 1; i < ent->client->enemy_routeindex + 50; i++)
			{
				if (i > TotalRouteNodes)
					break;

				if (Route[i].state == GRS_ITEMS)
				{
					if (Route[i].ent->item == it)
					{
						ent->client->havetarget = true;
						break;
					}
					else if (Route[i].ent->solid == SOLID_TRIGGER)
					{
						if (Route[i].ent->item == &itemlist[it_num])								//CW
						{
							ent->client->havetarget = true;
							break;
						}
					}
				}
			}

			ent->client->enemy_routeindex = i;
		}
		else
		{
			if ((it_num = ITEM_INDEX(item_quad)) > 0)												//CW
			{
				it = &itemlist[it_num];
				if ((ent->client->enemy_routeindex < ent->client->pers.routeindex) || (ent->client->enemy_routeindex >= TotalRouteNodes))
					ent->client->enemy_routeindex = ent->client->pers.routeindex;

				for (i = ent->client->enemy_routeindex + 1; i < ent->client->enemy_routeindex + 25; i++)
				{
					if (i > TotalRouteNodes)
						break;

					if ((Route[i].state == GRS_ITEMS) && (Route[i].ent->item == it))
					{
						if (Route[i].ent->solid == SOLID_TRIGGER)
						{
							ent->client->havetarget = true;
							break;
						}
					}
				}

				ent->client->enemy_routeindex = i;
			}
		}
	}
	else if (ent->client->havetarget)
	{
		if (ent->client->enemy_routeindex < ent->client->pers.routeindex)
		{
			ent->client->havetarget = false;
			ent->client->enemy_routeindex = ent->client->pers.routeindex;
		}
	}

//	Bot can rocket jump?

	canrocj = (ent->client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] && (ent->client->pers.inventory[ITEM_INDEX(item_rockets)] > 0));

//	Ducking check.

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		if ((ent->client->battleduckcnt > 0) && ent->groundentity)
			goto DCHCANC;

		VectorCopy(ent->s.origin, v);
		v[2] += 28;
		tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, v, ent, MASK_BOTSOLIDX);
		if (!tr.startsolid && !tr.allsolid && (tr.fraction == 1.0))
		{
			ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
			ent->maxs[2] = 32;
		}
	}
	else if ((ent->velocity[2] > 10) && (ent->groundentity == NULL))								//CW
	{
		if (!(ent->client->movestate & STS_SJMASK))
		{
			VectorSet(v, 16, 16, 40);
			tr = gi.trace(ent->s.origin, ent->mins, v, ent->s.origin, ent, MASK_BOTSOLIDX);
			if (tr.startsolid || tr.allsolid)
			{
				ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
				ent->maxs[2] = 4;
			}
		}
	}

DCHCANC:

//	Set moving speed.

	if (ent->groundentity || ent->waterlevel)
	{
		if (ent->waterlevel)
		{
			if (!(ent->client->movestate & STS_WATERJ))
				ent->client->movestate &= ~STS_SJMASK;
		}
		else
			ent->client->movestate &= ~STS_SJMASK;

		if (ent->groundentity && !ent->waterlevel)
			ent->moveinfo.speed = 1.0;
		else if (ent->waterlevel && (ent->velocity[2] <= 1))
			ent->moveinfo.speed = 1.0;
	}

	if ((ent->client->ps.pmove.pm_flags & PMF_DUCKED) && ent->groundentity)
		dist = MOVE_SPD_DUCK * ent->moveinfo.speed;
	else
	{
		if (!ent->waterlevel)																		//CW...
			dist = MOVE_SPD_RUN * ent->moveinfo.speed;
		else
		{
			if (ent->groundentity && (ent->waterlevel < 2))
				dist = MOVE_SPD_RUN * ent->moveinfo.speed;
			else
				dist = MOVE_SPD_WATER * ent->moveinfo.speed; 
		}

		if (ent->groundentity)
			dist *= ent->client->ground_slope;
	}

//	Get water depth state of bot.

	Get_WaterState(ent);

//	Search for enemy.

	ent->client->enemysearchcnt += 2;
	if (ent->client->enemysearchcnt >= 10)
	{
		Bot_SearchEnemy(ent);

		ent->client->enemysearchcnt = 1 + (rand() % 10);											//CW
		if (ent->client->enemysearchcnt > 10)
			ent->client->enemysearchcnt = 10;
		else if (ent->client->enemysearchcnt < 0)
			ent->client->enemysearchcnt = 0;
	}

//	Set bot's combat status.

	Set_Combatstate(ent);

	if (trace_priority == TRP_ALLKEEP)
		goto VCHCANSEL;

//	Browse target status.

	if (ent->client->routetrace)
	{
		if (Route[ent->client->pers.routeindex].state >= GRS_NORMAL)
			Search_NearbyPod(ent);

		Get_RouteOrigin(ent->client->pers.routeindex, v);
		if (ent->client->movestate & STS_WAITSMASK)
			ent->client->routelocktime = level.time + 1.5;
		else if ((Route[ent->client->pers.routeindex].state <= GRS_ITEMS) && (v[2] - ent->s.origin[2] > JumpMax) && !ent->client->waterstate && !(ent->client->movestate & STS_LADDERUP))
		{
			if (ent->client->routelocktime <= level.time)
			{
				ent->client->routetrace = false;
				ent->client->routereleasetime = level.time + 2.0;
			}
		}
		else if (!TraceX(ent, v))
		{
			k = 0;
			if (ent->groundentity)
			{
				if (ent->groundentity->use == train_use)
				{
					ent->client->routelocktime = level.time + 1.5;
					k = 1;
				}
			}

			if ((ent->client->routelocktime <= level.time) && !k)
			{
				ent->client->routetrace = false;
				ent->client->routereleasetime = level.time + 2.0;
			}
		}
		else
			ent->client->routelocktime = level.time + 1.5;
	}

	if (trace_priority == TRP_ALLKEEP)
		goto VCHCANSEL;

//	Climbing a ladder.

	if (ent->client->movestate & STS_LADDERUP)
	{
		ent->velocity[2] = VEL_BOT_LADDERUP;
		VectorCopy(ent->mins, trmin);
		trmin[2] += 20;

		yaw = DEG2RAD(ent->client->moveyaw);
		touchmin[0] = cos(yaw) * 32;
		touchmin[1] = sin(yaw) * 32;
		touchmin[2] = 0;
		VectorAdd(ent->s.origin, touchmin, touchmax);

		tr = gi.trace(ent->s.origin, trmin, ent->maxs, touchmax, ent, MASK_BOTSOLID);
		plane = tr.plane;
		if (!(tr.contents & CONTENTS_LADDER) && !tr.allsolid)		// off ladder
		{
			if ((ent->velocity[2] <= VEL_BOT_LADDERUP) && !ent->waterlevel)
				ent->velocity[2] = VEL_BOT_LADDERUP;

			ent->client->movestate &= ~STS_LADDERUP;
			ent->moveinfo.speed = 0.25;

			if (ent->client->routetrace)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				if (VectorLength(v) > 32)
				{
					VectorSubtract(v, ent->s.origin, v);
					ent->client->moveyaw = Get_yaw(v);
					if (trace_priority < 2)
						ent->s.angles[YAW] = ent->client->moveyaw;
				}
				else
					ent->client->pers.routeindex++;
			}
		}
		else
		{
			if (!tr.allsolid)
				VectorCopy(tr.endpos, ent->s.origin);

			VectorCopy(ent->s.origin, touchmin);
			touchmin[2] += 8;
			tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, touchmin, ent, MASK_BOTSOLID);
			x = tr.endpos[2] - ent->s.origin[2];
			ent->s.origin[2] += x;
			e = tr.ent;

			if (x == 0)
			{
				x = Get_yaw(plane.normal);

				//right
				VectorCopy(ent->s.origin, v);
				yaw = x + 90;
				if (yaw > 180)
					yaw -= 360;

				yaw = DEG2RAD(yaw);
				touchmin[0] = cos(yaw) * 48;
				touchmin[1] = sin(yaw) * 48;
				touchmin[2] = 0;
				VectorAdd(ent->s.origin, touchmin, trmin);
				VectorCopy(trmin, trmax);
				trmin[2] += 32;
				trmax[2] += 64;
				tr = gi.trace(trmin, NULL,NULL, trmax, ent, MASK_BOTSOLID);
				f1 = tr.fraction;

				//left
				VectorCopy(ent->s.origin, v);
				iyaw = x - 90;
				if (iyaw < 180)
					iyaw += 360;

				iyaw = DEG2RAD(iyaw);
				touchmin[0] = cos(iyaw) * 48;
				touchmin[1] = sin(iyaw) * 48;
				touchmin[2] = 0;
				VectorAdd(ent->s.origin, touchmin, trmin);
				VectorCopy(trmin, trmax);
				trmin[2] += 32;
				trmax[2] += 64;
				tr = gi.trace(trmin, NULL, NULL, trmax, ent, MASK_BOTSOLID);
				f2 = tr.fraction;

				x = 0.0;
				if ((f1 == 1.0) && (f2 != 1.0))
					x = yaw;
				else if ((f1 != 1.0) && (f2 == 1.0))
					x = iyaw;

				if (x != 0.0)
				{
					touchmin[0] = cos(x) * 4;
					touchmin[1] = sin(x) * 4;
					touchmin[2] = 0;
					VectorAdd(ent->s.origin, touchmin, trmin);
					tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, trmin, ent, MASK_BOTSOLID);
					if (tr.startsolid || tr.allsolid)
						x = 0;
					else
						VectorCopy(tr.endpos, ent->s.origin);
				}

				if (x == 0.0)		// off ladder
				{
					k = 0;
					if (e)
					{
						if (e->use == door_use)
						{
							if (e->moveinfo.state == PSTATE_UP)
								k = 1;
						}
					}

					if (!k)
					{
						ent->client->moveyaw += 180;
						if (ent->client->moveyaw > 180)
							ent->client->moveyaw -= 360;

						ent->client->movestate &= ~STS_LADDERUP;
						ent->moveinfo.speed = 0.25;
					}
				}
			}
		}

		if (ent->client->movestate & STS_LADDERUP)
		{
			if (ent->client->routetrace)
			{
				Get_RouteOrigin(ent->client->pers.routeindex,v);
				if (v[2] < ent->s.origin[2])
				{
					VectorSubtract(ent->s.origin, v, vv);
					vv[2] = 0;
					if (VectorLength(vv) < 32)
						ent->client->pers.routeindex++;
				}
			}

			ent->velocity[0] = 0;
			ent->velocity[1] = 0;

			goto VCHCANSEL_L;
		}
	}

//	Bot's true moving yaw, yaw pitch set (j is used ground entity check section).

	if (ent->groundentity && (ent->waterlevel <= 1) && (trace_priority < TRP_ANGLEKEEP))
		ent->s.angles[PITCH] = 0;

	if (ent->groundentity && !ent->client->routetrace)
	{
		if (trace_priority < TRP_MOVEKEEP)
			ent->client->moveyaw = ent->s.angles[YAW];
	}
	else if (trace_priority < TRP_ANGLEKEEP)
		ent->s.angles[YAW] = ent->client->moveyaw;

	if (!ent->client->routetrace && (ent->client->routereleasetime <= level.time))
	{
		if (ent->client->pers.routeindex >= TotalRouteNodes)
			ent->client->pers.routeindex = 0;

		for (i = 0; (i < TotalRouteNodes) && (i < MAX_SEARCH); i++)
		{
			if (Route[ent->client->pers.routeindex].state == GRS_GRAPHOOK)
			{
				while (1)
				{
					++ent->client->pers.routeindex;
					if (ent->client->pers.routeindex >= TotalRouteNodes)
					{
						i = TotalRouteNodes;
						break;
					}

					if (Route[ent->client->pers.routeindex].state == GRS_GRAPRELEASE)
					{
						++ent->client->pers.routeindex;
						break;
					}
				}

				continue;
			}

			else if (Route[ent->client->pers.routeindex].state == GRS_GRAPRELEASE)
			{
				++ent->client->pers.routeindex;
				continue;
			}

			Get_RouteOrigin(ent->client->pers.routeindex, v);
			if ((Route[ent->client->pers.routeindex].state <= GRS_ITEMS) && TraceX(ent, v))
			{
				if ((fabs(v[2] - ent->s.origin[2]) <= JumpMax) || (ent->client->waterstate == WAS_IN))
				{
					ent->client->routetrace = true;
					ent->client->routelocktime = level.time + 1.5;
					
					break;
				}
			}

			if (++ent->client->pers.routeindex >= TotalRouteNodes)
				ent->client->pers.routeindex = 0;
		}
	}
	else if (ent->client->routetrace)
	{
		if (Route[ent->client->pers.routeindex].state == GRS_ONDOOR)
		{
			it_ent = Route[ent->client->pers.routeindex].ent;

			if (ent->client->pers.routeindex + 1 < TotalRouteNodes)
			{
				Get_RouteOrigin(ent->client->pers.routeindex+1, v);
				ent->client->routetrace = false;
				j = TraceX(ent,v);
				ent->client->routetrace = true;

				if ((!j || (v[2] - ent->s.origin[2] > JumpMax)) && it_ent->union_ent)
				{
					k = ((it_ent->union_ent->s.origin[2] - ent->s.origin[2]) > JumpMax)?1:0;

					VectorSubtract(it_ent->union_ent->s.origin, ent->s.origin, temppos);
					yaw = Get_yaw(temppos);
					if (trace_priority < TRP_ANGLEKEEP)
					{
						ent->s.angles[PITCH] = Get_pitch(temppos);
						ent->s.angles[YAW] = yaw;
					}

					temppos[2] = 0;
					x = VectorLength(temppos);
					if ((x == 0) || k)
					{
						if (it_ent->nextthink >= level.time)
							ent->client->routelocktime = level.time + 1.5;

						goto VCHCANSEL;
					}

					if (x < dist)
						dist = x;

					if (it_ent->nextthink > level.time)
						ent->client->routelocktime = it_ent->nextthink + 1.5;
					else
						ent->client->routelocktime = level.time + 1.5;

					if (trace_priority < TRP_MOVEKEEP)
						ent->client->moveyaw = yaw;

					goto GOMOVE;
				}
			}
		
			ent->client->pers.routeindex++;
		}

		if (ent->client->pers.routeindex < TotalRouteNodes)
		{
			Get_RouteOrigin(ent->client->pers.routeindex, v);
			k = 0;
			if (Route[ent->client->pers.routeindex].state == GRS_PUSHBUTTON)
			{
				it_ent = Route[ent->client->pers.routeindex].ent;
				if (it_ent->health && (it_ent->takedamage || (it_ent->moveinfo.state != PSTATE_TOP)))
					k = 2;
				else if (it_ent->health)
				{
					ent->client->pers.routeindex++;
					if (ent->client->pers.routeindex < TotalRouteNodes)
						Get_RouteOrigin(ent->client->pers.routeindex, v);
				}
			}
			else
			{
				VectorSet(touchmax,  16,  16, 4);
				VectorSet(touchmin, -16, -16, 0);
				tr = gi.trace(ent->s.origin, touchmin, touchmax, v, ent, MASK_SHOT);
				if ((tr.fraction != 1.0) && tr.ent)
				{
					if (tr.ent->health || tr.ent->takedamage)
					{
						if ((tr.ent->classname[0] != 'p') && (tr.ent->classname[0] != 'b'))
						{
							ent->client->routelocktime = level.time + 1.5;
							it_ent = tr.ent;
							k = 1;
						}
					}
				}
			}

			if (k && !(ent->client->buttons & BUTTON_ATTACK))
			{
				trmin[0] = (it_ent->absmin[0] + it_ent->absmax[0]) * 0.5;
				trmin[1] = (it_ent->absmin[1] + it_ent->absmax[1]) * 0.5;
				trmin[2] = (it_ent->absmin[2] + it_ent->absmax[2]) * 0.5;

				if (k == 2)		// if button
				{
					VectorSet(touchmin, 0, 0, ent->viewheight-8);
					VectorAdd(ent->s.origin, touchmin, touchmin);
					tr = gi.trace(it_ent->union_ent->s.origin, NULL, NULL, trmin, it_ent->union_ent, MASK_SHOT);
					VectorSubtract(tr.endpos, ent->s.origin, trmax);
				}
				else
					VectorSubtract(v, ent->s.origin, trmax);

				if (!ent->client->current_enemy && it_ent->takedamage)		// shoot!
				{
					ent->client->newweapon = item_deserteagle;									//CW...
					ChangeWeapon(ent);
					ent->client->pers.weapon->use(ent, item_deserteagle);
				}

				if (!ent->client->current_enemy || it_ent->takedamage)
				{
					ent->s.angles[YAW] = Get_yaw(trmax);
					ent->s.angles[PITCH] = Get_pitch(trmax);
				}

				if (it_ent->takedamage)
					ent->client->buttons |= BUTTON_ATTACK;

				if (k == 2)
				{
					if (it_ent->moveinfo.state != PSTATE_TOP)
						goto VCHCANSEL;
				}
				else
				{
					if (!TraceX(ent,v))
						goto VCHCANSEL;
				}
			}

			if ((Route[ent->client->pers.routeindex].state == GRS_ONTRAIN) && !ent->client->waterstate)
			{
				Get_RouteOrigin(ent->client->pers.routeindex-1, trmin);
				if ((trmin[2] - ent->s.origin[2] > JumpMax) && (v[2] - ent->s.origin[2] > JumpMax) && (ent->waterlevel < 3))
					ent->client->routetrace = false;
			}

			f2 = (ent->client->waterstate == WAS_IN)?20:((ent->groundentity)?-8:0);
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				f1 = -16;
			else
			{
				if (ent->client->waterstate == WAS_IN)
					f1 = 24;
				else if (ent->waterlevel && (ent->waterlevel < 3))
					f1 = (((v[0] == ent->s.origin[0]) && (v[1] == ent->s.origin[1]))?-300:(-(JumpMax + 64)));
				else
					f1 = -(JumpMax + 64);
			}

			yaw = (Route[ent->client->pers.routeindex].state == GRS_ONROTATE) ? -48 : 12;
			if ((v[0] <= ent->absmax[0] - yaw) && (v[0] >= ent->absmin[0] + yaw))
			{
				if ((v[1] <= ent->absmax[1] - yaw) && (v[1] >= ent->absmin[1] + yaw))
				{
					if (((v[2] <= ent->absmax[2] - f1) && (v[2] >= ent->absmin[2] + f2)) || (Route[ent->client->pers.routeindex].state == GRS_ONROTATE))
					{
						if (ent->client->pers.routeindex < TotalRouteNodes)
						{
							if (Route[ent->client->pers.routeindex].state <= GRS_ITEMS)
							{
								if (ent->client->havetarget)
								{
									for (i = 0; i < MAXLINKPOD; i++)
									{
										if (!(k = Route[ent->client->pers.routeindex].linkpod[i]))
											break;

										// try to change index
										if ((k > ent->client->pers.routeindex) && (k < ent->client->enemy_routeindex))
										{
											ent->client->pers.routeindex = k;
											break;
										}
									}
								}
								else if (random() < 0.2)
								{
									for (i = 0; i < MAXLINKPOD; i++)
									{
										if (!(k = Route[ent->client->pers.routeindex].linkpod[i]))
											break;

										// try to change index
										if ((k > ent->client->pers.routeindex) && (k < ent->client->enemy_routeindex))
										{
											if (random() < 0.5)
											{
												ent->client->pers.routeindex = k;
												break;
											}
										}
									}
								}
							}

							ent->client->pers.routeindex++;

							// not a normal pod
							if (!(ent->client->pers.routeindex < TotalRouteNodes))
								ent->client->pers.routeindex = 0;
						}
					}
				}
			}


			if ((ent->client->pers.routeindex < TotalRouteNodes) && trace_priority)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				VectorSubtract(v, ent->s.origin, temppos);
				if (trace_priority < TRP_ANGLEKEEP)
					ent->s.angles[PITCH] = Get_pitch(temppos);

				k = 0;
				if (ent->groundentity || ent->waterlevel)
				{
					yaw = temppos[2];
					temppos[2] = 0;
					x = VectorLength(temppos);
																									//CW
					if (trace_priority < TRP_MOVEKEEP)
						ent->client->moveyaw = Get_yaw(temppos);	// set the moving yaw

					if ((ent->groundentity || ent->waterlevel) && (trace_priority < TRP_ANGLEKEEP))
					{
						ent->s.angles[YAW] = ent->client->moveyaw;
						k = 1;
					}

					if ((x < dist) && (fabs(yaw) < 20) && k)
					{
						iyaw = Get_yaw(temppos);
						i = Bot_TestMove(ent, iyaw, temppos, x, &bottom);

						tr = gi.trace(v, ent->mins, ent->maxs, v, ent, MASK_BOTSOLIDX);
						if ((Route[ent->client->pers.routeindex].state == GRS_ITEMS) && !i)
						{
							if (x < 30)
								ent->client->pers.routeindex++;
						}
						else if (((Route[ent->client->pers.routeindex].state == GRS_ITEMS) || (Route[ent->client->pers.routeindex].state == GRS_NORMAL))
							&& !tr.allsolid && !tr.startsolid && HazardCheck(ent, v) && (fabs(bottom) < 20) && i && !ent->waterlevel)
						{
							if (((v[2] < ent->s.origin[2]) && (bottom < 0)) || ((v[2] >= ent->s.origin[2]) && (bottom >= 0)))
							{
								VectorCopy(temppos, ent->s.origin);
								VectorCopy(v, trmin);
								dist -= x;
								if (Route[ent->client->pers.routeindex].state <= GRS_ITEMS)
								{
									if (ent->client->havetarget)
									{
										for (i = 0; i < MAXLINKPOD; i++)
										{
											if (!(j = Route[ent->client->pers.routeindex].linkpod[i]))
												break;

											if ((j > ent->client->pers.routeindex) && (j < ent->client->enemy_routeindex))
											{
												ent->client->pers.routeindex = j;
												break;
											}
										}
									}
								}

								ent->client->pers.routeindex++;
								if (i == 2)
									ent->client->ps.pmove.pm_flags |= PMF_DUCKED;

								Get_RouteOrigin(ent->client->pers.routeindex, v);
								VectorSubtract(v, ent->s.origin, temppos);
								
								if (trace_priority < TRP_ANGLEKEEP)
									ent->s.angles[PITCH] = Get_pitch(temppos);

								if (trace_priority < TRP_MOVEKEEP)
									ent->client->moveyaw = Get_yaw(temppos);

								if (k && trace_priority < TRP_ANGLEKEEP)
									ent->s.angles[YAW] = ent->client->moveyaw;
							}
						}
						else if (((Route[ent->client->pers.routeindex].state == GRS_ITEMS) || (Route[ent->client->pers.routeindex].state == GRS_NORMAL))
							&& (fabs(bottom) < 20) && ent->waterlevel)
						{
							if (((v[2] < ent->s.origin[2]) && (bottom < 0)) || ((v[2] >= ent->s.origin[2]) && (bottom >= 0)))
							{
								VectorCopy(temppos, ent->s.origin);
								VectorCopy(v, trmin);
								dist -= x;

								ent->client->pers.routeindex++;
								Get_RouteOrigin(ent->client->pers.routeindex, v);
								VectorSubtract(v, ent->s.origin, temppos);

								if (trace_priority < TRP_ANGLEKEEP)
									ent->s.angles[PITCH] = Get_pitch(temppos);

								if (trace_priority < TRP_MOVEKEEP)
									ent->client->moveyaw = Get_yaw(temppos);

								if (k && trace_priority < TRP_ANGLEKEEP)
									ent->s.angles[YAW] = ent->client->moveyaw;
							}
							else
								dist = x;
						}
						else
							dist = x;
					}
					else if (x < dist)
						dist = x;

					k = 0;
					if ((ent->client->pers.routeindex - 1 >= 0) && ((Route[ent->client->pers.routeindex].state == GRS_ONPLAT) || (Route[ent->client->pers.routeindex].state == GRS_ONTRAIN)))
					{
						Get_RouteOrigin(ent->client->pers.routeindex-1, v);
						if (fabs(v[2] - ent->s.origin[2]) <= JumpMax)
						{
							if ((ent->client->waterstate < WAS_IN) && (Route[ent->client->pers.routeindex].ent->nextthink > level.time))
								k = 1;
						}
					}

					if (k && !(ent->client->movestate & STS_WAITS))
					{
						if (ent->client->pers.routeindex + 1 < TotalRouteNodes)
						{
							Get_RouteOrigin(ent->client->pers.routeindex+1, v);
							if (v[2] - ent->s.origin[2] > JumpMax)
							{
								if (Route[ent->client->pers.routeindex].ent->union_ent->s.origin[2] - ent->s.origin[2] > JumpMax)
								{
									ent->client->waiting_obj = Route[ent->client->pers.routeindex].ent;
									ent->client->movestate |= STS_W_COMEPLAT;
									k = 0;
									for (i = 1; i <= 3; i++)
									{
										if (ent->client->pers.routeindex - i >= 0)
										{
											Get_RouteOrigin(ent->client->pers.routeindex-i, v);
											if (ent->client->waiting_obj->absmax[0] < v[0] + ent->mins[0])
												k = 1;
											else if (ent->client->waiting_obj->absmax[1] < v[1] + ent->mins[1])
												k = 1;
											else if (ent->client->waiting_obj->absmin[0] > v[0] + ent->maxs[0])
												k = 1;
											else if (ent->client->waiting_obj->absmin[1] > v[1] + ent->maxs[1])
												k = 1;

											if (k)
												break;
										}
									}

									if (k)
										VectorCopy(v, ent->client->movtarget_pt);
									else
										Get_RouteOrigin(ent->client->pers.routeindex-1, ent->client->movtarget_pt);
									goto VCHCANSEL;
								}
							}
						}
					}
				}
			}
			else if (ent->client->pers.routeindex >= TotalRouteNodes)
			{
				ent->client->pers.routeindex = 0;
				ent->client->routetrace = false;
			}
		}
		else
		{
			ent->client->pers.routeindex = 0;
			ent->client->routetrace = false;
		}
	}

//	Ground entity check.

	if (!(ent->client->movestate & STS_W_DOOROPEN) && (!ent->groundentity || (ent->groundentity != ent->client->waiting_obj)))
	{
		if (!(ent->client->waiting_obj && (ent->client->waiting_obj->use == door_use)))
		{
			ent->client->movestate &= ~STS_WAITS;
			ent->client->waiting_obj = NULL;
		}
	}

	if (ent->groundentity && !(ent->client->movestate & STS_WAITS))
	{
		it_ent = ent->groundentity;
		if (it_ent->classname[0] == 'f')
		{
			// on platform
			if (it_ent->use == Use_Plat)
			{
				if ((it_ent->pos1[2] > it_ent->pos2[2]) && (((it_ent->moveinfo.state == PSTATE_UP) && (it_ent->velocity[2] > 0)) || (it_ent->moveinfo.state == PSTATE_BOTTOM)))
				{
					ent->client->waiting_obj = it_ent;
					ent->client->movestate |= STS_W_ONPLAT;

					if (ent->client->routetrace)
					{
						if (Route[ent->client->pers.routeindex].ent == ent->client->waiting_obj)
						{
							if (Route[ent->client->pers.routeindex].state == GRS_ONPLAT)
							{
								if (ent->client->waiting_obj->union_ent->s.origin[2] > ent->s.origin[2] + 32)
								{
									ent->client->movestate &= ~STS_W_ONPLAT;
									ent->client->movestate |= STS_W_COMEPLAT;
								}
								else
									ent->client->pers.routeindex++;
							}
						}
					}
				}
			}

			// on train
			else if ((it_ent->use == train_use) && (it_ent->nextthink >= level.time) && ((it_ent->s.origin[2] - it_ent->s.old_origin[2] > 0) || ent->client->routetrace))
			{
				if (ent->client->routetrace && (ent->client->pers.routeindex > 0))
				{
					j = 0;
					k = ent->client->pers.routeindex - 1;
					for (i = 0; i < 3; i++)
					{
						if (k + i < TotalRouteNodes)
						{
							if (Route[k+i].state == GRS_ONTRAIN)
							{
								if (Route[k+i].ent == it_ent)
									j = 1;
								else if (it_ent->trainteam)
								{
									e = it_ent->trainteam;
									while (1)
									{
										if (e == it_ent)
											break;

										if (e == Route[k+i].ent)
										{
											j = 1;
											it_ent = e;
											Route[k+i].ent = e;
											break;
										}
										e = e->trainteam;
									}
								}
								else if (it_ent->target_ent)
								{
									if (VectorCompare(Route[k+i].Tcorner, it_ent->target_ent->s.origin))
									{
										j = 1;
										break;
									}
								}

								if (j)
									break;
							}
						}
						else break;
					}

					if (j)		// on train
					{
						ent->client->movestate |= STS_W_ONTRAIN;
						ent->client->waiting_obj = it_ent;
						ent->client->pers.routeindex = k + i + 1;
					}
				}
				else
				{
					if (it_ent->s.origin[2] - it_ent->s.old_origin[2] > 0)
					{
						ent->client->movestate |= STS_W_ONTRAIN;
						ent->client->waiting_obj = it_ent;
					}
					else if ((it_ent->s.origin[2] - it_ent->s.old_origin[2] > -2) && trace_priority)
					{
						ent->client->movestate |= STS_W_ONTRAIN;
						ent->client->waiting_obj = it_ent; 
					}
					else
						ent->client->movestate |= STS_W_DONT;
				}
			}
		}
	}

	// clear waiting state if flagged
	if ((ent->client->movestate & STS_W_DONT) && ent->groundentity)
	{
		if (ent->client->movestate & STS_W_ONPLAT)
		{
			if (ent->groundentity->use == Use_Plat)
			{
				ent->client->movestate &= ~STS_WAITS;
				ent->client->waiting_obj = NULL;
			}
		}
		else if (ent->client->movestate & STS_W_ONTRAIN)
		{
			if (ent->groundentity->use == train_use)
			{
				ent->client->movestate &= ~STS_WAITS;
				ent->client->waiting_obj = NULL;
			}
		}
		else if (ent->client->movestate & (STS_W_ONDOORUP | STS_W_ONDOORDWN))
		{
			if (ent->groundentity->use == door_use)
			{
				ent->client->movestate &= ~STS_WAITS;
				ent->client->waiting_obj = NULL;
			}
		}
		else
		{
			ent->client->movestate &= ~STS_WAITS;
			ent->client->waiting_obj = NULL;
		}
	}

	// on plat
	else if ((ent->client->movestate & (STS_W_ONPLAT | STS_W_COMEPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN)) && !(ent->client->movestate & STS_W_DONT))
	{
		k = 0;

		//if door
		if (ent->client->movestate & (STS_W_ONDOORUP | STS_W_ONDOORDWN))
		{
			if (ent->client->movestate & STS_W_ONDOORUP)		// going up
			{
				if ((ent->client->waiting_obj->moveinfo.state == PSTATE_UP) || (ent->client->waiting_obj->moveinfo.state == PSTATE_BOTTOM))
					k = 1;
			}
			else												// going down
			{
				if ((ent->client->waiting_obj->moveinfo.state == PSTATE_TOP) || (ent->client->waiting_obj->moveinfo.state == PSTATE_DOWN))
					k = 1;
			}
		}
		else if (ent->client->movestate & STS_W_COMEPLAT)
		{
			if (Route[ent->client->pers.routeindex].state == GRS_ONTRAIN)
			{
				if (!TraceX(ent, Route[ent->client->pers.routeindex].ent->union_ent->s.origin))
					k = 1;

				if (Route[ent->client->pers.routeindex].ent->union_ent->s.origin[2] + 8 - ent->s.origin[2] > JumpMax)
					k = 1;
			}
			else
			{
				if (ent->client->waiting_obj->union_ent->s.origin[2] - ent->s.origin[2] > JumpMax)
					k = 1;
			}

			if ((ent->client->pers.routeindex - 1 > 0) && (ent->client->waterstate < WAS_IN))
			{
				Get_RouteOrigin(ent->client->pers.routeindex-1, trmin);
				if ((trmin[2] - ent->s.origin[2] > JumpMax) && (v[2] - ent->s.origin[2] > JumpMax))
					k = 0;
			}
		}
		else
		{
			if ((ent->client->waiting_obj->moveinfo.state == PSTATE_UP) || (ent->client->waiting_obj->moveinfo.state == PSTATE_BOTTOM))
				k = 1;

			if (ent->client->waiting_obj->moveinfo.state == PSTATE_BOTTOM)
				plat_go_up(ent->client->waiting_obj);

			if (ent->client->routetrace)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				if (ent->s.origin[2] > v[2])
					k = 2;
			}
		}

		// have target
		if (k != 1)
		{
			if (k == 2)
				ent->client->movestate |= STS_W_DONT;
			else
			{
				ent->client->movestate &= ~STS_WAITS;
				ent->client->waiting_obj = NULL;
			}
		}
		else
		{
			if (ent->client->movestate & STS_W_COMEPLAT)
			{
				k = 0;
				if (ent->client->pers.routeindex - 1 > 0)
				{
					VectorCopy(ent->client->movtarget_pt, trmax);
					trmax[2] = 0;
					k = 1;
				}

				if (!k)
					goto VCHCANSEL;
			}
			else
			{
				trmax[0] = (ent->client->waiting_obj->absmin[0] + ent->client->waiting_obj->absmax[0]) * 0.5;
				trmax[1] = (ent->client->waiting_obj->absmin[1] + ent->client->waiting_obj->absmax[1]) * 0.5;
				trmax[2] = 0;
			}

			VectorSubtract(trmax, ent->s.origin, temppos);
			yaw = temppos[2];
			temppos[2] = 0;
			x = VectorLength(temppos);
			if (x == 0)
				goto VCHCANSEL;			// if center position move cancel

			if (x < dist)
				dist = x;

			if (trace_priority < TRP_MOVEKEEP)
				ent->client->moveyaw = Get_yaw(temppos);
		}
	}

	// on train
	else if (ent->client->movestate & STS_W_ONTRAIN)
	{
		i = 0;
		if (ent->client->routetrace)
		{
			Get_RouteOrigin(ent->client->pers.routeindex, v);
			if (ent->client->pers.routeindex - 1 >= 0)
			{
				if (Route[ent->client->pers.routeindex-1].state != GRS_ONTRAIN)
					i = 1;
			}
			else
				i = 1;

			if (TraceX(ent, v))
			{
				if (v[2] - ent->s.origin[2] <= JumpMax)
					i = 1;
				else
					ent->client->routelocktime = level.time + 1.5;
			}
			else
				ent->client->routelocktime = level.time + 1.5;
		}
		else if (j || (ent->client->waiting_obj->s.origin[2] - ent->client->waiting_obj->s.old_origin[2] <= 0))
			i = 1;																					//CW

		if (i)																						//CW...
		{
			ent->client->movestate |= STS_W_DONT;
			ent->client->movestate &= ~STS_WAITS;
		}
		else
		{
			k = 0;
			if (ent->client->routetrace)
			{
				tr = gi.trace(ent->s.origin, NULL, NULL, v, ent, MASK_BOTSOLIDX);
				if (tr.ent == ent->client->waiting_obj)
				{
					tr = gi.trace(v, NULL, NULL, ent->s.origin, ent, MASK_BOTSOLIDX);
					if (tr.ent == ent->client->waiting_obj)
					{
						VectorSubtract(v, ent->s.origin, temppos);
						k = 1;
					}
				}
			}

			if (!k)
			{
				VectorCopy(ent->client->waiting_obj->union_ent->s.origin, trmax);
				trmax[2] += 8;
				VectorSubtract(trmax, ent->s.origin, temppos);
				yaw = temppos[2];
				temppos[2] = 0;
				x = VectorLength(temppos);
				if (x < dist)
					dist = x;
			}

			if (trace_priority < TRP_MOVEKEEP)
				ent->client->moveyaw = Get_yaw(temppos);
		}

		goto GOMOVE;
	}

	//wait for door open
	else if (ent->client->movestate & STS_W_DOOROPEN)
	{
		if (!trace_priority || (ent->client->waiting_obj->moveinfo.state == PSTATE_TOP))
		{
			ent->client->movestate &= ~STS_WAITS;
			ent->client->waiting_obj = NULL;
		}
		else if ((ent->client->waiting_obj->moveinfo.state == PSTATE_BOTTOM) || (ent->client->waiting_obj->moveinfo.state == PSTATE_UP))
		{
			VectorSubtract(ent->client->movtarget_pt, ent->s.origin, temppos);
			temppos[2] = 0;
			dist *= 0.25;
			if ((VectorLength(temppos) < 10) || VectorCompare(ent->s.origin, ent->client->movtarget_pt)) 
			{
				if (!ent->client->waiting_obj->union_ent)
				{
					trmin[0] = (ent->client->waiting_obj->absmin[0] + ent->client->waiting_obj->absmax[0]) * 0.5;
					trmin[1] = (ent->client->waiting_obj->absmin[1] + ent->client->waiting_obj->absmax[1]) * 0.5;
					trmin[2] = (ent->client->waiting_obj->absmin[2] + ent->client->waiting_obj->absmax[2]) * 0.5;
				}
				else
					VectorCopy(ent->client->waiting_obj->union_ent->s.origin, trmin);

				trmin[2] += 8;
				VectorSubtract(trmin, ent->s.origin, temppos);
				if (trace_priority < TRP_MOVEKEEP)
					ent->client->moveyaw = Get_yaw(temppos);

				if (trace_priority < TRP_ANGLEKEEP)
				{
					ent->s.angles[YAW] = ent->client->moveyaw;
					ent->s.angles[PITCH] = Get_pitch(temppos);
				}

				goto VCHCANSEL;
			}
			else {
				if (trace_priority < TRP_MOVEKEEP)
					ent->client->moveyaw = Get_yaw(temppos);

				if (!ent->client->waiting_obj->union_ent)
				{
					trmin[0] = (ent->client->waiting_obj->absmin[0] + ent->client->waiting_obj->absmax[0]) * 0.5;
					trmin[1] = (ent->client->waiting_obj->absmin[1] + ent->client->waiting_obj->absmax[1]) * 0.5;
					trmin[2] = (ent->client->waiting_obj->absmin[2] + ent->client->waiting_obj->absmax[2]) * 0.5;
				}
				else
					VectorCopy(ent->client->waiting_obj->union_ent->s.origin, trmin);

				trmin[2] += 8;
				VectorSubtract(trmin,ent->s.origin, temppos);
				if (trace_priority < TRP_ANGLEKEEP)
				{
					ent->s.angles[YAW] = Get_yaw(temppos);
					ent->s.angles[PITCH] = Get_pitch(temppos);
				}
			}
		}
	}

	//bot move to moveyaw
GOMOVE:

	//jumping
	if (!ent->groundentity && !ent->waterlevel)
	{
		if ((ent->velocity[2] > VEL_BOT_JUMP) && !(ent->client->movestate & STS_SJMASKEXW))			//CW
			ent->velocity[2] = VEL_BOT_JUMP;

		k = (ent->client->ps.pmove.pm_flags & PMF_DUCKED)?1:0;

		for (x = 0; x < 90; x += 10)
		{
			dist = MOVE_SPD_RUN * ent->moveinfo.speed;												//CW
			yaw = ent->client->moveyaw + x;
			if (yaw > 180)
				yaw -= 360;

			i = Bot_TestMove(ent, yaw, temppos, dist, &bottom);										//CW
			if (i)
			{
				if ((bottom <= 24) && (bottom > 0))
				{
					if (ent->velocity[2] <= 10)
					{
						VectorCopy(temppos, ent->s.origin);
						break;
					}
				}

				if (!ent->waterlevel && (ent->s.origin[2] > ent->s.old_origin[2]) && ent->client->routetrace
					&& !(ent->client->movestate & STS_LADDERUP) && !(ent->client->movestate & STS_SJMASK)	//CW
					&& (ent->client->pers.routeindex + 1 < TotalRouteNodes)
					&& (ent->velocity[2] >= 100) && (ent->velocity[2] < 100 + (ent->gravity * sv_gravity->value * 0.1)))
				{
					Get_RouteOrigin(ent->client->pers.routeindex, v);
					Get_RouteOrigin(ent->client->pers.routeindex+1, vv);
					k = 0;
					j = Bot_TestMove(ent, yaw, trmin, 16, &f1);
					VectorSubtract(v, ent->s.origin, trmin);
					if (vv[2] - v[2] > JumpMax)
						k = 1;
					else if (v[2] - ent->s.origin[2] > JumpMax)
						k = 2;
					else if (!TargetJump_Chk(ent, vv, 0) && (VectorLength(trmin) < 64))
					{
						if (TargetJump_Chk(ent, vv, ent->velocity[2]))
							k = 1;
					}

					if (!j)
						k = 0;
					else
					{
						if ((f1 > 10) && (f1 < -10))
							k = 0;
					}

					if (k)
					{
						if (k == 2)
							VectorCopy(v, vv);

						if (TargetJump(ent, vv))
						{
							VectorSubtract(vv, ent->s.origin, v);
							ent->client->moveyaw = Get_yaw(v);
							if (ent->velocity[2] > VEL_BOT_JUMP)									//CW
								ent->client->movestate |= STS_TURBOJ;								//CW

							if (k == 1)
								ent->client->pers.routeindex++;

							break;
						}
					}
				}

				if (bottom <= 0)
				{
					VectorCopy(temppos, ent->s.origin);
					if (i == 2)
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					else
						ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
					break;
				}
				else
					ent->moveinfo.speed = 0.3;
			}
			else
				ent->moveinfo.speed = 0.3;

			if (x == 0)
				continue;

			// left trace
			yaw = ent->client->moveyaw - x;
			if (yaw < -180)
				yaw += 360;

			i = Bot_TestMove(ent, yaw, temppos, dist, &bottom);
			if (i)
			{
				if ((bottom <= 24) && (bottom > 0) && (ent->velocity[2] <= 10))
				{
					VectorCopy(temppos, ent->s.origin);
					break;
				}

				// turbo
				if (!ent->waterlevel && (ent->s.origin[2] > ent->s.old_origin[2])
					&& ent->client->routetrace && !(ent->client->movestate & STS_LADDERUP) && !(ent->client->movestate & STS_SJMASK)	//CW
					&& (ent->client->pers.routeindex + 1 < TotalRouteNodes)
					&& (ent->velocity[2] >= 100) && (ent->velocity[2] < 100 + (ent->gravity * sv_gravity->value * 0.1)))
				{
					Get_RouteOrigin(ent->client->pers.routeindex, v);
					Get_RouteOrigin(ent->client->pers.routeindex+1, vv);
					k = 0;
					j = Bot_TestMove(ent, yaw, trmin, 16, &f1);
					VectorSubtract(v, ent->s.origin, trmin);
					if (vv[2] - v[2] > JumpMax)
						k = 1;
					else if (v[2] - ent->s.origin[2] > JumpMax)
						k = 2;
					else if (!TargetJump_Chk(ent, vv, 0) && (VectorLength(trmin) < 64))
					{
						if (TargetJump_Chk(ent, vv, ent->velocity[2]))
							k = 1;
					}

					if (!j)
						k = 0;
					else if ((f1 > 10) && (f1 < -10))
						k = 0;

					if (k)
					{
						if (k == 2)
							VectorCopy(v, vv);

						if (TargetJump(ent, vv))
						{
							VectorSubtract(vv, ent->s.origin, v);
							ent->client->moveyaw = Get_yaw(v);
							if (ent->velocity[2] > VEL_BOT_JUMP)									//CW
								ent->client->movestate |= STS_TURBOJ;								//CW

							if (k == 1)
								ent->client->pers.routeindex++;

							break;
						}
					}
				}

				if (bottom <= 0)
				{
					VectorCopy(temppos, ent->s.origin);
					if (i == 2)
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					else
						ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;

					break;
				}
				else
					ent->moveinfo.speed = 0.3;
			}
			else
				ent->moveinfo.speed = 0.3;
		}

		if (x >= 90)
		{	// jump fail!
			if (trace_priority < TRP_ANGLEKEEP)
				ent->s.angles[YAW] += ((random() - 0.5) * 360);

			if (ent->s.angles[YAW] > 180)
				ent->s.angles[YAW] -= 360;
			else if (ent->s.angles[YAW] < -180)
				ent->s.angles[YAW] += 360;
		}

		goto VCHCANSEL;
	}

	// on ground or in water
	waterjumped = false;
	if (ent->groundentity || ent->waterlevel)
	{
		if (ent->groundentity && (ent->waterlevel <= 0))
			k = 1;
		else if (ent->waterlevel)
		{
			k = 2;
			if (ent->client->routetrace)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				VectorSubtract(v, ent->s.origin, vv);
				vv[2] = 0;
				if ((v[2] < ent->s.origin[2]) && (VectorLength(vv) < 24))
					k = 0;
			}

			if (ent->waterlevel == 3)
				k = 0;
		}
		else if (ent->waterlevel)
			k = 0;
		else
			k = 1;

		if (k)
		{
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				k = 0;
		}
		
		f1 = (ent->client->waterstate) ? BOTTOM_LIMIT_WATER : -JumpMax;

		if (ent->client->nextcheck < level.time + 1.0)
		{
			VectorSubtract(ent->client->my_old_origin, ent->s.origin, temppos);
			if (VectorLength(temppos) < 64)
			{
				if (ent->client->routetrace)
				{
					if (!(int)chedit->value)
					{
						ent->client->routetrace = false;
						ent->client->pers.routeindex++;
					}
				}
				else
					f1 = BOTTOM_LIMITM;
			}

			if (ent->client->nextcheck < level.time)
			{
				VectorCopy(ent->s.origin, ent->client->my_old_origin);
				ent->client->nextcheck = level.time + 4.0;
			}
		}

		f3 = 20;		//movablegap

		//this v not modify till do special
		if (ent->client->routetrace)
			Get_RouteOrigin(ent->client->pers.routeindex, v);

		if (ent->waterlevel && ent->client->routetrace)
		{
			if (v[2] + 20 <= ent->s.origin[2])
			{
				f2 = 20;
				f3 = 0;
			}
			else
				f2 = JumpMax;
		}
		else
			f2 = JumpMax;

		ladderdrop = true;
		for (x = 0; (x <= 180) && (dist != 0); x += 10)
		{
			// right trace
			yaw = ent->client->moveyaw + x;
			if (yaw > 180)
				yaw -= 360;

			if ((j = Bot_TestMove(ent, yaw, temppos, dist, &bottom)) > 0)							//CW
			{
				if ((x == 0) && !ent->waterlevel && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				{
					if (ent->client->routetrace)
					{
						if ((v[2] - (ent->s.origin[2] + bottom) > f2) || ((bottom > 20) && (v[2] > ent->s.origin[2])))
						{
							ladderdrop = false;
							if (Bot_Fall(ent, temppos, dist) && !ent->client->waterstate)
							{
								ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
								break;
							}

							if (v[2] - ent->s.origin[2] <= JumpMax)
							{
								if ((Route[ent->client->pers.routeindex].state == GRS_ONTRAIN) && (ent->client->waterstate < WAS_IN))
									break;

								if (ent->client->pers.routeindex > 0)
								{
									if ((Route[ent->client->pers.routeindex-1].state == GRS_ONTRAIN) && (Route[ent->client->pers.routeindex-1].ent == ent->groundentity))
										break;
								}
							}
						}
						else if (ent->groundentity)
						{
							if (ent->groundentity->use == rotating_use)
							{
								if (Bot_Fall(ent, temppos, dist))
								{
									ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
									break;
								}
							}
							else if (Route[ent->client->pers.routeindex].state == GRS_ONROTATE)
							{
								if (!TraceX(ent, v))
									break;

								if (!HazardCheck(ent, v))
									break;

								if (!BankCheck(ent, v))
									break;

								if (Bot_Fall(ent, temppos, dist))
								{
									ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
									break;
								}
							}
						}
					}
				}

				// jumpable 1
				if ((bottom > 20) && (bottom <= f2) && (j == 1) && k && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				{
					ent->moveinfo.speed = 0.15;
					if (k == 1)
						ent->velocity[2] = VEL_BOT_JUMP;											//CW
					else
					{
						ent->moveinfo.speed = 0.1;

						// waterjumped
						if ((ent->velocity[2] < VEL_BOT_WJUMP) || VectorCompare(ent->s.origin, ent->s.old_origin))
						{
							ent->velocity[2] = VEL_BOT_WJUMP;
							ent->client->movestate |= STS_WATERJ;
						}

						goto VCHCANSEL;
					}

					SetBotAnim(ent);
					ent->client->moveyaw = yaw;
					ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
					break;
				}

				//dropable1
				else if ((bottom <= f3) && ((bottom >= f1) || ent->waterlevel))
				{
					if ((bottom < 0) && !ent->client->waterstate)
					{
						f2 = FRAMETIME * (ent->velocity[2] - (ent->gravity * sv_gravity->value * FRAMETIME));
						if ((bottom >= f2) && (ent->velocity[2] < 0))
							temppos[2] += bottom;
						else
							temppos[2] += f2;
					}

					if (!ent->tractored)															//CW++
						VectorCopy(temppos, ent->s.origin);

					if (f1 > BOTTOM_LIMIT)
						ent->moveinfo.speed = 0.25;

					if (j != 1)
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					else
						ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;

					if ((x > 30) || !ent->client->routetrace)
					{
						f2 = ent->client->moveyaw;
						ent->client->moveyaw = yaw;
						if ((f2 == ent->s.angles[YAW]) && (trace_priority < TRP_ANGLEKEEP))
							ent->s.angles[YAW] = yaw;
					}

					break;
				}

				// dropable?1
				else if ((bottom < f1) && !ent->client->waterstate && (x <= 30))
				{
					if (ladderdrop && (bottom != -9999) && (ent->client->ground_contents & CONTENTS_LADDER))
					{
						VectorCopy(temppos, ent->s.origin);
						ent->client->moveyaw = yaw;
						ent->moveinfo.speed = 0.2;
						goto VCHCANSEL;
					}

					if (ladderdrop && (bottom < 0) && !ent->client->waterstate)
					{
						if (Bot_moveW(ent, yaw, temppos, dist, &bottom))
						{
							iyaw = -41;
							if ((bottom > -20) && (iyaw < -40))
							{
								VectorCopy(temppos, ent->s.origin);
								break;
							}
						}
					}

					// fall!
					if (Bot_Fall(ent, temppos, dist))
						break;
				}
			}

			if ((x == 0) && (ent->client->battlemode & FIRE_SHIFT))
				ent->client->battlemode &= ~FIRE_SHIFT;

			if ((x == 0) || (x == 180))
				continue;

			yaw = ent->client->moveyaw - x;
			if (yaw < -180)
				yaw += 360;

			if ((j = Bot_TestMove(ent, yaw, temppos, dist, &bottom)) > 0)							//CW
			{
				f2 = (ent->client->waterstate == WAS_FLOAT)?TOP_LIMIT_WATER:JumpMax;
				if ((bottom > 20) && (bottom <= f2) && (j == 1) && k && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				{
					ent->moveinfo.speed = 0.15;
					if (k == 1)
						ent->velocity[2] = VEL_BOT_JUMP;
					else
					{
						ent->moveinfo.speed = 0.1;

						// waterjumped
						if ((ent->velocity[2] < VEL_BOT_WJUMP) || VectorCompare(ent->s.origin, ent->s.old_origin))
						{
							ent->velocity[2] = VEL_BOT_WJUMP;
							ent->client->movestate |= STS_WATERJ;
						}

						goto VCHCANSEL;
					}

					SetBotAnim(ent);
					ent->client->moveyaw = yaw;
					ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
					break;
				}

				// dropable2
				else if ((bottom <= f3) && ((bottom >= f1) || ent->waterlevel))
				{
					if ((bottom < 0) && !ent->client->waterstate)
					{
						f2 = FRAMETIME * (ent->velocity[2] - (ent->gravity * sv_gravity->value * FRAMETIME));
						if ((bottom >= f2) && (ent->velocity[2] < 0))
							temppos[2] += bottom;
						else
							temppos[2] += f2;
					}

					VectorCopy(temppos, ent->s.origin);
					if (f1 > BOTTOM_LIMIT)
						ent->moveinfo.speed = 0.25;

					if (j != 1)
						ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
					else
						ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;

					if ((x > 30) || !ent->client->routetrace)
					{
						f2 = ent->client->moveyaw;
						ent->client->moveyaw = yaw;
						if ((f2 == ent->s.angles[YAW]) && (trace_priority < TRP_ANGLEKEEP))
							ent->s.angles[YAW] = yaw;
					}

					break;
				}

				// dropable?2
				else if ((bottom < f1) && !ent->client->waterstate && (x <= 30))
				{
					if (ladderdrop && (ent->client->ground_contents & CONTENTS_LADDER) && (bottom != -9999))
					{
						VectorCopy(temppos, ent->s.origin);
						ent->client->moveyaw = yaw;
						ent->moveinfo.speed = 0.2;
						goto VCHCANSEL;
					}

					if (ladderdrop && (bottom < 0) && !ent->client->waterstate)
					{
						if (Bot_moveW(ent, yaw, temppos, dist, &bottom))
						{
							iyaw = -41;
							if ((bottom > -54) && (iyaw < -40))
							{
								VectorCopy(temppos, ent->s.origin);
								break;
							}
						}
					}
					
					// fall2
					if (Bot_Fall(ent, temppos, dist))
						break;
				}
			}
		}

		if (!ent->client->routetrace && !ent->client->current_enemy)
		{
			if (trace_priority < TRP_ANGLEKEEP)
				ent->s.angles[YAW] = yaw;
		}

		if (x >= 70)
		{
			if (!ent->client->routetrace && (ent->client->current_enemy == NULL))
			{
				if (trace_priority < TRP_ANGLEKEEP)
					ent->s.angles[YAW] = yaw;
			}
			else if (ent->client->routetrace)
			{
				k = 0;

				if ((x > 90) && ent->groundentity)
				{
					if (ent->groundentity->use == train_use)										//CW
						k = 1;
				}
				else if ((x > 90) && (Route[ent->client->routeindex].state == GRS_ONTRAIN))
					k = 1;

				if (k && (trace_priority < TRP_ANGLEKEEP))
				{
					VectorCopy(tmp_org, ent->s.origin);
					VectorCopy(tmp_vel, ent->velocity);
					ent->s.angles[YAW] = tmp_yaw;
					goto VCHCANSEL;
				}

				if (!k)
				{
					ent->velocity[2] += VEL_BOT_JUMP_NUDGE;											//CW

					if (++ent->client->routeindex >= CurrentIndex)
						ent->client->routeindex = 0;

					ent->client->routetrace = false;
				}
			}
		}

		if (ent->waterlevel && !waterjumped)
		{
			k = 0;
			VectorCopy(ent->s.origin, temppos);
			if (ent->client->routetrace)
			{
				Get_RouteOrigin(ent->client->pers.routeindex, v);
				k = 2;
				x = v[2] - ent->s.origin[2];
				if (x > 13)
					x = 13;
				else if (x < -13)
					x = -13;

				if (x < 0)
				{
					if (Bot_Watermove(ent, temppos, dist, x))
					{	// down
						VectorCopy(temppos, ent->s.origin);
						k = 1;
					}
				}
				else if ((x > 0) && (ent->client->waterstate == WAS_IN) && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				{	// up
					if (ent->velocity[2] < -10)
						ent->velocity[2] = 0;

					if (Bot_Watermove(ent, temppos, dist, x))
					{
						VectorCopy(temppos, ent->s.origin);
						k = 1;
					}
				}
			}
			else if ((ent->air_finished - 2.0 < level.time) && (ent->client->waterstate == WAS_IN))
			{
				if (Bot_Watermove(ent, temppos, dist, 13))
				{
					VectorCopy(temppos, ent->s.origin);
					k = 1;
				}
				else
					k = 2;
			}

			if (k == 1)
				Get_WaterState(ent);

			if (ent->client->routetrace && (v[2] == ent->s.origin[2]))
				k = 3;

			if ((!ent->groundentity && !ent->client->waterstate && k && (ent->velocity[2] < 1))	|| ((ent->client->waterstate == WAS_IN) && (ent->client->ps.pmove.pm_flags & PMF_DUCKED)))
			{
				if (Bot_Watermove(ent, temppos, dist, -7) && (k != 3))
					VectorCopy(temppos, ent->s.origin);
			}

			if (ent->client->waterstate == WAS_IN)
				ent->moveinfo.decel = level.time;
			else if (!k)
			{
				if ((level.time - ent->moveinfo.decel > 4.0) && !ent->client->routetrace)
				{
					ent->velocity[2] = -200;
					ent->moveinfo.decel = level.time;
				}
			}

			if (ent->groundentity && (ent->waterlevel == 1))
			{
				VectorSubtract(ent->s.origin, ent->s.old_origin, temppos);
				if (!temppos[0] && !temppos[1] && !temppos[2])
					ent->velocity[2] += 80;
			}
		}

		// not in water
		else if (ent->client->routetrace && !dist)
		{
			Get_RouteOrigin(ent->client->pers.routeindex, v);
			if (v[2] < ent->s.origin[2] - 20)
			{
				if (Bot_Watermove(ent, temppos, dist, -20))
					VectorCopy(temppos, ent->s.origin);
			}
		}
	}

	// player check door and corner
	if (!ent->client->routetrace && trace_priority && (random() < 0.2))
	{
		VectorCopy(ent->s.origin, v);
		VectorCopy(ent->mins, touchmin);
		touchmin[2] += 16;
		VectorCopy(ent->maxs,touchmax);
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			touchmax[2] = 0;
		else
			v[2] += 20;

		//right
		if (random() < 0.5)
		{
			f1 = ent->client->moveyaw + 90;
			if (f1 > 180)
				iyaw -= 360;

			f2 = ent->client->moveyaw + 135;
			if (f2 > 180)
				iyaw -= 360;
		}

		// left
		else
		{
			f1 = ent->client->moveyaw - 90;
			if (f1 < 180)
				iyaw += 360;

			f2 = ent->client->moveyaw - 135;
			if (f2 < 180)
				iyaw += 360;
		}

		yaw = DEG2RAD(f1);
		trmin[0] = cos(yaw) * 128;
		trmin[1] = sin(yaw) * 128;
		trmin[2] = 0;
		VectorAdd(v, trmin, trmax);
		tr = gi.trace(v, NULL, NULL, trmax, ent, MASK_BOTSOLIDX);
		x = tr.fraction;

		yaw = DEG2RAD(f2);
		trmin[0] = cos(yaw) * 128;
		trmin[1] = sin(yaw) * 128;
		trmin[2] = 0;
		VectorAdd(v, trmin, trmax);
		tr = gi.trace(v, NULL, NULL, trmax, ent, MASK_BOTSOLIDX);
		if ((x > tr.fraction) && (x > 0.5))
			ent->client->moveyaw = f1;
	}

	// push button
	it_ent = NULL;
	k = 0;
	VectorCopy(ent->absmin, touchmin);
	VectorCopy(ent->absmax, touchmax);
	touchmin[0] -= 48;
	touchmin[1] -= 48;
	touchmin[2] -= 5;
	touchmax[0] += 48;
	touchmax[1] += 48;

	if ((i = gi.BoxEdicts(touchmin, touchmax, touch, MAX_EDICTS, AREA_SOLID)) > 0)					//CW
	{
		for (j = i-1; j >= 0; j--)
		{
			trent = touch[j];
			if (trent->classname)
			{
				if (trent->use == button_use)
				{
					k = 1;
					it_ent = trent;
					break;
				}
				else if ((trent->use == door_use) || (trent->use == rotating_use))
				{
					if (!trent->targetname && !trent->takedamage)
					{
						if (ent->groundentity != trent)
						{
							k = 2;
							it_ent = trent;
							break;
						}
					}
				}
			}
		}
	}

	// when touch the button
	if ((it_ent != NULL) && (k == 1))																//CW
	{
		if (it_ent->use && (it_ent->moveinfo.state == PSTATE_BOTTOM) && !it_ent->health)
		{
			k = 0;
			if (ent->client->routetrace && (ent->client->pers.routeindex - 1 > 0))
			{
				k = 1;
				i = ent->client->pers.routeindex;
				if (Route[i].state == GRS_PUSHBUTTON)
					k = 0;
				else if (Route[--i].state == GRS_PUSHBUTTON)
					k = 0;

				if (!k && (Route[i].ent == it_ent))
					ent->client->pers.routeindex = i + 1;
				else
					k = 1;
			}

			if (!k && it_ent->target)
			{
				str = it_ent->target;
				e = &g_edicts[(int)maxclients->value+1];
				for (i = maxclients->value+1; i < globals.num_edicts; i++, e++)
				{
					if (!e->inuse || !e->targetname)
						continue;

					if (!stricmp(str, e->targetname))
					{
						if (e->classname[0] == 't')
						{
							if (e->use == trigger_relay_use)
							{
								if (e->target)
								{
									str = e->target;
									e = &g_edicts[(int)maxclients->value];
									i = maxclients->value;
									continue;
								}
							}
						}
						else if (e->classname[0] == 'f')
						{
							it_ent->use(it_ent, ent, it_ent);
							if ((e->use == door_use) || (e->use == rotating_use))
							{
								k = 0;
								if (!ent->client->routetrace)
								{
									v[0] = (it_ent->absmin[0] + it_ent->absmax[0]) * 0.5;
									v[1] = (it_ent->absmin[1] + it_ent->absmax[1]) * 0.5;
									v[2] = (it_ent->absmin[2] + it_ent->absmax[2]) * 0.5;
									VectorSubtract(it_ent->union_ent->s.origin, v, temppos);
									VectorScale(temppos, 3, v);
									VectorAdd(ent->s.origin, v, ent->client->movtarget_pt);
								}
								else
									VectorCopy(ent->s.origin, ent->client->movtarget_pt);

								if (fabs(e->moveinfo.start_origin[2] - e->moveinfo.end_origin[2]) > JumpMax)
								{
									if (e->union_ent == NULL)										//CW
									{
										it = item_navi3;
										trent = G_Spawn();
										trent->classname = it->classname;
										trent->s.origin[0] = (e->absmin[0] + e->absmax[0]) * 0.5;
										trent->s.origin[1] = (e->absmin[1] + e->absmax[1]) * 0.5;
										trent->s.origin[2] = e->absmax[2] + 16;
										trent->union_ent = e;
										e->union_ent = trent;
										SpawnItem3(trent, it);
									}
									else
									{
										trent = e->union_ent;
										trent->solid = SOLID_TRIGGER;
										trent->svflags &= ~SVF_NOCLIENT;
									}

									trent->target_ent = ent;
									if (e->spawnflags & PDOOR_TOGGLE)
									{
										f1 = e->moveinfo.start_origin[2] - e->moveinfo.end_origin[2];
										k = 1;
									}
									else
									{
										f1 = e->moveinfo.start_origin[2] - e->moveinfo.end_origin[2];
										if (f1 > 0)
										{
											if ((e->moveinfo.state == PSTATE_BOTTOM) || (e->moveinfo.state == PSTATE_UP))
											{
												if (fabs(trent->s.origin[2] - ent->s.origin[2]) < JumpMax)
													k = 1;
											}
										}
										else
										{
											if ((e->moveinfo.state == PSTATE_BOTTOM) || (e->moveinfo.state == PSTATE_UP))
											{
												if (fabs(trent->s.origin[2] - ent->s.origin[2]) < JumpMax)
													k = 1;
											}
										}
									}
								}

								if (!k)
								{
									ent->client->waiting_obj = e;
									ent->client->movestate &= ~STS_WAITS;
									ent->client->movestate |= STS_W_DOOROPEN;
								}
								else
								{
									if (e->union_ent->s.origin[2] + 8 - ent->s.origin[2] > JumpMax)
									{
										ent->client->routetrace = false;
										ent->client->movestate &= ~STS_WAITS;
									}
								}

								break;
							}
						}
					}
				}
			}
			else if (!k)
				it_ent->use(it_ent, ent, it_ent);
		}
	}

	// when touch the door
	else if ((it_ent != NULL) && (k == 2))															//CW
	{
		if (it_ent->moveinfo.state == PSTATE_BOTTOM)
		{
			if (it_ent->flags & FL_TEAMSLAVE)
				it_ent->teammaster->use(it_ent->teammaster, ent, it_ent->teammaster);
			else
				it_ent->use(it_ent, ent, it_ent);
		}

		if (it_ent->moveinfo.state == PSTATE_BOTTOM)
		{
			VectorCopy(ent->s.origin, ent->client->movtarget_pt);
			ent->client->waiting_obj = it_ent;
			ent->client->movestate &= ~STS_WAITS;
			ent->client->movestate |= STS_W_DOOROPEN;

			if (it_ent->flags & FL_TEAMSLAVE)
			{
				trmin[0] = (it_ent->teammaster->absmin[0] + it_ent->teammaster->absmax[0]) * 0.5;
				trmin[1] = (it_ent->teammaster->absmin[1] + it_ent->teammaster->absmax[1]) * 0.5;
				trmax[0] = (it_ent->absmin[0] + it_ent->absmax[0]) * 0.5;
				trmax[1] = (it_ent->absmin[1] + it_ent->absmax[1]) * 0.5;
				temppos[0] = (trmin[0] + trmax[0]) * 0.5;
				temppos[1] = (trmin[1] + trmax[1]) * 0.5;

				if (trace_priority < TRP_ANGLEKEEP)
					ent->s.angles[YAW] = Get_yaw(temppos);
			}
			else
			{
				trmax[0] = (it_ent->absmin[0] + it_ent->absmax[0]) * 0.5;
				trmax[1] = (it_ent->absmin[1] + it_ent->absmax[1]) * 0.5;
				VectorSubtract(trmax, ent->s.origin, temppos);
				if (trace_priority < TRP_ANGLEKEEP)
					ent->s.angles[YAW] = Get_yaw(temppos);
			}
		}
		else if (it_ent->moveinfo.state == PSTATE_UP)
		{
			VectorCopy(ent->s.origin, ent->client->movtarget_pt);
			ent->client->waiting_obj = it_ent;
			ent->client->movestate &= ~STS_WAITS;
			ent->client->movestate |= STS_W_DOOROPEN;
		}
	}

VCHCANSEL:

	// ladder check
	front = left = right = NULL;
	k = 0;

	if (ent->client->routetrace && (ent->client->pers.routeindex + 1 < TotalRouteNodes))
	{
		Get_RouteOrigin(ent->client->pers.routeindex+1, v);
		if (v[2] - ent->s.origin[2] >= 32)
			k = 1;
	}

	if (k && trace_priority && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
	{
		tempflag = 0;
		VectorCopy(ent->mins, trmin);
		VectorCopy(ent->maxs, trmax);
		trmin[2] += 20;

		// front
		iyaw = ent->client->moveyaw;
		yaw = DEG2RAD(iyaw);
		touchmin[0] = cos(yaw) * 32;
		touchmin[1] = sin(yaw) * 32;
		touchmin[2] = 0;
		VectorAdd(ent->s.origin, touchmin, touchmax);

		tr = gi.trace(ent->s.origin, trmin, ent->maxs, touchmax, ent, MASK_BOTSOLID);
		front = tr.ent;
		if (tr.contents & CONTENTS_LADDER)
			tempflag = 1;

		// upper
		if (!tempflag && !ent->client->waterstate)
		{
			trmax[2] += 32;
			tr = gi.trace(ent->s.origin, trmin,trmax, touchmax,ent, MASK_BOTSOLID);
			if (tr.contents & CONTENTS_LADDER)
				tempflag = 2;
		}

		if (!tempflag && ent->groundentity)
		{
			Get_RouteOrigin(ent->client->pers.routeindex, v);
			v[2] = ent->s.origin[2];
			tr = gi.trace(ent->s.origin, trmin, ent->maxs, v, ent, MASK_BOTSOLID);
			if (tr.contents & CONTENTS_LADDER)
				tempflag = 3;
		}

		// right
		if (tempflag == 0)
		{
			iyaw = ent->client->moveyaw + 90;
			if (iyaw > 180)
				iyaw -= 360;

			yaw = DEG2RAD(iyaw);
			touchmin[0] = cos(yaw) * 32;
			touchmin[1] = sin(yaw) * 32;
			touchmin[2] = 0;
			VectorAdd(ent->s.origin, touchmin, touchmax);
			tr = gi.trace(ent->s.origin, trmin, ent->maxs, touchmax, ent, MASK_BOTSOLID);
			right = tr.ent;
			if (tr.contents & CONTENTS_LADDER)
				tempflag = 1;
		}

		// 
		if (tempflag == 0)
		{
			iyaw = ent->client->moveyaw - 90;
			if (iyaw < -180)
				iyaw += 360;

			yaw = DEG2RAD(iyaw);
			touchmin[0] = cos(yaw) * 32;
			touchmin[1] = sin(yaw) * 32;
			touchmin[2] = 0;
			VectorAdd(ent->s.origin, touchmin, touchmax);
			tr = gi.trace(ent->s.origin, trmin, ent->maxs, touchmax, ent, MASK_BOTSOLID);
			left = tr.ent;
			if (tr.contents & CONTENTS_LADDER)
				tempflag = 1;
		}

		// found ladder
		if (tempflag)
		{
			VectorCopy(tr.endpos, trmax);
			VectorCopy(trmax, touchmax);
			touchmax[2] += WORLD_SIZE;	// was 8192
			tr = gi.trace(trmax, trmin, ent->maxs, touchmax, ent, MASK_SOLID);
			e = tr.ent;
			k = 0;
			VectorCopy(tr.endpos, temppos);
			VectorAdd(tr.endpos, touchmin, touchmax);
			tr = gi.trace(temppos, trmin,ent->maxs, touchmax, ent, MASK_BOTSOLID);

			if (e && (e->use == door_use))
				k = 1;

			if ((!(tr.contents & CONTENTS_LADDER) || k))
			{
				if (!ent->tractored)																//CW
				{
					ent->velocity[0] = 0;
					ent->velocity[1] = 0;
				}

				if ((ent->client->moveyaw == iyaw) || ent->client->routetrace)
				{	// on ladder
					if (ent->client->moveyaw != iyaw)
						ent->client->moveyaw = iyaw;

					ent->s.angles[YAW] = ent->client->moveyaw;
					if (tempflag != 3)
						VectorCopy(trmax, ent->s.origin);

					ent->client->movestate |= STS_LADDERUP;
					ent->s.angles[YAW] = ent->client->moveyaw;
					ent->s.angles[PITCH] = -29;

					if (tempflag == 2)
					{
						ent->velocity[2] = VEL_BOT_JUMP;
						SetBotAnim(ent);
						ent->client->movestate |= STS_SJMASK;
						ent->moveinfo.speed = 0;
					}
					else if (tempflag == 3)
					{
						ent->velocity[2] = VEL_BOT_JUMP;
						SetBotAnim(ent);
						ent->client->movestate |= STS_SJMASK;
						ent->moveinfo.speed = MOVE_SPD_JUMP;
					}
//CW++
					else if(ent->waterlevel > 1)
						ent->velocity[2] = VEL_BOT_WLADDERUP;
					else
//CW--
						ent->velocity[2] = VEL_BOT_LADDERUP;
				}
				else
				{
					ent->client->moveyaw = iyaw;
					ent->s.angles[YAW] = ent->client->moveyaw;
				}
			}
		}
	}

VCHCANSEL_L:

	// player sizebox and special duck set
	if ((ent->client->battleduckcnt > 0) && ent->groundentity && (ent->velocity[2] < 10))
	{
		ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
		ent->client->battleduckcnt--;
	}

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{	// ducked
		ent->client->duckedtime = 0;
		ent->maxs[2] = 4;
		ent->viewheight = -2;
	}
	else
	{	// not ducked
		if (ent->client->duckedtime < 1)
			ent->client->duckedtime += FRAMETIME;

		ent->maxs[2] = 32;
		ent->viewheight = 22;
	}

	VectorCopy(ent->s.angles, ent->client->v_angle);
	if (ent->s.angles[PITCH] < -29)
		ent->s.angles[PITCH] = -29;
	else if (ent->s.angles[PITCH] > 29)
		ent->s.angles[PITCH] = 29;
}

//==============================================
void Bot_Camp(edict_t *ent)
{
//CW++
	//	Reset camping flag once we've had time to move on from the last site.

	if (level.time > ent->client->camptime + 5.0)
		ent->client->camping = false;
//CW--

	if (ent->client->camptime < level.time)
		return;

	VectorCopy(ent->client->lastorigin, ent->s.origin);

	if (random() < 0.2)																				//CW
		ent->client->ps.pmove.pm_flags |= PMF_DUCKED;

//	Don't camp if we're using Traps or C4.

	if ((ent->client->pers.weapon == item_trap) || (ent->client->pers.weapon == item_c4))			//CW
		ent->client->camptime = level.time;
}

//==============================================
void Bot_Think(edict_t *ent)
{
//	Reset the bot's enemy info if the enemy has died.

	if (ent->client->current_enemy)
	{
		if (!(ent->client->current_enemy->die && (ent->client->current_enemy->die == Trap_DieFromDamage)))	//CW++
		{
			if (!G_ClientInGame(ent->client->current_enemy))
			{
				ent->client->battleduckcnt = 0;
				ent->client->current_enemy = NULL;
				ent->client->combatstate &= ~CTS_ENEM_NSEE;
				ent->client->battlemode = FIRE_NULL;
			}
		}
	}

//	If the bot is dead, put it back into the game.

	if (!G_ClientNotDead(ent))
	{
		ent->s.modelindex2 = 0;
		ent->s.modelindex3 = 0;																		//CW++
		ent->s.modelindex4 = 0;																		//CW++
		ent->client->routetrace = false;

		if (ent->client->respawn_time <= level.time)
		{
			ent->client->respawn_time = level.time;
			PutClientInServer(ent);
		}

		ent->nextthink = level.time + FRAMETIME;
		return;
	}

//	Perform general AI routines for the bot (movement, combat, chatting, camping, etc).
	
	Bot_AI(ent);

	if (ent->health < 25)																			//CW
		ent->client->camptime = level.time;
//CW++
	else if (ent->client->held_by_agm || ent->client->flung_by_agm || ent->client->thrown_by_agm)
		ent->client->camptime = level.time;
	else if ((int)sv_bots_camp->value && Bot[ent->client->pers.botindex].camper)
		Bot_Camp(ent);

	if ((int)sv_bots_chat->value && !ent->client->pers.muted)
		RandomChat(ent);

	gi.linkentity(ent);
	G_TouchTriggers(ent);

//	If the bot has a Personal Teleporter and is in trouble, use it to escape.

	if (ent->client->pers.inventory[ITEM_INDEX(item_teleporter)])
	{
		if (ent->health < BOT_TELE_MINHEALTH)
		{
			if (ent->client->current_enemy && InSight(ent, ent->client->current_enemy))
			{
				vec3_t	v;
				float	range;

				VectorSubtract(ent->client->current_enemy->s.origin, ent->s.origin, v);
				if ((range = VectorLength(v)) < BOT_TELE_ENEMYRANGE)
					Use_Teleporter(ent, item_teleporter);
			}
		}

		if (ent->velocity[2] < BOT_TELE_FALLSPEED)
		{
			if (TriggerHurtCheck(ent))
				Use_Teleporter(ent, item_teleporter);
		}
	}
//CW--

	ent->nextthink = level.time + FRAMETIME;
}
//Maj/Pon--
