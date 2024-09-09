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

// Knightmare- moved these vars here to fix GCC compile
bot_team_t	*bot_teams[MAX_TEAMS];
int			total_teams;

int	    num_players;
edict_t *players[MAX_CLIENTS];		// pointers to all players in the game
edict_t	*weapons_head;				// pointers to all weapons in the game (use node_target and last_goal to traverse forward/back)
edict_t	*health_head;
edict_t	*bonus_head;				// armour, Quad, etc
edict_t	*ammo_head;

// the following are just faster ways of accessing FindItem("item_name"), set in Worldspawn
gitem_t	*item_shells, *item_cells, *item_rockets, *item_grenades, *item_slugs, *item_bullets;
gitem_t	*item_shotgun, *item_hyperblaster, *item_supershotgun, *item_grenadelauncher, *item_chaingun, *item_railgun, *item_machinegun, *item_bfg10k, *item_rocketlauncher, *item_blaster;
gitem_t *item_proxlauncher, *item_disruptor, *item_etfrifle, *item_chainfist, *item_plasmabeam, *item_rounds, *item_flechettes, *item_tesla, *item_prox, *item_plasmarifle, *item_shockwave;

bot_info_t	*botinfo_list;
int			total_bots;		// number of bots read in from bots.cfg

bot_info_t	*teambot_list;	// bots that were generated solely for teamplay

int	 spawn_bots;
int	 roam_calls_this_frame;
int	 bestdirection_callsthisframe;

char	*bot_chat_text[NUM_CHAT_SECTIONS][MAX_CHAT_PER_SECTION];
int		bot_chat_count[NUM_CHAT_SECTIONS];
float	last_bot_chat[NUM_CHAT_SECTIONS];

int		num_view_weapons;
char	view_weapon_models[64][64];

int     botdebug;

int     max_bots;
float	last_bot_spawn;
int     bot_male_names_used;
int     bot_female_names_used;
int     bot_count;

double	bot_frametime;

// end Knightmare

void CopyToBodyQue (edict_t *ent);

void bot_AnimateFrames (edict_t *self)
{
	int	minframe, maxframe;
	vec3_t themove;

	VectorSubtract(self->s.origin, self->animate_org, themove);
	VectorCopy(self->s.origin, self->animate_org);

	themove[2] = 0;

	if (self->s.modelindex != (MAX_MODELS-1)) // was 255
	{	// gibbed, ready to spawn
		self->s.frame = 0;

		if (self->timestamp < (level.time - 2))	// wait a bit
			respawn_bot(self);

		return;
	}

	if (self->s.frame >= FRAME_crdeath1)
	{
		self->client->anim_priority = ANIM_DEATH;

		if (((self->s.frame >= FRAME_crdeath1) && (self->s.frame < FRAME_crdeath5)) ||
			((self->s.frame >= FRAME_death101) && (self->s.frame < FRAME_death106)) ||
			((self->s.frame >= FRAME_death201) && (self->s.frame < FRAME_death206)) ||
			((self->s.frame >= FRAME_death301) && (self->s.frame < FRAME_death308)))
		{
			self->s.frame++;
		}
		else	// must be ready to spawn
		{
			CopyToBodyQue(self);
			respawn_bot(self);
		}

		return;
	}
	else if (self->health <= 0)	// FIXME: this shouldn't happen, but it does
	{	// pick a random death frame, and go with it
		float	rnd;

		rnd = random() * 3;

		if (self->viewheight < 0)
			self->s.frame = FRAME_crdeath1;
		else if (rnd <= 1)
			self->s.frame = FRAME_death101;
		else if (rnd <= 2)
			self->s.frame = FRAME_death201;
		else
			self->s.frame = FRAME_death301;
	}
	else if (self->client->anim_priority == ANIM_PAIN)
	{
		self->s.frame++;

		if (self->s.frame > self->client->anim_end)
		{
			self->s.frame = FRAME_stand01;
			self->client->anim_priority = ANIM_BASIC;
		}

	}
	else if (!self->groundentity && (self->waterlevel || (self->last_inair > (level.time - 0.3))))
	{	// jumping
		self->client->anim_priority = ANIM_JUMP;

		minframe = FRAME_jump1;
		maxframe = FRAME_jump3;

		self->s.frame++;
		if (self->s.frame < minframe)
			self->s.frame = minframe;
		else if (self->s.frame > maxframe)
			self->s.frame = maxframe;
	}
	else if ((self->s.frame >= FRAME_jump3) &&
			 (self->s.frame < FRAME_jump6))	// jump landing
	{
		self->client->anim_priority = ANIM_JUMP;

		maxframe = FRAME_jump3;

		self->s.frame++;
	}
	else if ((self->s.frame <= FRAME_point12) && (self->s.frame >= FRAME_salute01))
	{
		self->client->anim_priority = ANIM_WAVE;

		self->s.frame++;

		if (self->enemy || (self->s.frame > self->radius_dmg))	// abort if enemy found
			self->s.frame = FRAME_stand01;
	}
	else if ((self->s.frame >= FRAME_attack1) && (self->s.frame <= FRAME_attack3))
	{
		self->client->anim_priority = ANIM_ATTACK;

		maxframe = FRAME_attack3;

		self->s.frame++;
		if (self->s.frame > maxframe)
			self->s.frame = FRAME_run1;		// finshed firing, loop is created by each fire, in bot_Attack()
	}
	else if ((self->s.frame >= FRAME_crattak1) && (self->s.frame < FRAME_crattak3))
	{
		self->client->anim_priority = ANIM_ATTACK;

		maxframe = FRAME_crattak3;

		self->s.frame++;
		if (self->s.frame > maxframe)
			self->s.frame = FRAME_run1;		// finshed firing, loop is created by each fire, in bot_Attack()
	}
	else if (VectorLength(themove) > 4)	// running
	{
		self->client->anim_priority = ANIM_BASIC;

		if (self->maxs[2] == 4)
		{
			minframe = FRAME_crwalk1;
			maxframe = FRAME_crwalk6;
		}
		else
		{
			minframe = FRAME_run1;
			maxframe = FRAME_run6;

			if (!self->waterlevel && self->client->pickup_msg_time < (level.time - 0.3))
			{
				self->s.event = EV_FOOTSTEP;
				self->client->pickup_msg_time = level.time;
			}
		}

		self->s.frame++;
		if (self->s.frame < minframe)
			self->s.frame = minframe;
		else if (self->s.frame > maxframe)
			self->s.frame = minframe;

	}
	else	// standing
	{
		self->client->anim_priority = ANIM_BASIC;

		if (self->maxs[2] == 4)
		{
			minframe = FRAME_crstnd01;
			maxframe = FRAME_crstnd19;
		}
		else
		{
			minframe = FRAME_stand01;
			maxframe = FRAME_stand40;
		}

		self->s.frame++;
		if (self->s.frame < minframe)
			self->s.frame = minframe;
		else if (self->s.frame > maxframe)
			self->s.frame = minframe;
	}

}

