/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

This file is part of Lazarus Quake 2 Mod source code.

Lazarus Quake 2 Mod source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Lazarus Quake 2 Mod source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lazarus Quake 2 Mod source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"

// Note that the pmenu entries are duplicated
// this is so that a static set of pmenu entries can be used
// for multiple clients and changed without interference
// note that arg will be freed when the menu is closed, it must be allocated memory
pmenuhnd_t *PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, void *arg)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (!ent->client)
		return NULL;

	if (ent->client->menu) {
		gi.dprintf("warning, ent already has a menu\n");
		PMenu_Close(ent);
	}

	hnd = gi.TagMalloc(sizeof(*hnd), TAG_LEVEL);

	hnd->arg = arg;
	hnd->entries = entries;
	memcpy(hnd->entries, entries, sizeof(pmenu_t) * num);
	// duplicate the strings since they may be from static memory
	for (i = 0; i < num; i++)
		if (entries[i].text)
			hnd->entries[i].text = strdup(entries[i].text);

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

	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->menu = hnd;

	PMenu_Update(ent);
	gi.unicast (ent, true);

	return hnd; // Knightmare added
}

void PMenu_Close(edict_t *ent)
{
	if (!ent->client->menu)
		return;

	gi.TagFree(ent->client->menu);
	ent->client->menu = NULL;
	ent->client->showscores = false;
}

// only use on pmenu's that have been called with PMenu_Open
void PMenu_UpdateEntry(pmenu_t *entry, const char *text, int align, SelectFunc_t SelectFunc)
{
	if (entry->text)
		free(entry->text);
	entry->text = strdup(text);
	entry->align = align;
	entry->SelectFunc = SelectFunc;
}


void PMenu_Do_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

//	strncpy(string, "xv 32 yv 8 picn inventory ");
	Q_strncpyz(string, "xv 32 yv 8 picn inventory ", sizeof(string));

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
	//	sprintf(string + strlen(string), "yv %d ", 32 + i * 8);
		Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - (int)strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - (int)strlen(t)*8);
		else
			x = 64;

	//	sprintf(string + strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));
		Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
		//	sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
		//	sprintf(string + strlen(string), "string2 \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string2 \"%s\" ", t);
		else
		//	sprintf(string + strlen(string), "string \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*void PMenu_Update(edict_t *ent)
{
	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	if (level.time - ent->client->menutime >= 1.0) {
		// been a second or more since last update, update now
		PMenu_Do_Update(ent);
		gi.unicast (ent, true);
		ent->client->menutime = level.time;
		ent->client->menudirty = false;
	}
	ent->client->menutime = level.time + 0.2;
	ent->client->menudirty = true;
}*/

void PMenu_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

//	strncpy(string, "xv 32 yv 8 picn inventory ");
	Q_strncpyz (string, "xv 32 yv 8 picn inventory ", sizeof(string));

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
	//	sprintf(string + strlen(string), "yv %d ", 32 + i * 8);
		Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - (int)strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - (int)strlen(t)*8);
		else
			x = 64;

	//	sprintf(string + strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));
		Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
		//	sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
		//	sprintf(string + strlen(string), "string2 \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string2 \"%s\" ", t);
		else
		//	sprintf(string + strlen(string), "string \"%s\" ", t);
			Com_sprintf (string + strlen(string), sizeof(string)-strlen(string), "string \"%s\" ", t);
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
		p->SelectFunc(ent, hnd);
}

