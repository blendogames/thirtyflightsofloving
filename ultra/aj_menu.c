/* the menu thingy */
#include "g_local.h"

void lithium_observer(edict_t *ent, pmenu_t *menu);
void camera_on(edict_t *ent, pmenu_t *menu);
void ultra_open_creditsmenu(edict_t *ent, pmenu_t *menu);
void ultra_open_dmsettingsmenu(edict_t *ent, pmenu_t *menu);
void ultra_open_banningmenu(edict_t *ent, pmenu_t *menu);
void ultra_open_featuresmenu(edict_t *ent, pmenu_t *menu);
void ultra_open_votingmenu(edict_t *ent, pmenu_t *menu);
void ultra_open_creditsmenu(edict_t *ent, pmenu_t *menu);

pmenu_t ultra_menu[] = {
	{ "*Eraser ULTRA v0.48a",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Main Menu",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Mod Info",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Credits",					PMENU_ALIGN_LEFT, NULL, ultra_open_creditsmenu },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Observer",					PMENU_ALIGN_LEFT, NULL, lithium_observer },
	{ "Chase Camera",				PMENU_ALIGN_LEFT, NULL, camera_on },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
//	{ "Voting Menu",				PMENU_ALIGN_LEFT, NULL, ultra_open_votingmenu },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
};


pmenu_t ultra_creditsmenu[] = {
	{ "*Eraser Ultra v0.48a",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "~~~~~~~~~~~~~~~~~",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Eraser: Ryan Feltrin",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ULTRA: Anthony Jacques",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "MapMod: Jeremy Mappus",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Cam: Paul Jordan",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Client Emul: SABIN Team",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "File compression: Zlib",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Quake2: ID Software",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "CTF: ThreeWave",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",			PMENU_ALIGN_LEFT, NULL, NULL },
};


pmenu_t ultra_votingmenu[] = {
	{ "*Voting Menu",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Map Voting",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Features",					PMENU_ALIGN_LEFT, NULL, ultra_open_featuresmenu },
	{ "Weapon Banning",				PMENU_ALIGN_LEFT, NULL, ultra_open_banningmenu },
	{ "Deathmatch settings",		PMENU_ALIGN_LEFT, NULL, ultra_open_dmsettingsmenu },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Current Vote:",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*no_bfg",					PMENU_ALIGN_RIGHT, NULL, NULL },
};


pmenu_t ultra_featuresmenu[] = {
	{ "*Voting - features",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Use runes/techs",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use offhand grapple",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use Safety Mode",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Current Vote:",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*use_runes",					PMENU_ALIGN_RIGHT, NULL, NULL },
};


pmenu_t ultra_banningmenu[] = {
	{ "*Voting - Banning",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "No Blaster",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Shotgun",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Machinegun",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Super Shotgun",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Chaingun",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Grenade Launcher",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Rocket Launcher",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Hyperblaster",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Railcannon",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No BFG",						PMENU_ALIGN_LEFT, NULL, NULL },
	{ "No Quad",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Current Vote:",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*no_bfg",					PMENU_ALIGN_RIGHT, NULL, NULL },
};


pmenu_t ultra_dmsettingsmenu[] = {
	{ "*Voting - DM Settings",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Fraglimit",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Timelimit",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Teamplay",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Current Vote:",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*teamplay",					PMENU_ALIGN_RIGHT, NULL, NULL },
};

void ultra_openmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_menu, 7, sizeof(ultra_menu) / sizeof(pmenu_t));
}

void ultra_open_creditsmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_creditsmenu, 5, sizeof(ultra_creditsmenu) / sizeof(pmenu_t));
}

void ultra_open_votingmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_votingmenu, 5, sizeof(ultra_votingmenu) / sizeof(pmenu_t));
}

void ultra_open_featuresmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_featuresmenu, 5, sizeof(ultra_featuresmenu) / sizeof(pmenu_t));
}

void ultra_open_banningmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_banningmenu, 5, sizeof(ultra_banningmenu) / sizeof(pmenu_t));
}

void ultra_open_dmsettingsmenu(edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	PMenu_Open(ent, ultra_dmsettingsmenu, 5, sizeof(ultra_dmsettingsmenu) / sizeof(pmenu_t));
}


