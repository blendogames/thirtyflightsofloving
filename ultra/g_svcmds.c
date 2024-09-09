
#include "g_local.h"
#include "bot_procs.h"
#include "aj_replacelist.h" // AJ

void Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void Svcmd_Bots_f (void)
{
	int		i=2, j, len;
	char	name[128];

	while (i < gi.argc())
	{
		Com_strcpy(name, sizeof(name), gi.argv(i));
		len = (int)strlen(name);

		// convert '~' to ' '
		for (j=0; j<len; j++)
			if (name[j] == '~')
				name[j] = ' ';

		spawn_bot(name);

		i++;
	}
}

void Svcmd_Teams_f (void)
{
	int		arg=2, i;
	char	team[128];

	while (arg < gi.argc())
	{
		Com_strcpy (team, sizeof(team), gi.argv(arg));

		i=0;
		while (i<MAX_TEAMS)
		{
			if (!bot_teams[i])
				break;

			if ( !Q_stricmp(bot_teams[i]->teamname, team) || !Q_stricmp(bot_teams[i]->abbrev, team) )
			{	// found the team, so add the bots
				bot_teams[i]->ingame = true;	// bots will be added automatically (below)
				break;
			}

			i++;
		}

		arg++;
	}
}

int	force_team = CTF_NOTEAM;

void Svcmd_Blueteam_f (void)
{
	int i=2;

	if (!ctf->value) //ScarFace- only allow this command when in ctf mode
	{
		gi.dprintf ("Commmand only available in CTF mode\n");
		return;
	}
	force_team = CTF_TEAM2;
	while (i < gi.argc())
	{
//gi.dprintf("Spawning: \"%s\"\n", gi.argv(i));
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}

void Svcmd_Redteam_f (void)
{
	int i=2;

	if (!ctf->value) //ScarFace- only allow this command when in ctf mode
	{
		gi.dprintf ("Commmand only available in CTF mode\n");
		return;
	}
	force_team = CTF_TEAM1;
	while (i < gi.argc())
	{
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}

// AJ
void Svcmd_Greenteam_f (void)
{
	int i=2;

	if (!ttctf->value) //ScarFace- only allow this command when in 3tctf mode
	{
		gi.dprintf ("Commmand only available in 3TCTF mode\n");
		return;
	}
	force_team = CTF_TEAM3;
	while (i < gi.argc())
	{
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}
// end AJ

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "bots") == 0)
		Svcmd_Bots_f ();
	else if (Q_stricmp (cmd, "teams") == 0)
		Svcmd_Teams_f ();
	else if (Q_stricmp (cmd, "bluebots") == 0)
		Svcmd_Blueteam_f ();
	else if (Q_stricmp (cmd, "redbots") == 0)
		Svcmd_Redteam_f ();
// AJ 3tctf
	else if (Q_stricmp (cmd, "greenbots") == 0)
		Svcmd_Greenteam_f ();
// end AJ
// AJ - item replacement
	else if (Q_stricmp (cmd, "replace") == 0)
		Svcmd_Replace();
	else if (Q_stricmp (cmd, "onegun") == 0)
		Svcmd_OneGun();
// end AJ
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

