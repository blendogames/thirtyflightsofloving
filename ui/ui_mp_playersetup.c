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

// ui_playersetup.c -- the player setup menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

PLAYER CONFIG MENU

=============================================================================
*/
extern menuframework_s	s_multiplayer_menu;

static menuframework_s	s_player_config_menu;
static menufield_s		s_player_name_field;
static menulist_s		s_player_model_box;
static menulist_s		s_player_skin_box;
static menulist_s		s_player_handedness_box;
static menulist_s		s_player_rate_box;
static menuseparator_s	s_player_skin_title;
static menuseparator_s	s_player_model_title;
static menuseparator_s	s_player_hand_title;
static menuseparator_s	s_player_rate_title;
//static menuaction_s		s_player_download_action;
static menuaction_s		s_player_back_action;

// save skins and models here so as to not have to re-register every frame
struct model_s *playermodel;
struct model_s *weaponmodel;
struct image_s *playerskin;
char *currentweaponmodel;

#define MAX_DISPLAYNAME 16
#define MAX_PLAYERMODELS 1024
#define	NUM_SKINBOX_ITEMS 7

typedef struct
{
	int		nskins;
	char	**skindisplaynames;
	char	displayname[MAX_DISPLAYNAME];
	char	directory[MAX_QPATH];
} playermodelinfo_s;

static playermodelinfo_s s_pmi[MAX_PLAYERMODELS];
static char *s_pmnames[MAX_PLAYERMODELS];
static int s_numplayermodels;

static int rate_tbl[] = { 2500, 3200, 5000, 10000, 15000, 25000, 0 };
static const char *rate_names[] = { "28.8 Modem", "33.6 Modem", "56K/Single ISDN",
	"Dual ISDN", "Cable/DSL", "T1/LAN", "User defined", 0 };


static void HandednessCallback (void *unused)
{
	Cvar_SetValue ("hand", s_player_handedness_box.curvalue);
}


static void RateCallback (void *unused)
{
	if (s_player_rate_box.curvalue != sizeof(rate_tbl) / sizeof(*rate_tbl) - 1)
		Cvar_SetValue ("rate", rate_tbl[s_player_rate_box.curvalue]);
}


static void ModelCallback (void *unused)
{
	char scratch[MAX_QPATH];

	s_player_skin_box.itemnames = s_pmi[s_player_model_box.curvalue].skindisplaynames;
	s_player_skin_box.curvalue = 0;

	// only register model and skin on starup or when changed
	Com_sprintf( scratch, sizeof(scratch), "players/%s/tris.md2", s_pmi[s_player_model_box.curvalue].directory );
	playermodel = R_RegisterModel (scratch);

	Com_sprintf( scratch, sizeof(scratch), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );
	playerskin = R_RegisterSkin (scratch);

	// show current weapon model (if any)
	if (currentweaponmodel && strlen(currentweaponmodel)) {
		Com_sprintf (scratch, sizeof(scratch), "players/%s/%s", s_pmi[s_player_model_box.curvalue].directory, currentweaponmodel);
		weaponmodel = R_RegisterModel(scratch);
		if (!weaponmodel) {
			Com_sprintf (scratch, sizeof(scratch), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory);
			weaponmodel = R_RegisterModel (scratch);
		}
	}
	else {
		Com_sprintf (scratch, sizeof(scratch), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory);
		weaponmodel = R_RegisterModel (scratch);
	}
}


// only register skin on starup and when changed
static void SkinCallback (void *unused)
{
	char scratch[MAX_QPATH];

	Com_sprintf(scratch, sizeof(scratch), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue]);
	playerskin = R_RegisterSkin(scratch);
}