void botDebugPrint (char *msg, ...)
{
	if ( !bot_debug->value )
		return;
	else
	{
		char	    bigbuffer[0x10000];
		int		    len;
		va_list		argptr;

		va_start (argptr, msg);
	//	len = vsprintf (bigbuffer,msg,argptr);
		len = Q_vsnprintf (bigbuffer, sizeof(bigbuffer), msg, argptr);
		va_end (argptr);
		gi.dprintf (bigbuffer);
	}
}

void ReadBotChat (void)
{
	FILE	*f;
	int		section_index, line_count = 0, total_lines = 0;
	int		i, j;
	char	filename[256];
	char	buffer;
	size_t	len;
/*	cvar_t	*game_dir;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
//	i =  sprintf(filename, ".\\");
//	i += sprintf(filename + i, game_dir->string);
//	i += sprintf(filename + i, "\\chat.txt");
	// Knightmare- use safe string functions
	Com_sprintf (filename, sizeof(filename), ".\\");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "\\chat.txt");
#else
	Com_strcpy (filename, sizeof(filename), "./");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "/chat.txt");
#endif */
	// Knightmare- use GameDir() for all platforms
	Com_strcpy (filename, sizeof(filename), GameDir());
	Com_strcat (filename, sizeof(filename), "/chat.txt");

	f = fopen (filename, "r");
	if ( !f )
	{
		gi.error ("\nUnable to read chat.txt\nChat functions not available.\n\n");
		return;
	}

	memset (bot_chat_text, 0, sizeof(bot_chat_text));

	section_index = -1;

	gi.dprintf ("\nReading chat.txt...\n");

	while ( !feof(f) )
	{
		fscanf (f, "%c", &buffer);

		if (buffer == '#')
		{	// read to the end of the line
			while ( !feof(f) && (buffer != '\n') )
				fscanf(f, "%c", &buffer);
		}
		else if (buffer == '-')
		{	// new section
			// set the number of lines for the current section
		//	if (section_index >= 0)
        //		bot_chat_count[section_index] = line_count;

			// Knightmare- add to total of chat lines
			if ( (section_index >= 0) && (line_count >= 0) ) {
				total_lines += (line_count + 1);
			//	gi.dprintf ("%i chat lines read in section %i.\n", line_count + 1, section_index);
			}

			// increment section
			section_index++;
			line_count = -1;

			while (!feof(f) && (buffer != '\n'))
				fscanf(f, "%c", &buffer);
		}
		else if ( ((buffer >= 'a') && (buffer <= 'z')) ||
				 ((buffer >= 'A') && (buffer <= 'Z')) ||
				 (buffer == '%') )
		{	// read this entire line
			i = 0;
			line_count++;

			// allocate memory for new string
			bot_chat_text[section_index][line_count] = gi.TagMalloc(256, TAG_GAME);
			memset (bot_chat_text[section_index][line_count], 0, 256);

			while ( !feof(f) && (buffer != '\n') )
			{
				bot_chat_text[section_index][line_count][i++] = buffer;

				fscanf (f, "%c", &buffer);
			}

			// Knightmare- replace carriage returns on Linux
#ifndef _WIN32
			len = strlen(bot_chat_text[section_index][line_count]);
			for (j = 0; j < len; j++) {
				if (bot_chat_text[section_index][line_count][j] == '\r')
					bot_chat_text[section_index][line_count][j] = ' ';
			}
#endif // _WIN32

			if (i > 0)
			{
				bot_chat_text[section_index][line_count][i] = '\0';
            //  gi.dprintf (bot_chat_text[section_index][line_count]);
            //  gi.dprintf ("\n");
			}

			// update the count now
			bot_chat_count[section_index] = line_count;
		}
	}

	// Knightmare- add to total of chat lines
	if ( (section_index >= 0) && (line_count >= 0) ) {
		total_lines += (line_count + 1);
	//	gi.dprintf ("%i chat lines read in section %i.\n", line_count + 1, section_index);
	}

	// Knightmare- added output of total lines
	gi.dprintf ("%i total chat lines read.\n", total_lines);
	gi.dprintf ("\n");

	fclose (f);
}

