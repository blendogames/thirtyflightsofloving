#include "g_local.h"

#define FRAME_stand1          	0
#define FRAME_stand2          	1
#define FRAME_stand3          	2
#define FRAME_stand4          	3
#define FRAME_stand5          	4
#define FRAME_stand6          	5
#define FRAME_stand7          	6
#define FRAME_stand8          	7
#define FRAME_stand9          	8
#define FRAME_stand10         	9
#define FRAME_stand11         	10
#define FRAME_stand12         	11
#define FRAME_stand13         	12
#define FRAME_stand14         	13
#define FRAME_stand15         	14
#define FRAME_stand16         	15
#define FRAME_stand17         	16
#define FRAME_stand18         	17
#define FRAME_stand19         	18
#define FRAME_stand20         	19
#define FRAME_stand21         	20
#define FRAME_stand22         	21
#define FRAME_stand23         	22
#define FRAME_stand24         	23
#define FRAME_stand25         	24
#define FRAME_stand26         	25
#define FRAME_stand27         	26
#define FRAME_stand28         	27
#define FRAME_stand29         	28
#define FRAME_stand30         	29
#define FRAME_stand31         	30
#define FRAME_stand32         	31
#define FRAME_stand33         	32
#define FRAME_stand34         	33
#define FRAME_stand35         	34
#define FRAME_walk1           	35
#define FRAME_walk2           	36
#define FRAME_walk3           	37
#define FRAME_walk4           	38
#define FRAME_walk5           	39
#define FRAME_walk6           	40
#define FRAME_walk7           	41
#define FRAME_walk8           	42
#define FRAME_walk9           	43
#define FRAME_walk10          	44
#define FRAME_run1           	45
#define FRAME_run2           	46
#define FRAME_run3           	47
#define FRAME_run4           	48
#define FRAME_run5           	49
#define FRAME_run6           	50
#define FRAME_pain1				51
#define FRAME_pain2			 	52
#define FRAME_pain3			  	53
#define FRAME_pain4        		54
#define FRAME_death1		  	55
#define FRAME_death2			56
#define FRAME_death3		   	57
#define FRAME_death4		   	58
#define FRAME_death5		   	59
#define FRAME_death6		   	60
#define MODEL_SCALE				1.000000




void civilian_think (edict_t *self);
void civilian_stand (edict_t *self);
void civilian_dead (edict_t *self);
void civilian_walk (edict_t *self);
void civilian_run (edict_t *self);
void civilian_onground (edict_t *self);
void monster_footstep (edict_t *ent);


mframe_t civilian_frames_stand [] =
{
	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL,	ai_stand, 0, NULL
};
mmove_t civilian_move_stand = {FRAME_stand1, FRAME_stand35, civilian_frames_stand, civilian_stand};



mframe_t civilian_frames_walk [] =
{
	ai_walk,	5,	monster_footstep,
	ai_walk,	5,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	5,	monster_footstep,
	ai_walk,	5,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	5,	civilian_think

};
mmove_t civilian_move_walk = {FRAME_walk1, FRAME_walk10, civilian_frames_walk, civilian_walk};


mframe_t civilian_frames_slow_run [] =
{
	ai_walk,	20,	NULL,
	ai_walk,	20,	monster_footstep,
	ai_walk,	20,	NULL,
	ai_walk,	20,	NULL,
	ai_walk,	20,	monster_footstep,
	ai_walk,	20,	civilian_think
};
mmove_t civilian_move_slow_run = {FRAME_run1, FRAME_run6, civilian_frames_slow_run, civilian_run};


mframe_t civilian_frames_run [] =
{
	ai_walk,	28,	NULL,
	ai_walk,	28,	monster_footstep,
	ai_walk,	28,	civilian_think,
	ai_walk,	28,	NULL,
	ai_walk,	28,	monster_footstep,
	ai_walk,	28,	civilian_think
};
mmove_t civilian_move_run = {FRAME_run1, FRAME_run6, civilian_frames_run, civilian_run};



mframe_t civilian_frames_death [] =
{
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL

};
mmove_t civilian_move_death = {FRAME_death1, FRAME_death6, civilian_frames_death, civilian_dead};


mframe_t civilian_frames_pain [] =
{
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL

};
mmove_t civilian_move_pain = {FRAME_pain1, FRAME_pain4, civilian_frames_pain, civilian_stand};



