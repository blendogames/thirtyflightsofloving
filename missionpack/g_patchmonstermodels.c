// g_patchmonstermodels.c

#include "g_local.h"

#include "pak.h"

#define MAX_SKINS		32 // max is 32, and we need all of them!
#define MAX_SKINNAME	64

int PatchMonsterModel (char *modelname)
{
	cvar_t		*basedir = NULL;
	cvar_t		*gamedir = NULL;
	int			j = 0;
	int			numskins = 0;		// number of skin entries
	char		skins[MAX_SKINS][MAX_SKINNAME];	// skin entries
	char		infilename[MAX_OSPATH];
	char		outfilename[MAX_OSPATH];
	char		tempname[MAX_OSPATH];
	char		*p = NULL;
	FILE		*infile = NULL;
	FILE		*outfile = NULL;
	dmdl_t		model = {0};		// model header
	byte		*data = NULL;		// model data
	int			datasize = 0;		// model data size (bytes)
	int			newoffset = 0;		// model data offset (after skins)
	qboolean	is_tank = false;
	qboolean	is_soldier = false;
	// Knightmare added
#ifdef CITADELMOD_FEATURES
	qboolean	is_brain = false;
#endif
	qboolean	is_gekk = false;
	qboolean	is_fixbot = false;
	qboolean	is_chick = false;
	qboolean	is_gunner = false;
	qboolean	is_soldierh = false;
	qboolean	is_soldierq25 = false;
	qboolean	is_carrier = false;
	qboolean	is_hover = false;
	qboolean	is_medic = false;
	qboolean	is_turret = false;
	qboolean	is_vulture = false;
	qboolean	is_zboss_mech = false;
	qboolean	is_zboss_pilot = false;
	qboolean	is_q1monster = false;

	qboolean	gamedirpakfile = false;

	// get game (moddir) name
	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("game", "", 0);
	if ( !*gamedir->string )
		return 0;	// we're in baseq2

//	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
	Com_sprintf (tempname, sizeof(tempname), modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	if (outfile = fopen (outfilename, "rb"))
	{
		// output file already exists, move along
		fclose (outfile);
	//	gi.dprintf ("PatchMonsterModel: Could not save %s, file already exists\n", outfilename);
		return 0;
	}

	numskins = 8;
	// special cases
	if (!strcmp(modelname, "models/monsters/tank/tris.md2"))
	{
		is_tank = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/soldier/tris.md2"))
	{
		is_soldier = true;
		numskins = 32;	// was 24
	}
	// Knightmare added
#ifdef CITADELMOD_FEATURES
	else if (!strcmp(modelname, "models/monsters/brain/tris.md2"))
	{
		is_brain = true;
		numskins = 16;
	}
#endif
	else if (!strcmp(modelname, "models/monsters/gekk/tris.md2"))
	{
		is_gekk = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/fixbot/tris.md2"))
	{
		is_fixbot = true;
		numskins = 4;
	}
	else if (!strcmp(modelname, "models/monsters/bitch/tris.md2") ||
			!strcmp(modelname, "models/monsters/bitch2/tris.md2"))
	{
		is_chick = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/gunner/tris.md2") ||
		!strcmp(modelname, "models/monsters/gunner2/tris.md2") )
	{
		is_gunner = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/soldierh/tris.md2"))
	{
		is_soldierh = true;
		numskins = 24;
	}
	else if (!strcmp(modelname, "models/monsters/soldrq25/tris.md2"))
	{
		is_soldierq25 = true;
		numskins = 24;
	}
	else if (!strcmp(modelname, "models/monsters/carrier/tris.md2"))
	{
		is_carrier = true;
		numskins = 8;
	}
	else if (!strcmp(modelname, "models/monsters/hover/tris.md2"))
	{
		is_hover = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/medic/tris.md2"))
	{
		is_medic = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/turret/tris.md2"))
	{
		is_turret = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/vulture/tris.md2"))
	{
		is_vulture = true;
		numskins = 4;
	}
	else if (!strcmp(modelname, "models/monsters/bossz/mech/tris.md2"))
	{
		is_zboss_mech = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/bossz/pilot/tris.md2"))
	{
		is_zboss_pilot = true;
		numskins = 12;
	}
	else if ( !strcmp(modelname, "models/monsters/q1dog/tris.md2") ||  !strcmp(modelname, "models/monsters/q1enforcer/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1fiend/tris.md2") || !strcmp(modelname, "models/monsters/q1freddie/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1grunt/tris.md2") || !strcmp(modelname, "models/monsters/q1hknight/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1knight/tris.md2") || !strcmp(modelname, "models/monsters/q1ogre/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1rotfish/tris.md2") || !strcmp(modelname, "models/monsters/q1scrag/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1shalrath/tris.md2") || !strcmp(modelname, "models/monsters/q1shambler/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1tarbaby/tris.md2") || !strcmp(modelname, "models/monsters/q1zombie/tris.md2") )
	{
		is_q1monster = true;
		numskins = 4;
	}

	// end Knightmare

	for (j=0; j<numskins; j++)
	{
		memset (skins[j], 0, MAX_SKINNAME);
		Com_strcpy( skins[j], sizeof(skins[j]), modelname );
		p = strstr( skins[j], "tris.md2" );
		if (!p)
		{
			fclose (outfile);
			gi.dprintf( "Error patching %s\n",modelname);
			return 0;
		}
		*p = 0;
		if (is_soldier)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_lt.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ltp.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ss.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ssp.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_pl.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_plp.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_lt.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_lt.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_ss.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_ss.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_pl.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_pl.pcx"); break;
			case 16:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_lt.pcx"); break;
			case 17:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_lt.pcx"); break;
			case 18:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 19:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 20:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_ss.pcx"); break;
			case 21:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_ss.pcx"); break;
			case 22:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_pl.pcx"); break;
			case 23:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_pl.pcx"); break;
			case 24:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_lt.pcx"); break;
			case 25:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_lt.pcx"); break;
			case 26:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 27:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 28:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_ss.pcx"); break;
			case 29:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_ss.pcx"); break;
			case 30:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_pl.pcx"); break;
			case 31:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_pl.pcx"); break;
			}
		}
		else if (is_tank)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/skin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain3.pcx"); break;
			}
		}
		// Knightmare added
#ifdef CITADELMOD_FEATURES
		else if (is_brain)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "beta.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "betapain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
#endif
		else if (is_gekk)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "gekk.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "gekpain1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "gekpain2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1pain1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1pain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2pain1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2pain2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3pain1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3pain2.pcx"); break;
			}
		}
		else if (is_fixbot)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "droid.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		else if (is_chick)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcpy (skins[j], sizeof(skins[j]), "models/monsters/bitch/bi_sk3.pcx"); break;
			case 3:
				Com_strcpy (skins[j], sizeof(skins[j]), "models/monsters/bitch/bi_pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
		else if (is_gunner)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcpy (skins[j], sizeof(skins[j]), "tact_skin.pcx"); break;
			case 3:
				Com_strcpy (skins[j], sizeof(skins[j]), "tact_pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
		else if ( is_soldierh || is_soldierq25 )
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "sold01.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "sold01_p.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "sold02.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "sold02_p.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "sold03.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "sold03_p.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold1_p.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold2_p.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold3.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold3_p.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold1.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold1_p.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold2.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold2_p.pcx"); break;
			case 16:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold3.pcx"); break;
			case 17:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold3_p.pcx"); break;
			case 18:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold1.pcx"); break;
			case 19:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold1_p.pcx"); break;
			case 20:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold2.pcx"); break;
			case 21:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold2_p.pcx"); break;
			case 22:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold3.pcx"); break;
			case 23:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold3_p.pcx"); break;
			}
		}
		else if (is_carrier)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain2.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			}
		}
		else if (is_hover || is_medic)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "rskin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "rpain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			}
		}
		else if (is_turret)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "skin1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "skin2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_3.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_2.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_3.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_1.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_3.pcx"); break;
			}
		}
		else if (is_vulture)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "vulture.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		else if (is_zboss_mech || is_zboss_pilot)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "pain2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_p1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_p2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_p2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_p1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_p2.pcx"); break;
			}
		}
		else if (is_q1monster)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin0.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		// end Knightmare
		else
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			}
		}
	}

	// load original model from baseq2