gitem_t	*GetWeaponForNumber (int i)
{
	switch (i)
	{
	case 2 :
		return item_shotgun;
	case 3 :
		return item_supershotgun;
	case 4 :
		return item_machinegun;
	case 5 :
		return item_chaingun;
	case 6 :
		return item_grenadelauncher;
	case 7 :
		return item_rocketlauncher;
	case 8 :
		return item_railgun;
	case 9 :
		return item_hyperblaster;
	case 0 :
		return item_bfg10k;
	}

	return item_shotgun;
}

bot_info_t	*GenerateBotData (bot_team_t *bot_team, char *botname)
{
	int			i, gen;
	bot_info_t	*botdata;
	size_t		botdata_stringSize = BOTDATA_STRINGSIZE;

	botdata = gi.TagMalloc (sizeof(bot_info_t), TAG_GAME);

	botdata->ingame_count = 0;

	// name
//	botdata->name = gi.TagMalloc (128, TAG_GAME);
//	strcpy(botdata->name, botname);
	botdata->name = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy(botdata->name, botdata_stringSize, botname);

	// skin
//	botdata->skin = gi.TagMalloc (128, TAG_GAME);
//	strcpy(botdata->skin, bot_team->default_skin);
	botdata->skin = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy(botdata->skin, botdata_stringSize, bot_team->default_skin);

	i = gen = 0;
	while (botname[i])
	{
		gen += botname[i] + i;
		i++;
	}

	// Knightmare added
	// rail color
	botdata->color1 = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy(botdata->color1, botdata_stringSize, "1430B0");
	// end Knightmare

	// stats
	botdata->bot_stats.accuracy = (float) (1 + ((int)(gen*3.234) % 5));
	botdata->bot_stats.aggr		= (float) (1 + ((int)(gen*5.132) % 5));
	botdata->bot_stats.combat	= (float) (1 + ((int)(gen*4.476) % 5));

	botdata->bot_stats.fav_weapon = GetWeaponForNumber(3 + ((int)(gen*2.356) % 7));

	botdata->bot_stats.quad_freak	= (((int)(gen*1.453) % 3) < 2);
	botdata->bot_stats.camper		= (((int)(gen*2.376) % 3) == 2);
	botdata->bot_stats.avg_ping		=  ((int)(gen*0.678) % 4) * 100;

	return botdata;
}

// Advances the pointer to the FILE to the next non-space, non-tab character
char	next_nonspace (FILE **f)
{
	char ch=9;

	while (!feof(*f) && ((ch==9) || (ch==' ')))
	{
		fscanf(*f, "%c", &ch);	// fscanf will skip spaces, but not TABS
	}

	return ch;
};

bot_team_t	*ReadTeamData (FILE **f)
{
	char		strbuf[256], ch;
	int			i, numbots;
	bot_team_t	*bot_team;
	size_t		teamLen;

	bot_team = gi.TagMalloc (sizeof(bot_team_t), TAG_GAME);

	// name
	i=0;
	fscanf(*f, "%c", &strbuf[i]);
	while ((strbuf[i] != '\"') && (i < 255))
	{
		i++;
		fscanf(*f, "%c", &strbuf[i]);
	}
	strbuf[i] = '\0';	// strip the trailing "

//	bot_team->teamname = gi.TagMalloc (strlen(strbuf)+1, TAG_GAME);
//	strcpy(bot_team->teamname, strbuf);
	teamLen = strlen(strbuf)+1;
	bot_team->teamname = gi.TagMalloc (teamLen, TAG_GAME);
	Com_strcpy (bot_team->teamname, teamLen, strbuf);

	// abbrev
	i=0;
	next_nonspace(f);
	fscanf(*f, "%c", &strbuf[i]);
	while ((strbuf[i] != '\"') && (i < 255))
	{
		i++;
		fscanf(*f, "%c", &strbuf[i]);
	}
	strbuf[i] = '\0';	// strip the trailing "

//	bot_team->abbrev = gi.TagMalloc (strlen(strbuf)+1, TAG_GAME);
//	strcpy(bot_team->abbrev, strbuf);
	teamLen = strlen(strbuf)+1;
	bot_team->abbrev = gi.TagMalloc (teamLen, TAG_GAME);
	Com_strcpy(bot_team->abbrev, teamLen, strbuf);

	// default model/skin
	i=0;
	next_nonspace(f);
	fscanf(*f, "%c", &strbuf[i]);
	while ((strbuf[i] != '\"') && (i < 255))
	{
		i++;
		fscanf(*f, "%c", &strbuf[i]);
	}
	strbuf[i] = '\0';	// strip the trailing "

	bot_team->default_skin = G_CopyString(strbuf);
//	bot_team->default_skin = gi.TagMalloc (strlen(strbuf)+1, TAG_GAME);
//	strcpy(bot_team->default_skin, strbuf);
	teamLen = strlen(strbuf)+1;
	bot_team->default_skin = gi.TagMalloc (teamLen, TAG_GAME);
	Com_strcpy(bot_team->default_skin, teamLen, strbuf);

	// start of the bots
	ch = 0;
	while ((ch != '[') && !feof(*f))
		fscanf(*f, "%c", &ch);

	if (feof(*f))
	{
		gi.error("Bot team \"%s\"doesn't have bot list\n", bot_team->teamname);
		return NULL;
	}

	fscanf(*f, "%c", &ch);	// skip the opening "

	numbots = 0;
	while ((numbots < MAX_PLAYERS_PER_TEAM) && (ch != ']'))
	{
		// read the name

		i=0;
		fscanf(*f, "%c", &strbuf[i]);

		while ((strbuf[i] != '\"') && (i < 255))
		{
			i++;
			fscanf(*f, "%c", &strbuf[i]);
		}
		strbuf[i] = '\0';	// strip the trailing "

		if (!(bot_team->bots[numbots] = GetBotData(strbuf)))
		{
			bot_team->bots[numbots] = GenerateBotData(bot_team, strbuf);

			// add the bot to the teambot_list
			if (!teambot_list)
			{
				teambot_list = bot_team->bots[numbots];
				teambot_list->next = NULL;
			}
			else	// add to the start of the list
			{
				bot_team->bots[numbots]->next = teambot_list;
				teambot_list = bot_team->bots[numbots];
			}

		}
	/*
		// add the abbreviation to the name
		strcat(bot_team->bots[numbots]->name, "[");
		strcat(bot_team->bots[numbots]->name, bot_team->abbrev);
		strcat(bot_team->bots[numbots]->name, "]");
	*/
		numbots++;

		if (next_nonspace(f) == ']')
			break;		// end of group
	}

	// clear any remaing bot slots
	while (numbots < MAX_PLAYERS_PER_TEAM)
		bot_team->bots[numbots++] = NULL;

	bot_team->score = 0;
	bot_team->ingame = 0;
	bot_team->num_players = 0;

	return bot_team;
};