qboolean Surface(char *name, int type);
void monster_footstep (edict_t *ent)
{
	trace_t tr;
	int sound;
	vec3_t end, down = { 0, 0, -200};
	float volume;

	VectorMA (ent->s.origin, 50, end, end);
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_ALL);



	if (!ent->groundentity)
		return;
	



	
//		float volume = (float)(VectorLength(ent->velocity))/200;

	volume = .4;


	VectorMA (ent->s.origin, 50, down, end);
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID);
	//gi.dprintf("surface name: %s\n", tr.surface->name);
	
	if (Surface(tr.surface->name, SURF_GRASS)) // grass, snow etc
	{
		if (random() < 0.5)
			sound = gi.soundindex("foot/grass1.wav");
		else
			sound = gi.soundindex("foot/grass2.wav");
		volume /= 3;				
	}
	else if (Surface(tr.surface->name, SURF_WOOD)) // wood
	{
		if (random() < 0.5)
			sound = gi.soundindex("foot/wood1.wav");
		else
			sound = gi.soundindex("foot/wood2.wav");
		volume /= 3;
	}
	else if (Surface(tr.surface->name, SURF_METAL)) // metal
	{
		if (random() < 0.5)
			sound = gi.soundindex("foot/metal1.wav");
		else
			sound = gi.soundindex("foot/metal2.wav");
		volume /= 3;
	}
	else if (Surface(tr.surface->name, SURF_SAND)) // sand (beach)
	{
		if (random() < 0.5)
			sound = gi.soundindex("foot/sand1.wav");
		else
			sound = gi.soundindex("foot/sand2.wav");
		volume /= 3;				
	}
	else //default
	{
		volume = volume * .8; //faf

		if (random() < 0.25)
			sound = gi.soundindex("player/step1.wav");
		else if (random() < 0.50)
			sound = gi.soundindex("player/step2.wav");
		else if (random() < 0.75)
			sound = gi.soundindex("player/step3.wav");
		else
			sound = gi.soundindex("player/step4.wav");
	}		
		
	gi.sound (ent, CHAN_AUTO, sound, volume, ATTN_NORM, 0);

}



void civilian_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &civilian_move_walk;
}

void civilian_run (edict_t *self)
{
	self->monsterinfo.currentmove = &civilian_move_slow_run;
	if (self->master)
	{	
		vec3_t dist;

		VectorSubtract (self->s.origin, self->master->s.origin, dist);
		if (VectorLength(dist) > 200)
			self->monsterinfo.currentmove = &civilian_move_run;
										
	}

}


void civilian_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int	r,s;

//	if (self->health < (self->max_health / 2))
//		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 1;

	r = 1 + (rand()&1);
	s = (1 +(rand()&3))*25;
	gi.sound (self, CHAN_VOICE, gi.soundindex (va("../players/usa/pain%i_%i.wav", s,r)), 1, ATTN_IDLE, 0);
	self->monsterinfo.currentmove = &civilian_move_pain;

}

void civilian_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &civilian_move_stand;
}

void civilian_dead (edict_t *self)
{
	if (self->spawnflags & 8)
	{
		self->flags |= FL_FLY;
	}
	else
	{
		VectorSet (self->mins, -16, -16, -24);
		VectorSet (self->maxs, 16, 16, -8);
		self->movetype = MOVETYPE_TOSS;
	}
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void civilian_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_IDLE, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, gi.soundindex(va("#players/usa/death%i.wav", (rand()%4)+1)), 1, ATTN_IDLE, 0);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &civilian_move_death;
}



