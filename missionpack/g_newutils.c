#include "g_local.h"

/*
===================================================
Check we have a valid target.

Should be set as a think function with
nextthink level.time+1 after spawning.
Sets self->think to NULL, so if you want
another think function after it you'll 
need this check in the code
===================================================
*/

void VerifyTarget (edict_t *self)
{
	edict_t	*ent;

	if(self->target)
	{
		ent = G_Find (NULL, FOFS(targetname), self->target);

		if (!ent)
		{
			gi.dprintf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
			G_FreeEdict (self);
			return;
		}
		self->enemy = ent;
	}
	else
	{
		gi.dprintf("target_set_effect at %s has no target\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->think = NULL;
	self->nextthink = 0;
}