// reads a line of bot data from bots.cfg
bot_info_t	*ReadBotData (FILE **f)
{
	char		strbuf[256];
	char		buffer=0;
	int			i;
	bot_info_t	*botdata;
	size_t		botdata_stringSize = BOTDATA_STRINGSIZE;
	char		*defaultColor1 = "1430B0";	// "B0B014"

	botdata = gi.TagMalloc (sizeof(bot_info_t), TAG_GAME);

	botdata->ingame_count = 0;

	// Knightmare- Rail color
	botdata->color1 = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy (botdata->color1, botdata_stringSize, defaultColor1);

	// name
	i = 0;
	fscanf (*f, "%c", &strbuf[i]);
	while ((strbuf[i] != '\"') && (i < 255))
	{
		i++;
		fscanf (*f, "%c", &strbuf[i]);

		if (strbuf[i] == '\n')
			return NULL;
	}
	strbuf[i] = '\0';	// strip the trailing "

//	botdata->name = gi.TagMalloc (128, TAG_GAME);
//	strcpy (botdata->name, strbuf);
	botdata->name = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy (botdata->name, botdata_stringSize, strbuf);

	while ( !feof(*f) && (buffer != '"') )
	{
		fscanf (*f, "%c", &buffer);
		if (buffer == '\n')
			return NULL;
	}

	// skin
	i = 0;
	fscanf (*f, "%c", &strbuf[i]);
	if (strbuf[i] == '\n')
		return NULL;

	while ( (strbuf[i] != '\"') && (i < 255) )
	{
		i++;
		fscanf (*f, "%c", &strbuf[i]);

		if (strbuf[i] == '\n')
			return NULL;
	}
	strbuf[i] = '\0';	// strip the trailing "

//	botdata->skin = gi.TagMalloc (128, TAG_GAME);
//	strcpy (botdata->skin, strbuf);
	botdata->skin = gi.TagMalloc (botdata_stringSize, TAG_GAME);
	Com_strcpy (botdata->skin, botdata_stringSize, strbuf);

	fscanf(*f, "%c", &buffer);

	if (buffer == '\n')
		return NULL;

	// stats
	fscanf (*f, "%i", &i);
	botdata->bot_stats.accuracy = (float) i;
	fscanf (*f, "%i", &i);
	botdata->bot_stats.aggr = (float) i;
	fscanf (*f, "%i", &i);
	botdata->bot_stats.combat = (float) i;
	fscanf (*f, "%i", &i);

	if (i == 1)	// overwrite blaster with RL
		i = 7;

	botdata->bot_stats.fav_weapon = GetWeaponForNumber(i);

	fscanf (*f, "%i", &(botdata->bot_stats.quad_freak));
	fscanf (*f, "%i", &(botdata->bot_stats.camper));
	fscanf (*f, "%i", &(botdata->bot_stats.avg_ping));

/*	gi.dprintf ("Read stats %3f %3f %3f %i %i %i %i for bot %s.\n",
				botdata->bot_stats.accuracy, botdata->bot_stats.aggr, botdata->bot_stats.combat,
				i, botdata->bot_stats.quad_freak, botdata->bot_stats.camper, botdata->bot_stats.avg_ping, botdata->name);
*/
	// Knightmare added
	while ( !feof(*f) && (buffer != '"') )
	{
		fscanf (*f, "%c", &buffer);
		if (buffer == '\n')
			return botdata;
	}

	// Rail color (optional)
	i = 0;
	fscanf (*f, "%c", &strbuf[i]);
	if (strbuf[i] == '\n')
		return botdata;

	while ( (strbuf[i] != '\"') && (i < 255) )
	{
		i++;
		fscanf(*f, "%c", &strbuf[i]);

		if (strbuf[i] == '\n')
			return botdata;
	}
	strbuf[i] = '\0';	// strip the trailing "

	Com_strcpy (botdata->color1, botdata_stringSize, strbuf);
//	gi.dprintf ("Read color %s for bot %s.\n", strbuf, botdata->name);
	// end Knightmare

	return botdata;
}

void ReadViewWeaponModel (FILE **f)
{
	char	buffer=0;
	int		i=0;

	while (!feof(*f) && (buffer != '\n'))
	{
		fscanf(*f, "%c", &buffer);

		if ((buffer != '"') && (buffer != '\n') && (buffer != '\r'))
			view_weapon_models[num_view_weapons][i++] = buffer;
	}

	if (i>0)
	{
		view_weapon_models[num_view_weapons][i] = 0;
		num_view_weapons++;
	}
}

