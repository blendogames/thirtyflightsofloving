#include "g_local.h"

// prototypes from p_client.c and bot_misc.c 
qboolean IsFemale (edict_t *ent);
qboolean SameTeam(edict_t *plyr1, edict_t *plyr2);


// send a message to your team-mates
void lmctf_playradio(edict_t *who, char *sound)
{
	int			i;
	edict_t		*cl_ent;
	char		sample[50];

	if (IsFemale(who))
		Com_strcpy(sample, sizeof(sample), "radio/fem_");
	else
		Com_strcpy(sample, sizeof(sample), "radio/male_");

	Com_strcat (sample, sizeof(sample), sound);
	Com_strcat (sample, sizeof(sample), ".wav");
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->bot_client)
			continue;

		if (!SameTeam(who, cl_ent))
			continue;
			
		gi.sound(cl_ent, CHAN_VOICE, gi.soundindex(sample), 1, ATTN_NORM, 0);
	}

}


void lmctf_radio(edict_t *who, char *enabled)
{
//	who->radio_enabled = strcmp("0", enabled);
}

void lmctf_radiomenu(edict_t *who)
{
//	output list of samples, somehow!
}

