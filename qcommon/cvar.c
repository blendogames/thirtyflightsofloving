/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.

This file is part of Quake 2 source code.

Quake 2 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// cvar.c -- dynamic variable tracking

#include "qcommon.h"
#include "wildcard.h"
cvar_t	*cvar_vars;

qboolean	cvar_allowCheats = true;

/*
============
Cvar_InfoValidate
============
*/
static qboolean Cvar_InfoValidate (char *s)
{
	if (strstr (s, "\\"))
		return false;
	if (strstr (s, "\""))
		return false;
	if (strstr (s, ";"))
		return false;
	return true;
}

/*
============
Cvar_FindVar
============
*/
static cvar_t *Cvar_FindVar (char *var_name)
{
	cvar_t	*var;
	
	for (var=cvar_vars ; var ; var=var->next)
		if (!strcmp (var_name, var->name))
			return var;

	return NULL;
}


/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}


/*
=================
Cvar_VariableInteger
=================
*/
int Cvar_VariableInteger (char *var_name)
{
	cvar_t	*var;

	var = Cvar_FindVar(var_name);
	if (!var)
		return 0;
	return atoi (var->string);
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->string;
}


/*
============
Cvar_DefaultValue
Knightmare added
============
*/
float Cvar_DefaultValue (char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->default_string);
}


/*
============
Cvar_DefaultInteger
Knightmare added
============
*/
int Cvar_DefaultInteger (char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atoi (var->default_string);
}


/*
============
Cvar_DefaultString
Knightmare added
============
*/
char *Cvar_DefaultString (char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->default_string;
}


/*
============
Cvar_IsModified
Knightmare added
============
*/
qboolean Cvar_IsModified (char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return false;
	return var->modified;
}


/*
============
Cvar_CompleteVariable
============
*/
char *Cvar_CompleteVariable (char *partial)
{
	cvar_t		*cvar;
	int			len;
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
	// check exact match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!strcmp (partial,cvar->name))
			return cvar->name;

	// check partial match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!strncmp (partial,cvar->name, len))
			return cvar->name;

	return NULL;
}


/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *Cvar_Get (char *var_name, char *var_value, int flags)
{
	cvar_t	*var;
	
	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_name))
		{
			Com_Printf("invalid info cvar name\n");
			return NULL;
		}
	}

	var = Cvar_FindVar (var_name);
	if (var)
	{
		var->flags |= flags;
		// Knightmare- added cvar defaults
		Z_Free(var->default_string);
		var->default_string = CopyString (var_value);

		return var;
	}

	if (!var_value)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_value))
		{
			Com_Printf("invalid info cvar value\n");
			return NULL;
		}
	}

	var = Z_Malloc (sizeof(*var));
	var->name = CopyString (var_name);
	var->string = CopyString (var_value);
	// Knightmare- added cvar defaults
	var->default_string = CopyString (var_value);
	var->modified = true;
	var->value = atof (var->string);
	var->integer = atoi(var->string);
	var->description = NULL;	// Knightmare- added descriptions from From Maraa'kate's cvar code

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	var->flags = flags;

	return var;
}