qboolean	read_bot_cfg = false;

// Reads data from bots.cfg (called from worldspawn)
void ReadBotConfig (void)
{
	FILE	*f;
	int		i, mode=0;		// mode 0 for reading bots, 1 for teams
	char	filename[256];
	char	buffer;
	bot_info_t	*botdata=NULL, *last_botdata=NULL;
//	cvar_t	*game_dir;

	if (read_bot_cfg)	// must have already read in the config
		return;

/*	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
//	i =  sprintf(filename, ".\\");
//	i += sprintf(filename + i, game_dir->string);
//	i += sprintf(filename + i, "\\bots.cfg");
	// Knightmare- use safe string functions
	Com_sprintf (filename, sizeof(filename), ".\\");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "\\bots.cfg");
#else
	Com_strcpy (filename, sizeof(filename), "./");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "/bots.cfg");
#endif */
	// Knightmare- use GameDir() for all platforms
	Com_strcpy (filename, sizeof(filename), GameDir());
	Com_strcat (filename, sizeof(filename), "/bots.cfg");

	f = fopen (filename, "r");
	if ( !f )
	{
		gi.error ("Unable to read bots.cfg. Cannot continue.\n");
		return;
	}

	// initialise the teams
	for (i=0; i<MAX_TEAMS; i++)
		bot_teams[i] = NULL;

	gi.dprintf ("\nReading bots.cfg...\n");

	// Add Eraser, the hard-coded bot
	botinfo_list = gi.TagMalloc (sizeof(bot_info_t), TAG_GAME);
	botinfo_list->ingame_count			= 0;
	botinfo_list->name					= "Eraser";
	botinfo_list->skin					= "male\razor.pcx";
	botinfo_list->color1				= "1430B0";			// Knightmare added

	botinfo_list->bot_stats.accuracy	= 5;
	botinfo_list->bot_stats.aggr		= 0;
	botinfo_list->bot_stats.combat		= 5;
	botinfo_list->bot_stats.fav_weapon	= GetWeaponForNumber(8);
	botinfo_list->bot_stats.quad_freak	= 1;
	botinfo_list->bot_stats.camper		= 0;
	botinfo_list->bot_stats.avg_ping	= 50;
	// done.

	botdata = botinfo_list;

	total_bots = 1;
	total_teams = 0;
	teambot_list = NULL;
	num_view_weapons = 0;
	memset (view_weapon_models, 0, sizeof(view_weapon_models));

	while ( !feof(f) )
	{
		fscanf (f, "%c", &buffer);

		if ( feof(f) )
			break;

		if (buffer == '#')		// commented line
		{
			while ( !feof(f) && (buffer != '\n') )
				fscanf (f, "%c", &buffer);
		}
		else if (buffer == '[')	// mode specifier (bots/teams)
		{
			fscanf (f, "%c", &buffer);

			if (buffer == 'b')
				mode = 0;
			else if (buffer == 't')
			{
			//	if (!teamplay->value /*|| ctf->value*/)
			//		break;
				mode = 1;
			}
			else if (buffer == 'v')
			{
				if ( !view_weapons->value )
					break;
				mode = 2;
			}

			fscanf (f, "\n");
		}
		else if (buffer == '"')	// start of some data
		{
			if (mode == 0)
			{
				last_botdata = botdata;
				if ( !(botdata = ReadBotData(&f)) )
				{
					gi.error ("\nError in BOTS.CFG: Invalid BOT (#%i)\nEither re-install Eraser, or check your bots.cfg file for errors\n\n", total_bots);
					break;
				}

				if ( !Q_stricmp(botdata->name, "Eraser") )
				{	// ignore this bot
					gi.TagFree (botdata);
					botdata = last_botdata;
				}
				else
				{
				//	gi.dprintf ("read bot (%i): %s\n", total_bots, botdata->name);

					total_bots++;

					if (last_botdata)
						last_botdata->next = botdata;
					else	// first bot
						botinfo_list = botdata;

					botdata->next = NULL;
				}
			}
			else if (mode == 1)	// teamplay data
			{
				if ( !(bot_teams[total_teams] = ReadTeamData(&f)) )
				{
					gi.error ("\nError in BOTS.CFG: Invalid TEAM (#%i)\nEither re-install Eraser, or check your bots.cfg file for errors\n\n", total_teams);
					break;
				}

				total_teams++;

				if (total_teams == MAX_TEAMS)
				{
					gi.dprintf ("Warning: MAX_TEAMS reached, unable to process all teams\n");
					break;
				}
			}
			else if (mode == 2)	// view weapon models
			{
				ReadViewWeaponModel (&f);
			}
		}

	}

	// Knightmare- total bots from file is (total_bots - 1) because the first bot "Eraser" is skipped
	gi.dprintf ("%i bots read.\n", total_bots - 1);

	if (teamplay->value)
		gi.dprintf("%i teams read.\n", total_teams);

//	gi.dprintf ("\n");

	fclose (f);

	ReadBotChat ();

	read_bot_cfg = true;	// don't load again
}

qboolean ViewModelSupported (char *model)
{
	int i;

	for (i=0; i < num_view_weapons; i++)
	{
		if (!strcmp(view_weapon_models[i], model))
			return true;
	}

	return false;
}