void civilian_think (edict_t *self)
{
	vec3_t v,temp,dist;
	int i;
	edict_t *e, *nearest;
	float temp_distance, nearest_distance;
	nearest_distance = 9999999999;


	if (level.intermissiontime)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time +.1;
		return;
	}


	if (self->wait)
	{
		level.obj_time =(int)(self->wait + self->count - level.time + 1);
		level.obj_team = self->obj_owner;

		if (level.obj_time <= 0)
		{
			if (dedicated->value)
				safe_cprintf(NULL, PRINT_HIGH, "Civilian has been Successfully Held by the %s!\n", 
					team_list[self->obj_owner]->teamname);

			safe_bprintf (PRINT_HIGH, "Civilian Has been Successfully Held by the %s!\n", 
					team_list[self->obj_owner]->teamname);

			gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

			team_list[self->obj_owner]->score += 100;
			
			level.obj_time = 0;
			
	//		self->obj_count = level.framenum; // reset the touch count
			self->think = G_FreeEdict;
			self->nextthink = level.time +.1;
			return;
		}
	}


	
	//make the nearest info_reinforcement_start belong to the team that owns you, all others belong to other team
	if (self->obj_owner != -1/* &&
		(self->monsterinfo.currentmove == &civilian_move_run ||
		self->monsterinfo.currentmove == &civilian_move_slow_run)*/)
	{
		edict_t *e,*nearest = NULL;
		float temp_distance,nearest_distance;

		nearest_distance = 9999999999;
        for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
        {
			if (!e->inuse)
				continue;
			if (strcmp(e->classname, "info_reinforcements_start"))
				continue;
			
			e->obj_owner = (self->obj_owner + 1)%2; //set to other team

			VectorSubtract (e->s.origin, self->s.origin, dist);
			
			temp_distance = VectorLength(dist);
//			gi.dprintf ("%f\n",temp_distance);

			if (temp_distance < nearest_distance)
			{
				nearest_distance = temp_distance;
				nearest = e;
			}
        }
		if (nearest)
		{	
			nearest->obj_owner = self->obj_owner;
//			gi.dprintf ("%s\n",vtos(nearest->s.origin));
		}
	}


	if (self->master && self->master->health <= 0)
	{
		self->master = NULL;
		self->goalentity = NULL;
		self->movetarget = NULL;
		self->enemy = NULL;
		self->monsterinfo.currentmove = &civilian_move_stand;
	}

	if (self->master)
	{
		VectorSubtract (self->s.origin, self->master->s.origin, v);

		if (VectorLength (v) > 120)
		{
			self->goalentity = self->master;
			self->monsterinfo.currentmove = &civilian_move_slow_run;
		}
		else if (VectorLength (v) <= 120)

		{
			self->goalentity = NULL;
			self->monsterinfo.currentmove = &civilian_move_stand;
			VectorSubtract (self->master->s.origin, self->s.origin, temp);
			self->ideal_yaw = vectoyaw(temp);
			M_ChangeYaw(self);
		}

	}
	else
	{
		nearest = NULL;
		self->goalentity = NULL;
		for (i=0 ; i < game.maxclients ; i++)
		{
			e = g_edicts + 1 + i;
			if (!e->inuse || !e->client || !e->client->resp.team_on || e->solid != SOLID_BBOX)
				continue;
			
			VectorSubtract (e->s.origin, self->s.origin, dist);
			
			temp_distance = VectorLength(dist);
			if (temp_distance < nearest_distance)
			{
				nearest_distance = temp_distance;
				nearest = e;
			}

		}

		if (nearest)
		{
			VectorSubtract (nearest->s.origin, self->s.origin, temp);
			self->ideal_yaw = vectoyaw(temp);
			M_ChangeYaw(self);
		}
	}






	//self->enemy = NULL;

	self->health = 1000000000;

}

void SP_misc_civilian (edict_t *self)
{
//	static int skin = 0;	//@@

//	sound_fist = gi.soundindex ("civilian/civilian11.wav");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/civilian/tris.md2");

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 23);

	self->health = 1000000000;
	self->gib_health = -50;
	self->mass = 300;

	self->pain = civilian_pain;
	self->die = civilian_die;

	self->monsterinfo.stand = civilian_stand;
	self->monsterinfo.walk = civilian_walk;
	self->monsterinfo.run = civilian_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.aiflags |= AI_GOOD_GUY;

//@@
//	self->s.skinnum = skin;
//	skin++;
//	if (skin > 12)
//		skin = 0;

	gi.linkentity (self);

	self->monsterinfo.aiflags |= AI_STAND_GROUND;

	self->monsterinfo.currentmove = &civilian_move_stand;
	
	self->monsterinfo.scale = MODEL_SCALE;



	walkmonster_start (self);
	if (self->style)
		self->s.skinnum = self->style;//rand()%2;
	else
		self->s.skinnum = 0;

	if (!self->count)
		self->count = 180;

	self->obj_owner = -1;
	self->flags |= FL_NO_KNOCKBACK;
	self->s.renderfx   = RF_FULLBRIGHT;
	self->flags |= FL_IMMUNE_LAVA;

}



