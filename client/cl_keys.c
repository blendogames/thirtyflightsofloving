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

#include "client.h"

#ifdef _WIN32
#include "../win32/winquake.h"
#endif

#if defined(__APPLE__) || (MACOSX)
#include <ctype.h>
#endif // __APPLE__ || MACOSX

/*

key up events are sent even if in console mode

*/


char	key_lines[32][MAXCMDLINE];
int		key_linepos;
int		shift_down=false;
int	anykeydown;

int		edit_line=0;
int		history_line=0;

int		key_waiting;
char	*keybindings[256];
qboolean	consolekeys[256];	// if true, can't be rebound while in console
qboolean	menubound[256];	// if true, can't be rebound while in menu
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
qboolean	keydown[256];

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

#if defined(__APPLE__) || (MACOSX)
	{"OPTION", K_ALT},
#else
	{"ALT", K_ALT},
#endif // __APPLE__ || MACOSX
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"NUMLOCK", K_NUMLOCK},
	{"CAPSLOCK", K_CAPSLOCK},
	{"SCROLLOCK", K_SCROLLOCK},
//	{"PRINTSCRN", K_PRINTSCRN},

#if defined(__APPLE__) || (MACOSX)
	{"F13", K_F13},
	{"F14", K_F14},
	{"F15", K_F15},
	{"COMMAND", K_COMMAND},
#endif // __APPLE__ || MACOSX

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	//Knightmare 12/22/2001
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},
	//end Knightmare

#ifdef NOTTHIRTYFLIGHTS
	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
#else
	{"^2[A]", K_JOY1},
	{"^1[B]", K_JOY2},
	{"^4[X]", K_JOY3},
	{"^3[Y]", K_JOY4},
#endif

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
#ifdef NOTTHIRTYFLIGHTS
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
#else
	{"[LEFT BUMPER]", K_AUX5},
	{"[RIGHT BUMPER]", K_AUX6},
	{"[BACK]", K_AUX7},
	{"[START]", K_AUX8},
	{"[L3]", K_AUX9},
	{"[R3]", K_AUX10},
#endif
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
#ifdef NOTTHIRTYFLIGHTS
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},
#else
	{"[LEFT TRIGGER]", K_AUX27},
	{"[RIGHT TRIGGER]", K_AUX28},
	{"[D-PAD UP]", K_AUX29},
	{"[D-PAD RIGHT]", K_AUX30},
	{"[D-PAD DOWN]", K_AUX31},
	{"[D-PAD LEFT]", K_AUX32},
#endif

#if defined(__APPLE__) || (MACOSX)
	{"KP_0", K_KP_INS},
	{"KP_1", K_KP_END},
	{"KP_2", K_KP_DOWNARROW},
	{"KP_3", K_KP_PGDN},
	{"KP_4", K_KP_LEFTARROW},
	{"KP_5", K_KP_5},
	{"KP_6", K_KP_RIGHTARROW},
	{"KP_7", K_KP_HOME},
	{"KP_8", K_KP_UPARROW},
	{"KP_9", K_KP_PGUP},
	{"KP_SLASH", K_KP_SLASH},
	{"KP_MINUS", K_KP_MINUS},
	{"KP_PLUS", K_KP_PLUS},
	{"KP_MULT", K_KP_MULT},
	{"KP_ENTER", K_KP_ENTER},
	{"KP_DOT", K_KP_DEL},
	{"KP_EQUAL", K_KP_EQUAL},
#else
	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },
	{"KP_MULT",			K_KP_MULT },
