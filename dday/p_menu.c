/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_menu.c,v $
 *   $Revision: 1.6 $
 *   $Date: 2002/06/04 19:49:49 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

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

#include "g_local.h"

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (!ent->client)
		return;
	
	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	ent->client->chasetarget = NULL;

	if (ent->client->menu) {
		gi.dprintf("warning, ent already has a menu\n");
		PMenu_Close(ent);
	}

	hnd = gi.TagMalloc(sizeof(*hnd), TAG_TEMP);

	hnd->entries = entries;
	hnd->num = num;

	if (cur < 0 || !entries[cur].SelectFunc) {
		for (i = 0, p = entries; i < num; i++, p++)
			if (p->SelectFunc)
				break;
	} else
		i = cur;

	if (i >= num)
		hnd->cur = -1;
	else
		hnd->cur = i;

	ent->client->layout_type = SHOW_SCORES;

	ent->client->inmenu = true;
	ent->client->menu = hnd;

	PMenu_Update(ent);
	gi.unicast (ent, true);
}

void PMenu_Close(edict_t *ent)
{

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	//pbowens: this fixes the 'no-hud' bug
	memset (&ent->client->menu_cur, 0, sizeof(ent->client->menu_cur));


	if (!ent->client->menu)
		return;


	gi.TagFree(ent->client->menu);
	ent->client->menu = NULL;


	if (ent->client->resp.mos == MEDIC)
		ent->client->layout_type = SHOW_MEDIC_SCREEN;
	else
		ent->client->layout_type = SHOW_NONE;


}

void PMenu_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;


	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

//	strncpy (string, "xv 32 yv 8 picn inventory ");
	Q_strncpyz (string, sizeof(string), "xv 32 yv 8 picn inventory ");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text) || *p->text == '^')
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
//		sprintf (string + strlen(string), "yv %d ", 32 + i * 8);
		Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - (int)strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - (int)strlen(t)*8);
		else
			x = 64;

	//	sprintf (string + strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));
		Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
		//	sprintf (string + strlen(string), "string2 \"\x0d%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
		//	sprintf (string + strlen(string), "string2 \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), "string2 \"%s\" ", t);
		else
		//	sprintf (string + strlen(string), "string \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void PMenu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;


	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		i++, p++;
		if (i == hnd->num)
			i = 0, p = hnd->entries;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
	gi.unicast (ent, true);
}

void PMenu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		if (i == 0) {
			i = hnd->num - 1;
			p = hnd->entries + i;
		} else
			i--, p--;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
	gi.unicast (ent, true);
}

void PMenu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;

	if (p->SelectFunc)
		p->SelectFunc(ent, p, hnd->cur);
}