static qboolean IconOfSkinExists (char *skin, char **files, int nfiles, char *suffix)
{
	int i;
	char scratch[1024];

//	strncpy(scratch, skin);
	Q_strncpyz(scratch, skin, sizeof(scratch));
	*strrchr(scratch, '.') = 0;
//	strncat(scratch, suffix);
	Q_strncatz(scratch, suffix, sizeof(scratch));
	//strncat(scratch, "_i.pcx");

	for (i = 0; i < nfiles; i++)
	{
		if ( strcmp(files[i], scratch) == 0 )
			return true;
	}

	return false;
}


// adds menu support for TGA and JPG skins
static qboolean IsValidSkin (char **filelist, int numFiles, int index)
{
	int		len = (int)strlen(filelist[index]);

	if (	!strcmp (filelist[index]+max(len-4,0), ".pcx")
		||  !strcmp (filelist[index]+max(len-4,0), ".tga")
#ifdef PNG_SUPPORT
		||  !strcmp (filelist[index]+max(len-4,0), ".png")
#endif // PNG_SUPPORT
		||	!strcmp (filelist[index]+max(len-4,0), ".jpg") )
	{
		if (	strcmp (filelist[index]+max(len-6,0), "_i.pcx")
			&&  strcmp (filelist[index]+max(len-6,0), "_i.tga")
#ifdef PNG_SUPPORT
			&&  strcmp (filelist[index]+max(len-6,0), "_i.png")
#endif // PNG_SUPPORT
			&&  strcmp (filelist[index]+max(len-6,0), "_i.jpg") )

			if (	IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.pcx")
				||	IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.tga")
#ifdef PNG_SUPPORT
				||	IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.png")
#endif // PNG_SUPPORT
				||	IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.jpg"))
				return true;
	}
	return false;
}


static qboolean PlayerConfig_ScanDirectories (void)
{
	char findname[1024];
	char scratch[1024];
	int ndirs = 0, npms = 0;
	char **dirnames;
	char *path = NULL;
	int i;

	//extern char **FS_ListFiles (char *, int *, unsigned, unsigned);

	s_numplayermodels = 0;

	// loop back to here if there were no valid player models found in the selected path
	do
	{
		//
		// get a list of directories
		//
		do 
		{
			path = FS_NextPath(path);
			Com_sprintf( findname, sizeof(findname), "%s/players/*.*", path );

			if ( (dirnames = FS_ListFiles(findname, &ndirs, SFF_SUBDIR, 0)) != 0 )
				break;
		} while (path);

		if (!dirnames)
			return false;

		//
		// go through the subdirectories
		//
		npms = ndirs;
		if (npms > MAX_PLAYERMODELS)
			npms = MAX_PLAYERMODELS;
		if ( (s_numplayermodels + npms) > MAX_PLAYERMODELS )
			npms = MAX_PLAYERMODELS - s_numplayermodels;

		for (i = 0; i < npms; i++)
		{
			int			k, s;
			char		*a, *b, *c;
			char		**skinnames;
			char		**imagenames;
			int			nimagefiles;
			int			nskins = 0;
			qboolean	already_added = false;	

			if (dirnames[i] == 0)
				continue;

			// check if dirnames[i] is already added to the s_pmi[i].directory list
			a = strrchr(dirnames[i], '/');
			b = strrchr(dirnames[i], '\\');
			c = (a > b) ? a : b;
			for (k=0; k < s_numplayermodels; k++)
				if (!strcmp(s_pmi[k].directory, c+1))
				{	already_added = true;	break;	}
			if (already_added)
			{	// todo: add any skins for this model not already listed to skindisplaynames
				continue;
			}

			// verify the existence of tris.md2
		//	strncpy(scratch, dirnames[i]);
		//	strncat(scratch, "/tris.md2");
			Q_strncpyz(scratch, dirnames[i], sizeof(scratch));
			Q_strncatz(scratch, "/tris.md2", sizeof(scratch));
			if ( !Sys_FindFirst(scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) )
			{
				free(dirnames[i]);
				dirnames[i] = 0;
				Sys_FindClose();
				continue;
			}
			Sys_FindClose();

			// verify the existence of at least one skin
		//	strncpy(scratch, va("%s%s", dirnames[i], "/*.*")); // was "/*.pcx"
			Q_strncpyz(scratch, va("%s%s", dirnames[i], "/*.*"), sizeof(scratch)); // was "/*.pcx"
			imagenames = FS_ListFiles (scratch, &nimagefiles, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);

			if (!imagenames)
			{
				free(dirnames[i]);
				dirnames[i] = 0;
				continue;
			}

			// count valid skins, which consist of a skin with a matching "_i" icon
			for (k = 0; k < nimagefiles-1; k++)
				if ( IsValidSkin(imagenames, nimagefiles, k) )
					nskins++;

			if (!nskins)
				continue;

			skinnames = malloc(sizeof(char *) * (nskins+1));
			memset(skinnames, 0, sizeof(char *) * (nskins+1));

			// copy the valid skins
			if (nimagefiles)
				for (s = 0, k = 0; k < nimagefiles-1; k++)
				{
					char *a, *b, *c;
					if ( IsValidSkin(imagenames, nimagefiles, k) )
					{
						a = strrchr(imagenames[k], '/');
						b = strrchr(imagenames[k], '\\');

						c = (a > b) ? a : b;

					//	strncpy(scratch, c+1);
						Q_strncpyz(scratch, c+1, sizeof(scratch));

						if ( strrchr(scratch, '.') )
							*strrchr(scratch, '.') = 0;

						skinnames[s] = strdup(scratch);
						s++;
					}
				}

			// at this point we have a valid player model
			s_pmi[s_numplayermodels].nskins = nskins;
			s_pmi[s_numplayermodels].skindisplaynames = skinnames;

			// make short name for the model
			a = strrchr(dirnames[i], '/');
			b = strrchr(dirnames[i], '\\');

			c = (a > b) ? a : b;

			strncpy(s_pmi[s_numplayermodels].displayname, c+1, MAX_DISPLAYNAME-1);
		//	strncpy(s_pmi[s_numplayermodels].directory, c+1);
			Q_strncpyz(s_pmi[s_numplayermodels].directory, c+1, sizeof(s_pmi[s_numplayermodels].directory));

			FS_FreeFileList (imagenames, nimagefiles);

			s_numplayermodels++;
		}
		
		if (dirnames)
			FS_FreeFileList (dirnames, ndirs);

	// if no valid player models found in path,
	// try next path, if there is one
	} while (path);	// (s_numplayermodels == 0 && path);

	return true;	//** DMP warning fix
}


static int pmicmpfnc (const void *_a, const void *_b)
{
	const playermodelinfo_s *a = (const playermodelinfo_s *) _a;
	const playermodelinfo_s *b = (const playermodelinfo_s *) _b;

	//
	// sort by male, female, then alphabetical
	//
	if ( strcmp(a->directory, "male") == 0 )
		return -1;
	else if (strcmp( b->directory, "male") == 0 )
		return 1;

	if ( strcmp(a->directory, "female") == 0 )
		return -1;
	else if (strcmp( b->directory, "female") == 0 )
		return 1;

	return strcmp(a->directory, b->directory);
}


qboolean PlayerConfig_MenuInit (void)
{
	extern cvar_t *name;
	extern cvar_t *team;
	extern cvar_t *skin;
	char currentdirectory[1024];
	char currentskin[1024];
	char scratch[MAX_QPATH];
	int i = 0;
	int y = 0;

	int currentdirectoryindex = 0;
	int currentskinindex = 0;

	cvar_t *hand = Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	static const char *handedness[] = { "right", "left", "center", 0 };

	PlayerConfig_ScanDirectories ();

	if (s_numplayermodels == 0)
		return false;

//	if ( hand->value < 0 || hand->value > 2 )
	if ( hand->integer < 0 || hand->integer > 2 )
		Cvar_SetValue( "hand", 0 );

//	strncpy( currentdirectory, skin->string );
	Q_strncpyz( currentdirectory, skin->string, sizeof(currentdirectory) );

	if ( strchr( currentdirectory, '/' ) )
	{
	//	strncpy( currentskin, strchr( currentdirectory, '/' ) + 1 );
		Q_strncpyz( currentskin, strchr( currentdirectory, '/' ) + 1, sizeof(currentskin) );
		*strchr( currentdirectory, '/' ) = 0;
	}
	else if ( strchr( currentdirectory, '\\' ) )
	{
	//	strncpy( currentskin, strchr( currentdirectory, '\\' ) + 1 );
		Q_strncpyz( currentskin, strchr( currentdirectory, '\\' ) + 1, sizeof(currentskin) );
		*strchr( currentdirectory, '\\' ) = 0;
	}
	else
	{
	//	strncpy( currentdirectory, "male" );
	//	strncpy( currentskin, "grunt" );
		Q_strncpyz( currentdirectory, "male", sizeof(currentdirectory) );
		Q_strncpyz( currentskin, "grunt", sizeof(currentskin) );
	}

	qsort( s_pmi, s_numplayermodels, sizeof( s_pmi[0] ), pmicmpfnc );

	memset( s_pmnames, 0, sizeof( s_pmnames ) );
	for ( i = 0; i < s_numplayermodels; i++ )
	{
		s_pmnames[i] = s_pmi[i].displayname;
		if ( Q_stricmp( s_pmi[i].directory, currentdirectory ) == 0 )
		{
			int j;

			currentdirectoryindex = i;

			for ( j = 0; j < s_pmi[i].nskins; j++ )
			{
				if ( Q_stricmp( s_pmi[i].skindisplaynames[j], currentskin ) == 0 )
				{
					currentskinindex = j;
					break;
				}
			}
		}
	}
	
	s_player_config_menu.x = SCREEN_WIDTH*0.5 - 210;
	s_player_config_menu.y = SCREEN_HEIGHT*0.5 - 70;
	s_player_config_menu.nitems = 0;
	
	s_player_name_field.generic.type = MTYPE_FIELD;
	s_player_name_field.generic.textSize = MENU_FONT_SIZE;
	s_player_name_field.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_name_field.generic.name = "name";
	s_player_name_field.generic.callback = 0;
	s_player_name_field.generic.x		= -MENU_FONT_SIZE;
	s_player_name_field.generic.y		= y;
	s_player_name_field.length	= 20;
	s_player_name_field.visible_length = 20;
	Q_strncpyz( s_player_name_field.buffer, name->string, sizeof(s_player_name_field.buffer) );
	s_player_name_field.cursor = (int)strlen( name->string );
	
	s_player_model_title.generic.type = MTYPE_SEPARATOR;
	s_player_model_title.generic.textSize = MENU_FONT_SIZE;
	s_player_model_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_model_title.generic.name = "model";
	s_player_model_title.generic.x    = -MENU_FONT_SIZE;
	s_player_model_title.generic.y	 = y += 3*MENU_LINE_SIZE;
	
	s_player_model_box.generic.type = MTYPE_SPINCONTROL;
	s_player_model_box.generic.textSize = MENU_FONT_SIZE;
	s_player_model_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_model_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_model_box.generic.callback = ModelCallback;
	s_player_model_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_model_box.curvalue = currentdirectoryindex;
	s_player_model_box.itemnames = s_pmnames;
	
	s_player_skin_title.generic.type = MTYPE_SEPARATOR;
	s_player_skin_title.generic.textSize = MENU_FONT_SIZE;
	s_player_skin_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_skin_title.generic.name = "skin";
	s_player_skin_title.generic.x    = -2*MENU_FONT_SIZE;
	s_player_skin_title.generic.y	 = y += 2*MENU_LINE_SIZE;
	
	s_player_skin_box.generic.type = MTYPE_SPINCONTROL;
	s_player_skin_box.generic.textSize = MENU_FONT_SIZE;
	s_player_skin_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_skin_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_skin_box.generic.name	= 0;
	s_player_skin_box.generic.callback = SkinCallback; // Knightmare added, was 0
	s_player_skin_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_skin_box.curvalue = currentskinindex;
	s_player_skin_box.itemnames = s_pmi[currentdirectoryindex].skindisplaynames;
	s_player_skin_box.generic.flags |= QMF_SKINLIST;
	
	s_player_hand_title.generic.type = MTYPE_SEPARATOR;
	s_player_hand_title.generic.textSize = MENU_FONT_SIZE;
	s_player_hand_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_hand_title.generic.name = "handedness";
	s_player_hand_title.generic.x    = 4*MENU_FONT_SIZE;
	s_player_hand_title.generic.y	 = y += 2*MENU_LINE_SIZE;
	
	s_player_handedness_box.generic.type = MTYPE_SPINCONTROL;
	s_player_handedness_box.generic.textSize = MENU_FONT_SIZE;
	s_player_handedness_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_handedness_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_handedness_box.generic.name	= 0;
	s_player_handedness_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_handedness_box.generic.callback = HandednessCallback;
	s_player_handedness_box.curvalue = Cvar_VariableValue( "hand" );
	s_player_handedness_box.itemnames = handedness;
	
	for (i = 0; i < sizeof(rate_tbl) / sizeof(*rate_tbl) - 1; i++)
		if (Cvar_VariableValue("rate") == rate_tbl[i])
			break;
		
	s_player_rate_title.generic.type = MTYPE_SEPARATOR;
	s_player_rate_title.generic.textSize = MENU_FONT_SIZE;
	s_player_rate_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_rate_title.generic.name = "connect speed";
	s_player_rate_title.generic.x    = 7*MENU_FONT_SIZE;
	s_player_rate_title.generic.y	 = y += 2*MENU_LINE_SIZE;
		
	s_player_rate_box.generic.type = MTYPE_SPINCONTROL;
	s_player_rate_box.generic.textSize = MENU_FONT_SIZE;
	s_player_rate_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_rate_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_rate_box.generic.name	= 0;
	s_player_rate_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_rate_box.generic.callback = RateCallback;
	s_player_rate_box.curvalue = i;
	s_player_rate_box.itemnames = rate_names;
	
	s_player_back_action.generic.type = MTYPE_ACTION;
	s_player_back_action.generic.textSize = MENU_FONT_SIZE;
	s_player_back_action.generic.name	= "back to multiplayer";
	s_player_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_back_action.generic.x	= -5*MENU_FONT_SIZE;
	s_player_back_action.generic.y	= y += 2*MENU_LINE_SIZE;
	s_player_back_action.generic.statusbar = NULL;
	s_player_back_action.generic.callback = UI_BackMenu;

	// only register model and skin on starup or when changed
	Com_sprintf( scratch, sizeof( scratch ), "players/%s/tris.md2", s_pmi[s_player_model_box.curvalue].directory );
	playermodel = R_RegisterModel( scratch );

	Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );
	playerskin = R_RegisterSkin( scratch );

	// show current weapon model (if any)
	if (currentweaponmodel && strlen(currentweaponmodel)) {
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s", s_pmi[s_player_model_box.curvalue].directory, currentweaponmodel );
		weaponmodel = R_RegisterModel( scratch );
		if (!weaponmodel) {
			Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
			weaponmodel = R_RegisterModel( scratch );
		}
	}
	else
	{
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
		weaponmodel = R_RegisterModel( scratch );
	}

	Menu_AddItem( &s_player_config_menu, &s_player_name_field );
	Menu_AddItem( &s_player_config_menu, &s_player_model_title );
	Menu_AddItem( &s_player_config_menu, &s_player_model_box );
	if ( s_player_skin_box.itemnames )
	{
		Menu_AddItem( &s_player_config_menu, &s_player_skin_title );
		Menu_AddItem( &s_player_config_menu, &s_player_skin_box );
	}
	Menu_AddItem( &s_player_config_menu, &s_player_hand_title );
	Menu_AddItem( &s_player_config_menu, &s_player_handedness_box );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_title );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_box );
	Menu_AddItem( &s_player_config_menu, &s_player_back_action );

	return true;
}