void VIP_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int	r,s;

//	if (self->health < (self->max_health / 2))
//		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + .5;

	r = 1 + (rand()&1);
	s = (1 +(rand()&3))*25;
	gi.sound (self, CHAN_VOICE, gi.soundindex (va("../players/grm/pain%i_%i.wav", s,r)), 1, ATTN_IDLE, 0);

	self->s.frame = 51;
}

void Respawn_VIP (edict_t *self)
{
	self->solid = SOLID_BBOX;
	self->health = 50;
	self->mass = 300;

	self->think = VIP_think;
	self->nextthink = level.time + .1;

	self->s.frame = 0;

	self->deadflag = DEAD_NO;

}
void VIP_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	otherteam;

/*	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_IDLE, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}*/

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, gi.soundindex(va("#players/grm/death%i.wav", (rand()%4)+1)), 1, ATTN_IDLE, 0);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->s.frame = 55;

	otherteam = (self->obj_owner+1)%2;

	team_list[otherteam]->score += self->obj_gain;

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_HIGH, "%s killed by %s [%s]\n", 
			self->obj_name, 
			attacker->client->pers.netname,
			team_list[attacker->client->resp.team_on->index]->teamname);

	centerprintall("%s killed by \n%s\n%s",
		self->obj_name, 
		attacker->client->pers.netname,
		team_list[attacker->client->resp.team_on->index]->teamname);


//		if (!team_list[otherteam]->kills_and_points && team_list[otherteam]->score < team_list[otherteam]->need_points ||
//			(team_list[otherteam]->kills_and_points && 
//				team_list[otherteam]->kills < team_list[otherteam]->need_kills))
			gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[otherteam]->teamid)), 1, 0, 0);

	if (self->deathtarget)
	{	
		self->target = self->deathtarget;
		if (self->target)
			G_UseTargets (self, attacker);
	}

	self->solid = SOLID_NOT;




}


void VIP_think (edict_t *self)
{

	if (self->s.frame < 34)
		self->s.frame++;
	else if (self->s.frame == 34)
		self->s.frame = 0;

	else if (self->s.frame < 54)
		self->s.frame++;
	else if (self->s.frame == 54)
		self->s.frame = 0;
	else if (self->s.frame < 60)
		self->s.frame++;
	if (self->s.frame == 60)
	{
		edict_t *cl_ent;
		int j;

		if (!level.intermissiontime)
		{
			for (j=0 ; j < game.maxclients ; j++)
			{
				cl_ent = g_edicts + 1 + j;
				if (!cl_ent->inuse)
					continue;
				if (!cl_ent->client->resp.team_on)
					continue;
				if (cl_ent->deadflag)
					continue;
				if (cl_ent->health < 1)
					continue;
				if (cl_ent->burnout)
					continue;

				Find_Mission_Start_Point(cl_ent, cl_ent->s.origin, cl_ent->s.angles);
				cl_ent->solid = SOLID_TRIGGER;
			}
		}



		self->nextthink = 0;
		if (self->wait)
		{
			self->think = Respawn_VIP;
			self->nextthink = level.time + self->wait;
		}
		return;
	}

	self->nextthink = level.time + .1;
}

void SP_objective_VIP (edict_t *self)
{
	self->classnameb = OBJECTIVE_VIP;
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex(self->model);//gi.modelindex("models/monsters/hitler/tris.md2");

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 23);

	self->health = 50;
	self->gib_health = -50;
	self->mass = 300;

	self->pain = VIP_pain;
	self->die = VIP_die;

	self->think = VIP_think;
	self->nextthink = level.time + .1;

	if (self->style)
		self->s.skinnum = self->style;

	gi.linkentity (self);

	self->takedamage = DAMAGE_YES;

	self->flags |= FL_NO_KNOCKBACK;
	self->s.renderfx   = RF_FULLBRIGHT;
	//self->flags |= FL_IMMUNE_LAVA;


	VectorCopy (self->s.origin, self->obj_origin);
	self->obj_origin[2]+=15;
}