//	Com_sprintf (infilename, sizeof(infilename), "baseq2/%s", modelname);
	// Knightmare- use basedir for compatibility on all platforms
	Com_sprintf (infilename, sizeof(infilename), "%s/baseq2/%s", basedir->string, modelname);

	// If file doesn't exist on user's hard disk, it must be in 
	// a pak file
	if ( !(infile = fopen (infilename, "rb")) )
	{
		pak_header_t	pakheader;
		pak_item_t		pakitem;
		char			pakfile[MAX_OSPATH];
		FILE			*fpak;
		int				i, k, numitems;

		// Knightmare- look in pak0-99.pak in baseq2
		for (i = 0; i < 100; i++)
		{
		//	Com_sprintf (pakfile, sizeof(pakfile), "baseq2/pak%i.pak", i);
			// Knightmare- use basedir for compatibility on all platforms
			Com_sprintf (pakfile, sizeof(pakfile), "%s/baseq2/pak%i.pak", basedir->string, i);
			fpak = fopen (pakfile, "rb");
			if ( !fpak && (i == 0) ) // if pak0.pak isn't on hard disk, try CD
			{
				cvar_t	*cddir;

				cddir = gi.cvar("cddir", "", 0);
				Com_sprintf(pakfile, sizeof(pakfile), "%s/baseq2/pak0.pak", cddir->string);
				fpak = fopen (pakfile, "rb");
				if (!fpak)
				{
					gi.dprintf("PatchMonsterModel: Cannot find pak0.pak on CD\n");
				//	return 0;
					continue;
				}
			}
			else if (!fpak) // this pak not found, go on to next
				continue;
			fread (&pakheader, 1, sizeof(pak_header_t), fpak);
			numitems = pakheader.dsize / sizeof(pak_item_t);
			fseek (fpak, pakheader.dstart, SEEK_SET);
			data = NULL;
			for (k = 0; k < numitems && !data; k++)
			{
				fread (&pakitem, 1, sizeof(pak_item_t), fpak);
				if ( !Q_stricmp(pakitem.name, modelname) )
				{
					fseek (fpak, pakitem.start, SEEK_SET);
					fread (&model, sizeof(dmdl_t), 1, fpak);
					datasize = model.ofs_end - model.ofs_skins;
					if ( !(data = malloc (datasize)) )	// make sure freed locally
					{
						fclose (fpak);
						gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
						return 0;
					}
					fread (data, sizeof (byte), datasize, fpak);
				}
			}
			fclose (fpak);
			if (data) // we found it, so stop searching
				break;
		}
		if (!data) // if not in baseq2 pak file, check pakfiles in current gamedir
		{
			char		pakname[MAX_OSPATH];

			// look in pak0-pak9.pak in current gamedir
			for (i = 0; i < 10; i++)
			{
				Com_sprintf (pakname, sizeof(pakname), "pak%i.pak", i);
				GameDirRelativePath (pakname, pakfile, sizeof(pakfile));
				fpak = fopen (pakfile, "rb");
				if (!fpak) // this pak not found, go on to next
					continue;
				fread (&pakheader, 1, sizeof(pak_header_t), fpak);
				numitems = pakheader.dsize / sizeof(pak_item_t);
				fseek (fpak, pakheader.dstart, SEEK_SET);
				data = NULL;
				for (k = 0; k < numitems && !data; k++)
				{
					fread (&pakitem, 1, sizeof(pak_item_t), fpak);
					if ( !Q_stricmp(pakitem.name, modelname) )
					{
						fseek (fpak, pakitem.start, SEEK_SET);
						fread (&model, sizeof(dmdl_t), 1, fpak);
						datasize = model.ofs_end - model.ofs_skins;
						if ( !(data = malloc (datasize)) )	// make sure freed locally
						{
							fclose (fpak);
							gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
							return 0;
						}
						fread (data, sizeof (byte), datasize, fpak);
					}
				}
				fclose (fpak);
				if (data) // we found it, so stop searching
				{
					gamedirpakfile = true;
					break;
				}
			}
			if (!data)
			{
				gi.dprintf("PatchMonsterModel: Could not find %s in pak file(s)\n",modelname);
				return 0;
			}
		}
	}
	else
	{
		fread (&model, sizeof (dmdl_t), 1, infile);
	
		datasize = model.ofs_end - model.ofs_skins;
		if ( !(data = malloc (datasize)) )	// make sure freed locally
		{
			gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
			return 0;
		}
		fread (data, sizeof (byte), datasize, infile);
	
		fclose (infile);
	}
	
	// update model info
	model.num_skins = numskins;
	
	newoffset = numskins * MAX_SKINNAME;
	model.ofs_st     += newoffset;
	model.ofs_tris   += newoffset;
	model.ofs_frames += newoffset;
	model.ofs_glcmds += newoffset;
	model.ofs_end    += newoffset;
	
	// save new model
/*	Com_sprintf (outfilename, sizeof(outfilename), "%s/models", gamedir->string);	// make some dirs if needed
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/monsters");
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
	p = strstr(outfilename,"/tris.md2");
	*p = 0;
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
*/	
	Com_sprintf (tempname, sizeof(tempname), modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	CreatePath (outfilename);

	if ( !(outfile = fopen (outfilename, "wb")) )
	{
		// file couldn't be created for some other reason
		gi.dprintf ("PatchMonsterModel: Could not save %s\n", outfilename);
		free (data);
		return 0;
	}
	
	fwrite (&model, sizeof (dmdl_t), 1, outfile);
	fwrite (skins, sizeof (char), newoffset, outfile);
	fwrite (data, sizeof (byte), datasize, outfile);
	
	fclose (outfile);

	// Knightmare- if we loaded the model from a pak file in the same gamedir,
	// then we need to insert it into a higher-numbered pakfile, otherwise it won't be loaded

	gi.dprintf ("PatchMonsterModel: Saved %s\n", outfilename);
	free (data);
	return 1;
}