qboolean PlayerConfig_CheckIncerement (int dir, float x, float y, float w, float h)
{
	float min[2], max[2], x1, y1, w1, h1;
	char *sound = NULL;

	x1 = x;	y1 = y;	w1 = w;	h1 = h;
	SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	min[0] = x1;	max[0] = x1 + w1;
	min[1] = y1;	max[1] = y1 + h1;

	if (cursor.x>=min[0] && cursor.x<=max[0] &&
		cursor.y>=min[1] && cursor.y<=max[1] &&
		!cursor.buttonused[MOUSEBUTTON1] &&
		cursor.buttonclicks[MOUSEBUTTON1]==1)
	{
		if (dir) // dir==1 is left
		{
			if (s_player_skin_box.curvalue>0)
				s_player_skin_box.curvalue--;
		}
		else
		{
			if (s_player_skin_box.curvalue<s_pmi[s_player_model_box.curvalue].nskins)
				s_player_skin_box.curvalue++;
		}

		sound = menu_move_sound;
		cursor.buttonused[MOUSEBUTTON1] = true;
		cursor.buttonclicks[MOUSEBUTTON1] = 0;

		if ( sound )
			S_StartLocalSound( sound );
		SkinCallback (NULL);

		return true;
	}
	return false;
}


void PlayerConfig_MouseClick (void)
{
	float	icon_x = SCREEN_WIDTH*0.5 - 5, //width - 325
			icon_y = SCREEN_HEIGHT - 108,
			icon_offset = 0;
	int		i, count;
	char	*sound = NULL;
	buttonmenuobject_t buttons[NUM_SKINBOX_ITEMS];

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
		buttons[i].index =- 1;

	if (s_pmi[s_player_model_box.curvalue].nskins < NUM_SKINBOX_ITEMS || s_player_skin_box.curvalue < 4)
		i=0;
	else if (s_player_skin_box.curvalue > s_pmi[s_player_model_box.curvalue].nskins-4)
		i=s_pmi[s_player_model_box.curvalue].nskins-NUM_SKINBOX_ITEMS;
	else
		i=s_player_skin_box.curvalue-3;

	if (i > 0)
		if (PlayerConfig_CheckIncerement (1, icon_x-39, icon_y, 32, 32))
			return;

	for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
	{
		if (i<0 || i>=s_pmi[s_player_model_box.curvalue].nskins)
			continue;

		UI_AddButton (&buttons[count], i, icon_x+icon_offset, icon_y, 32, 32);
		icon_offset += 34;
	}

	icon_offset = NUM_SKINBOX_ITEMS*34;
	if (s_pmi[s_player_model_box.curvalue].nskins-i > 0)
		if (PlayerConfig_CheckIncerement (0, icon_x+icon_offset+5, icon_y, 32, 32))
			return;

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
	{
		if (buttons[i].index == -1)
			continue;

		if (cursor.x>=buttons[i].min[0] && cursor.x<=buttons[i].max[0] &&
			cursor.y>=buttons[i].min[1] && cursor.y<=buttons[i].max[1])
		{
			if (!cursor.buttonused[MOUSEBUTTON1] && cursor.buttonclicks[MOUSEBUTTON1]==1)
			{
				s_player_skin_box.curvalue = buttons[i].index;

				sound = menu_move_sound;
				cursor.buttonused[MOUSEBUTTON1] = true;
				cursor.buttonclicks[MOUSEBUTTON1] = 0;

				if (sound)
					S_StartLocalSound (sound);
				SkinCallback (NULL);

				return;
			}
			break;
		}
	}
}


