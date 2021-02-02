// g_svcmds.c

#include "g_local.h"


void SVCmd_Test_f(void)
{
	gi.cprintf(NULL, PRINT_HIGH, "SVCmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value,
so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a 
subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter 
lists are not saved and restored by default, because I believe it would cause too much confusion.

filterban <0 or 1>
If 1 (the default), then ip addresses matching the current list will be prohibited from entering 
the game (this is the default setting).
If 0, then only addresses matching the list will be allowed.  This lets you easily set up a 
private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned mask;
	unsigned compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter(char *s, ipfilter_t *f)
{
	char	num[128];
	int		i;
	int		j;
	byte	b[4];
	byte	m[4];
	
	for (i = 0; i < 4;  i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i = 0; i < 4; i++)
	{
		if ((*s < '0') || (*s > '9'))
		{
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while ((*s >= '0') && (*s <= '9'))
		{
			num[j++] = *s++;
		}
		num[j] = 0;

		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;

		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket(char *from)
{
	char		*p;
	byte		m[4];
	int			i;
	unsigned	in;

	i = 0;
	p = from;
	while (*p && (i < 4))
	{
		m[i] = 0;
		while ((*p >= '0') && (*p <= '9'))
		{
			m[i] = (m[i] * 10) + (*p - '0');
			p++;
		}
		if (!*p || (*p == ':'))
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;
	for (i = 0; i < numipfilters; i++)
	{
		if ((in & ipfilters[i].mask) == ipfilters[i].compare)
			return (int)filterban->value;
	}

	return (int)!filterban->value;
}


/*
=================
SVCmd_AddIP_f
=================
*/
void SVCmd_AddIP_f(void)
{
	int i;
	
	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i = 0; i < numipfilters; i++)
	{
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	}

	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.cprintf(NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	if (!StringToFilter(gi.argv(2), &ipfilters[i]))
		ipfilters[i].compare = 0xffffffff;
}

/*
=================
SVCmd_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f(void)
{
	ipfilter_t	f;
	int			i;
	int			j;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter(gi.argv(2), &f))
		return;

	for (i = 0; i < numipfilters; i++)
	{
		if ((ipfilters[i].mask == f.mask) && (ipfilters[i].compare == f.compare))
		{
			for (j = i+1; j < numipfilters; j++)
				ipfilters[j-1] = ipfilters[j];

			numipfilters--;
			gi.cprintf(NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	}

	gi.cprintf(NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SVCmd_ListIP_f
=================
*/
void SVCmd_ListIP_f(void)
{
	int		i;
	byte	b[4];

	gi.cprintf(NULL, PRINT_HIGH, "Filter list:\n");
	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf(NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SVCmd_WriteIP_f
=================
*/
void SVCmd_WriteIP_f(void)
{
	FILE	*f;
	cvar_t	*game;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
		Com_sprintf(name, sizeof(name), "%s/listip.cfg", GAMEVERSION);
	else
		Com_sprintf(name, sizeof(name), "%s/listip.cfg", game->string);

	gi.cprintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen(name, "wb");
	if (!f)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf(f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose(f);
}

//CW++
/*
=================
SVCmd_AddOP_f
=================
*/
void SVCmd_AddOP_f(void)
{
	edict_t	*targ;
	int		i = 0;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv add_op <player_num>\n");
		return;
	}

	i = atoi(gi.argv(2));
	if ((i < 1) || (i > (int)maxclients->value))
	{
		gi.cprintf(NULL, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse)
	{
		gi.cprintf(NULL, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

	if (targ->client->pers.op_status)
	{
		gi.cprintf(NULL, PRINT_HIGH, "That player already has OP status.\n");
		return;
	}

	targ->client->pers.op_status = true;
	gi_bprintf(PRINT_CHAT, "OP status given to %s.\n", targ->client->pers.netname);
}

/*
=================
SVCmd_DelOP_f
=================
*/
void SVCmd_DelOP_f(void)
{
	edict_t	*targ;
	int		i = 0;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv del_op <player_num>\n");
		return;
	}

	i = atoi(gi.argv(2));
	if ((i < 1) || (i > (int)maxclients->value))
	{
		gi.cprintf(NULL, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse)
	{
		gi.cprintf(NULL, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

	if (!targ->client->pers.op_status)
	{
		gi.cprintf(NULL, PRINT_HIGH, "That player already lacks OP status.\n");
		return;
	}

	targ->client->pers.op_status = false;
	gi_bprintf(PRINT_CHAT, "OP status removed from %s.\n", targ->client->pers.netname);
}


/*
==================
SVCmd_DevEdicts_f
==================
*/
void SVCmd_DevEdicts_f(void)
{
	edict_t	*ent;
	int		i;

	ent = &g_edicts[0];
	for (i = 0; i < globals.num_edicts ; ++i, ++ent)
		gi.dprintf("%4d %-24.24s %s %s\n", i, ent->classname, (ent->inuse)?"T":".", vtosf(ent->s.origin));
}


/*
=================
SVCmd_AddBots_f
=================
*/
void SVCmd_AddBots_f(void)
{
	edict_t *remover = NULL;
	int		i = 0;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv addbots <number>\n");
		return;
	}

	if (!(int)sv_allow_bots->value)
	{
		gi.cprintf(NULL, PRINT_HIGH, "AwakenBots are not enabled on this server.\n");
		return;
	}

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
	{
		gi.cprintf(NULL, PRINT_HIGH, "AwakenBots are not available for this gametype.\n");
		return;
	}

	if ((int)chedit->value)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Can't add bots during route creation.");
		return;
	}

	if (teamgame.match > MATCH_SETUP)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Cannot add bots during a match.\n");
		return;
	}

	if ((remover = G_Find(NULL, FOFS(classname), "bot_remover")) != NULL)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Cannot add bots - some are still being removed.\n");
		return;
	}

	i = atoi(gi.argv(2));
	if (i < 1)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Invalid number of bots.\n");
		return;
	}

	SpawnNumBots_Safe(i);
}


/*
=================
SVCmd_ClearBots_f
=================
*/
void SVCmd_ClearBots_f(void)
{
	edict_t	*spawner = NULL;

	if (!(int)sv_allow_bots->value)
		return;

	if ((int)chedit->value)
		return;

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
		return;

	if (teamgame.match > MATCH_SETUP)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Cannot remove bots during a match.\n");
		return;
	}

	if ((spawner = G_Find(NULL, FOFS(classname), "bot_spawner")) != NULL)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Cannot clear bots - some are still being added.\n");
		return;
	}

	if (gi.argc() < 3)
		RemoveNumBots_Safe(MAXBOTS);
	else
		RemoveNumBots_Safe(atoi(gi.argv(2)));
}

/*
=================
SVCmd_ListBots_f
=================
*/
void SVCmd_ListBots_f(void)
{
	edict_t	*ent;
	int		i;

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		if (!ent->isabot)
			continue;
 
		gi.dprintf("%3d %-16.16s %03d [%d:%d:%d]\n", i,
					ent->client->pers.netname,
					ent->client->resp.score,
					Bot[ent->client->pers.botindex].skill[AIMACCURACY],
					Bot[ent->client->pers.botindex].skill[AGGRESSION],
					Bot[ent->client->pers.botindex].skill[COMBATSKILL]);
	}
}
//CW--

//Pon++
/*
=================
SVCmd_SaveChain_f
=================
*/
void SVCmd_SaveChain_f(void)
{
	unsigned int	size;
	char			name[256];
	FILE			*fpout;

//CW++
	cvar_t	*game;
//CW--

	if (!(int)chedit->value)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Not in chaining mode!\n");
		return;
	}

//CW++
	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(name, sizeof(name), "%s/botroutes/%s.chn", GAMEVERSION, level.mapname);
	else
		Com_sprintf(name, sizeof(name), "%s/botroutes/%s.chn", game->string, level.mapname);
//CW--

	fpout = fopen(name, "wb");
	if (fpout == NULL)
		gi.cprintf(NULL, PRINT_HIGH, "Can't open %s\n", name);
	else
	{
		fwrite("3ZBRGDTM", sizeof(char), 8, fpout);
		fwrite(&CurrentIndex, sizeof(int), 1, fpout);
		size = (unsigned int)CurrentIndex * sizeof(route_t);
		fwrite(Route, size, 1, fpout);

		gi.cprintf(NULL, PRINT_HIGH, "Route data saved to: %s\n", name);
		fclose(fpout);
	}
}
//Pon--

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void ServerCommand(void)
{
	char	*cmd;

	cmd = gi.argv(1);

	if (Q_stricmp(cmd, "test") == 0)
		SVCmd_Test_f();
	else if (Q_stricmp(cmd, "addip") == 0)
		SVCmd_AddIP_f();
	else if (Q_stricmp(cmd, "removeip") == 0)
		SVCmd_RemoveIP_f();
	else if (Q_stricmp(cmd, "listip") == 0)
		SVCmd_ListIP_f();
	else if (Q_stricmp(cmd, "writeip") == 0)
		SVCmd_WriteIP_f();

//CW++
	else if (!Q_stricmp(cmd, "add_op"))
		SVCmd_AddOP_f();
	else if (!Q_stricmp(cmd, "del_op"))
		SVCmd_DelOP_f();
	else if (!Q_stricmp(cmd, "dev_edicts"))
		SVCmd_DevEdicts_f();
	else if (!Q_stricmp(cmd, "addbots"))
		SVCmd_AddBots_f();
	else if (!Q_stricmp(cmd, "clearbots"))
		SVCmd_ClearBots_f();
	else if (!Q_stricmp(cmd, "listbots"))
		SVCmd_ListBots_f();
//CW--

//Pon++
	else if (!Q_stricmp(cmd, "savechain"))
		SVCmd_SaveChain_f();
//Pon--

	else
		gi.cprintf(NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}
