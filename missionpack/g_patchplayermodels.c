// Argh!
// 
// Add function declaration to g_local.h:
//   void PatchPlayerModels (edict_t *ent);
//
// Add function call where ever you want:
//   PatchPlayerModels (ent);


#include "g_local.h"


// id md2 stuff
#define MAX_MD2SKINS	32
#define MAX_SKINNAME	64

// Argh! - loads id baseq2/player models, "patches" their skin links 
//         for misc_actor (all id skins, and slots for 10 custom 
//         skins), and saves them to the current moddir location
//
int PatchPlayerModels (char *modelname)
{
	cvar_t	*game;
	int		j;
	int		numskins;		// number of skin entries
	char	skins[MAX_MD2SKINS][MAX_SKINNAME];	// skin entries
	char	infilename[MAX_OSPATH];
	char	outfilename[MAX_OSPATH];
	char	tempname[MAX_OSPATH];
	FILE	*infile;
	FILE	*outfile;
	dmdl_t	model;		// model header
	byte	*data;		// model data
	int		datasize;	// model data size (bytes)
	int		newoffset;	// model data offset (after skins)

	// get game (moddir) name
	game = gi.cvar("game", "", 0);
	if (!*game->string)
		return 0;	// we're in baseq2

//	Com_sprintf (outfilename, sizeof(outfilename), "%s/players/%s/tris.md2", game->string, modelname);
	Com_sprintf (tempname, sizeof(tempname), "players/%s/tris.md2", modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	if (outfile = fopen (outfilename, "rb"))
	{
		// output file already exists, move along
		fclose (outfile);
	//	gi.dprintf ("PatchPlayerModels: Could not save %s, file already exists\n", outfilename);
		return 0;
	}

	// clear skin names (just in case)
	for (j = 0; j < MAX_MD2SKINS; j++)
		memset (skins[j], 0, MAX_SKINNAME);

	// set model-specific data
	if (!Q_stricmp(modelname,"male"))
	{
		numskins = 15;
		Com_sprintf (skins[0],	sizeof(skins[0]), "players/male/cipher.pcx");
		Com_sprintf (skins[1],	sizeof(skins[1]), "players/male/claymore.pcx");
		Com_sprintf (skins[2],	sizeof(skins[2]), "players/male/flak.pcx");
		Com_sprintf (skins[3],	sizeof(skins[3]), "players/male/grunt.pcx");
		Com_sprintf (skins[4],	sizeof(skins[4]), "players/male/howitzer.pcx");
		Com_sprintf (skins[5],	sizeof(skins[5]), "players/male/major.pcx");
		Com_sprintf (skins[6],	sizeof(skins[6]), "players/male/nightops.pcx");
		Com_sprintf (skins[7],	sizeof(skins[7]), "players/male/pointman.pcx");
		Com_sprintf (skins[8],	sizeof(skins[8]), "players/male/psycho.pcx");
		Com_sprintf (skins[9],	sizeof(skins[9]), "players/male/rampage.pcx");
		Com_sprintf (skins[10], sizeof(skins[10]), "players/male/razor.pcx");
		Com_sprintf (skins[11], sizeof(skins[11]), "players/male/recon.pcx");
		Com_sprintf (skins[12], sizeof(skins[12]), "players/male/scout.pcx");
		Com_sprintf (skins[13], sizeof(skins[13]), "players/male/sniper.pcx");
		Com_sprintf (skins[14], sizeof(skins[14]), "players/male/viper.pcx");
	}
	else if (!Q_stricmp(modelname,"female"))
	{
		numskins = 10;
		Com_sprintf (skins[0],  sizeof(skins[0]), "players/female/athena.pcx");
		Com_sprintf (skins[1],  sizeof(skins[1]), "players/female/brianna.pcx");
		Com_sprintf (skins[2],  sizeof(skins[2]), "players/female/cobalt.pcx");
		Com_sprintf (skins[3],  sizeof(skins[3]), "players/female/ensign.pcx");
		Com_sprintf (skins[4],  sizeof(skins[4]), "players/female/jezebel.pcx");
		Com_sprintf (skins[5],  sizeof(skins[5]), "players/female/jungle.pcx");
		Com_sprintf (skins[6],  sizeof(skins[6]), "players/female/lotus.pcx");
		Com_sprintf (skins[7],  sizeof(skins[7]), "players/female/stiletto.pcx");
		Com_sprintf (skins[8],  sizeof(skins[8]), "players/female/venus.pcx");
		Com_sprintf (skins[9],  sizeof(skins[9]), "players/female/voodoo.pcx");
	}
	else if (!Q_stricmp(modelname,"cyborg"))
	{
		numskins = 3;
		Com_sprintf (skins[0],  sizeof(skins[0]), "players/cyborg/oni911.pcx");
		Com_sprintf (skins[1],  sizeof(skins[1]), "players/cyborg/ps9000.pcx");
		Com_sprintf (skins[2],  sizeof(skins[2]), "players/cyborg/tyr574.pcx");
	}
	else
		return 0;

	// fill in 32 slots with "customXX"
	for (j=numskins; j<32; j++)
		Com_sprintf( skins[j], sizeof(skins[j]), "players/%s/custom%d.pcx", modelname, j-numskins+1);
	numskins = 32;

	// load original player model
	Com_sprintf (infilename, sizeof(infilename), "baseq2/players/%s/tris.md2", modelname);
	if ( !(infile = fopen (infilename, "rb")) )
		return 0;	// no player model (this shouldn't happen)
	
	fread (&model, sizeof (dmdl_t), 1, infile);
	
	datasize = model.ofs_end - model.ofs_skins;
	if ( !(data = malloc (datasize)) )	// make sure freed locally
	{
		gi.dprintf ("PatchPlayerModels: Could not allocate memory for model\n");
		return 0;
	}
	fread (data, sizeof (byte), datasize, infile);
	
	fclose (infile);
	
	// update model info
	model.num_skins = numskins;
	
	newoffset = numskins * MAX_SKINNAME;
	model.ofs_st     += newoffset;
	model.ofs_tris   += newoffset;
	model.ofs_frames += newoffset;
	model.ofs_glcmds += newoffset;
	model.ofs_end    += newoffset;
	
	// save new player model
/*	Com_sprintf (outfilename, sizeof(outfilename), "%s/players", game->string);	// make some dirs if needed
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/players/%s", game->string, modelname);
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/players/%s/tris.md2", game->string, modelname);
*/	
	Com_sprintf (tempname, sizeof(tempname), "players/%s/tris.md2", modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	CreatePath (outfilename);

	if ( !(outfile = fopen (outfilename, "wb")) )
	{
		// file couldn't be created for some other reason
		gi.dprintf ("PatchPlayerModels: Could not save %s\n", outfilename);
		free (data);
		return 0;
	}
	
	fwrite (&model, sizeof (dmdl_t), 1, outfile);
	fwrite (skins, sizeof (char), newoffset, outfile);
	fwrite (data, sizeof (byte), datasize, outfile);
	
	fclose (outfile);
	gi.dprintf ("PatchPlayerModels: Saved %s\n", outfilename);
	free (data);
	return 1;
}