void PlayerConfig_DrawSkinSelection (void)
{
	char	scratch[MAX_QPATH];
	float	icon_x = SCREEN_WIDTH*0.5 - 5; //width - 325
	float	icon_y = SCREEN_HEIGHT - 108;
	float	icon_offset = 0;
	float	x, y, w, h;
	int		i, count, color[3];

	CL_TextColor ((int)Cvar_VariableValue("alt_text_color"), &color[0], &color[1], &color[2]);

	if (s_pmi[s_player_model_box.curvalue].nskins<NUM_SKINBOX_ITEMS || s_player_skin_box.curvalue<4)
		i=0;
	else if (s_player_skin_box.curvalue > s_pmi[s_player_model_box.curvalue].nskins-4)
		i=s_pmi[s_player_model_box.curvalue].nskins-NUM_SKINBOX_ITEMS;
	else
		i=s_player_skin_box.curvalue-3;

	// left arrow
	if (i>0)
		Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left.pcx");
	else
		Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left_d.pcx");
	SCR_DrawPic (icon_x-39, icon_y+2, 32, 32,  ALIGN_CENTER, scratch, 1.0);

	// background
	SCR_DrawFill (icon_x-3, icon_y-3, NUM_SKINBOX_ITEMS*34+4, 38, ALIGN_CENTER, 0,0,0,255);
	if (R_DrawFindPic("/gfx/ui/listbox_background.pcx")) {
		x = icon_x-2;	y = icon_y-2;	w = NUM_SKINBOX_ITEMS*34+2;	h = 36;
		SCR_AdjustFrom640 (&x, &y, &w, &h, ALIGN_CENTER);
		R_DrawTileClear ((int)x, (int)y, (int)w, (int)h, "/gfx/ui/listbox_background.pcx");
	}
	else
		SCR_DrawFill (icon_x-2, icon_y-2, NUM_SKINBOX_ITEMS*34+2, 36, ALIGN_CENTER, 60,60,60,255);
		
	for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
	{
		if (i<0 || i>=s_pmi[s_player_model_box.curvalue].nskins)
			continue;

		Com_sprintf (scratch, sizeof(scratch), "/players/%s/%s_i.pcx", 
			s_pmi[s_player_model_box.curvalue].directory,
			s_pmi[s_player_model_box.curvalue].skindisplaynames[i] );

		if (i==s_player_skin_box.curvalue)
			SCR_DrawFill (icon_x + icon_offset-1, icon_y-1, 34, 34, ALIGN_CENTER, color[0],color[1],color[2],255);
		SCR_DrawPic (icon_x + icon_offset, icon_y, 32, 32,  ALIGN_CENTER, scratch, 1.0);
		icon_offset += 34;
	}

	// right arrow
	icon_offset = NUM_SKINBOX_ITEMS*34;
	if (s_pmi[s_player_model_box.curvalue].nskins-i>0)
		Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right.pcx");
	else
		Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right_d.pcx"); 
	SCR_DrawPic (icon_x+icon_offset+5, icon_y+2, 32, 32,  ALIGN_CENTER, scratch, 1.0);
}