bot_info_t	*GetBotData (char *botname)
{
	bot_info_t	*trav, *fallback=NULL;
	qboolean	done;

	if (!botinfo_list)
	{
		gi.dprintf("No bots available!\n");
		return NULL;
	}

	if (!botname)
	{		// pick a random bot
		int i, repeat_count=0;

		if ((total_bots-1) == bot_count)	// already using all bots!
			return NULL;

		i = (int) ceil(random() * ((total_bots-1) - bot_count));

		// prevent Eraser from joining unless he is specifically asked for
		trav = botinfo_list->next;

		while (trav && (i > 0))
		{
			if (!trav->ingame_count)	// not being used
				i--;

			if (i>0)
				trav = trav->next;
		}

		done = false;
		while (!done && trav)
		{
			done = true;

			if (!fallback)
				fallback = trav;

			if (trav->ingame_count)
				done = false;
/* removed by AJ... dont want this nasty vwep hack stuff....
			else if (view_weapons->value)
			{	// if this model isn't supported by view weapons, look for another one
				char heldmodel[128];
				int	len;

				Com_strcpy(heldmodel sizeof(heldmodel), trav->skin);

				for(len = 0; heldmodel[len]; len++)
				{
					if(heldmodel[len] == '/')
					{
						heldmodel[len] = '\0';
						break;
					}
				}

				if (!ViewModelSupported(heldmodel))
				{	// model not supported by view_weapons
					done = false;
				}
			}
*/
			if (!done)
			{
				if (!(trav = trav->next))
				{
					if (repeat_count < 2)
						trav = botinfo_list->next;

					repeat_count++;
				}
			}
		}

		if (trav)
		{
			return trav;
		}
		else if (fallback)
		{
			return fallback;
		}
		else
		{
			gi.dprintf("GetBotData(): random selection didn't work\n");
			return NULL;
		}
	}
	else	// see if we can find the bot in the list
	{
		char	name[128], checkname[128];
		int	i;

		trav = botinfo_list;

		// remove the team abbrev.
		Com_strcpy (checkname, sizeof(checkname), botname);
		i=0;
		while (checkname[i] && (checkname[i] != '['))
			i++;

		if (checkname[i] == '[')
			checkname[i] = 0;

		// remove the team abbrev.
	//	strcpy(name, trav->name);
		Com_strcpy(name, sizeof(name), trav->name);
		i=0;
		while (i<128 && name[i] && (name[i] != '['))
			i++;

		if (name[i] == '[')
			name[i] = 0;

		while ( trav && Q_stricmp(checkname, name) )	// not case-sensitive
		{
			trav = trav->next;
			if (trav)
			{
				// remove the team abbrev.
			//	strcpy(name, trav->name);
				Com_strcpy(name, sizeof(name), trav->name);
				i=0;
				while (i<128 && name[i] && (name[i] != '['))
					i++;

				if (name[i] == '[')
					name[i] = 0;
			}
		}

		if (!trav)
		{	// search through the teamplay bots
			trav = teambot_list;

			if (trav)
			{
				// remove the team abbrev.
			//	strcpy(name, trav->name);
				Com_strcpy(name, sizeof(name), trav->name);
				i=0;
				while (i<128 && name[i] && (name[i] != '['))
					i++;

				if (name[i] == '[')
					name[i] = 0;
			}

			while ( trav && Q_stricmp(checkname, name) )	// not case-sensitive
			{
				trav = trav->next;
				if (trav)
				{
					// remove the team abbrev.
				//	strcpy(name, trav->name);
					Com_strcpy(name, sizeof(name), trav->name);
					i=0;
					while (i<128 && name[i] && (name[i] != '['))
						i++;

					if (name[i] == '[')
						name[i] = 0;
				}
			}
		}

		return trav;
	}
}

void	FindVisibleItemsFromNode (edict_t	*node)
{
	int	list;
	int pi=0, besti=-1;
	float	bestdist=99999, thisdist;
	vec3_t	org;
	edict_t	*trav;

	for (list=0; list<4; list++)
	{
		if (list==0)
			trav = weapons_head;
		else if (list==1)
			trav = health_head;
		else if (list==2)
			trav = ammo_head;
		else
			trav = bonus_head;

		while (trav)
		{
			// make sure it's not in a dangerous position
			VectorSubtract(trav->s.origin, tv(0,0,8), org);
			if (!(gi.pointcontents(org) & (CONTENTS_LAVA | CONTENTS_SLIME)))
			{

				if ((thisdist = entdist(trav, node)) > 256)
					goto next_ent;

				// find the end of the item's visible node list
				pi = 0;
				while ((trav->paths[pi] > -1) && (pi < MAX_PATHS))
					pi++;

				if (pi == MAX_PATHS)
					goto next_ent;

				if (visible_box(node, trav) && CanReach(node, trav))
				{
					trav->paths[pi] = node->trail_index;

					// see if this node is closer than the previous closest node
					if (!trav->movetarget || (thisdist < entdist(trav, trav->movetarget)))
					{
						trav->movetarget = node;
					}
				}
			}

next_ent:

			trav = trav->node_target;
		}
	}
}

// skill_level ranges from 0 -> 3
void	AdjustRatingsToSkill (edict_t *self)
{
	self->bot_stats->accuracy = self->botdata->bot_stats.accuracy + (float)(self->skill_level - 1) * 2.5;
	if (self->bot_stats->accuracy > 5)
		self->bot_stats->accuracy = 5;
	else if (self->bot_stats->accuracy < 1)
		self->bot_stats->accuracy = 1;

	self->bot_stats->combat = self->botdata->bot_stats.combat + (float)(self->skill_level - 1) * 2.5;
	if (self->bot_stats->combat > 5)
		self->bot_stats->combat = 5;
	else if (self->bot_stats->combat < 1)
		self->bot_stats->combat = 1;

	self->bot_stats->aggr = self->botdata->bot_stats.aggr - (float)(self->skill_level - 1) * 2;	// a good player, will make sure they stock up on armour/items before attacking
	if (self->bot_stats->aggr > 5)
		self->bot_stats->aggr = 5;
	else if (self->bot_stats->aggr < 1)
		self->bot_stats->aggr = 1;
}