#endif // __APPLE__ || MACOSX

	{"MWHEELUP", K_MWHEELUP },
	{"MWHEELDOWN", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

qboolean Cmd_IsComplete (char *cmd);

void CompleteCommand (void)
{
	char	*cmd, *s;

	s = key_lines[edit_line]+1;
	if (*s == '\\' || *s == '/')
		s++;

	cmd = Cmd_CompleteCommand (s);
	// Knightmare - added command auto-complete
	if (cmd)
	{
		key_lines[edit_line][1] = '/';
	//	strncpy (key_lines[edit_line]+2, cmd);
		Q_strncpyz (key_lines[edit_line]+2, sizeof(key_lines[edit_line])-2, cmd);
		key_linepos = (int)strlen(cmd)+2;
		if (Cmd_IsComplete(cmd)) {
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
		} else {
			key_lines[edit_line][key_linepos] = 0;
		}
		return;
	}
}


/*
================
Key_ParseKeypad
================
*/
int Key_ParseKeypad (int inKey)
{
	int outKey;

	switch (inKey)
	{
	case K_KP_SLASH:
		outKey = '/';
		break;
	case K_KP_MULT:
		outKey = '*';
		break;
	case K_KP_MINUS:
		outKey = '-';
		break;
	case K_KP_PLUS:
		outKey = '+';
		break;
	case K_KP_HOME:
		outKey = '7';
		break;
	case K_KP_UPARROW:
		outKey = '8';
		break;
	case K_KP_PGUP:
		outKey = '9';
		break;
	case K_KP_LEFTARROW:
		outKey = '4';
		break;
	case K_KP_5:
		outKey = '5';
		break;
	case K_KP_RIGHTARROW:
		outKey = '6';
		break;
	case K_KP_END:
		outKey = '1';
		break;
	case K_KP_DOWNARROW:
		outKey = '2';
		break;
	case K_KP_PGDN:
		outKey = '3';
		break;
	case K_KP_INS:
		outKey = '0';
		break;
	case K_KP_DEL:
		outKey = '.';
		break;
#if defined(__APPLE__) || (MACOSX)
    case K_KP_EQUAL:
		outKey = '=';
		break;
#endif // __APPLE__ || MACOSX
	default:
		outKey = inKey;
		break;
	}
	return outKey;
}


/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console (int key)
{
	int i;

	key = Key_ParseKeypad (key);

#if defined(__APPLE__) || (MACOSX)
	if ( ( toupper( key ) == 'V' && keydown[K_COMMAND] ) ||
#else
	if ( ( toupper( key ) == 'V' && keydown[K_CTRL] ) ||
#endif // __APPLE__ || MACOSX
		 ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keydown[K_SHIFT] ) )
	{
		char *cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 )
		{
			int i;

			strtok( cbd, "\n\r\b" );

			i = (int)strlen( cbd );
			if ( i + key_linepos >= MAXCMDLINE)
				i= MAXCMDLINE - key_linepos;

			if ( i > 0 )
			{
				cbd[i]=0;
			//	strncat(key_lines[edit_line], cbd);
				Q_strncatz (key_lines[edit_line], sizeof(key_lines[edit_line]), cbd);
				key_linepos += i;
			}
			free( cbd );
		}
		con.backedit = 0;

		return;
	}

	if ( key == 'l' && keydown[K_CTRL] )
	{
		Cbuf_AddText ("clear\n");
		con.backedit = 0;
		return;
	}

	if ( key == K_ENTER || key == K_KP_ENTER )
	{	// backslash text are commands, else chat
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText (key_lines[edit_line]+2);	// skip the >
		else
			Cbuf_AddText (key_lines[edit_line]+1);	// valid command

		Cbuf_AddText ("\n");
		Com_Printf ("%s\n",key_lines[edit_line]);
		// majik's fix for buffer overflow
		/*if (edit_line == 31)
		{
		//	strncpy(key_lines[1], key_lines[31]);
			Q_strncpyz (key_lines[1], sizeof(key_lines[1]), key_lines[31]);
			for (edit_line = 2; edit_line < 32; edit_line++)
				memset(key_lines[edit_line], 0, sizeof(key_lines[edit_line]));
			edit_line = 0;
		}*/
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_linepos = 1;
		con.backedit = 0;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
									// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		CompleteCommand ();
		con.backedit = 0; // Knightmare added
		return;
	}
	
	if (key == K_BACKSPACE)// || ( key == K_LEFTARROW ) || ( key == K_KP_LEFTARROW ) || ( ( key == 'h' ) && ( keydown[K_CTRL] ) ) )
	{
		if (key_linepos > 1)
		{
			if (con.backedit && con.backedit<key_linepos)
			{
				if (key_linepos-con.backedit<=1)
					return;

				for (i=key_linepos-con.backedit-1; i<key_linepos; i++)
					key_lines[edit_line][i] = key_lines[edit_line][i+1];

				if (key_linepos > 1)
					key_linepos--;
			}
			else
			{
				key_linepos--;
			}
		}
		return;
	}

	if (key == K_DEL)
	{
		if (key_linepos>1 && con.backedit)
		{
			for (i=key_linepos-con.backedit; i<key_linepos; i++)
				key_lines[edit_line][i] = key_lines[edit_line][i+1];

			con.backedit--;
			key_linepos--;
		}
		return;
	}

	if (key == K_LEFTARROW) // added from Quake2max
	{
		if (key_linepos>1)
		{
			con.backedit++;
			if (con.backedit>key_linepos-1) con.backedit = key_linepos-1;
		}
		return;
	}
	if (key == K_RIGHTARROW)
	{
		if (key_linepos>1)
		{
			con.backedit--;
			if (con.backedit<0) con.backedit = 0;
		}
		return;
	} // end Q2max

	if ( ( key == K_UPARROW ) || ( key == K_KP_UPARROW ) ||
		 ( ( key == 'p' ) && keydown[K_CTRL] ) )
	{
		do
		{
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line
				&& !key_lines[history_line][1]);
		if (history_line == edit_line)
			history_line = (edit_line+1)&31;
	//	strncpy(key_lines[edit_line], key_lines[history_line]);
		Q_strncpyz (key_lines[edit_line], sizeof(key_lines[edit_line]), key_lines[history_line]);
		key_linepos = (int)strlen(key_lines[edit_line]);
		return;
	}

	if ( ( key == K_DOWNARROW ) || ( key == K_KP_DOWNARROW ) ||
		 ( ( key == 'n' ) && keydown[K_CTRL] ) )
	{
		if (history_line == edit_line) return;
		do
		{
			history_line = (history_line + 1) & 31;
		}
		while (history_line != edit_line
			&& !key_lines[history_line][1]);
		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = ']';
			key_linepos = 1;
		}
		else
		{
		//	strncpy(key_lines[edit_line], key_lines[history_line]);
			Q_strncpyz (key_lines[edit_line], sizeof(key_lines[edit_line]), key_lines[history_line]);
			key_linepos = (int)strlen(key_lines[edit_line]);
		}
		return;
	}

	if (key == K_PGUP || key == K_KP_PGUP||key == K_MWHEELUP)
	{
		con.display -= 2;
		return;
	}

	if (key == K_PGDN || key == K_KP_PGDN||key == K_MWHEELDOWN) // Quake2max change
	{
		con.display += 2;
		if (con.display > con.current)
			con.display = con.current;
		return;
	}

	if (key == K_HOME || key == K_KP_HOME)
	{
		con.display = con.current - con.totallines + 10;
		return;
	}

	if (key == K_END || key == K_KP_END )
	{
		con.display = con.current;
		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable
		
	if (key_linepos < MAXCMDLINE-1)
	{	// Knightmare- added from Quake2Max
		if (con.backedit) //insert character...
		{
			for (i=key_linepos; i>key_linepos-con.backedit; i--)
					key_lines[edit_line][i] = key_lines[edit_line][i-1];

			key_lines[edit_line][i] = key;
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
		}
		else
		{			
			key_lines[edit_line][key_linepos++] = key;
			key_lines[edit_line][key_linepos] = 0;
		}
		//key_lines[edit_line][key_linepos] = key;
		//key_linepos++;
		//key_lines[edit_line][key_linepos] = 0;
	}

}