void PlayerConfig_MenuDraw (void)
{
	refdef_t	refdef;
	float		rx, ry, rw, rh;

	Menu_DrawBanner ("m_banner_plauer_setup"); // typo for image name is id's fault

	memset(&refdef, 0, sizeof(refdef));

	rx = 0;							ry = 0;
	rw = SCREEN_WIDTH;				rh = SCREEN_HEIGHT;
	SCR_AdjustFrom640 (&rx, &ry, &rw, &rh, ALIGN_CENTER);
	refdef.x = rx;		refdef.y = ry;
	refdef.width = rw;	refdef.height = rh;
	refdef.fov_x = 50;
	refdef.fov_y = CalcFov (refdef.fov_x, refdef.width, refdef.height);
	refdef.time = cls.realtime*0.001;
 
	if ( s_pmi[s_player_model_box.curvalue].skindisplaynames )
	{
		int			yaw;
		int			maxframe = 29;
		vec3_t		modelOrg;
		// Psychopspaz's support for showing weapon model
		entity_t	entity[2], *ent;

		refdef.num_entities = 0;
		refdef.entities = entity;

		yaw = anglemod(cl.time/10);

	//	VectorSet (modelOrg, 150, (hand->value==1)?25:-25, 0); // was 80, 0, 0
		VectorSet (modelOrg, 150, (hand->integer == 1)?25:-25, 0); // was 80, 0, 0

		// Setup player model
		ent = &entity[0];
		memset (&entity[0], 0, sizeof(entity[0]));

		// moved registration code to init and change only
		ent->model = playermodel;
		ent->skin = playerskin;

		ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
	//	if (hand->value == 1)
		if (hand->integer == 1)
			ent->flags |= RF_MIRRORMODEL;

		ent->origin[0] = modelOrg[0];
		ent->origin[1] = modelOrg[1];
		ent->origin[2] = modelOrg[2];

		VectorCopy( ent->origin, ent->oldorigin );
		ent->frame = 0;
		ent->oldframe = 0;
		ent->backlerp = 0.0;
		ent->angles[1] = yaw;
		//if ( ++yaw > 360 )
		//	yaw -= 360;
		
	//	if (hand->value == 1)
		if (hand->integer == 1)
			ent->angles[1] = 360 - ent->angles[1];

		refdef.num_entities++;


		// Setup weapon model
		ent = &entity[1];
		memset (&entity[1], 0, sizeof(entity[1]));

		// moved registration code to init and change only
		ent->model = weaponmodel;

		if (ent->model)
		{
			ent->skinnum = 0;

			ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
		//	if (hand->value == 1)
			if (hand->integer == 1)
				ent->flags |= RF_MIRRORMODEL;

			ent->origin[0] = modelOrg[0];
			ent->origin[1] = modelOrg[1];
			ent->origin[2] = modelOrg[2];

			VectorCopy( ent->origin, ent->oldorigin );
			ent->frame = 0;
			ent->oldframe = 0;
			ent->backlerp = 0.0;
			ent->angles[1] = yaw;
			
		//	if (hand->value == 1)
			if (hand->integer == 1)
				ent->angles[1] = 360 - ent->angles[1];

			refdef.num_entities++;
		}


		refdef.areabits = 0;
		refdef.lightstyles = 0;
		refdef.rdflags = RDF_NOWORLDMODEL;

		Menu_Draw( &s_player_config_menu );

		// skin selection preview
		PlayerConfig_DrawSkinSelection ();

		R_RenderFrame( &refdef );
	}
}


