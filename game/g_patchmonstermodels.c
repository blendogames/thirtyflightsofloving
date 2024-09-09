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

#include "pak.h"

#define MAX_SKINS		24 // max is 32, but we only need 24
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
	if (!strcmp(modelname,"models/monsters/tank/tris.md2"))
	{
		is_tank = true;
		numskins = 16;
	}
	else if (!strcmp(modelname,"models/monsters/soldier/tris.md2"))
	{
		is_soldier = true;
		numskins = 24;
	}

	for (j=0; j<numskins; j++)
	{
		memset (skins[j], 0, MAX_SKINNAME);
	//	strncpy( skins[j], modelname );
		Q_strncpyz (skins[j], sizeof(skins[j]), modelname);
		p = strstr(skins[j], "tris.md2");
		if (!p)
		{
			fclose (outfile);
			gi.dprintf( "Error patching %s\n",modelname);
			return 0;
		}
		*p = 0;
		if (is_soldier)
		{
			switch (j) {
			case 0:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin_lt.pcx"); break;
			case 1:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin_ltp.pcx"); break;
			case 2:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 3:
				Q_strncatz (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 4:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin_ss.pcx"); break;
			case 5:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin_ssp.pcx"); break;
			case 6:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom1_lt.pcx"); break;
			case 7:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain1_lt.pcx"); break;
			case 8:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 9:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 10:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom1_ss.pcx"); break;
			case 11:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain1_ss.pcx"); break;
			case 12:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom2_lt.pcx"); break;
			case 13:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain2_lt.pcx"); break;
			case 14:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 15:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 16:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom2_ss.pcx"); break;
			case 17:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain2_ss.pcx"); break;
			case 18:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom3_lt.pcx"); break;
			case 19:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain3_lt.pcx"); break;
			case 20:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 21:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 22:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom3_ss.pcx"); break;
			case 23:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain3_ss.pcx"); break;
			}
		}
		else if (is_tank)
		{
			switch (j) {
			case 0:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Q_strncatz (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/skin.pcx"); break;
			case 3:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/pain.pcx"); break;
			case 4:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custom1.pcx"); break;
			case 7:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custompain1.pcx"); break;
			case 8:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custom2.pcx"); break;
			case 11:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custompain2.pcx"); break;
			case 12:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custom3.pcx"); break;
			case 15:
				Q_strncatz (skins[j], sizeof(skins[j]), "../ctank/custompain3.pcx"); break;
			}
		}
		else
		{
			switch (j) {
			case 0:
				Q_strncatz (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Q_strncatz (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 3:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 4:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 5:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 6:
				Q_strncatz (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 7:
				Q_strncatz (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
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
				Com_sprintf (pakfile, sizeof(pakfile), "%s/baseq2/pak0.pak", cddir->string);
				fpak = fopen (pakfile, "rb");
				if (!fpak)
				{
					gi.dprintf("PatchMonsterModel: Cannot find pak0.pak\n");
					return 0;
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
						fseek (fpak,pakitem.start, SEEK_SET);
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
		}
		if (!data)
		{
			gi.dprintf ("PatchMonsterModel: Could not find %s in baseq2/pak0.pak\n",modelname);
			return 0;
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
	Q_strncatz (outfilename, sizeof(outfilename), "/monsters");
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
	gi.dprintf ("PatchMonsterModel: Saved %s\n", outfilename);
	free (data);
	return 1;
}