//============================================================================

qboolean	chat_team;
char		chat_buffer[MAXCMDLINE];
int			chat_bufferlen = 0;
int			chat_backedit = 0;

void Key_Message (int key)
{
	int i;

	if ( key == K_ENTER || key == K_KP_ENTER )
	{
		if (chat_team)
			Cbuf_AddText ("say_team \"");
		else
			Cbuf_AddText ("say \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		chat_backedit = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		chat_backedit = 0;
		return;
	}

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			if (chat_backedit)
			{
				if (chat_bufferlen-chat_backedit==0)
					return;

				for (i=chat_bufferlen-chat_backedit-1; i<chat_bufferlen; i++)
					chat_buffer[i] = chat_buffer[i+1];

				chat_bufferlen--;
				chat_buffer[chat_bufferlen] = 0;
			}
			else
			{
				chat_bufferlen--;
				chat_buffer[chat_bufferlen] = 0;
			}
		}
		return;
	}
	if (key == K_DEL)
	{
		if (chat_bufferlen && chat_backedit)
		{
			for (i=chat_bufferlen-chat_backedit; i<chat_bufferlen; i++)
				chat_buffer[i] = chat_buffer[i+1];

			chat_backedit--;
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}
	if (key == K_LEFTARROW)
	{
		if (chat_bufferlen)
		{
			chat_backedit++;
			if (chat_backedit>chat_bufferlen) chat_backedit = chat_bufferlen;
			if (chat_backedit<0) chat_backedit = 0;
		}
		return;
	}
	if (key == K_RIGHTARROW)
	{
		if (chat_bufferlen)
		{
			chat_backedit--;
			if (chat_backedit>chat_bufferlen) chat_backedit = chat_bufferlen;
			if (chat_backedit<0) chat_backedit = 0;
		}
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable
	/*if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}*/
	if (chat_bufferlen == sizeof(chat_buffer)-1)
		return; // all full

	if (chat_backedit) //insert character...
	{
		for (i=chat_bufferlen; i>chat_bufferlen-chat_backedit; i--)
				chat_buffer[i] = chat_buffer[i-1];

		chat_buffer[chat_bufferlen-chat_backedit] = key;
		chat_bufferlen++;
		chat_buffer[chat_bufferlen] = 0;
	}
	else
	{
		chat_buffer[chat_bufferlen++] = key;
		chat_buffer[chat_bufferlen] = 0;
	}
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;
	
	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

#if defined(__APPLE__) || (MACOSX)
	if (!Q_strcasecmp (str, "ALT"))
	{
		for (kn=keynames ; kn->name ; kn++)
		{
			if (!Q_strcasecmp ("OPTION", kn->name))
				return (kn->keynum);
		}
	}
#endif // __APPLE__ || MACOSX

	for (kn=keynames ; kn->name ; kn++)
	{
		if (!Q_strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;	
	static	char	tinystr[2];
	
	if (keynum == -1)
		return "<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}
	
	for (kn=keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, char *binding)
{
	char	*new;
	int		l;
			
	if (keynum == -1)
		return;

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
			
// allocate memory for new binding
	l = (int)strlen (binding);	
	new = Z_Malloc (l+1);
//	strncpy (new, binding);
	Q_strncpyz (new, l+1, binding);
	new[l] = 0;
	keybindings[keynum] = new;	
/*
#if defined(__APPLE__) || (MACOSX)
	if (keynum == K_F12)
	{
		extern void	IN_SetF12EjectEnabled (qboolean theState);
		IN_SetF12EjectEnabled (keybindings[keynum][0] == 0x00);
	}
#endif // __APPLE__ || MACOSX
*/
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_SetBinding (i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];
	
	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b] );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}
	
// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
	//	strncat (cmd, Cmd_Argv(i));
		Q_strncatz (cmd, sizeof(cmd), Cmd_Argv(i));
		if (i != (c-1))
		//	strncat (cmd, " ");
			Q_strncatz (cmd, sizeof(cmd), " ");
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			fprintf (f, "bind %s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			Com_Printf ("%s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	for (i=0 ; i<32 ; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;
	
//
// init ascii characters in console mode
//
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_KP_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_KP_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_KP_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_KP_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_KP_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_KP_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_KP_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_KP_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_KP_PGDN] = true;
    consolekeys[K_KP_MULT] = true;

	consolekeys[K_SHIFT] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_KP_INS] = true;
	consolekeys[K_KP_DEL] = true;
	consolekeys[K_KP_SLASH] = true;
	consolekeys[K_KP_PLUS] = true;
	consolekeys[K_KP_MINUS] = true;
	consolekeys[K_KP_5] = true;

	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;

	consolekeys['`'] = false;
	consolekeys['~'] = false;

	for (i=0 ; i<256 ; i++)
		keyshift[i] = i;
	for (i='a' ; i<='z' ; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;
	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;

//
// register our functions
//
	Cmd_AddCommand ("bind", Key_Bind_f);
	Cmd_AddCommand ("unbind", Key_Unbind_f);
	Cmd_AddCommand ("unbindall", Key_Unbindall_f);
	Cmd_AddCommand ("bindlist", Key_Bindlist_f);
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
extern int scr_draw_loading;

void Key_Event (int key, qboolean down, unsigned time)
{
	char	*kb;
	char	cmd[1024];

	// hack for modal presses
	if (key_waiting == -1)
	{
		if (down)
			key_waiting = key;
		return;
	}

	// update auto-repeat status
	if (down)
	{
/*
#if defined(__APPLE__) || (MACOSX)
		extern int	Sys_CheckSpecialKeys (int theKey);

		// don't accept key down events at the loading screen!
		if (cls.disable_screen == true || Sys_CheckSpecialKeys (key) != 0)
		{
			return;
		}
#endif // __APPLE__ || MACOSX
*/
		key_repeats[key]++;
		if (key != K_BACKSPACE 
			&& key != K_UPARROW		// added from Quake2max
			&& key != K_DOWNARROW 
			&& key != K_LEFTARROW 
			&& key != K_RIGHTARROW
			&& key != K_PAUSE 
			&& key != K_PGUP 
			&& key != K_KP_PGUP 
			&& key != K_PGDN
			&& key != K_KP_PGDN
			&& key != K_DEL
			&& !(key>='a' && key<='z')
			&& key_repeats[key] > 1)
			return;	// ignore most autorepeats
			
	//	if (key >= 200 && !keybindings[key])
		if ( (key >= 200) && !keybindings[key] && (key != K_MWHEELDOWN) && (key != K_MWHEELUP)
			&& (key != K_JOY_UP) && (key != K_JOY_DOWN))	// bitshifter's fix
			Com_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString (key) );
	}
	else
	{
		key_repeats[key] = 0;
	}

	if (key == K_SHIFT)
		shift_down = down;

	// console key is hardcoded, so the user can never unbind it
	if (key == '`' || key == '~')
	{
		if (!down)
			return;
#ifndef NOTTHIRTYFLIGHTS
		else if (!cl_enableconsole->value && !cls.consoleActive)
			return;
#endif
		Con_ToggleConsole_f ();
		return;
	}

	// Knightmare changed
	if ( (!cls.consoleActive && cl.attractloop) && (cls.key_dest != key_menu)
		&& !(key >= K_F1 && key <= K_F12) && (cl.cinematictime > 0)
		&& (cls.realtime - cl.cinematictime > 1000) )
		key = K_ESCAPE;

	// any key during the attract mode will bring up the menu
	/*if (cl.attractloop && cls.key_dest != key_menu 
		&& !(key >= K_F1 && key <= K_F12))
		key = K_ESCAPE;*/

	// menu key is hardcoded, so the user can never unbind it
	if (key == K_ESCAPE)
	{
		if (!down)
			return;

		// Knightmare- allow disconnected menu
		if (cls.state == ca_disconnected && cls.key_dest != key_menu) // added from Quake2Max
		{
			SCR_EndLoadingPlaque ();	// get rid of loading plaque
			Cbuf_AddText ("d1\n");

			cls.consoleActive = false;

			UI_RootMenu ();
			return;
		}

		// Knightmare- skip cinematic
		if (cl.cinematictime > 0 && !cl.attractloop && cls.realtime - cl.cinematictime > 1000)
		{	// skip the rest of the cinematic
			SCR_FinishCinematic ();
		}

		if (cl.frame.playerstate.stats[STAT_LAYOUTS] && cls.key_dest == key_game)
		{	// put away help computer / inventory
			Cbuf_AddText ("cmd putaway\n");
			return;
		}
		switch (cls.key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			UI_Keydown (key);
			break;
		case key_game:
		case key_console:
			UI_RootMenu ();
			break;
		default:
			Com_Error (ERR_FATAL, "Bad cls.key_dest");
		}
		return;
	}

	// track if any key is down for BUTTON_ANY
	keydown[key] = down;
	if (down)
	{
		if (key_repeats[key] == 1)
			anykeydown++;
	}
	else
	{
		anykeydown--;
		if (anykeydown < 0)
			anykeydown = 0;
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
			Cbuf_AddText (cmd);
		}
		if (keyshift[key] != key)
		{
			kb = keybindings[keyshift[key]];
			if (kb && kb[0] == '+')
			{
				Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
				Cbuf_AddText (cmd);
			}
		}
		return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	// Knightmare changed
	if ( (cls.key_dest == key_menu && menubound[key])
	|| (cls.consoleActive && !consolekeys[key])
	|| (cls.key_dest == key_game && ( cls.state == ca_active || !consolekeys[key] ) && !cls.consoleActive) )
	//|| (cls.key_dest == key_console && !consolekeys[key])
	//|| (cls.key_dest == key_game && ( cls.state == ca_active || !consolekeys[key] ) ) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum and time as a parm
				Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", kb, key, time);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	if (shift_down)
		key = keyshift[key];

	if (cls.consoleActive) // Knightmare added
		Key_Console (key);
	else if (!scr_draw_loading) // added check from Quake2Max
	{
		switch (cls.key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			UI_Keydown (key);
			break;

		case key_game:
		case key_console:
			Key_Console (key);
			break;
		default:
			Com_Error (ERR_FATAL, "Bad cls.key_dest");
		}
	} // end Knightmare
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	anykeydown = false;

	for (i=0 ; i<256 ; i++)
	{
		if ( keydown[i] || key_repeats[i] )
			Key_Event( i, false, 0 );
		keydown[i] = 0;
		key_repeats[i] = 0;
	}
}


/*
===================
Key_GetKey
===================
*/
int Key_GetKey (void)
{
	key_waiting = -1;

	while (key_waiting == -1)
		Sys_SendKeyEvents ();

	return key_waiting;
}