/*
============
Cvar_Set2
============
*/
cvar_t *Cvar_Set2 (char *var_name, char *value, qboolean force)
{
	cvar_t		*var;

	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		return Cvar_Get (var_name, value, 0);
	}

	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (value))
		{
			Com_Printf("invalid info cvar value\n");
			return var;
		}
	}

	if (!force)
	{
		if (var->flags & CVAR_NOSET)
		{
			Com_Printf ("%s is write protected.\n", var_name);
			return var;
		}

		if ((var->flags & CVAR_CHEAT) && !cvar_allowCheats)
		{
			Com_Printf ("%s is cheat protected.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_LATCH)
		{
			if (var->latched_string)
			{
				if (strcmp(value, var->latched_string) == 0)
					return var;
				Z_Free (var->latched_string);
			}
			else
			{
				if (strcmp(value, var->string) == 0)
					return var;
			}

			if (Com_ServerState())
			{
				Com_Printf ("%s will be changed for next game.\n", var_name);
				var->latched_string = CopyString(value);
			}
			else
			{
				var->string = CopyString(value);
				var->value = atof (var->string);
				var->integer = atoi (var->string);
				if (!strcmp(var->name, "game"))
				{
					FS_SetGamedir (var->string);
					FS_ExecAutoexec ();
				}
			}
			return var;
		}
	}
	else
	{
		if (var->latched_string)
		{
			Z_Free (var->latched_string);
			var->latched_string = NULL;
		}
	}

	if (!strcmp(value, var->string))
		return var;		// not changed

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true;	// transmit at next oportunity
	
	Z_Free (var->string);	// free the old value string
	
	var->string = CopyString(value);
	var->value = atof (var->string);
	var->integer = atoi(var->string);

	return var;
}

/*
============
Cvar_ForceSet
============
*/
cvar_t *Cvar_ForceSet (char *var_name, char *value)
{
	return Cvar_Set2 (var_name, value, true);
}

/*
============
Cvar_Set
============
*/
cvar_t *Cvar_Set (char *var_name, char *value)
{
	return Cvar_Set2 (var_name, value, false);
}


/*
============
Cvar_SetToDefault
Knightmare added
============
*/
cvar_t *Cvar_SetToDefault (char *var_name)
{
	return Cvar_Set2 (var_name, Cvar_DefaultString(var_name), false);
}


/*
============
Cvar_SetDescription
Knightmare added
From Maraa'kate's cvar code
============
*/
void Cvar_SetDescription (char *var_name, char *description)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var) {
		Com_DPrintf ("Cvar_SetDescription: cvar %s is invalid, can't set description!\n", var_name);
		return;
	}
	if (var->description) {
		free(var->description);
	}
	if ( description && (strlen(description) > 1) ) {
		var->description = strdup(description);
	}
}


/*
============
Cvar_SetModified
Knightmare added
Used to force the modified field of a cvar on.
Used mainly for vid_ref, to force a vid_restart. 
============
*/
void Cvar_SetModified (char *var_name, qboolean value)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return;
	var->modified = value;
}


/*
============
Cvar_FullSet
============
*/
cvar_t *Cvar_FullSet (char *var_name, char *value, int flags)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		return Cvar_Get (var_name, value, flags);
	}

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true;	// transmit at next oportunity
	
	Z_Free (var->string);	// free the old value string
	
	var->string = CopyString(value);
	var->value = atof (var->string);
	var->integer = atoi (var->string);
	var->flags = flags;

	return var;
}


/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (char *var_name, float value)
{
	char	val[32];

	if (value == (int)value)
		Com_sprintf (val, sizeof(val), "%i",(int)value);
	else
		Com_sprintf (val, sizeof(val), "%f",value);
	Cvar_Set (var_name, val);
}


/*
=================
Cvar_SetInteger
=================
*/
void Cvar_SetInteger (char *var_name, int integer)
{
	char	val[32];

	Com_sprintf (val, sizeof(val), "%i",integer);
	Cvar_Set (var_name, val);
}


/*
=================
Cvar_ClampValue

From Q2Pro
=================
*/
float Cvar_ClampValue (cvar_t *var, float min, float max)
{
    char    val[32];

    if (var->value < min)
	{
        if (min == (int)min) {
            Com_sprintf (val, sizeof(val), "%i", (int)min);
        }
		else {
            Com_sprintf (val, sizeof(val), "%f", min);
        }
		Cvar_Set (var->name, val);
        return min;
    }
    if (var->value > max)
	{
        if (max == (int)max) {
            Com_sprintf (val, sizeof(val), "%i", (int)max);
        }
		else {
            Com_sprintf (val, sizeof(val), "%f", max);
        }
		Cvar_Set (var->name, val);
        return max;
    }
    return var->value;
}


/*
=================
Cvar_ClampInteger

From Q2Pro
=================
*/
int Cvar_ClampInteger (cvar_t *var, int min, int max)
{
    char    val[32];

    if (var->integer < min)
	{
        Com_sprintf (val, sizeof(val), "%i", min);
        Cvar_Set (var->name, val);
        return min;
    }
    if (var->integer > max)
	{
        Com_sprintf (val, sizeof(val), "%i", max);
        Cvar_Set (var->name, val);
        return max;
    }
    return var->integer;
}


