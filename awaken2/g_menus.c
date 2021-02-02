// g_menus.c

//CW++
#include "g_local.h"

void GMenu_Main(edict_t *ent, pmenuhnd_t *p);
void GMenu_Play(edict_t *ent, pmenuhnd_t *p);
void GMenu_Observer(edict_t *ent, pmenuhnd_t *p);
void GMenu_VoteSettings(edict_t *ent, pmenuhnd_t *p);
void GMenu_JoinTeam1(edict_t *ent, pmenuhnd_t *p);
void GMenu_JoinTeam2(edict_t *ent, pmenuhnd_t *p);

void GMenu_Help_Weapons(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_ClientCmds1(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_ClientCmds2(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_ClientCmds3(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_OpCmds1(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_OpCmds2(edict_t *ent, pmenuhnd_t *p);
void GMenu_Help_OpCmds3(edict_t *ent, pmenuhnd_t *p);

void GMenu_Playerlist(edict_t *ent, pmenuhnd_t *p);
void PlayerList_Update(edict_t *ent, pmenuhnd_t *pdisplay);

void GMenu_VoteSettings(edict_t *ent, pmenuhnd_t *p);
void Vote_SettingsApply(edict_t *ent, pmenuhnd_t *p);
void Vote_SettingsUpdate(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_ChangeMap(edict_t *ent, pmenuhnd_t *p);

void GMenu_VoteWeapons(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_WeaponsApply(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_WeaponsUpdate(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_ChangeGaussPistol(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeMac10(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeJackhammer(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeC4(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeTraps(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeESG(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeRocketLauncher(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeFlamethrower(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeRailgun(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeShockRifle(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeAGM(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeDiscLauncher(edict_t *ent, pmenuhnd_t *p);

void GMenu_VotePowerups(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_PowerupsApply(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_PowerupsUpdate(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_ChangeInvuln(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeQuad(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeSiphon(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeD89(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeHaste(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeTele(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeAntibeam(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeEnviro(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeSilencer(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeBreather(edict_t *ent, pmenuhnd_t *p);

void GMenu_VoteDMFlags(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_DMFlagsApply(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_DMFlagsUpdate(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_ChangeNoHealth(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeNoItems(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeWeaponsStay(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeNoFalling(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeNoArmor(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeInfiniteAmmo(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeQuadDrop(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeFastSwitch(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeExtraItems(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeNoReplacements(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeCTFNoTechs(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeCTFSpawnsOnly(edict_t *ent, pmenuhnd_t *p);

void GMenu_VoteBots(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_BotsApply(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_BotsUpdate(edict_t *ent, pmenuhnd_t *setmenu);
void Vote_ChangeNumBotsAdded(edict_t *ent, pmenuhnd_t *p);
void Vote_ChangeNumBotsRemoved(edict_t *ent, pmenuhnd_t *p);
void Vote_RemoveAllBots(edict_t *ent, pmenuhnd_t *p);

void OpMenu_Main(edict_t *ent, pmenuhnd_t *p);
void OpMenu_Kick(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_Ban(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_Mute(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_Unmute(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_SwapTeam(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_Vote(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_VoteYes(edict_t *ent, pmenuhnd_t *hmenu);
void OpMenu_VoteNo(edict_t *ent, pmenuhnd_t *hmenu);
void Op_MenuUpdate(edict_t *ent, pmenuhnd_t *hmenu);
void Op_ChangePlayerNum(edict_t *ent, pmenuhnd_t *p);

static const int gmenu_game = 1;
static const int gmenu_level = 2;
static const int gmenu_match = 3;
static const int gmenu_vote = 7;
static const int gmenu_team = 9;

static const int votemenu_bots = 15;

static const int plist_next = 15;
static const int plist_opreturn = 16;

static const int opmenu_pnum = 5;
static const int opmenu_start = 16;


pmenu_t gamemenu[] = {					// In-game menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Gametype : FFA",				PMENU_ALIGN_CENTER, NULL },			// gametype
	{ NULL,								PMENU_ALIGN_CENTER, NULL },			// level
	{ NULL,								PMENU_ALIGN_CENTER, NULL },			// match status
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "Return to Play",					PMENU_ALIGN_LEFT, GMenu_Play },
	{ "Enter OBSERVER Mode",			PMENU_ALIGN_LEFT, GMenu_Observer },
	{ "Voting Menu",					PMENU_ALIGN_LEFT, GMenu_VoteSettings },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// join other team
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Player List",					PMENU_ALIGN_LEFT, GMenu_Playerlist },
	{ "Message Of The Day",				PMENU_ALIGN_LEFT, ShowMOTD },
	{ "Help",							PMENU_ALIGN_LEFT, GMenu_Help },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] to move cursor",		PMENU_ALIGN_LEFT, NULL },
	{ "ENTER to select",				PMENU_ALIGN_LEFT, NULL },
	{ "v" AWK_STRING_VERSION,			PMENU_ALIGN_RIGHT, NULL }
};

pmenu_t votemenu[] = {					// Top-level voting menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Voting Menu: Game Settings",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeMap },	// next map
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// execute config file
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// match (on/off)
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// timelimit
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// fraglimit
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// CTF: capture limit
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// hook (on/off)
	{ NULL,								PMENU_ALIGN_LEFT, NULL },			// off-hand hook (on/off)
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Weapon Toggles...",				PMENU_ALIGN_LEFT, GMenu_VoteWeapons },
	{ "Powerup Toggles...",				PMENU_ALIGN_LEFT, GMenu_VotePowerups },
	{ "DM Flags...",					PMENU_ALIGN_LEFT, GMenu_VoteDMFlags },
	{ "AwakenBots...",					PMENU_ALIGN_LEFT, GMenu_VoteBots },
	{ "Propose Change",					PMENU_ALIGN_LEFT, Vote_SettingsApply },
	{ "Return To Main Menu",			PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t weaponsmenu[] = {				// Weapons voting menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Voting Menu: Weapon Toggles",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeGaussPistol },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeMac10 },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeJackhammer },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeC4 },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeTraps },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeESG },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeRocketLauncher },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeFlamethrower },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeRailgun },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeShockRifle },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeAGM },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeDiscLauncher },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Propose Change",					PMENU_ALIGN_LEFT, Vote_SettingsApply },
	{ "Return To Main Menu",			PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t powerupsmenu[] = {				// Powerup voting menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Voting Menu: Powerup Toggles",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeInvuln },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeQuad },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeSiphon },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeD89 },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeHaste },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeTele },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeAntibeam },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeEnviro },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeSilencer },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeBreather },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Propose Change",					PMENU_ALIGN_LEFT, Vote_SettingsApply },
	{ "Return To Main Menu",			PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t dmflagsmenu[] = {				// DM flags voting menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Voting Menu: DM Flags",			PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNoHealth },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNoItems },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeWeaponsStay },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNoFalling },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNoArmor },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeInfiniteAmmo },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeQuadDrop },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeFastSwitch },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeExtraItems },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNoReplacements },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeCTFNoTechs },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeCTFSpawnsOnly },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Propose Change",					PMENU_ALIGN_LEFT, Vote_SettingsApply },
	{ "Return To Main Menu",			PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t botsmenu[] = {					// AwakenBots configuration menu
	{ "*- AWAKENING II -",				PMENU_ALIGN_CENTER, NULL },
	{ "*Voting Menu: AwakenBots",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNumBotsAdded },
	{ NULL,								PMENU_ALIGN_LEFT, Vote_ChangeNumBotsRemoved },
	{ "Remove All Bots",				PMENU_ALIGN_LEFT, Vote_RemoveAllBots },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Propose Change",					PMENU_ALIGN_LEFT, Vote_SettingsApply },
	{ "Return To Main Menu",			PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t opmenu[] = {				// Operator menu
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Operator Commands",			PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Player List",				PMENU_ALIGN_LEFT, GMenu_Playerlist },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, Op_ChangePlayerNum },	// selected player num
	{ "Kick Player",				PMENU_ALIGN_LEFT, OpMenu_Kick },
	{ "Ban Player",					PMENU_ALIGN_LEFT, OpMenu_Ban },
	{ "Mute Player",				PMENU_ALIGN_LEFT, OpMenu_Mute },
	{ "Unmute Player",				PMENU_ALIGN_LEFT, OpMenu_Unmute },
	{ "Swap Player to Other Team",	PMENU_ALIGN_LEFT, OpMenu_SwapTeam },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Operator Voting Menu...",	PMENU_ALIGN_LEFT, OpMenu_Vote },
	{ "Force Vote YES",				PMENU_ALIGN_LEFT, OpMenu_VoteYes },
	{ "Force Vote NO",				PMENU_ALIGN_LEFT, OpMenu_VoteNo },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },				// start match
	{ NULL,							PMENU_ALIGN_LEFT, NULL }
};

pmenu_t playerlist[] = {			// Playerlist display
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Player List",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Game Menu",					PMENU_ALIGN_LEFT, GMenu_Main },			// game menu OR next page
	{ NULL,							PMENU_ALIGN_LEFT, NULL },				// return to operator menu (if Op'd)
	{ "(ESC to close window)",		PMENU_ALIGN_LEFT, NULL }
};

pmenu_t motd[] = {					// Message-of-the-day display
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "Main Menu",					PMENU_ALIGN_LEFT, GMenu_Main }
};

pmenu_t help_menu[] = {				// Top-level help menu
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help",						PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Weapon Binds",				PMENU_ALIGN_LEFT, GMenu_Help_Weapons },
	{ "Client Commands",			PMENU_ALIGN_LEFT, GMenu_Help_ClientCmds1 },
	{ "Operator Commands",			PMENU_ALIGN_LEFT, GMenu_Help_OpCmds1 },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "(ESC to close window)",		PMENU_ALIGN_LEFT, NULL }
};

pmenu_t help_weapons[] = {			// Weapon commands help menu
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Weapon Binds",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*use AG Manipulator",		PMENU_ALIGN_LEFT, NULL },
	{ "Activate the AGM / CD",		PMENU_ALIGN_LEFT, NULL },
	{ "*+push",						PMENU_ALIGN_LEFT, NULL },
	{ "Move AGM victim away",		PMENU_ALIGN_LEFT, NULL },
	{ "*+pull",						PMENU_ALIGN_LEFT, NULL },
	{ "Bring AGM victim closer",	PMENU_ALIGN_LEFT, NULL },
	{ "*fling",						PMENU_ALIGN_LEFT, NULL },
	{ "Throw AGM victim",			PMENU_ALIGN_LEFT, NULL },
	{ "*use Grapple",				PMENU_ALIGN_LEFT, NULL },
	{ "Activate standard Grapple",	PMENU_ALIGN_LEFT, NULL },
	{ "*+hook",						PMENU_ALIGN_LEFT, NULL },
	{ "Fire offhand Grapple",		PMENU_ALIGN_LEFT, NULL },
	{ "*+use",						PMENU_ALIGN_LEFT, NULL },
	{ "Use Teleporter or Terminal",	PMENU_ALIGN_LEFT, NULL },
	{ "(ESC to close window)",		PMENU_ALIGN_LEFT, NULL }
};

pmenu_t help_clientcmds1[] = {		// Client commands help menu (page 1)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Client Commands",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*menu",						PMENU_ALIGN_LEFT, NULL },
	{ "Display the main game menu",	PMENU_ALIGN_LEFT, NULL },
	{ "*observer",					PMENU_ALIGN_LEFT, NULL },
	{ "Enter Observer mode",		PMENU_ALIGN_LEFT, NULL },
	{ "*vote",						PMENU_ALIGN_LEFT, NULL },
	{ "Display the voting menu",	PMENU_ALIGN_LEFT, NULL },
	{ "*yes / no",					PMENU_ALIGN_LEFT, NULL },
	{ "Agree/disagree with vote",	PMENU_ALIGN_LEFT, NULL },
	{ "*ready / notready",			PMENU_ALIGN_LEFT, NULL },
	{ "Readyness state for match",	PMENU_ALIGN_LEFT, NULL },
	{ "*team <name>",				PMENU_ALIGN_LEFT, NULL },
	{ "Switch to specified team",	PMENU_ALIGN_LEFT, NULL },
	{ "*id",						PMENU_ALIGN_LEFT, NULL },
	{ "Toggle player I.D.",			PMENU_ALIGN_LEFT, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, GMenu_Help_ClientCmds2 }
};

pmenu_t help_clientcmds2[] = {		// Client commands help menu (page 2)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Client Commands",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*mission",					PMENU_ALIGN_LEFT, NULL },
	{ "Display Assault mission",	PMENU_ALIGN_LEFT, NULL },
	{ "*play_voice <wav>",			PMENU_ALIGN_LEFT, NULL },
	{ "Heard by nearby players",	PMENU_ALIGN_LEFT, NULL },
	{ "*play_team <wav>",			PMENU_ALIGN_LEFT, NULL },
	{ "Heard by team-mates only",	PMENU_ALIGN_LEFT, NULL },
	{ "*ghost <code>",				PMENU_ALIGN_LEFT, NULL },
	{ "Code for resuming a match",	PMENU_ALIGN_LEFT, NULL },
	{ "*stats",						PMENU_ALIGN_LEFT, NULL },
	{ "Display player statistics",	PMENU_ALIGN_LEFT, NULL },
	{ "*playerlist",				PMENU_ALIGN_LEFT, NULL },
	{ "List details of players",	PMENU_ALIGN_LEFT, NULL },
	{ "*maplist",					PMENU_ALIGN_LEFT, NULL },
	{ "List the available maps",	PMENU_ALIGN_LEFT, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, GMenu_Help_ClientCmds3 }
};

pmenu_t help_clientcmds3[] = {		// Client commands help menu (page 3)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Client Commands",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*idtrap",					PMENU_ALIGN_LEFT, NULL },
	{ "Toggle C4/Trap I.D.",		PMENU_ALIGN_LEFT, NULL },
	{ "*weap_note <mode>",			PMENU_ALIGN_LEFT, NULL },
	{ "Weapon choice notification",	PMENU_ALIGN_LEFT, NULL },
	{ "*addbots <number>",			PMENU_ALIGN_LEFT, NULL },
	{ "Add AwakenBots to game",		PMENU_ALIGN_LEFT, NULL },
	{ "*clearbots <number>",		PMENU_ALIGN_LEFT, NULL },
	{ "Remove AwakenBots from game",PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "(ESC to close window)",		PMENU_ALIGN_LEFT, NULL }
};

pmenu_t help_opcmds1[] = {			// Operator commands help menu (page 1)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Operator Commands",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*op <op_password>",			PMENU_ALIGN_LEFT, NULL },
	{ "Request Operator status",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_menu",					PMENU_ALIGN_LEFT, NULL },
	{ "Display Operator menu",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_kick <playernum>",		PMENU_ALIGN_LEFT, NULL },
	{ "Kick specified player",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_ban <playernum>",		PMENU_ALIGN_LEFT, NULL },
	{ "Ban specified player",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_mute <playernum>",		PMENU_ALIGN_LEFT, NULL },
	{ "Mute specified player",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_unmute <playernum>",		PMENU_ALIGN_LEFT, NULL },
	{ "Unmute specified player",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_swapteam <playernum>",	PMENU_ALIGN_LEFT, NULL },
	{ "Move player to other team",	PMENU_ALIGN_LEFT, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, GMenu_Help_OpCmds2 }
};

pmenu_t help_opcmds2[] = {			// Operator commands help menu (page 2)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Operator Commands",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*op_iplist",					PMENU_ALIGN_LEFT, NULL },
	{ "Display player IP list",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_vote",					PMENU_ALIGN_LEFT, NULL },
	{ "Display Operator vote menu",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_yes",					PMENU_ALIGN_LEFT, NULL },
	{ "Force vote to be passed",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_no",						PMENU_ALIGN_LEFT, NULL },
	{ "Force vote to be dropped",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_start",					PMENU_ALIGN_LEFT, NULL },
	{ "Force match to start",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_dropagm",				PMENU_ALIGN_LEFT, NULL },
	{ "Write coords to AGM file",	PMENU_ALIGN_LEFT, NULL },
	{ "*op_slots",					PMENU_ALIGN_LEFT, NULL },
	{ "List client slots in use",	PMENU_ALIGN_LEFT, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, GMenu_Help_OpCmds3 }
};

pmenu_t help_opcmds3[] = {			// Operator commands help menu (page 3)
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Help: Operator Commands",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "*op_addbots <number>",		PMENU_ALIGN_LEFT, NULL },
	{ "Add AwakenBots to game",		PMENU_ALIGN_LEFT, NULL },
	{ "*op_clearbots <number>",		PMENU_ALIGN_LEFT, NULL },
	{ "Remove AwakenBots from game",PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "(ESC to close window)",		PMENU_ALIGN_LEFT, NULL }
};

/*
===============================
Voting Menu functions
===============================
*/
void ToggleItemState(char *item_name, int spawn)
{
	edict_t	*e;
	int		i;

//	Search through global entity list for the named weapons; derezz or spawn them as required.

	for (i = 0; i < globals.num_edicts; ++i)
	{
		e = &g_edicts[i];
		if (!e->inuse)
			continue;

		if (e->item && !Q_stricmp(e->item->pickup_name, item_name))
		{
			if (spawn)
			{
				e->solid = SOLID_TRIGGER;
				if (e->svflags & SVF_NOCLIENT)
					e->svflags &= ~SVF_NOCLIENT;

				gi.linkentity(e);
			}
			else
			{
				e->svflags |= SVF_NOCLIENT;
				e->solid = SOLID_NOT;
				e->nextthink = 0.0;
				if (e->flags & FL_RESPAWN)
					e->flags &= ~FL_RESPAWN;

				gi.linkentity(e);
			}
		}

//		Remove the item from any players that have one. If it's a weapon, force
//		players that are wielding it to switch to another one.

		if (!e->client)
			continue;
		if (e->health < 1)
			continue;

		if (!spawn)
		{
			e->client->pers.inventory[ITEM_INDEX(FindItem(item_name))] = 0;
			if (!Q_stricmp(e->client->pers.weapon->pickup_name, item_name))
				NoAmmoWeaponChange(e);
		}
	}
}

void G_VoteWin(qboolean op_forced)
{
	vote_t	votetype = g_vote.vote;
	gitem_t	*item;
	edict_t	*e;
	char	text[80];
	char	st[80];
	int		i;

	gi_bprintf(PRINT_HIGH, "%s\n", g_vote.vmsg);
	if (!op_forced)
		gi_bprintf(PRINT_CHAT, "--> VOTE PASSED!\n");

	g_vote.vote = VOTE_NONE;

	switch (votetype)
	{

//		Game settings...

		case VOTE_NONE:
			return;

		case VOTE_MAP:
			strncpy(level.forcemap, g_vote.vstring, sizeof(level.forcemap) - 1);
			EndDMLevel();
			break;

		case VOTE_CONFIG:
			Com_sprintf(text, sizeof(text), "exec %s", g_vote.vstring);
			gi.AddCommandString(text);
			break;

		case VOTE_MATCH:
			if (g_vote.vnum)
			{
				gi.cvar_set("competition", "2");
				teamgame.match = MATCH_SETUP;
				CTFResetAllPlayers();
				RemoveNumBots_Safe(MAXBOTS);
			}
			else
			{
				gi.cvar_set("competition", "0");
				teamgame.match = MATCH_NONE;
			}
			gi_bprintf(PRINT_HIGH, "Match mode is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_TIMELIMIT:
			gi.cvar_set("timelimit", va("%d", g_vote.vnum));
			break;

		case VOTE_FRAGLIMIT:
			gi.cvar_set("fraglimit", va("%d", g_vote.vnum));
			break;

		case VOTE_CAPLIMIT:
			gi.cvar_set("capturelimit", va("%d", g_vote.vnum));
			break;

		case VOTE_GRAPPLE:
			gi.cvar_forceset("allow_hook", va("%d", g_vote.vnum));
			item = FindItem("Grapple");
			if ((int)sv_hook_offhand->value)
			{
				item->use = NULL;
				item->view_model = '\0';
				item->icon = '\0';
			}
			else
			{
				item->use = Use_Weapon;
				item->view_model = "models/weapons/grapple/tris.md2";
				item->icon = "w_grapple";
			}

			for (i = 1; i <= (int)maxclients->value; ++i)	// giveth or taketh away players' grapples
			{												// depending on what the vote was
				e = g_edicts + i;
				if (!e->client)
					continue;
				if (!e->inuse)
					continue;

				if (g_vote.vnum)
				{
					e->client->pers.inventory[ITEM_INDEX(item)] = 1;
					if ((int)sv_hook_offhand->value)
						e->client->hookstate = WEAPON_READY;
				}
				else
				{
					if (e->client->ctf_grapple)
						CTFResetGrapple(e->client->ctf_grapple);

					if (e->health < 1)
						continue;

					if (e->client->pers.weapon->weapmodel == WEAP_GRAPPLE)
						NoAmmoWeaponChange(e);

					e->client->pers.inventory[ITEM_INDEX(item)] = 0;
				}
			}
			gi_bprintf(PRINT_HIGH, "Grapple changed to %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_GRAPPLE_OFFHAND:
			gi.cvar_forceset("hook_offhand", va("%d", g_vote.vnum));
			item = FindItem("Grapple");
			if (g_vote.vnum)
			{
				item->use = NULL;
				item->view_model = '\0';
				item->icon = '\0';
			}
			else
			{
				item->use = Use_Weapon;
				item->view_model = "models/weapons/grapple/tris.md2";
				item->icon = "w_grapple";
			}

			for (i = 1; i <= (int)maxclients->value; ++i)
			{
				e = g_edicts + i;
				if (!e->client)
					continue;
				if (!e->inuse)
					continue;
				
				if (e->client->ctf_grapple)
					CTFResetGrapple(e->client->ctf_grapple);

				if (e->health < 1)
					continue;

				if (!Q_stricmp(e->client->pers.weapon->pickup_name, "Grapple"))
					NoAmmoWeaponChange(e);
			}

			gi_bprintf(PRINT_HIGH, "Grapple is now %s.\n", (g_vote.vnum)?"OFF-HAND":"NOT OFF-HAND");
			break;

//		Weapon settings...

		case VOTE_GAUSSPISTOL:
			gi.cvar_forceset("allow_gausspistol", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Gauss Pistol is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_MAC10:
			gi.cvar_forceset("allow_mac10", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Mac-10 is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_JACKHAMMER:
			gi.cvar_forceset("allow_jackhammer", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Jackhammer is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_C4:
			gi.cvar_forceset("allow_c4", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "C4 Explosives are %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_TRAPS:
			gi.cvar_forceset("allow_traps", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Traps are %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_ESG:
			gi.cvar_forceset("allow_spikegun", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Explosive Spike Gun is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_ROCKETLAUNCHER:
			gi.cvar_forceset("allow_rocketlauncher", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Rocket Launcher is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_FLAMETHROWER:
			gi.cvar_forceset("allow_flamethrower", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Flamethrower is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_RAILGUN:
			gi.cvar_forceset("allow_railgun", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Railgun is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_SHOCKRIFLE:
			gi.cvar_forceset("allow_shockrifle", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Shock Rifle is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_AGM:
			gi.cvar_forceset("allow_agm", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "AG Manipulator is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_DISCLAUNCHER:
			gi.cvar_forceset("allow_disclauncher", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Disc Launcher is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

//		Powerup settings...

		case VOTE_INVULN:
			gi.cvar_forceset("allow_invuln", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Invulnerability is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_QUAD:
			gi.cvar_forceset("allow_quad", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Quad Damage is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_SIPHON:
			gi.cvar_forceset("allow_siphon", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "The Awakening is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_D89:
			gi.cvar_forceset("allow_d89", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "D89 is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_HASTE:
			gi.cvar_forceset("allow_haste", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Haste is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_TELE:
			gi.cvar_forceset("allow_tele", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Personal Teleporter is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_ANTIBEAM:
			gi.cvar_forceset("allow_antibeam", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Beam Reflector is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_ENVIRO:
			gi.cvar_forceset("allow_enviro", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Environment Suit is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_SILENCER:
			gi.cvar_forceset("allow_silencer", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Silencer is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_BREATHER:
			gi.cvar_forceset("allow_breather", va("%d", g_vote.vnum));
			gi_bprintf(PRINT_HIGH, "Rebreather is %s (next map).\n", (g_vote.vnum)?"ON":"OFF");
			break;

//		dmflags settings...

		case VOTE_NOHEALTH:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_NO_HEALTH;
			else
				i &= ~DF_NO_HEALTH;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No Health dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_NOITEMS:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_NO_ITEMS;
			else
				i &= ~DF_NO_ITEMS;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No Items dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_WEAPONSSTAY:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_WEAPONS_STAY;
			else
				i &= ~DF_WEAPONS_STAY;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "Weapons Stay dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_NOFALLING:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_NO_FALLING;
			else
				i &= ~DF_NO_FALLING;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No Falling Damage dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_NOARMOR:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_NO_ARMOR;
			else
				i &= ~DF_NO_ARMOR;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No Armor dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_INFINITEAMMO:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_INFINITE_AMMO;
			else
				i &= ~DF_INFINITE_AMMO;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "Infinite Ammo dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_QUADDROP:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_QUAD_DROP;
			else
				i &= ~DF_QUAD_DROP;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "Quad Drop dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_FASTSWITCH:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_FAST_SWITCH;
			else
				i &= ~DF_FAST_SWITCH;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "Fast Weapon Switching dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_EXTRAITEMS:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_EXTRA_ITEMS;
			else
				i &= ~DF_EXTRA_ITEMS;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "Extra Items dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_NOREPLACEMENTS:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_NO_REPLACEMENTS;
			else
				i &= ~DF_NO_REPLACEMENTS;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No Replacements dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_NOTECHS:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_CTF_NO_TECH;
			else
				i &= ~DF_CTF_NO_TECH;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "No CTF Techs dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

		case VOTE_CTFSPAWNS:
			i = (int)dmflags->value;
			if (g_vote.vnum)
				i |= DF_CTF_SPAWNS_ONLY;
			else
				i &= ~DF_CTF_SPAWNS_ONLY;
			Com_sprintf(st, sizeof(st), "%d", i);
			gi.cvar_set("dmflags", st);
			gi_bprintf(PRINT_HIGH, "CTF Spawns Only dmflag is now %s.\n", (g_vote.vnum)?"ON":"OFF");
			break;

//		AwakenBot settings...

		case VOTE_ADDBOTS:
			SpawnNumBots_Safe(g_vote.vnum);
			gi_bprintf(PRINT_HIGH, "Added %d AwakenBots to the game.\n", g_vote.vnum);
			break;

		case VOTE_REMOVEBOTS:
			RemoveNumBots_Safe(g_vote.vnum);
			gi_bprintf(PRINT_HIGH, "Removed %d AwakenBots from the game.\n", g_vote.vnum);
			break;

		case VOTE_REMOVEALLBOTS:
			RemoveNumBots_Safe(MAXBOTS);
			gi_bprintf(PRINT_HIGH, "Removed all AwakenBots from the game.\n");
			break;

		default:
			gi_bprintf(PRINT_HIGH, "An unknown vote type was specified.\n");
			break;
	}
}

qboolean G_VoteBegin(edict_t *ent, vote_t type, char *msg)
{
	edict_t	*e;
	int		i;
	int		count = 0;

//	Check if the call for a vote is allowed.

	if (sv_vote_percentage->value == 0.0)
	{
		gi_cprintf(ent, PRINT_HIGH, "Voting is disabled.\n");
		return false;
	}

	if (g_vote.vote != VOTE_NONE)
	{
		gi_cprintf(ent, PRINT_HIGH, "A vote is already in progress.\n");
		return false;
	}

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot request a vote during a match.\n");
		return false;
	}

	for (i = 1; i <= (int)maxclients->value; ++i)
	{
		e = g_edicts + i;
		e->client->resp.voted = false;
		if (e->inuse && !e->isabot)
			++count;
	}
	if (count < (int)sv_vote_minclients->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "Not enough players for a vote.\n");
		return false;
	}

//	Set global vote info (assume the player who requested the vote says "aye!").

	ent->client->resp.voted = true;
	g_vote.nvotes = 1;
	g_vote.vote = type;
	g_vote.needvotes = (int)ceil(count * (sv_vote_percentage->value / 100.0));
	g_vote.votetime = level.time + sv_vote_time->value;
	strncpy(g_vote.vmsg, msg, sizeof(g_vote.vmsg) - 1);

//	Tell everyone about the vote.

	gi_bprintf(PRINT_CHAT, "%s has called for a vote!\n", ent->client->pers.netname);
	gi_bprintf(PRINT_HIGH, "%s\n", g_vote.vmsg);
	gi_bprintf(PRINT_HIGH, "Type YES or NO to vote on this request.\n");
	gi_bprintf(PRINT_CHAT, "Votes needed: %d  Time left: %ds\n", g_vote.needvotes - g_vote.nvotes, (int)(g_vote.votetime - level.time));

	return true;
}

void Vote_SettingsApply(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;
	char			text[1024];
	qboolean		result = false;

	if (settings->v_type == VOTE_NONE)
	{
		gi_cprintf(ent, PRINT_HIGH, "You haven't changed anything!\n");
		return;
	}
	
//	Set the proposal message, and request the vote.

	switch (settings->v_type)
	{

//		Game settings...

		case VOTE_NONE:
			return;

		case VOTE_MAP:
			if (!Q_stricmp(settings->g_maplist[settings->map - 1], level.mapname))
			{
				gi_cprintf(ent, PRINT_HIGH, "Requested map is the current one!");
				settings->v_type = VOTE_NONE;
				return;
			}
			Com_sprintf(text, sizeof(text), "Proposal: change to level \"%s\"", settings->g_maplist[settings->map - 1]);
			break;

		case VOTE_CONFIG:
			Com_sprintf(text, sizeof(text), "Proposal: execute the config script \"%s\"", settings->g_configlist[settings->config - 1]);
			break;

		case VOTE_MATCH:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s Match mode", (settings->match)?"ON":"OFF");
			break;

		case VOTE_TIMELIMIT:
			if (settings->timelimit == (int)timelimit->value)
			{
				gi_cprintf(ent, PRINT_HIGH, "Requested timelimit is the current one!");
				settings->v_type = VOTE_NONE;
				return;
			}
			Com_sprintf(text, sizeof(text), "Proposal: change the timelimit to %d min(s)", settings->timelimit);
			break;

		case VOTE_FRAGLIMIT:
			if (settings->fraglimit == (int)fraglimit->value)
			{
				gi_cprintf(ent, PRINT_HIGH, "Requested fraglimit is the current one!");
				settings->v_type = VOTE_NONE;
				return;
			}
			Com_sprintf(text, sizeof(text), "Proposal: change the fraglimit to %d", settings->fraglimit);
			break;

		case VOTE_CAPLIMIT:
			if (settings->caplimit == (int)capturelimit->value)
			{
				gi_cprintf(ent, PRINT_HIGH, "Requested fraglimit is the current one!");
				settings->v_type = VOTE_NONE;
				return;
			}
			Com_sprintf(text, sizeof(text), "Proposal: change the capturelimit to %d", settings->caplimit);
			break;

		case VOTE_GRAPPLE:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Grapple", (settings->hook)?"ON":"OFF");
			break;

		case VOTE_GRAPPLE_OFFHAND:
			Com_sprintf(text, sizeof(text), "Proposal: the Grapple is %s", (settings->hook_offhand)?"Off-hand":"NOT Off-hand");
			break;

//		Weapon settings...

		case VOTE_GAUSSPISTOL:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Gauss Pistol", (settings->gausspistol)?"ON":"OFF");
			break;

		case VOTE_MAC10:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Mac-10", (settings->mac10)?"ON":"OFF");
			break;

		case VOTE_JACKHAMMER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Jackhammer", (settings->jackhammer)?"ON":"OFF");
			break;

		case VOTE_C4:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s C4 Explosives", (settings->c4)?"ON":"OFF");
			break;

		case VOTE_TRAPS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s Traps", (settings->traps)?"ON":"OFF");
			break;

		case VOTE_ESG:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Explosive Spike Gun", (settings->esg)?"ON":"OFF");
			break;

		case VOTE_ROCKETLAUNCHER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Rocket Launcher", (settings->rocketlauncher)?"ON":"OFF");
			break;

		case VOTE_FLAMETHROWER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Flaemthrower", (settings->flamethrower)?"ON":"OFF");
			break;

		case VOTE_RAILGUN:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Railgun", (settings->railgun)?"ON":"OFF");
			break;

		case VOTE_SHOCKRIFLE:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Shock Rifle", (settings->shockrifle)?"ON":"OFF");
			break;

		case VOTE_AGM:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the AG Manipulator", (settings->agm)?"ON":"OFF");
			break;

		case VOTE_DISCLAUNCHER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Disc Launcher", (settings->disclauncher)?"ON":"OFF");
			break;

//		Powerup settings...

		case VOTE_INVULN:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Invulnerability", (settings->invuln)?"ON":"OFF");
			break;

		case VOTE_QUAD:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Quad Damage", (settings->quad)?"ON":"OFF");
			break;

		case VOTE_SIPHON:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Awakening", (settings->siphon)?"ON":"OFF");
			break;

		case VOTE_D89:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the D89", (settings->d89)?"ON":"OFF");
			break;

		case VOTE_HASTE:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Haste", (settings->haste)?"ON":"OFF");
			break;

		case VOTE_TELE:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Personal Teleporter", (settings->tele)?"ON":"OFF");
			break;

		case VOTE_ANTIBEAM:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Beam Reflector", (settings->antibeam)?"ON":"OFF");
			break;

		case VOTE_ENVIRO:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Personal Teleporter", (settings->enviro)?"ON":"OFF");
			break;

		case VOTE_SILENCER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Personal Teleporter", (settings->silencer)?"ON":"OFF");
			break;

		case VOTE_BREATHER:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Personal Teleporter", (settings->breather)?"ON":"OFF");
			break;

//		dmflags settings...

		case VOTE_NOHEALTH:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No Health dmflag", (settings->no_health)?"ON":"OFF");
			break;

		case VOTE_NOITEMS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No Items dmflag", (settings->no_items)?"ON":"OFF");
			break;

		case VOTE_WEAPONSSTAY:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Weapons Stay dmflag", (settings->weapons_stay)?"ON":"OFF");
			break;

		case VOTE_NOFALLING:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No Falling dmflag", (settings->no_falling)?"ON":"OFF");
			break;

		case VOTE_NOARMOR:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No Armor dmflag", (settings->no_armor)?"ON":"OFF");
			break;

		case VOTE_INFINITEAMMO:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Infinite Ammo dmflag", (settings->infinite_ammo)?"ON":"OFF");
			break;

		case VOTE_QUADDROP:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Quad Drop dmflag", (settings->quad_drop)?"ON":"OFF");
			break;

		case VOTE_FASTSWITCH:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Fast Switch dmflag", (settings->fast_switch)?"ON":"OFF");
			break;

		case VOTE_EXTRAITEMS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the Extra Items dmflag", (settings->extra_items)?"ON":"OFF");
			break;

		case VOTE_NOREPLACEMENTS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No Replacements dmflag", (settings->no_replacements)?"ON":"OFF");
			break;

		case VOTE_NOTECHS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the No CTF Techs dmflag", (settings->no_techs)?"ON":"OFF");
			break;

		case VOTE_CTFSPAWNS:
			Com_sprintf(text, sizeof(text), "Proposal: switch %s the CTF Spawns Only dmflag", (settings->ctf_spawns)?"ON":"OFF");
			break;

//		AwakenBot settings...

		case VOTE_ADDBOTS:
			Com_sprintf(text, sizeof(text), "Proposal: add %d AwakenBots", settings->bots_added);
			break;

		case VOTE_REMOVEBOTS:
			Com_sprintf(text, sizeof(text), "Proposal: remove %d AwakenBots", settings->bots_removed);
			break;

		case VOTE_REMOVEALLBOTS:
			Com_sprintf(text, sizeof(text), "Proposal: remove ALL AwakenBots");
			break;

		default:
			gi_bprintf(PRINT_HIGH, "An unknown vote type was proposed.\n");
			PMenu_Close(ent);
			return;
	}

//	If the requested vote is valid, put the correct data in the g_vote structure.

	if (ent->client->op_override)
	{
		strncpy(g_vote.vmsg, text, sizeof(g_vote.vmsg) - 1);
		g_vote.vote = settings->v_type;
		result = true;
	}
	else
		result = G_VoteBegin(ent, settings->v_type, text);

	if (result)
	{
		switch (settings->v_type)
		{

//			Game settings...

			case VOTE_NONE:
				return;

			case VOTE_MAP:
				strncpy(g_vote.vstring, settings->g_maplist[settings->map - 1], sizeof(g_vote.vstring) - 1);
				break;

			case VOTE_CONFIG:
				strncpy(g_vote.vstring, settings->g_configlist[settings->config - 1], sizeof(g_vote.vstring) - 1);
				break;

			case VOTE_MATCH:
				g_vote.vnum = (int)settings->match;
				break;

			case VOTE_TIMELIMIT:
				g_vote.vnum = settings->timelimit;
				break;

			case VOTE_FRAGLIMIT:
				g_vote.vnum = settings->fraglimit;
				break;

			case VOTE_CAPLIMIT:
				g_vote.vnum = settings->caplimit;
				break;

			case VOTE_GRAPPLE:
				g_vote.vnum = (int)settings->hook;
				break;

			case VOTE_GRAPPLE_OFFHAND:
				g_vote.vnum = (int)settings->hook_offhand;
				break;

//			Weapon settings...

			case VOTE_GAUSSPISTOL:
				g_vote.vnum = (int)settings->gausspistol;
				break;

			case VOTE_MAC10:
				g_vote.vnum = (int)settings->mac10;
				break;

			case VOTE_JACKHAMMER:
				g_vote.vnum = (int)settings->jackhammer;
				break;

			case VOTE_C4:
				g_vote.vnum = (int)settings->c4;
				break;

			case VOTE_TRAPS:
				g_vote.vnum = (int)settings->traps;
				break;

			case VOTE_ESG:
				g_vote.vnum = (int)settings->esg;
				break;

			case VOTE_ROCKETLAUNCHER:
				g_vote.vnum = (int)settings->rocketlauncher;
				break;

			case VOTE_FLAMETHROWER:
				g_vote.vnum = (int)settings->flamethrower;
				break;

			case VOTE_RAILGUN:
				g_vote.vnum = (int)settings->railgun;
				break;

			case VOTE_SHOCKRIFLE:
				g_vote.vnum = (int)settings->shockrifle;
				break;

			case VOTE_AGM:
				g_vote.vnum = (int)settings->agm;
				break;

			case VOTE_DISCLAUNCHER:
				g_vote.vnum = (int)settings->disclauncher;
				break;

//			Powerup settings...

			case VOTE_INVULN:
				g_vote.vnum = (int)settings->invuln;
				break;

			case VOTE_QUAD:
				g_vote.vnum = (int)settings->quad;
				break;

			case VOTE_SIPHON:
				g_vote.vnum = (int)settings->siphon;
				break;

			case VOTE_D89:
				g_vote.vnum = (int)settings->d89;
				break;

			case VOTE_HASTE:
				g_vote.vnum = (int)settings->haste;
				break;

			case VOTE_TELE:
				g_vote.vnum = (int)settings->tele;
				break;

			case VOTE_ANTIBEAM:
				g_vote.vnum = (int)settings->antibeam;
				break;

			case VOTE_ENVIRO:
				g_vote.vnum = (int)settings->enviro;
				break;

			case VOTE_SILENCER:
				g_vote.vnum = (int)settings->silencer;
				break;

			case VOTE_BREATHER:
				g_vote.vnum = (int)settings->breather;
				break;

//			dmflags settings...

			case VOTE_NOHEALTH:
				g_vote.vnum = (int)settings->no_health;
				break;

			case VOTE_NOITEMS:
				g_vote.vnum = (int)settings->no_items;
				break;

			case VOTE_WEAPONSSTAY:
				g_vote.vnum = (int)settings->weapons_stay;
				break;

			case VOTE_NOFALLING:
				g_vote.vnum = (int)settings->no_falling;
				break;

			case VOTE_NOARMOR:
				g_vote.vnum = (int)settings->no_armor;
				break;

			case VOTE_INFINITEAMMO:
				g_vote.vnum = (int)settings->infinite_ammo;
				break;

			case VOTE_QUADDROP:
				g_vote.vnum = (int)settings->quad_drop;
				break;

			case VOTE_FASTSWITCH:
				g_vote.vnum = (int)settings->fast_switch;
				break;

			case VOTE_EXTRAITEMS:
				g_vote.vnum = (int)settings->extra_items;
				break;

			case VOTE_NOREPLACEMENTS:
				g_vote.vnum = (int)settings->no_replacements;
				break;

			case VOTE_NOTECHS:
				g_vote.vnum = (int)settings->no_techs;
				break;

			case VOTE_CTFSPAWNS:
				g_vote.vnum = (int)settings->ctf_spawns;
				break;

//			AwakenBot settings...

			case VOTE_ADDBOTS:
				g_vote.vnum = (int)settings->bots_added;
				break;

			case VOTE_REMOVEBOTS:
				g_vote.vnum = (int)settings->bots_removed;
				break;

			case VOTE_REMOVEALLBOTS:
				break;
		}

		if (ent->client->op_override)
		{
			gi_bprintf(PRINT_CHAT, "Action forced by Operator: %s\n", ent->client->pers.netname);
			ent->client->op_override = false;
			G_VoteWin(true);
		}
		else if (g_vote.nvotes >= g_vote.needvotes)
			G_VoteWin(false);
	}

	PMenu_Close(ent);
}


//------------------------------
// Addition and removal of AwakenBots.

void Vote_ChangeNumBotsAdded(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_ADDBOTS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (NumBotsInGame >= MAXBOTS)
	{
		gi_cprintf(ent, PRINT_HIGH, "There are no bots left to add!\n");
		return;
	}

	if (g_reserve_used + g_public_used >= game.maxclients)
	{
		gi_cprintf(ent, PRINT_HIGH, "There are no free client spaces left!\n");
		return;
	}

	++settings->bots_added;
	if ((settings->bots_added > game.maxclients - (g_reserve_used + g_public_used)) || (settings->bots_added > MAXBOTS - 1))
		settings->bots_added = 0;

	settings->v_type = VOTE_ADDBOTS;
	Vote_BotsUpdate(ent, p);
}

void Vote_ChangeNumBotsRemoved(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_REMOVEBOTS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (NumBotsInGame == 0)
	{
		gi_cprintf(ent, PRINT_HIGH, "There are no bots to remove!\n");
		return;
	}

	++settings->bots_removed;
	if (settings->bots_removed > NumBotsInGame)
		settings->bots_removed = 0;

	settings->v_type = VOTE_REMOVEBOTS;
	Vote_BotsUpdate(ent, p);
}

void Vote_RemoveAllBots(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_REMOVEALLBOTS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (NumBotsInGame == 0)
	{
		gi_cprintf(ent, PRINT_HIGH, "There are no bots to remove!\n");
		return;
	}

	settings->v_type = VOTE_REMOVEALLBOTS;
	Vote_SettingsApply(ent, p);
}

void Vote_BotsUpdate(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings = setmenu->arg;
	int				i = 3;
	char			text[MENU_MSGLEN];

//	Adding AwakenBots.

	if ((int)sv_vote_allow_bots->value)
	{
		Com_sprintf(text, sizeof(text), "Add AwakenBots:    %2d", settings->bots_added);
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNumBotsAdded);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Add AwakenBots:    [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_bots->value)
	{
		Com_sprintf(text, sizeof(text), "Remove AwakenBots: %2d", settings->bots_removed);
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNumBotsRemoved);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Remove AwakenBots: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	PMenu_Update(ent);
}

void GMenu_VoteBots(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings;
	pmenuhnd_t		*menu;

	PMenu_Close(ent);

	settings = gi.TagMalloc(sizeof(*settings), TAG_GAME);											//CW

	settings->bots_added = 0;
	settings->bots_removed = 0;

	settings->v_type = VOTE_NONE;

//	Open menu.

	menu = PMenu_Open(ent, botsmenu, -1, sizeof(botsmenu) / sizeof(pmenu_t), settings);
	Vote_BotsUpdate(ent, menu);
}


//------------------------------
// Changes to dmflags settings.

void Vote_ChangeNoHealth(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOHEALTH))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOHEALTH;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_health = !settings->no_health;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeNoItems(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOITEMS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOITEMS;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_items = !settings->no_items;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeWeaponsStay(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_WEAPONSSTAY))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_WEAPONSSTAY;
	else
		settings->v_type = VOTE_NONE;
	
	settings->weapons_stay = !settings->weapons_stay;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeNoFalling(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOFALLING))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOFALLING;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_falling = !settings->no_falling;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeNoArmor(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOARMOR))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOARMOR;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_armor = !settings->no_armor;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeInfiniteAmmo(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_INFINITEAMMO))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_INFINITEAMMO;
	else
		settings->v_type = VOTE_NONE;
	
	settings->infinite_ammo = !settings->infinite_ammo;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeQuadDrop(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_QUADDROP))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_QUADDROP;
	else
		settings->v_type = VOTE_NONE;
	
	settings->quad_drop = !settings->quad_drop;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeFastSwitch(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_FASTSWITCH))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_FASTSWITCH;
	else
		settings->v_type = VOTE_NONE;
	
	settings->fast_switch = !settings->fast_switch;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeExtraItems(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_EXTRAITEMS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_EXTRAITEMS;
	else
		settings->v_type = VOTE_NONE;
	
	settings->extra_items = !settings->extra_items;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeNoReplacements(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOREPLACEMENTS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOREPLACEMENTS;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_replacements = !settings->no_replacements;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeCTFNoTechs(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_NOTECHS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_NOTECHS;
	else
		settings->v_type = VOTE_NONE;
	
	settings->no_techs = !settings->no_techs;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_ChangeCTFSpawnsOnly(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_CTFSPAWNS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_CTFSPAWNS;
	else
		settings->v_type = VOTE_NONE;
	
	settings->ctf_spawns = !settings->ctf_spawns;
	Vote_DMFlagsUpdate(ent, p);
}

void Vote_DMFlagsUpdate(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings = setmenu->arg;
	int				i = 3;
	char			text[MENU_MSGLEN];

	if ((int)sv_vote_allow_nohealth->value)
	{
		Com_sprintf(text, sizeof(text), "No Health:           %s", settings->no_health ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNoHealth);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "No Health:           [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_noitems->value)
	{
		Com_sprintf(text, sizeof(text), "No Items:            %s", settings->no_items ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNoItems);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "No Items:            [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_weaponsstay->value)
	{
		Com_sprintf(text, sizeof(text), "Weapons Stay:        %s", settings->weapons_stay ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeWeaponsStay);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Weapons Stay:        [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_nofalling->value)
	{
		Com_sprintf(text, sizeof(text), "No Falling Damage:   %s", settings->no_falling ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNoFalling);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "No Falling Damage:   [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_noarmor->value)
	{
		Com_sprintf(text, sizeof(text), "No Armor:            %s", settings->no_armor ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNoArmor);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "No Armor:            [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_infiniteammo->value)
	{
		Com_sprintf(text, sizeof(text), "Infinite Ammo:       %s", settings->infinite_ammo ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeInfiniteAmmo);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Infinite Ammo:       [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_quaddrop->value)
	{
		Com_sprintf(text, sizeof(text), "Quad Drop:           %s", settings->quad_drop ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeQuadDrop);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Quad Drop:           [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_fastswitch->value)
	{
		Com_sprintf(text, sizeof(text), "Fast Weapon Switch:  %s", settings->fast_switch ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeFastSwitch);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Fast Weapon Switch:  [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_extraitems->value)
	{
		Com_sprintf(text, sizeof(text), "Extra Items:         %s", settings->extra_items ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeExtraItems);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Extra Items:         [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_noreplacements->value)
	{
		Com_sprintf(text, sizeof(text), "Vanilla Q2 Powerups: %s", settings->no_replacements ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeNoReplacements);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Vanilla Q2 Powerups: [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_notech->value)
	{
		Com_sprintf(text, sizeof(text), "No CTF Techs:        %s", settings->no_techs ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeCTFNoTechs);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "No CTF Techs:        [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_ctfspawns->value)
	{
		Com_sprintf(text, sizeof(text), "CTF Spawns Only:     %s", settings->ctf_spawns ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeCTFSpawnsOnly);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "CTF Spawns Only:     [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	PMenu_Update(ent);
}

void GMenu_VoteDMFlags(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings;
	pmenuhnd_t		*menu;

	PMenu_Close(ent);

	settings = gi.TagMalloc(sizeof(*settings), TAG_GAME);											//CW

	settings->no_health = !!((int)dmflags->value & DF_NO_HEALTH);
	settings->no_items = !!((int)dmflags->value & DF_NO_ITEMS);
	settings->weapons_stay = !!((int)dmflags->value & DF_WEAPONS_STAY);
	settings->no_falling = !!((int)dmflags->value & DF_NO_FALLING);
	settings->no_armor = !!((int)dmflags->value & DF_NO_ARMOR);
	settings->infinite_ammo = !!((int)dmflags->value & DF_INFINITE_AMMO);
	settings->quad_drop = !!((int)dmflags->value & DF_QUAD_DROP);
	settings->fast_switch = !!((int)dmflags->value & DF_FAST_SWITCH);
	settings->extra_items = !!((int)dmflags->value & DF_EXTRA_ITEMS);
	settings->no_replacements = !!((int)dmflags->value & DF_NO_REPLACEMENTS);
	settings->no_techs = !!((int)dmflags->value & DF_CTF_NO_TECH);
	settings->ctf_spawns = !!((int)dmflags->value & DF_CTF_SPAWNS_ONLY);

	settings->v_type = VOTE_NONE;

//	Open menu.

	menu = PMenu_Open(ent, dmflagsmenu, -1, sizeof(dmflagsmenu) / sizeof(pmenu_t), settings);
	Vote_DMFlagsUpdate(ent, menu);
}

//------------------------------
// Changes to Powerup settings.

void Vote_ChangeInvuln(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_INVULN))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_INVULN;
	else
		settings->v_type = VOTE_NONE;
	
	settings->invuln = !settings->invuln;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeQuad(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_QUAD))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_QUAD;
	else
		settings->v_type = VOTE_NONE;
	
	settings->quad = !settings->quad;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeSiphon(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_SIPHON))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_SIPHON;
	else
		settings->v_type = VOTE_NONE;
	
	settings->siphon = !settings->siphon;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeD89(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_D89))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_D89;
	else
		settings->v_type = VOTE_NONE;
	
	settings->d89 = !settings->d89;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeHaste(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_HASTE))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_HASTE;
	else
		settings->v_type = VOTE_NONE;
	
	settings->haste = !settings->haste;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeTele(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_TELE))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_TELE;
	else
		settings->v_type = VOTE_NONE;
	
	settings->tele = !settings->tele;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeAntibeam(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_ANTIBEAM))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_ANTIBEAM;
	else
		settings->v_type = VOTE_NONE;
	
	settings->antibeam = !settings->antibeam;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeEnviro(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_ENVIRO))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_ENVIRO;
	else
		settings->v_type = VOTE_NONE;
	
	settings->enviro = !settings->enviro;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeSilencer(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_SILENCER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_SILENCER;
	else
		settings->v_type = VOTE_NONE;
	
	settings->silencer = !settings->silencer;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_ChangeBreather(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_BREATHER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_BREATHER;
	else
		settings->v_type = VOTE_NONE;
	
	settings->breather = !settings->breather;
	Vote_PowerupsUpdate(ent, p);
}

void Vote_PowerupsUpdate(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings = setmenu->arg;
	int				i = 3;
	char			text[MENU_MSGLEN];

	if ((int)sv_vote_allow_invuln->value)
	{
		Com_sprintf(text, sizeof(text), "Invulnerability:     %s", settings->invuln ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeInvuln);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Invulnerability:     [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_quad->value)
	{
		Com_sprintf(text, sizeof(text), "Quad Damage:         %s", settings->quad ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeQuad);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Quad Damage:         [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_siphon->value)
	{
		Com_sprintf(text, sizeof(text), "The Awakening:       %s", settings->siphon ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeSiphon);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "The Awakening:       [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_d89->value)
	{
		Com_sprintf(text, sizeof(text), "D89 (Needle):        %s", settings->d89 ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeD89);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "D89 (Needle):        [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_haste->value)
	{
		Com_sprintf(text, sizeof(text), "Haste:               %s", settings->haste ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeHaste);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Haste:               [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_tele->value)
	{
		Com_sprintf(text, sizeof(text), "Personal Teleporter: %s", settings->tele ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeTele);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Personal Teleporter: [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_antibeam->value)
	{
		Com_sprintf(text, sizeof(text), "Beam Reflector:      %s", settings->antibeam ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeAntibeam);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Beam Reflector:      [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_enviro->value)
	{
		Com_sprintf(text, sizeof(text), "Environment Suit:    %s", settings->enviro ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeEnviro);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Environment Suit:    [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_silencer->value)
	{
		Com_sprintf(text, sizeof(text), "Silencer:            %s", settings->silencer ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeSilencer);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Silencer:            [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_breather->value)
	{
		Com_sprintf(text, sizeof(text), "Rebreather:          %s", settings->breather ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeBreather);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Rebreather:          [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	PMenu_Update(ent);
}

void GMenu_VotePowerups(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings;
	pmenuhnd_t		*menu;

	PMenu_Close(ent);

	settings = gi.TagMalloc(sizeof(*settings), TAG_GAME);											//CW

	settings->invuln = !!((int)sv_allow_invuln->value);
	settings->quad = !!((int)sv_allow_quad->value);
	settings->siphon = !!((int)sv_allow_siphon->value);
	settings->d89 = !!((int)sv_allow_d89->value);
	settings->haste = !!((int)sv_allow_haste->value);
	settings->tele = !!((int)sv_allow_tele->value);
	settings->antibeam = !!((int)sv_allow_antibeam->value);
	settings->enviro = !!((int)sv_allow_enviro->value);
	settings->silencer = !!((int)sv_allow_silencer->value);
	settings->breather = !!((int)sv_allow_breather->value);

	settings->v_type = VOTE_NONE;

//	Open menu.

	menu = PMenu_Open(ent, powerupsmenu, -1, sizeof(powerupsmenu) / sizeof(pmenu_t), settings);
	Vote_PowerupsUpdate(ent, menu);
}

//------------------------------
// Changes to weapon settings.

void Vote_ChangeGaussPistol(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_GAUSSPISTOL))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_GAUSSPISTOL;
	else
		settings->v_type = VOTE_NONE;

	settings->gausspistol = !settings->gausspistol;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeMac10(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_MAC10))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_MAC10;
	else
		settings->v_type = VOTE_NONE;

	settings->mac10 = !settings->mac10;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeJackhammer(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_JACKHAMMER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_JACKHAMMER;
	else
		settings->v_type = VOTE_NONE;

	settings->jackhammer = !settings->jackhammer;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeC4(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_C4))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_C4;
	else
		settings->v_type = VOTE_NONE;

	settings->c4 = !settings->c4;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeTraps(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_TRAPS))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_TRAPS;
	else
		settings->v_type = VOTE_NONE;

	settings->traps = !settings->traps;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeESG(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_ESG))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_ESG;
	else
		settings->v_type = VOTE_NONE;

	settings->esg = !settings->esg;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeRocketLauncher(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_ROCKETLAUNCHER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_ROCKETLAUNCHER;
	else
		settings->v_type = VOTE_NONE;

	settings->rocketlauncher = !settings->rocketlauncher;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeFlamethrower(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_FLAMETHROWER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_FLAMETHROWER;
	else
		settings->v_type = VOTE_NONE;

	settings->flamethrower = !settings->flamethrower;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeRailgun(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_RAILGUN))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_RAILGUN;
	else
		settings->v_type = VOTE_NONE;

	settings->railgun = !settings->railgun;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeShockRifle(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_SHOCKRIFLE))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_SHOCKRIFLE;
	else
		settings->v_type = VOTE_NONE;

	settings->shockrifle = !settings->shockrifle;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeAGM(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_AGM))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_AGM;
	else
		settings->v_type = VOTE_NONE;
	
	settings->agm = !settings->agm;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_ChangeDiscLauncher(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_DISCLAUNCHER))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_DISCLAUNCHER;
	else
		settings->v_type = VOTE_NONE;
	
	settings->disclauncher = !settings->disclauncher;
	Vote_WeaponsUpdate(ent, p);
}

void Vote_WeaponsUpdate(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings = setmenu->arg;
	int				i = 3;
	char			text[MENU_MSGLEN];

	if ((int)sv_vote_allow_gausspistol->value)
	{
		Com_sprintf(text, sizeof(text), "Gauss Pistol:        %s", settings->gausspistol ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeGaussPistol);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Gauss Pistol:        [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_mac10->value)
	{
		Com_sprintf(text, sizeof(text), "Mac-10:              %s", settings->mac10 ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeMac10);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Mac-10:              [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_jackhammer->value)
	{
		Com_sprintf(text, sizeof(text), "Jackhammer:          %s", settings->jackhammer ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeJackhammer);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Jackhammer:          [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_c4->value)
	{
		Com_sprintf(text, sizeof(text), "C4 Explosives:       %s", settings->c4 ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeC4);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "C4 Explosives:       [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_traps->value)
	{
		Com_sprintf(text, sizeof(text), "Traps:               %s", settings->traps ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeTraps);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Traps:               [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_esg->value)
	{
		Com_sprintf(text, sizeof(text), "Explosive Spike Gun: %s", settings->esg ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeESG);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Explosive Spike Gun: [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_rocketlauncher->value)
	{
		Com_sprintf(text, sizeof(text), "Rocket Launcher:     %s", settings->rocketlauncher ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeRocketLauncher);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Rocket Launcher:     [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_flamethrower->value)
	{
		Com_sprintf(text, sizeof(text), "Flamethrower:        %s", settings->flamethrower ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeFlamethrower);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Flamethrower:        [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_railgun->value)
	{
		Com_sprintf(text, sizeof(text), "Railgun:             %s", settings->railgun ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeRailgun);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Railgun:             [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_shockrifle->value)
	{
		Com_sprintf(text, sizeof(text), "Shock Rifle:         %s", settings->shockrifle ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeShockRifle);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Shock Rifle:         [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_agm->value)
	{
		Com_sprintf(text, sizeof(text), "AG Manipulator:      %s", settings->agm ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeAGM);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "AG Manipulator:      [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_disclauncher->value)
	{
		Com_sprintf(text, sizeof(text), "Disc Launcher:       %s", settings->disclauncher ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeDiscLauncher);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Disc Launcher:       [LKD]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	PMenu_Update(ent);
}

void GMenu_VoteWeapons(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings;
	pmenuhnd_t		*menu;

	PMenu_Close(ent);

	settings = gi.TagMalloc(sizeof(*settings), TAG_GAME);											//CW

	settings->gausspistol = !!((int)sv_allow_gausspistol->value);
	settings->mac10 = !!((int)sv_allow_mac10->value);
	settings->jackhammer = !!((int)sv_allow_jackhammer->value);
	settings->c4 = !!((int)sv_allow_c4->value);
	settings->traps = !!((int)sv_allow_traps->value);
	settings->esg = !!((int)sv_allow_spikegun->value);
	settings->rocketlauncher = !!((int)sv_allow_rocketlauncher->value);
	settings->flamethrower = !!((int)sv_allow_flamethrower->value);
	settings->railgun = !!((int)sv_allow_railgun->value);
	settings->shockrifle = !!((int)sv_allow_shockrifle->value);
	settings->agm = !!((int)sv_allow_agm->value);
	settings->disclauncher = !!((int)sv_allow_disclauncher->value);

	settings->v_type = VOTE_NONE;

//	Open menu.

	menu = PMenu_Open(ent, weaponsmenu, -1, sizeof(weaponsmenu) / sizeof(pmenu_t), settings);
	Vote_WeaponsUpdate(ent, menu);
}

//------------------------------
// Changes to game settings.

void Vote_ChangeMap(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_MAP))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (++settings->map > settings->g_nmaps)
		settings->map = 1;

	settings->v_type = VOTE_MAP;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeConfig(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_CONFIG))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (++settings->config > settings->g_nconfigs)
		settings->config = 1;

	settings->v_type = VOTE_CONFIG;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeMatch(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_MATCH))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_MATCH;
	else
		settings->v_type = VOTE_NONE;

	settings->match = !settings->match;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeTimelimit(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_TIMELIMIT))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	settings->timelimit += (int)sv_vote_timelimit_step->value;
	if (settings->timelimit > (int)sv_vote_timelimit_max->value)
		settings->timelimit = 0;

	settings->v_type = VOTE_TIMELIMIT;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeFraglimit(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_FRAGLIMIT))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	settings->fraglimit += (int)sv_vote_fraglimit_step->value;
	if (settings->fraglimit > (int)sv_vote_fraglimit_max->value)
		settings->fraglimit = 0;

	settings->v_type = VOTE_FRAGLIMIT;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeCapturelimit(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_CAPLIMIT))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	settings->caplimit += (int)sv_vote_capturelimit_step->value;
	if (settings->caplimit > (int)sv_vote_capturelimit_max->value)
		settings->caplimit = 0;

	settings->v_type = VOTE_CAPLIMIT;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeGrapple(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_GRAPPLE))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_GRAPPLE;
	else
		settings->v_type = VOTE_NONE;

	settings->hook = !settings->hook;
	Vote_SettingsUpdate(ent, p);
}

void Vote_ChangeGrappleOffhand(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t *settings = p->arg;

	if ((settings->v_type != VOTE_NONE) && (settings->v_type != VOTE_GRAPPLE_OFFHAND))
	{
		gi_cprintf(ent, PRINT_HIGH, "You can only change one item at a time!\n");
		return;
	}

	if (settings->v_type == VOTE_NONE)
		settings->v_type = VOTE_GRAPPLE_OFFHAND;
	else
		settings->v_type = VOTE_NONE;

	settings->hook_offhand = !settings->hook_offhand;
	Vote_SettingsUpdate(ent, p);
}

void Vote_SettingsUpdate(edict_t *ent, pmenuhnd_t *setmenu)
{
	vote_settings_t	*settings = setmenu->arg;
	int				i = 3;
	char			text[MENU_MSGLEN];

//	New map.

	if ((int)sv_vote_allow_map->value)
	{
		if (settings->g_nmaps == 0)
		{
			Com_sprintf(text, sizeof(text), "Map: [INVALID FILE]");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
		}
		else
		{
			if (settings->map == 0)
				Com_sprintf(text, sizeof(text), "Map: [Select]");
			else
				Com_sprintf(text, sizeof(text), "Map: %s", settings->g_maplist[settings->map - 1]);
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeMap);
		}
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Map: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

//	Execute config file.

	if ((int)sv_vote_allow_config->value)
	{
		if (settings->g_nconfigs == 0)
		{
			Com_sprintf(text, sizeof(text), "Config: [INVALID FILE]");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
		}
		else
		{
			if (settings->config == 0)
				Com_sprintf(text, sizeof(text), "Config: [Select]");
			else
				Com_sprintf(text, sizeof(text), "Config: %s", settings->g_configlist[settings->config - 1]);
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeConfig);
		}
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Config: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_match->value)
	{
		if ((int)sv_gametype->value > G_FFA)
		{
			Com_sprintf(text, sizeof(text), "Match Mode: %s", settings->match ? "ON" : "OFF");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeMatch);
		}
		else
		{
			Com_sprintf(text, sizeof(text), "Match Mode: (N/A)");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
		}
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Match Mode: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

//	Limits for DM/CTF rules.

	if ((int)sv_vote_allow_timelimit->value)
	{
		Com_sprintf(text, sizeof(text), "Timelimit: %2d mins", settings->timelimit);
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeTimelimit);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Timelimit: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_fraglimit->value)
	{
		Com_sprintf(text, sizeof(text), "Fraglimit: %2d", settings->fraglimit);
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeFraglimit);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Fraglimit: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_capturelimit->value)
	{
		if (sv_gametype->value == G_CTF)
		{
			Com_sprintf(text, sizeof(text), "Capturelimit: %2d", settings->caplimit);
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeCapturelimit);
		}
		else
		{
			Com_sprintf(text, sizeof(text), "Capturelimit: (N/A)");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
		}
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Capturelimit: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

//	Grappling hook.

	if ((int)sv_vote_allow_hook->value)
	{
		Com_sprintf(text, sizeof(text), "The Grapple: %s", settings->hook ? "Yes" : "No");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeGrapple);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "The Grapple: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	if ((int)sv_vote_allow_hook_offhand->value)
	{
		if (settings->hook)
		{
			Com_sprintf(text, sizeof(text), "Off-hand Grapple: %s", settings->hook_offhand ? "Yes" : "No");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, Vote_ChangeGrappleOffhand);
		}
		else
		{
			Com_sprintf(text, sizeof(text), "Off-hand Grapple: (N/A)");
			PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
		}
	}
	else
	{
		Com_sprintf(text, sizeof(text), "Off-hand Grapple: [LOCKED]");
		PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, NULL);
	}
	++i;

	PMenu_Update(ent);
}

void GMenu_VoteSettings(edict_t *ent, pmenuhnd_t *p)
{
	vote_settings_t	*settings;
	pmenuhnd_t		*menu;
	FILE			*mapstream;
	FILE			*cfgstream;
	char			filename[MAX_QPATH];
	qboolean		finished;
	int				fnum;

	if (ent->client->menu)
		PMenu_Close(ent);

//	Assign initial vote settings.

	settings = gi.TagMalloc(sizeof(*settings), TAG_GAME);											//CW

	settings->map = 0;
	settings->config = 0;
	settings->match = !!((int)competition->value);
	settings->timelimit = (int)timelimit->value;
	settings->fraglimit = (int)fraglimit->value;
	settings->caplimit = (int)capturelimit->value;
	settings->hook = !!((int)sv_allow_hook->value);
	settings->hook_offhand = !!((int)sv_hook_offhand->value);

	settings->v_type = VOTE_NONE;
	settings->g_nmaps = 0;
	settings->g_nconfigs = 0;

//	Read in list of bsp names from map list file.

	if ((mapstream = OpenMaplistFile(false)) != NULL)
	{
		finished = false;
		while (!finished)
		{
			fnum = fscanf(mapstream, "%s", filename);
			if (fnum != EOF)
			{
				if ((filename[0] == '/') && (filename[1] == '/'))
					continue;

				settings->g_maplist[settings->g_nmaps++] = strdup(filename);
				if (settings->g_nmaps == MAX_MAPS)
					finished = true;
			}
			else
				finished = true;
		}
		fclose(mapstream);
	}
	else
		gi.dprintf("** Failed to open list file for map vote (menu)\n");

//	Read in list of cfg names from config list file.

	if ((cfgstream = OpenConfiglistFile(false)) != NULL)
	{
		finished = false;
		while (!finished)
		{
			fnum = fscanf(cfgstream, "%s", filename);
			if (fnum != EOF)
			{
				if ((filename[0] == '/') && (filename[1] == '/'))
					continue;

				settings->g_configlist[settings->g_nconfigs++] = strdup(filename);
				if (settings->g_nconfigs == MAX_CONFIGS)
					finished = true;
			}
			else
				finished = true;
		}
		fclose(cfgstream);
	}
	else
		gi.dprintf("** Failed to open list file for config vote (menu)\n");

	if (!(int)sv_allow_bots->value || (int)chedit->value || (teamgame.match > MATCH_NONE) || ((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
	{
		votemenu[votemenu_bots].text = NULL;
		votemenu[votemenu_bots].SelectFunc = NULL;
	}

//	Open menu.

	menu = PMenu_Open(ent, votemenu, -1, sizeof(votemenu) / sizeof(pmenu_t), settings);
	Vote_SettingsUpdate(ent, menu);
}

/*
===============================
Operator Menu functions
===============================
*/
void OpMenu_Kick(edict_t *ent, pmenuhnd_t *hmenu)
{
	int	*n_player = hmenu->arg;
	int	i;

	i = *n_player;
	PMenu_Close(ent);
	Op_Boot(ent, false, i, "");
}

void OpMenu_Ban(edict_t *ent, pmenuhnd_t *hmenu)
{
	int	*n_player = hmenu->arg;
	int	i;

	i = *n_player;
	PMenu_Close(ent);
	Op_Boot(ent, true, i, "");
}

void OpMenu_Mute(edict_t *ent, pmenuhnd_t *hmenu)
{
	int	*n_player = hmenu->arg;
	int	i;

	i = *n_player;
	PMenu_Close(ent);
	Op_SayPrivs(ent, true, i, "");
}

void OpMenu_Unmute(edict_t *ent, pmenuhnd_t *hmenu)
{
	int	*n_player = hmenu->arg;
	int	i;

	i = *n_player;
	PMenu_Close(ent);
	Op_SayPrivs(ent, false, i, "");
}

void OpMenu_SwapTeam(edict_t *ent, pmenuhnd_t *hmenu)
{
	int *n_player = hmenu->arg;
	int	i;

	i = *n_player;
	PMenu_Close(ent);
	Op_SwapTeam(ent, i);
}

void OpMenu_Vote(edict_t *ent, pmenuhnd_t *hmenu)
{
	PMenu_Close(ent);
	Cmd_Vote_f(ent, true);
}

void OpMenu_VoteYes(edict_t *ent, pmenuhnd_t *hmenu)
{
	PMenu_Close(ent);
	Cmd_VoteYes_f(ent, true);
}

void OpMenu_VoteNo(edict_t *ent, pmenuhnd_t *hmenu)
{
	PMenu_Close(ent);
	Cmd_VoteNo_f(ent, true);
}

void OpMenu_ForceMatchStart(edict_t *ent, pmenuhnd_t *hmenu)
{
	PMenu_Close(ent);
	Cmd_MatchStart_f(ent);
}

void Op_ChangePlayerNum(edict_t *ent, pmenuhnd_t *hmenu)
{
	int *n_player = hmenu->arg;

	++(*n_player);
	if (*n_player > (int)maxclients->value)
		*n_player = 1;

	Op_MenuUpdate(ent, hmenu);
}

void Op_MenuUpdate(edict_t *ent, pmenuhnd_t *hmenu)
{
	char	text[MENU_MSGLEN];
	int		*n_player = hmenu->arg;

	Com_sprintf(text, sizeof(text), "Player Selected: [%d]", *n_player);
	PMenu_UpdateEntry(hmenu->entries + opmenu_pnum, text, PMENU_ALIGN_LEFT, Op_ChangePlayerNum);
	PMenu_Update(ent);
}

void OpenOpMenu(edict_t *ent)
{
	pmenuhnd_t	*menu;
	int			*n_player;

	if (ent->client->menu)
		PMenu_Close(ent);

	n_player = gi.TagMalloc(sizeof(*n_player), TAG_GAME);											//CW
	*n_player = 1;

	if (teamgame.match == MATCH_SETUP)
	{
		opmenu[opmenu_start].text =  "Force Match to Start";
		opmenu[opmenu_start].SelectFunc = OpMenu_ForceMatchStart;
	}

	menu = PMenu_Open(ent, opmenu, -1, sizeof(opmenu) / sizeof(pmenu_t), n_player);
	Op_MenuUpdate(ent, menu);
}

void OpMenu_Main(edict_t *ent, pmenuhnd_t *p)
{
	OpenOpMenu(ent);
}

/*
===============================
Game Menu functions
===============================
*/
void GMenu_Play(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
}

void GMenu_Observer(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	CTFObserver(ent);
}

void PlayerList_Update(edict_t *ent, pmenuhnd_t *pdisplay)
{
	edict_t	*e2;
	char	text[30];
	int		*p_offset = pdisplay->arg;
	int		i;
	int		linenum;

//	Setup list menu.

	for (i = 3; i < plist_next; ++i)
		PMenu_UpdateEntry(pdisplay->entries + i, " ", PMENU_ALIGN_LEFT, NULL);

	if (!ent->client->spectator)
		PMenu_UpdateEntry(pdisplay->entries + plist_next, "Game Menu", PMENU_ALIGN_LEFT, GMenu_Main);
	else
		PMenu_UpdateEntry(pdisplay->entries + plist_next, "Game Menu", PMENU_ALIGN_LEFT, CTFReturnToMain);

	PMenu_UpdateEntry(pdisplay->entries + plist_opreturn, " ", PMENU_ALIGN_LEFT, NULL);

//	Display number, name and operator status.

	linenum = *p_offset;
	for (i = *p_offset; i <= (int)maxclients->value; ++i)
	{
		e2 = g_edicts + i;
		if (!e2->client || !e2->inuse)
			continue;

		Com_sprintf(text, sizeof(text), "%3d %-16.16s %s\n", i, e2->client->pers.netname, (e2->client->pers.op_status)?"(OP)":((e2->isabot)?"(Bot)":""));
		PMenu_UpdateEntry(pdisplay->entries + (linenum - *p_offset) + 3, text, PMENU_ALIGN_LEFT, NULL);
		++linenum;

		if (linenum == 11 + *p_offset)
		{
			*p_offset += i;
			if (i < (int)maxclients->value)
				PMenu_UpdateEntry(pdisplay->entries + plist_next, "Continued...", PMENU_ALIGN_LEFT, PlayerList_Update);

			break;
		}
	}

	if (ent->client->pers.op_status)
		PMenu_UpdateEntry(pdisplay->entries + plist_opreturn, "Operator Menu", PMENU_ALIGN_LEFT, OpMenu_Main);

	PMenu_Update(ent);
}

void OpenPlayerList(edict_t *ent)
{
	pmenuhnd_t	*menu;
	int			*p_offset;

	if (ent->client->menu)
		PMenu_Close(ent);

	p_offset = gi.TagMalloc(sizeof(*p_offset), TAG_GAME);											//CW
	*p_offset = 1;

	menu = PMenu_Open(ent, playerlist, -1, sizeof(playerlist) / sizeof(pmenu_t), p_offset);
	PlayerList_Update(ent, menu);
}

void GMenu_Playerlist(edict_t *ent, pmenuhnd_t *p)
{
	OpenPlayerList(ent);
}

void GMenu_Help_Weapons(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_weapons, -1, sizeof(help_weapons) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_ClientCmds1(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_clientcmds1, -1, sizeof(help_clientcmds1) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_ClientCmds2(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_clientcmds2, -1, sizeof(help_clientcmds2) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_ClientCmds3(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_clientcmds3, -1, sizeof(help_clientcmds3) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_OpCmds1(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_opcmds1, -1, sizeof(help_opcmds1) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_OpCmds2(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_opcmds2, -1, sizeof(help_opcmds2) / sizeof(pmenu_t), NULL);
}

void GMenu_Help_OpCmds3(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_opcmds3, -1, sizeof(help_opcmds3) / sizeof(pmenu_t), NULL);
}

void GMenu_Help(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);

	PMenu_Open(ent, help_menu, -1, sizeof(help_menu) / sizeof(pmenu_t), NULL);
}

qboolean JoinTeam(edict_t *ent, int desired_team, qboolean op_forced)
{
	char *s;

//	Sanity checks.

	if (sv_gametype->value == G_FFA)
		return false;

	if ((teamgame.match > MATCH_SETUP) && !op_forced)
	{
		gi_cprintf(ent, PRINT_HIGH, "Can't change teams during a match.\n");
		return false;
	}

	if (ent->client->resp.ctf_team == desired_team)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n", CTFTeamName(ent->client->resp.ctf_team));
		return false;
	}

//	If the team change request is valid, kill the player and respawn them as a member of the
//	other team.

	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;

	s = Info_ValueForKey(ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	if (ent->client->spectator)	// no need to kill them first
	{
		PutClientInServer(ent);
		ent->s.event = EV_PLAYER_TELEPORT;
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi_bprintf(PRINT_HIGH, "%s joined the %s team.\n", ent->client->pers.netname, CTFTeamName(desired_team));
		return true;
	}

	ent->health = 0;

	if ((sv_gametype->value == G_TDM) || (sv_gametype->value == G_ASLT))
		ent->client->mod_changeteam = true;

	player_die(ent, ent, world, 100000, vec3_origin);
	ent->deadflag = DEAD_DEAD;
	if (!ent->isabot)
		Respawn(ent);

	ent->client->resp.score = 0;
	gi_bprintf(PRINT_HIGH, "%s changed to the %s team.\n", ent->client->pers.netname, CTFTeamName(desired_team));

	return true;
}

void GMenu_JoinTeam1(edict_t *ent, pmenuhnd_t *p)
{
	JoinTeam(ent, CTF_TEAM1, false);
}

void GMenu_JoinTeam2(edict_t *ent, pmenuhnd_t *p)
{
	JoinTeam(ent, CTF_TEAM2, false);
}

void OpenGameMenu(edict_t *ent)
{
	char msgbuf[32];

	if (ent->client->menu)
		PMenu_Close(ent);

	ent->client->op_override = false;

	if (ent->client->showinventory)
		ent->client->showinventory = false;

	SetLevelName(gamemenu + gmenu_level);

	switch (teamgame.match)
	{
		case MATCH_NONE:
			gamemenu[gmenu_match].text = NULL;
			break;

		case MATCH_SETUP:
			gamemenu[gmenu_match].text = "*MATCH SETUP IN PROGRESS";
			break;

		case MATCH_PREGAME:
			gamemenu[gmenu_match].text = "*MATCH STARTING";
			break;

		case MATCH_GAME:
			gamemenu[gmenu_match].text = "*MATCH IN PROGRESS";
			break;

		case MATCH_POST:
			gamemenu[gmenu_match].text = "*MATCH COMPLETED";
			break;
	}

	switch ((int)sv_gametype->value)
	{
		case G_FFA:
			gamemenu[gmenu_game].text = "*Gametype : FFA";
			gamemenu[gmenu_team].text = NULL;
			gamemenu[gmenu_team].SelectFunc = NULL;
			break;

		case G_CTF:
			gamemenu[gmenu_game].text = "*Gametype : CTF";
			break;

		case G_TDM:
			gamemenu[gmenu_game].text = "*Gametype : Team-DM";
			break;

		case G_ASLT:
			gamemenu[gmenu_game].text = "*Gametype : Assault";
			break;
	}

	if ((int)sv_gametype->value > G_FFA)
	{
		if (teamgame.match < MATCH_PREGAME)
		{
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				Com_sprintf(msgbuf, sizeof(msgbuf), "Join %s Team", sv_team2_name->string);
				gamemenu[gmenu_team].SelectFunc = GMenu_JoinTeam2;
			}
			else
			{
				Com_sprintf(msgbuf,sizeof(msgbuf),  "Join %s Team", sv_team1_name->string);
				gamemenu[gmenu_team].SelectFunc = GMenu_JoinTeam1;
			}
			gamemenu[gmenu_team].text = strdup(msgbuf);
		}
		else
		{
			gamemenu[gmenu_vote].text = NULL;
			gamemenu[gmenu_vote].SelectFunc = NULL;
			gamemenu[gmenu_team].text = "[Teams locked]";
			gamemenu[gmenu_team].SelectFunc = NULL;
		}
	}

	PMenu_Open(ent, gamemenu, -1, sizeof(gamemenu) / sizeof(pmenu_t), NULL);
}

void GMenu_Main(edict_t *ent, pmenuhnd_t *p)
{
	OpenGameMenu(ent);
}

/*
===============================
'Message Of The Day' functions
===============================
*/
FILE* OpenMOTDFile(void)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_motd_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
	{
		if (strlen(GAMEVERSION) + strlen(sv_motd_file->string) + 2 > MAX_OSPATH)
		{
			gi.dprintf("WARNING: MOTD filename is too long.\n");
			return NULL;
		}

		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_motd_file->string);
	}
	else
	{
		if (strlen(game->string) + strlen(sv_motd_file->string) + 2 > MAX_OSPATH)
		{
			gi.dprintf("WARNING: MOTD filename is too long.\n");
			return NULL;
		}

		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_motd_file->string);
	}

	if ((iostream = fopen(filename, "r")) == NULL)
		gi.dprintf("** Failed to open \"%s\"\n", filename);

	return iostream;
}

void PrintMOTD(edict_t *ent)
{
	FILE		*msgstream;
	char		txt_line[32];
	qboolean	finished = false;
	int			i;

	if (ent->client->menu)
		PMenu_Close(ent);

//	Clear the current MOTD in case the new one is shorter than the old one.

	for (i = 1; i < 17; ++i)
		motd[i].text = " ";

	if (!ent->client->spectator)
		motd[17].SelectFunc = GMenu_Main;
	else
		motd[17].SelectFunc = CTFReturnToMain;

//	Read in lines from the current MOTD file, and display them in a menu background.

	i = 1;
	if ((msgstream = OpenMOTDFile()) != NULL)
	{
		while (!finished && (++i < 17))
		{
			if (fgets(txt_line, 29, msgstream) != NULL)
				motd[i].text = strdup(txt_line);
			else
				finished = true;
		}

		fclose(msgstream);
		PMenu_Open(ent, motd, -1, sizeof(motd) / sizeof(pmenu_t), NULL);
	}
	else
		gi_centerprintf(ent, "Welcome to Awakening II\n");
}

void ShowMOTD(edict_t *ent, pmenuhnd_t *p)
{
	PrintMOTD(ent);
}
//CW--