void target_laser_think (edict_t *self);

// draws a translucent line from spos to epos
edict_t *DrawLine (edict_t *owner, vec3_t spos, vec3_t epos)
{
	edict_t *beam;

	beam = G_Spawn();

	beam->owner = owner;
	beam->spawnflags = 1 | 4;
	beam->classname = "path_beam";

	beam->movetype = MOVETYPE_NONE;
	beam->solid = SOLID_NOT;
	beam->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	beam->s.modelindex = 1; //gi.modelindex ("models/objects/gibs/skull/tris.md2");			// must be non-zero

	// set the beam diameter
	beam->s.frame = 4;
	// set the color (green)
	beam->s.skinnum = 0xd0d1d2d3;

	if (!beam->owner)
		beam->owner = beam;

	VectorSet (beam->mins, -8, -8, -8);
	VectorSet (beam->maxs, 8, 8, 8);

	beam->spawnflags |= 0x80000001;
	beam->svflags &= ~SVF_NOCLIENT;
	beam->flags |= FL_TEAMSLAVE;

	VectorCopy(spos, beam->s.origin);
	VectorCopy(epos, beam->s.old_origin);

	VectorSubtract(epos, spos, beam->movedir);
//	VectorNormalize2(beam->movedir, beam->movedir);

	beam->dmg = 0;
	beam->enemy = NULL;

	beam->think = target_laser_think;
	beam->nextthink = level.time + FRAMETIME;
	beam->think(beam);

	gi.linkentity (beam);

	return beam;
}

// Do team checking, which is different for CTF and non-CTF modes
qboolean botOnSameTeam (edict_t *p1, edict_t *p2)
{
	if (!ctf->value)
		return (p1->client->team == p2->client->team);
	else
		return (p1->client->resp.ctf_team == p2->client->resp.ctf_team);
}

/*
============
TeamGroup

  Issues a team grouping message, and alerts all bots accordingly
============
*/
void	TeamGroup (edict_t *ent)
{
	int		i, chat_line, chat_type;
	edict_t	*closest, *trav;
	float	best_dist;
	// Knightmare added
	char	text[2048];
	char	text2[1024];
	// end Knightmare

	if (!ent->client->team && !ctf->value)
		return;

	if (ent->last_pain == level.time)
		chat_type = CHAT_TEAMPLAY_HELP;
	else
		chat_type = CHAT_TEAMPLAY_GROUP;

	chat_line = (int) (random() * (float) bot_chat_count[chat_type]);

	// find closest item
	closest = NULL;
	best_dist = 512;

	trav    = weapons_head;
	while (trav)
	{
		if (trav->item && (entdist(trav, ent) < best_dist))
		{
			closest = trav;
		}

		trav = trav->node_target;
	}

	if (!closest || (best_dist > 256))
	{
		trav    = bonus_head;
		while (trav)
		{
			if (trav->item && (trav->item->tag != ARMOR_SHARD) && (entdist(trav, ent) < best_dist))
			{
				closest = trav;
			}

			trav = trav->node_target;
		}
	}

	if (!closest)	// abort if no item is close by
		return;

	for (i=0; i<num_players; i++)
	{
		if ((players[i] == ent) || (botOnSameTeam(players[i], ent) && ((!players[i]->target_ent) || (random() < 0.4))))
		{
			if (!players[i]->bot_client)
			{
			//	gi.cprintf(players[i], PRINT_CHAT, "%s: ", ent->client->pers.netname);
			//	gi.cprintf(players[i], PRINT_CHAT, bot_chat_text[chat_type][chat_line], closest->item->pickup_name);
			//	gi.cprintf(players[i], PRINT_CHAT, "\n");
				// Knightmare- use a single gi.cprintf() call
				Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);
				Com_sprintf (text2, sizeof(text2), bot_chat_text[chat_type][chat_line], closest->item->pickup_name);
				Com_strcat (text, sizeof(text), text2);
				Com_strcat (text, sizeof(text), "\n");
				gi.cprintf(players[i], PRINT_CHAT, text);
				// end Knightmare
			}
			else if (!players[i]->target_ent)
			{	// set bot to go straight to this player
				players[i]->target_ent = ent;
			}
		}
	}

	ent->group_pausetime = level.time + 10 + 5 * random();

	if (ent->client->team)
		ent->client->team->last_grouping = level.time + 5 + (random() * 10);
}

// tells all following bots to disperse
void TeamDisperse (edict_t *self)
{
	int		i, count=0;
	char	text[2048];	// Knightmare added

	for (i=0; i<num_players; i++)
	{
		if (players[i]->target_ent == self)
		{
			players[i]->target_ent = NULL;
			count++;
		}
	}

	if (count)
	{
	//	gi.cprintf(self, PRINT_CHAT, "%s: ", self->client->pers.netname);
	//	gi.cprintf(self, PRINT_CHAT, "all units disperse!\n");
		// Knightmare- use a single gi.cprintf() call
		Com_sprintf (text, sizeof(text), "%s: ", self->client->pers.netname);
		Com_strcat (text, sizeof(text), "all units disperse!\n");
		gi.cprintf(self, PRINT_CHAT, text);
	}
}