/*
============
Cvar_GetLatchedVars

Any variables with latched values will now be updated
============
*/
void Cvar_GetLatchedVars (void)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
	{
		if (!var->latched_string)
			continue;
		Z_Free (var->string);
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = atof(var->string);
		var->integer = atoi(var->string);
		if (!strcmp(var->name, "game"))
		{
			FS_SetGamedir (var->string);
			FS_ExecAutoexec ();
		}
	}
}


/*
=================
Cvar_FixCheatVars

Resets cvars that could be used for multiplayer cheating
Borrowed from Q2E
=================
*/
void Cvar_FixCheatVars (qboolean allowCheats)
{
	cvar_t	*var;

	if (cvar_allowCheats == allowCheats)
		return;
	cvar_allowCheats = allowCheats;

	if (cvar_allowCheats)
		return;

	for (var = cvar_vars; var; var = var->next)
	{
		if (!(var->flags & CVAR_CHEAT))
			continue;

		if (!Q_stricmp(var->string, var->default_string))
			continue;

		Cvar_Set2 (var->name, var->default_string, true);
	}
}


/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;
		
// perform a variable print or set
	if (Cmd_Argc() == 1)
	{	// Knightmare- show latched value if applicable
		if ((v->flags & CVAR_LATCH) && v->latched_string)
			Com_Printf ("\"%s\" is \"%s\" : default is \"%s\" : latched to \"%s\"\n", v->name, v->string, v->default_string, v->latched_string);
		else
			Com_Printf ("\"%s\" is \"%s\" : default is \"%s\"\n", v->name, v->string, v->default_string);

		// Knightmare- added descriptions from From Maraa'kate's cvar code
		if ( (v->description != NULL) && (con_show_description->integer || !strcmp(v->name, "con_show_description")) )
			Com_Printf ("Description: %s\n", v->description);
		// end Knightmare

		return true;
	}

	Cvar_Set (v->name, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_Set_f

Allows setting and defining of arbitrary cvars from console
============
*/
void Cvar_Set_f (void)
{
	int		c;
	int		flags;

	c = Cmd_Argc();
	if (c != 3 && c != 4)
	{
		Com_Printf ("usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4)
	{
		if (!strcmp(Cmd_Argv(3), "u"))
			flags = CVAR_USERINFO;
		else if (!strcmp(Cmd_Argv(3), "s"))
			flags = CVAR_SERVERINFO;
		else
		{
			Com_Printf ("flags can only be 'u' or 's'\n");
			return;
		}
		Cvar_FullSet (Cmd_Argv(1), Cmd_Argv(2), flags);
	}
	else
		Cvar_Set (Cmd_Argv(1), Cmd_Argv(2));
}


/*
=================
Cvar_Toggle_f

Allows toggling of arbitrary cvars from console
=================
*/	
void Cvar_Toggle_f (void)
{
	cvar_t	*var;

	if (Cmd_Argc() != 2){
		Com_Printf("Usage: toggle <variable>\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));
	if (!var){
		Com_Printf("'%s' is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_Set2(var->name, va("%i", !var->integer), false);
}


/*
=================
Cvar_Reset_f

Allows resetting of arbitrary cvars from console
=================
*/
void Cvar_Reset_f (void)
{
	cvar_t *var;

	if (Cmd_Argc() != 2){
		Com_Printf("Usage: reset <variable>\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));
	if (!var){
		Com_Printf("'%s' is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_Set2(var->name, var->default_string, false);
}


/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (char *path)
{
	cvar_t	*var;
	char	buffer[1024];
	FILE	*f;

	f = fopen (path, "a");
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			Com_sprintf (buffer, sizeof(buffer), "set %s \"%s\"\n", var->name, var->string);
			fprintf (f, "%s", buffer);
		}
	}
	fclose (f);
}


/*
============
Cvar_List_f
============
*/
void Cvar_List_f (void)
{
	cvar_t	*var;
	int		i, j, c;
	char	*wc;

	// RIOT's Quake3-sytle cvarlist
	c = Cmd_Argc();

	if (c != 1 && c!= 2)
	{
		Com_Printf ("usage: cvarlist [wildcard]\n");
		return;
	}

	if (c == 2)
		wc = Cmd_Argv(1);
	else
		wc = "*";

	i = 0;
	j = 0;
	for (var = cvar_vars; var; var = var->next, i++)
	{
		if (wildcardfit (wc, var->name))
		//if (strstr (var->name, Cmd_Argv(1)))
		{
			j++;
			if (var->flags & CVAR_ARCHIVE)
				Com_Printf ("A");
			else
				Com_Printf (" ");

			if (var->flags & CVAR_USERINFO)
				Com_Printf ("U");
			else
				Com_Printf (" ");

			if (var->flags & CVAR_SERVERINFO)
				Com_Printf ("S");
			else
				Com_Printf (" ");

			if (var->flags & CVAR_NOSET)
				Com_Printf ("-");
			else if (var->flags & CVAR_LATCH)
				Com_Printf ("L");
			else
				Com_Printf (" ");

			if (var->flags & CVAR_CHEAT)
				Com_Printf("C");
			else
				Com_Printf(" ");

			// Knightmare- added descriptions from From Maraa'kate's cvar code
			if (var->description != NULL)
				Com_Printf("D");
			else
				Com_Printf(" ");

			// show latched value if applicable
			if ((var->flags & CVAR_LATCH) && var->latched_string)
				Com_Printf (" %s \"%s\" - default: \"%s\" - latched: \"%s\"\n", var->name, var->string, var->default_string, var->latched_string);
			else
				Com_Printf (" %s \"%s\" - default: \"%s\"\n", var->name, var->string, var->default_string);
		}
	}
	Com_Printf ("Legend: \'A\'=Archive \'U\'=Userinfo \'S\'=Serverinfo \'-\'=Write Protected \'L\'=Latched  \'C\'=Cheat \'D\'=Has Description\n");
	Com_Printf (" %i cvars, %i matching\n", i, j);
}


qboolean userinfo_modified;


char	*Cvar_BitInfo (int bit)
{
	static char	info[MAX_INFO_STRING];
	cvar_t	*var;

	info[0] = 0;

	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->flags & bit)
		{
			// FS: Exploit.  Making a userinfo key (or value) longer than 63 chars (i.e. setting name longer than this)
			//    will actually delete the key possibly having checks elsewhere in the code turn up NULL
			//    So let's truncate it.
			if (bit & CVAR_USERINFO)
			{
				if ( strlen(var->name) > (MAX_INFO_KEY - 1) )
				{
					Com_Printf("WARNING: Userinfo key \"%s\" longer than %i chars.  Truncating.\n", var->name, MAX_INFO_KEY-1);
					Q_strncpyz (var->name, var->name, MAX_INFO_KEY);
				}
				if ( strlen(var->string) > (MAX_INFO_KEY - 1) )
				{
					Com_Printf("WARNING: Userinfo value \"%s\" for key \"%s\" longer than %i chars.  Truncating.\n", var->string, var->name, MAX_INFO_KEY-1);
					Q_strncpyz (var->string, var->string, MAX_INFO_KEY);
				}
			}

			Info_SetValueForKey (info, var->name, var->string);
		}
	}
	return info;
}

// returns an info string containing all the CVAR_USERINFO cvars
char	*Cvar_Userinfo (void)
{
	return Cvar_BitInfo (CVAR_USERINFO);
}

// returns an info string containing all the CVAR_SERVERINFO cvars
char	*Cvar_Serverinfo (void)
{
	return Cvar_BitInfo (CVAR_SERVERINFO);
}

/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init (void)
{
	Cmd_AddCommand ("set", Cvar_Set_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("reset", Cvar_Reset_f);
	Cmd_AddCommand ("cvarlist", Cvar_List_f);

}
