/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// console.c

#include "client.h"

console_t	con;

cvar_t		*con_notifytime;
cvar_t		*con_alpha;		// Knightare- Psychospaz's transparent console
//cvar_t		*con_height;	// Knightmare- how far the console drops down
cvar_t		*con_newconback;	// whether to use new console background
cvar_t		*con_oldconbar;		// whether to draw bottom bar on old console
qboolean	newconback_found = false;	// whether to draw Q3-style console


extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
		

/*
================
Con_DrawString
================
*/
void Con_DrawString (int x, int y, char *string, int alpha)
{
	DrawStringGeneric (x, y, string, alpha, SCALETYPE_CONSOLE, false);
}


/*
================
Key_ClearTyping
================
*/
void Key_ClearTyping (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
	con.backedit = 0;
}


/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
//	SCR_EndLoadingPlaque ();	// get rid of loading plaque

	// Knightmare- allow disconnected menu
	if (cls.state == ca_disconnected && cls.key_dest != key_menu)
	{	// start the demo loop again
		Cbuf_AddText ("d1\n");
		return;
	}

	Key_ClearTyping ();
	Con_ClearNotify ();

	if (cls.consoleActive) // Knightmare changed
	//if (cls.key_dest == key_console)
	{
		cls.consoleActive = false; // Knightmare added

		//UI_ForceMenuOff ();
		//if (cls.key_dest != key_menu)
		if (Cvar_VariableValue ("maxclients") == 1 
			&& Com_ServerState () && cls.key_dest != key_menu)
				Cvar_Set ("paused", "0");
	}
	else
	{
		cls.consoleActive = true; // Knightmare added
		//UI_ForceMenuOff ();
		//cls.key_dest = key_console;	

		if (Cvar_VariableValue ("maxclients") == 1 
			&& Com_ServerState () && cls.key_dest != key_menu)
			Cvar_Set ("paused", "1");
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Con_ToggleChat_f (void)
{
	Key_ClearTyping ();

	if (cls.consoleActive) // Knightmare added
	//if (cls.key_dest == key_console)
	{
		if (cls.state == ca_active)
		{
			UI_ForceMenuOff ();
			cls.consoleActive = false; // Knightmare added
			cls.key_dest = key_game;
		}
	}
	else
		//cls.key_dest = key_console;
		cls.consoleActive = true; // Knightmare added

	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	memset (con.text, ' ', CON_TEXTSIZE);
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("usage: condump <filename>\n");
		return;
	}

	Com_sprintf (name, sizeof(name), "%s/%s.txt", FS_Gamedir(), Cmd_Argv(1));

	Com_Printf ("Dumped console text to %s.\n", name);
	FS_CreatePath (name);
	f = fopen (name, "w");
	if (!f)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = con.current - con.totallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		for (x=0 ; x<con.linewidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != con.linewidth)
			break;
	}

	// write the remaining lines
	buffer[con.linewidth] = 0;
	for ( ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		strncpy (buffer, line, con.linewidth);
		for (x=con.linewidth-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;
	
	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con.times[i] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void)
{
	chat_team = false;
	cls.key_dest = key_message;
	cls.consoleActive = false; // Knightmare added
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	chat_team = true;
	cls.key_dest = key_message;
	cls.consoleActive = false; // Knightmare added
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	if (con_font_size)
		width = viddef.width/FONT_SIZE - 2;
	else	 // (viddef.width / 8) - 2
		width = (viddef.width >> 3) - 2;
	width = min((MAXCMDLINE - 2), width);	// clamp width to MAXCMDLINE - 2 to prevent buffer overflow

	if (width == con.linewidth)
		return;

	if (width < 1)		// video hasn't been initialized yet
	{
		width = 78; // was 38
		con.linewidth = width;
		con.backedit = 0;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		memset (con.text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		con.backedit = 0;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if (con.totallines < numlines)
			numlines = con.totallines;

		numchars = oldwidth;
	
		if (con.linewidth < numchars)
			numchars = con.linewidth;

		memcpy (tbuf, con.text, CON_TEXTSIZE);
		memset (con.text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con.text[(con.totallines - 1 - i) * con.linewidth + j] =
						tbuf[((con.current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con.linewidth = -1;
	con.backedit = 0;

	Con_CheckResize ();
	
	Com_Printf ("Console initialized.\n");

//
// register our commands
//
	con_notifytime = Cvar_Get ("con_notifytime", "4", 0); // Knightmare- increased for fade
	// Knightmare- Psychospaz's transparent console
	con_alpha = Cvar_Get ("con_alpha", "0.5", CVAR_ARCHIVE);
	// Knightmare- how far the console drops down
	//con_height = Cvar_Get ("con_height", "0.5", CVAR_ARCHIVE);
	con_newconback = Cvar_Get ("con_newconback", "0", CVAR_ARCHIVE);	// whether to use new console background
	con_oldconbar = Cvar_Get ("con_oldconbar", "1", CVAR_ARCHIVE);		// whether to draw bottom bar on old console

	// whether to use new-style console background
	newconback_found = false;
	if ( (FS_LoadFile("gfx/ui/newconback.tga", NULL) != -1)
#ifdef PNG_SUPPORT
		|| (FS_LoadFile("gfx/ui/newconback.png", NULL) != -1)
#endif	// PNG_SUPPORT
		|| (FS_LoadFile("gfx/ui/newconback.jpg", NULL) != -1) )
		newconback_found = true;

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f);
	con.initialized = true;
}


/*
===============
Con_StringSetParams
===============
*/
qboolean Con_StringSetParams (char modifier, char *colorCode, qboolean *bold, qboolean *shadow, qboolean *italic, qboolean *alt)
{
	// sanity check
	if (!colorCode || !bold || !shadow || !italic || !alt)
		return false;

	switch (modifier)
	{
		case 'R':
		case 'r':
			*colorCode = 0;
			*bold = *shadow = *italic = *alt = false;
			return true;
		case 'B':
		case 'b':
			if (*bold) 
				*bold = false;
			else 
				*bold = true;
			return true;
		case 'S':
		case 's':
			if (*shadow) 
				*shadow = false; 
			else 
				*shadow = true;
			return true;
		case 'I':
		case 'i':
			if (*italic) 
				*italic = false; 
			else 
				*italic = true;
			return true;
		case COLOR_RED:
		case COLOR_GREEN:
		case COLOR_YELLOW:
		case COLOR_BLUE:
		case COLOR_CYAN:
		case COLOR_MAGENTA:
		case COLOR_WHITE:
		case COLOR_BLACK:
		case COLOR_ORANGE:
		case COLOR_GRAY:
			*colorCode = modifier;
			return true;
		case 'A':	// alt text color
		case 'a':
			*alt = true;
			return true;
	}
	
	return false;
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (char colorCode, qboolean bold, qboolean shadow, qboolean italic, qboolean alt)
{
	int		y;

	con.x = 0;
	if (con.display == con.current)
		con.display++;
	con.current++;
	memset (&con.text[(con.current%con.totallines)*con.linewidth]
	, ' ', con.linewidth);

	// add any wrapped formatting
	y = con.current % con.totallines;
	if (colorCode != 0)
	{
		con.text[y*con.linewidth+con.x] = '^';
		con.x++;
		con.text[y*con.linewidth+con.x] = colorCode;
		con.x++;
	}
	if (bold)
	{
		con.text[y*con.linewidth+con.x] = '^';
		con.x++;
		con.text[y*con.linewidth+con.x] = 'b';
		con.x++;
	}
	if (shadow)
	{
		con.text[y*con.linewidth+con.x] = '^';
		con.x++;
		con.text[y*con.linewidth+con.x] = 's';
		con.x++;
	}
	if (italic)
	{
		con.text[y*con.linewidth+con.x] = '^';
		con.x++;
		con.text[y*con.linewidth+con.x] = 'i';
		con.x++;
	}
	if (alt)
	{
		con.text[y*con.linewidth+con.x] = '^';
		con.x++;
		con.text[y*con.linewidth+con.x] = 'a';
		con.x++;
	}
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void Con_Print (char *txt)
{
	int			y, c, l;	//, i;
	static int	cr;
	int			mask;
	// vars for format wrapping
	char		modifier, colorCode = 0;
	qboolean	nextCharModifierCheck = false,  bold = false, shadow = false, italic = false, alt = false;

	if (!con.initialized)
		return;

	if (txt[0] == 1 || txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( (c = *txt) )
	{
	// count word length
		for (l=0 ; l< con.linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con.linewidth && (con.x + l > con.linewidth) ) {
			con.x = 0;
		}

		txt++;

		if (cr)
		{
			con.current--;
			cr = false;
		}
		
		if (!con.x)
		{
			Con_Linefeed (colorCode, bold, shadow, italic, alt);
		// mark time for transparent overlay
			if (con.current >= 0)
				con.times[con.current % NUM_CON_TIMES] = cls.realtime;
		}

		// track formatting codes for word wrap
		modifier = (c & ~128);
		if (nextCharModifierCheck)	// last char was a ^
		{
			Con_StringSetParams (modifier, &colorCode, &bold, &shadow, &italic, &alt);
			nextCharModifierCheck = false;
		}
		else {
			// set var to check modifier if char is ^
			nextCharModifierCheck = (modifier == '^') ? true : false;
		}

		switch (c)
		{
		case '\n':
			con.x = 0;
			colorCode = 0;
			bold = shadow = italic = alt = false;
			break;

		case '\r':
			con.x = 0;
			cr = 1;
			colorCode = 0;
			bold = shadow = italic = alt = false;
			break;

		default:	// display character and advance
			y = con.current % con.totallines;
			con.text[y*con.linewidth+con.x] = c | mask | con.ormask;
			con.x++;
			if (con.x >= con.linewidth)
				con.x = 0;
			break;
		}
		
	}
}


/*
==============
Con_CenteredPrint
==============
*/
void Con_CenteredPrint (char *text)
{
	int		l;
	char	buffer[1024];

	l = strlen(text);
	l = (con.linewidth-l)/2;
	if (l < 0)
		l = 0;
	memset (buffer, ' ', l);
//	strncpy (buffer+l, text);
//	strncat (buffer, "\n");
	Q_strncpyz (buffer+l, text, sizeof(buffer)-1);
	Q_strncatz (buffer, "\n", sizeof(buffer));
	Con_Print (buffer);
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput
The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
	int		y;
	int		i;
	char	*text, output[2048], addch[8];
	float	conLeft = 0;

	if (!cls.consoleActive && cls.state == ca_active)
		return;		// don't draw anything (always draw if not active)

	SCR_AdjustFrom640 (&conLeft, NULL, NULL, NULL, ALIGN_STRETCH);

	/*if (cls.key_dest == key_menu)
		return;
	if (cls.key_dest != key_console && cls.state == ca_active)
		return;*/		// don't draw anything (always draw if not active)

	text = key_lines[edit_line];
	
// add the cursor frame
	if (con.backedit)
		text[key_linepos] = ' ';
	else
		text[key_linepos] = 10+((int)(cls.realtime>>8)&1);
	
// fill out remainder with spaces
	for (i=key_linepos+1 ; i< con.linewidth ; i++)
		text[i] = ' ';
		
//	prestep if horizontally scrolling
	if (key_linepos >= con.linewidth)
		text += 1 + key_linepos - con.linewidth;
		
// draw it
	y = con.vislines-FONT_SIZE*2; // was 16

	Com_sprintf (output, sizeof(output), "");
	for (i=0; i<con.linewidth; i++)
	{
		if (con.backedit == key_linepos-i && ((int)(cls.realtime>>8)&1)) {
		//	Com_sprintf (output, sizeof(output), "%s%c", output, 11 );
			addch[0] = 11;
			addch[1] = '\0';
			Q_strncatz (output, addch, sizeof(output));
		}
		else {
		//	Com_sprintf (output, sizeof(output), "%s%c", output, text[i]);
			addch[0] = text[i];
			addch[1] = '\0';
			Q_strncatz (output, addch, sizeof(output));
		}
	}
	Con_DrawString ( (int)conLeft + FONT_SIZE/2, con.vislines - (int)(2.75*FONT_SIZE), output, 255);

// remove cursor
	key_lines[edit_line][key_linepos] = 0;
}


/*
================
Con_DrawNotify
Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		x;
	char	*text, output[2048], addch[8];
	int		i, j;
	//int		time;
	char	*s;
	int		alpha, lines;
	float	v, time, conLeft, conWidth;

	lines = 0;
	v = 0;

	conLeft = 0;
	conWidth = SCREEN_WIDTH;
	SCR_AdjustFrom640 (&conLeft, NULL, &conWidth, NULL, ALIGN_STRETCH);

	Com_sprintf (output, sizeof(output), "");

	// this is the say msg while typeing...
	if (cls.key_dest == key_message)
	{
		if (chat_team)
			Com_sprintf (output, sizeof(output), "%s", " say_team: ");
		else
			Com_sprintf (output, sizeof(output), "%s", " say: ");

		s = chat_buffer;
		x = 0;
		if (chat_bufferlen > (viddef.width/FONT_SIZE)-(strlen(output)+1))
			x += chat_bufferlen - (int)((viddef.width/FONT_SIZE)-(strlen(output)+1));

		while (s[x])
		{
			if ( chat_backedit && (chat_backedit == chat_bufferlen-x) && ((int)(cls.realtime>>8)&1) ) {
			//	Com_sprintf (output, sizeof(output), "%s%c", output, 11 );
				addch[0] = 11;
				addch[1] = '\0';
				Q_strncatz (output, addch, sizeof(output));
			}
			else {
			//	Com_sprintf (output, sizeof(output), "%s%c", output, (char)s[x]);
				addch[0] = s[x];
				addch[1] = '\0';
				Q_strncatz (output, addch, sizeof(output));
			}

			x++;
		}

		if (!chat_backedit) {
		//	Com_sprintf (output, sizeof(output), "%s%c", output, 10+((int)(cls.realtime>>8)&1) );		
			addch[0] = 10+((int)(cls.realtime>>8)&1);
			addch[1] = '\0';
			Q_strncatz (output, addch, sizeof(output));
		}

		Con_DrawString ((int)conLeft, v, output, 255);

		v += FONT_SIZE*2; // make extra space so we have room
	}

	for (i = con.current-NUM_CON_TIMES+1; i <= con.current; i++)
	{
		if (i < 0)
			continue;
		time = con.times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = cls.realtime - time;
		if (time > con_notifytime->value*1000)
			continue;

		// vertical offset set by closest to dissapearing
		lines++;
	}

//	v = 0;
	if (lines)
		for (j = 0, i = con.current-NUM_CON_TIMES+1; i <= con.current; i++, j++)
		{
			if (i < 0)
				continue;
			time = con.times[i % NUM_CON_TIMES];
			if (time == 0)
				continue;
			time = cls.realtime - time;
			if (time > con_notifytime->value*1000)
				continue;

			text = con.text + (i % con.totallines)*con.linewidth;
			
			alpha = 255 * sqrt( (1.0-time/(con_notifytime->value*1000.0+1.0)) * (((float)v+8.0)) / (8.0*lines) );
		//	alpha = (1-time/(con_notifytime->value*1000.0+2)) * (255*(v+16)) / (4*lines);
			if (alpha < 0) alpha = 0;
			if (alpha > 255) alpha = 255;

			Com_sprintf (output, sizeof(output), "");
			for (x = 0; x < con.linewidth; x++) {
			//	Com_sprintf (output, sizeof(output), "%s%c", output, (char)text[x]);
				addch[0] = (char)text[x];
				addch[1] = '\0';
				Q_strncatz (output, addch, sizeof(output));
			}

			Con_DrawString ((int)conLeft + FONT_SIZE/2, v, output, alpha);

			v += FONT_SIZE;
		}
}


/*
================
Con_DrawDownloadProgress

Draws a download progress bar
================
*/
void Con_DrawDownloadProgress (float conLeft, float conWidth, float conLineHeight, int verLen, int red, int green, int blue)
{
	int		i, j, x, y, len;
	char	*text;
	char	dlbar[1024];
	int		graph_x, graph_y, graph_h, graph_w;

	if ((text = strrchr(cls.downloadname, '/')) != NULL)
		text++;
	else
		text = cls.downloadname;

	memset (dlbar, 0, sizeof(dlbar)); // clear dlbar

	// make this a little shorter in case of longer version string
	x = con.linewidth - ((con.linewidth * 7) / 40) - (verLen-14);

	if (cls.downloadrate > 0.0f)
		y = x - (int)strlen(text) - 19;
	else
		y = x - (int)strlen(text) - 8;
	i = con.linewidth/3;
	if (strlen(text) > i)
	{
		y = x - i - 11;
		strncpy(dlbar, text, i);
		dlbar[i] = 0;
	//	strncat(dlbar, "...");
		Q_strncatz(dlbar, "...", sizeof(dlbar));
	}
	else
	//	strncpy(dlbar, text);
		Q_strncpyz(dlbar, text, sizeof(dlbar));
//	strncat(dlbar, ": ");
	Q_strncatz(dlbar, ": ", sizeof(dlbar));
	i = (int)strlen(dlbar);
	
	// init solid color download bar
	graph_x = (i+1)*FONT_SIZE;
	graph_y = con.vislines - (int)(FONT_SIZE*1.5) - (int)conLineHeight; // was -12
	graph_w = y*FONT_SIZE;
	graph_h = FONT_SIZE;

	for (j = 0; j < y; j++) // add blank spaces
		Com_sprintf(dlbar + strlen(dlbar), sizeof(dlbar)-strlen(dlbar), " ");

	if (cls.downloadrate > 0.0f)
		Com_sprintf(dlbar + strlen(dlbar), sizeof(dlbar)-strlen(dlbar), " %2d%% (%4.2fKB/s)", cls.downloadpercent, cls.downloadrate);
	else
		Com_sprintf(dlbar + strlen(dlbar), sizeof(dlbar)-strlen(dlbar), " %2d%%", cls.downloadpercent);

	// draw it
	//y = graph_y;
	len = (int)strlen(dlbar);
	for (i = 0; i < len; i++)
		if (dlbar[i] != ' ')
			R_DrawChar((int)conLeft + (i+1)*FONT_SIZE, graph_y, dlbar[i], CON_FONT_SCALE, 255, 255, 255, 255, false, (i==(len-1)) );

	// new solid color download bar
	graph_x--; graph_y--; graph_w+=2; graph_h+=2;
	R_DrawFill ((int)conLeft + graph_x, graph_y, graph_w, graph_h, 255, 255, 255, 90);
	R_DrawFill ((int)conLeft + (int)(graph_x+graph_h*0.2), (int)(graph_y+graph_h*0.2),
		(int)((graph_w-graph_h*0.4)*cls.downloadpercent*0.01), (int)(graph_h*0.6),
		red, green, blue, 255);
}


/*
================
Con_DrawConsole
Draws the console with the solid background
================
*/

void Con_DrawConsole (float frac, qboolean trans)
{
	int				i, x, y;
	int				rows;
	char			*text, output[1024], addch[8];
	int				row;
	int				lines;
	char			version[64];
	float			alpha, barheight, conLeft, conWidth, picLeft, picWidth, picHeight, pboxWidth;
	// changeable download bar color
	int				red, green, blue;

	// Q3-style console bottom bar
	conLeft = picLeft = 0;
	conWidth = picWidth = SCREEN_WIDTH;
	picHeight = SCREEN_HEIGHT;
	SCR_AdjustFrom640 (&picLeft, NULL, &picWidth, &picHeight, ALIGN_CENTER);
	CL_TextColor ((int)alt_text_color->value, &red, &green, &blue);
	if ( (newconback_found && con_newconback->value) || con_oldconbar->value ) {
		barheight = 2;
		SCR_AdjustFrom640 (&conLeft, NULL, &conWidth, &barheight, ALIGN_STRETCH);
	}
	else {
		barheight = 0;
		SCR_AdjustFrom640 (&conLeft, NULL, &conWidth, NULL, ALIGN_STRETCH);
	}

	lines =  viddef.height * frac;
	if (lines <= 0)
		return;

	if (lines > viddef.height)
		lines = viddef.height;

	// Psychospaz's transparent console
	//alpha = (trans) ? ((frac/ (newconback_found?0.5:con_height->value) )*con_alpha->value) : 1;
	alpha = trans ? ((newconback_found && con_newconback->value) ? con_alpha->value : 2*frac*con_alpha->value) : 1;

	// draw the background
	//i = newconback_found ? lines - barheight : lines*(1/con_height->value);
	//j = newconback_found ? 0 : (con_height->value-1)*i - barheight;
	y = lines - barheight;
	if (y < 1)	y = 0;
	else if (newconback_found && con_newconback->value)	// Q3-style console
		R_DrawStretchPic ((int)picLeft, 0, picWidth, lines-barheight, "/gfx/ui/newconback.pcx", alpha);
	else
		R_DrawStretchPic ((int)picLeft, (lines-(int)picHeight-(int)barheight), picWidth, (int)picHeight, "conback", alpha);

	// pillarbox sides if console is wider than scaled pic
	if (conWidth > picWidth)
	{
		pboxWidth = picLeft - conLeft;
		R_DrawFill ((int)conLeft, 0, (int)pboxWidth, lines-(int)barheight, 0, 0, 0, (int)(alpha*255.0f));
		R_DrawFill ((int)picLeft + (int)picWidth, 0, (int)pboxWidth, lines-(int)barheight, 0, 0, 0, (int)(alpha*255.0f));
	}

	// changed to "KMQuake2 vx.xx"
#ifdef ERASER_COMPAT_BUILD
#ifdef NET_SERVER_BUILD
	Com_sprintf (version, sizeof(version), S_COLOR_BOLD S_COLOR_SHADOW S_COLOR_ALT"KMQuake2 v%4.2f (Eraser net server)", VERSION);
#else // NET_SERVER_BUILD
	Com_sprintf (version, sizeof(version), S_COLOR_BOLD S_COLOR_SHADOW S_COLOR_ALT"KMQuake2 v%4.2f (Eraser compatible)", VERSION);
#endif // NET_SERVER_BUILD
#else // ERASER_COMPAT_BUILD
#ifdef NET_SERVER_BUILD
	Com_sprintf (version, sizeof(version), S_COLOR_BOLD S_COLOR_SHADOW S_COLOR_ALT"KMQuake2 v%4.2f (net server)", VERSION);
#else
	Com_sprintf (version, sizeof(version), S_COLOR_BOLD S_COLOR_SHADOW S_COLOR_ALT"KMQuake2 v%4.2f", VERSION);
#endif // ERASER_COMPAT_BUILD
#endif // NEW_ENTITY_STATE_MEMBERS

	Con_DrawString ((int)(conLeft+conWidth)-FONT_SIZE*(stringLen((const char *)&version))-3, y-(int)(1.25*FONT_SIZE), version, 255);

	if ( (newconback_found && con_newconback->value) || con_oldconbar->value ) // Q3-style console bottom bar
		R_DrawFill ((int)conLeft, y, conWidth, barheight, red, green, blue, 255);

	// draw the text
	con.vislines = lines;
	rows = (lines-(int)(2.75*FONT_SIZE))/FONT_SIZE;		// rows of text to draw
	y = lines - (int)(3.75*FONT_SIZE);

	// draw from the bottom up
	if (con.display != con.current)
	{
	// draw arrows to show the buffer is backscrolled
		for (x = 0; x < con.linewidth; x+=4)
			R_DrawChar ((int)conLeft + (x+1)*FONT_SIZE, y, '^', CON_FONT_SCALE, 255, 0, 0, 255, false, ((x+4)>=con.linewidth) );
	
		y -= FONT_SIZE;
		rows--;
	}
	
	row = con.display;
	for (i=0; i<rows; i++, y-=FONT_SIZE, row--)
	{
		if (row < 0)
			break;
		if (con.current - row >= con.totallines)
			break;		// past scrollback wrap point
			
		text = con.text + (row % con.totallines)*con.linewidth;

		Com_sprintf (output, sizeof(output), "");
		for (x=0; x<con.linewidth; x++) {
		//	Com_sprintf (output, sizeof(output), "%s%c", output, text[x]);
			addch[0] = text[x];
			addch[1] = '\0';
			Q_strncatz (output, addch, sizeof(output));
		}
		Con_DrawString ((int)conLeft + 4, y, output, 255);
	}

	// ZOID- draw the download bar
#ifdef USE_CURL	// HTTP downloading from R1Q2
	if ( cls.downloadname[0] && (cls.download || cls.downloadposition) )
#else
	if (cls.download)
#endif	// USE_CURL
	{
		Con_DrawDownloadProgress (conLeft, conWidth, barheight, stringLen((const char *)&version), red, green, blue);
	}
//ZOID

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();
}