void BotGreeting (edict_t *chat)
{
	int		i;
	char	text[2048];	// Knightmare added

	if (!bot_chat->value)
		return;

	i = (int) (random() * (float) bot_chat_count[CHAT_GREETINGS]);

//	my_bprintf(PRINT_CHAT, "%s: ", chat->owner->client->pers.netname);
//	my_bprintf(PRINT_CHAT, bot_chat_text[CHAT_GREETINGS][i]);
//	my_bprintf(PRINT_CHAT, "\n");
	// Knightmare- use a single my_bprintf() call
	Com_sprintf (text, sizeof(text), "%s: ", chat->owner->client->pers.netname);
	Com_strcat (text, sizeof(text), bot_chat_text[CHAT_GREETINGS][i]);
	Com_strcat (text, sizeof(text), "\n");
	my_bprintf(PRINT_CHAT, text);

	G_FreeEdict(chat);
}

void BotComeback (edict_t *self)
{
	int		i;
	// Knightmare added
	char	text[2048];
	char	text2[1024];
	// end Knightmare

	if (!bot_chat->value)
		return;

	i = (int) (random() * (float) bot_chat_count[CHAT_COMEBACKS]);

//	my_bprintf(PRINT_CHAT, "%s: ", self->owner->client->pers.netname);
//	my_bprintf(PRINT_CHAT, bot_chat_text[CHAT_COMEBACKS][i], self->enemy->client->pers.netname);
//	my_bprintf(PRINT_CHAT, "\n");
	// Knightmare- use a single my_bprintf() call
	Com_sprintf (text, sizeof(text), "%s: ", self->owner->client->pers.netname);
	Com_sprintf (text2, sizeof(text2), bot_chat_text[CHAT_COMEBACKS][i], self->enemy->client->pers.netname);
	Com_strcat (text, sizeof(text), text2);
	Com_strcat (text, sizeof(text), "\n");
	my_bprintf(PRINT_CHAT, text);

	G_FreeEdict(self);
}

void BotInsultStart (edict_t *self)
{
	// insult?
	if (fabs(self->owner->client->resp.score - self->enemy->client->resp.score) < 5)
	{	// general insult
		if (last_bot_chat[CHAT_INSULTS_GENERAL] < (level.time - 5))
			BotInsult(self->owner, self->enemy, CHAT_INSULTS_GENERAL);
	}
	else
	{
		if (self->owner->client->resp.score > self->enemy->client->resp.score)
		{	// kickin ass
			if (last_bot_chat[CHAT_INSULTS_KICKASS] < (level.time - 5))
				BotInsult(self->owner, self->enemy, CHAT_INSULTS_KICKASS);
		}
		else
		{
			if (last_bot_chat[CHAT_INSULTS_LOSING] < (level.time - 5))
				BotInsult(self->owner, self->enemy, CHAT_INSULTS_LOSING);
		}
	}

	G_FreeEdict(self);
}

void BotInsult (edict_t *self, edict_t *enemy, int chat_type)
{
	int		i;
	// Knightmare added
	char	text[2048];
	char	text2[1024];
	// end Knightmare

	if (!bot_chat->value)
		return;

	i = (int) (random() * (float) bot_chat_count[chat_type]);

//	my_bprintf(PRINT_CHAT, "%s: ", self->client->pers.netname);
//	my_bprintf(PRINT_CHAT, bot_chat_text[chat_type][i], enemy->client->pers.netname);
//	my_bprintf(PRINT_CHAT, "\n");
	// Knightmare- use a single my_bprintf() call
	Com_sprintf (text, sizeof(text), "%s: ", self->client->pers.netname);
	Com_sprintf (text2, sizeof(text2), bot_chat_text[chat_type][i], enemy->client->pers.netname);
	Com_strcat (text, sizeof(text), text2);
	Com_strcat (text, sizeof(text), "\n");
	my_bprintf(PRINT_CHAT, text);

	last_bot_chat[chat_type] = level.time;

	if (enemy->bot_client && (random() < 0.3) && (last_bot_chat[CHAT_COMEBACKS] < (level.time - 3)))
	{
		edict_t *comeback;

		comeback = G_Spawn();
		comeback->think = BotComeback;
		comeback->nextthink = level.time + 2 + random();
		comeback->enemy = self;
		comeback->owner = enemy;

		last_bot_chat[CHAT_COMEBACKS] = level.time + 5;
	}
}

qboolean SameTeam (edict_t *plyr1, edict_t *plyr2)
{	// returns true if the 2 players are on the same team
	if (!plyr1->client) //ScarFace added
		return false;
	if (!plyr2->client)
		return false;

	if (ctf->value)
	{
		return (plyr1->client->resp.ctf_team ==	plyr2->client->resp.ctf_team);
	}
	else
	{
		// neutral players attack anyone
		if (!plyr1->client->team || !plyr2->client->team)
			return false;

		return (plyr1->client->team == plyr2->client->team);
	}
}

float	HomeFlagDist (edict_t *self)
{
	edict_t	*flag;

	if (self->client->resp.ctf_team == CTF_TEAM1)
		flag = flag1_ent;
	else
		flag = flag2_ent;

	return entdist(self, flag);
}

int CarryingFlag (edict_t *ent)
{	// return true if ent is carrying the enemy flag
/*
	if (!ctf->value)
		return false;
	if (!ent->client)
		return false;

	if (ent->client->resp.ctf_team == CTF_TEAM1)
		if ((ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) || (ent->client->pers.inventory[ITEM_INDEX(flag3_item)]))
			return true;
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		if ((ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) || (ent->client->pers.inventory[ITEM_INDEX(flag3_item)]))
			return true;
	else //team3
		if ((ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) || (ent->client->pers.inventory[ITEM_INDEX(flag2_item)]))
			return true;
*/
	return (ent->s.effects & (EF_FLAG1|EF_FLAG2));  //This will never support 3Team mode, because there is no EF_FLAG3
}