void PConfigAccept (void)
{
	int i;
	char scratch[1024];

	Cvar_Set( "name", s_player_name_field.buffer );

	Com_sprintf( scratch, sizeof( scratch ), "%s/%s", 
		s_pmi[s_player_model_box.curvalue].directory, 
		s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );

	Cvar_Set( "skin", scratch );

	for ( i = 0; i < s_numplayermodels; i++ )
	{
		int j;

		for ( j = 0; j < s_pmi[i].nskins; j++ )
		{
			if ( s_pmi[i].skindisplaynames[j] )
				free( s_pmi[i].skindisplaynames[j] );
			s_pmi[i].skindisplaynames[j] = 0;
		}
		free( s_pmi[i].skindisplaynames );
		s_pmi[i].skindisplaynames = 0;
		s_pmi[i].nskins = 0;
	}
}

const char *PlayerConfig_MenuKey (int key)
{
	if ( key == K_ESCAPE )
		PConfigAccept();

	return Default_MenuKey( &s_player_config_menu, key );
}


void M_Menu_PlayerConfig_f (void)
{
	if (!PlayerConfig_MenuInit())
	{
		Menu_SetStatusBar( &s_multiplayer_menu, "No valid player models found" );
		return;
	}
	Menu_SetStatusBar( &s_multiplayer_menu, NULL );
	UI_PushMenu( PlayerConfig_MenuDraw, PlayerConfig_MenuKey );
}
