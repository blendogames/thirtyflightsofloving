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
// cl_view.c -- player rendering positioning

#include "client.h"

//=============
//
// development tools for weapons
//
int			gun_frame;
struct model_s	*gun_model;

//=============

cvar_t		*crosshair;
//Knightmare- Psychospaz's scalable corsshair
cvar_t		*crosshair_scale;

cvar_t		*cl_testparticles;
cvar_t		*cl_testentities;
cvar_t		*cl_testlights;
cvar_t		*cl_testblend;

cvar_t		*cl_stats;

cvar_t		*hand;


int			r_numdlights;
dlight_t	r_dlights[MAX_DLIGHTS];

int			r_numentities;
entity_t	r_entities[MAX_ENTITIES];

int			r_numparticles;
particle_t	r_particles[MAX_PARTICLES];

#ifdef DECALS
int			r_numdecalfrags;
particle_t	r_decalfrags[MAX_DECAL_FRAGS];
#endif

lightstyle_t	r_lightstyles[MAX_LIGHTSTYLES];

char cl_weaponmodels[MAX_CLIENTWEAPONMODELS][MAX_QPATH];
int num_cl_weaponmodels;

/*
====================
V_ClearScene

Specifies the model that will be used as the world
====================
*/
void V_ClearScene (void)
{
	r_numdlights = 0;
	r_numentities = 0;
	r_numparticles = 0;
#ifdef DECALS
	r_numdecalfrags = 0;
#endif
}

// Knightmare- added Psychospaz's chasecam
/*
=====================

3D Cam Stuff -psychospaz

=====================
*/
#define CAM_MAXALPHADIST 0.000111
void vectoangles2 (vec3_t value1, vec3_t angles);
float viewermodelalpha;

void ClipCam (vec3_t start, vec3_t end, vec3_t newpos)
{
	int i;

	trace_t tr = CL_Trace (start, end, 5, MASK_SOLID);
	for (i=0;i<3;i++)
		newpos[i]=tr.endpos[i];
}

void AddViewerEntAlpha (entity_t *ent)
{
	if (viewermodelalpha == 1 || !cl_3dcam_alpha->value)
		return;

	ent->alpha *= viewermodelalpha;
	if (ent->alpha < 1.0F)
		ent->flags |= RF_TRANSLUCENT;
}

#define ANGLEMAX 90.0
void CalcViewerCamTrans (float distance)
{
	float alphacalc = cl_3dcam_dist->value;

	// no div by 0
	if (alphacalc < 1)
		alphacalc = 1;

	viewermodelalpha = distance/alphacalc;

	if (viewermodelalpha>1)
		viewermodelalpha = 1;
}


/*
=====================
V_AddEntity
=====================
*/
void V_AddEntity (entity_t *ent)
{
	// Knightmare- added Psychospaz's chasecam
	if (ent->flags & RF_VIEWERMODEL) //here is our client
	{	int i; 

		// what was i thinking before!?
		for (i=0;i<3;i++)
			clientOrg[i] = ent->oldorigin[i] = ent->origin[i] = cl.predicted_origin[i];

		if (hand->value == 1) //lefthanded
			ent->flags |= RF_MIRRORMODEL;

		if (cl_3dcam->value
			&& !(cl.attractloop && !(cl.cinematictime > 0 && cls.realtime - cl.cinematictime > 1000)))
		{
			AddViewerEntAlpha(ent);
			ent->flags &= ~RF_VIEWERMODEL;
			ent->renderfx |= RF2_CAMERAMODEL;
		}
	}
	// end Knightmare
	if (r_numentities >= MAX_ENTITIES)
		return;
	r_entities[r_numentities++] = *ent;
}


/*
=====================
V_AddParticle
=====================
*/
//Knightmare- Psychospaz's enhanced particle code
void V_AddParticle (vec3_t org, vec3_t angle, vec3_t color, float alpha,
				int alpha_src, int alpha_dst, float size, int image, int flags)
{
	int i;
	particle_t	*p;

	if (r_numparticles >= MAX_PARTICLES)
		return;
	p = &r_particles[r_numparticles++];

	for (i=0;i<3;i++)
	{
		p->origin[i] = org[i];
		p->angle[i] = angle[i];
	}
	p->red = color[0];
	p->green = color[1];
	p->blue = color[2];
	p->alpha = alpha;
	p->image = image;
	p->flags = flags;
	p->size  = size;
#ifdef DECALS
	p->decal = NULL;
#endif
	p->blendfunc_src = alpha_src;
	p->blendfunc_dst = alpha_dst;
}
//end Knightmare

/*
=====================
V_AddDecal
=====================
*/
#ifdef DECALS
void V_AddDecal (vec3_t org, vec3_t angle, vec3_t color, float alpha,
				int alpha_src, int alpha_dst, float size, int image, int flags, decalpolys_t *decal)
{
	int i;
	particle_t	*d;

	if (r_numdecalfrags >= MAX_DECAL_FRAGS)
		return;
	d = &r_decalfrags[r_numdecalfrags++];

	for (i=0;i<3;i++)
	{
		d->origin[i] = org[i];
		d->angle[i] = angle[i];
	}
	d->red = color[0];
	d->green = color[1];
	d->blue = color[2];
	d->alpha = alpha;
	d->image = image;
	d->flags = flags;
	d->size  = size;
	d->decal = decal;

	d->blendfunc_src = alpha_src;
	d->blendfunc_dst = alpha_dst;
}
#endif

/*
=====================
V_AddLight

=====================
*/
void V_AddLight (vec3_t org, float intensity, float r, float g, float b)
{
	dlight_t	*dl;

	if (r_numdlights >= MAX_DLIGHTS)
		return;
	dl = &r_dlights[r_numdlights++];
	VectorCopy (org, dl->origin);
	dl->intensity = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
	VectorCopy (vec3_origin, dl->direction);
	dl->spotlight = false;
}

/*
=====================
V_AddSpotLight

=====================
*/
void V_AddSpotLight (vec3_t org, vec3_t direction, float intensity, float r, float g, float b)
{
	dlight_t	*dl;

	if (r_numdlights >= MAX_DLIGHTS)
		return;
	dl = &r_dlights[r_numdlights++];
	VectorCopy (org, dl->origin);
	VectorCopy(direction, dl->direction);
	dl->intensity = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;

	dl->spotlight=true;
}

/*
=====================
V_AddLightStyle

=====================
*/
void V_AddLightStyle (int style, float r, float g, float b)
{
	lightstyle_t	*ls;

	if (style < 0 || style > MAX_LIGHTSTYLES)
		Com_Error (ERR_DROP, "Bad light style %i", style);
	ls = &r_lightstyles[style];

	ls->white = r+g+b;
	ls->rgb[0] = r;
	ls->rgb[1] = g;
	ls->rgb[2] = b;
}

/*
================
V_TestParticles

If cl_testparticles is set, create 4096 particles in the view
================
*/
void V_TestParticles (void)
{
	particle_t	*p;
	int			i, j;
	float		d, r, u;

	r_numparticles = 4096;
	for (i=0 ; i<r_numparticles ; i++)
	{
		d = i*0.25;
		r = 4*((i&7)-3.5);
		u = 4*(((i>>3)&7)-3.5);
		p = &r_particles[i];

		for (j=0; j<3; j++)
			p->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j]*d +
			cl.v_right[j]*r + cl.v_up[j]*u;

		p->color = 8;
		p->alpha = cl_testparticles->value;
	}
}

/*
================
V_TestEntities

If cl_testentities is set, create 32 player models
================
*/
void V_TestEntities (void)
{
	int			i, j;
	float		f, r;
	entity_t	*ent;

	r_numentities = 32;
	memset (r_entities, 0, sizeof(r_entities));

	for (i=0 ; i<r_numentities ; i++)
	{
		ent = &r_entities[i];

		r = 64 * ( (i%4) - 1.5 );
		f = 64 * (i/4) + 128;

		for (j=0; j<3; j++)
			ent->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j]*f +
			cl.v_right[j]*r;

		ent->model = cl.baseclientinfo.model;
		ent->skin = cl.baseclientinfo.skin;
	}
}

/*
================
V_TestLights

If cl_testlights is set, create 32 lights models
================
*/
void V_TestLights (void)
{
	int			i, j;
	float		f, r;
	dlight_t	*dl;

	r_numdlights = 32;
	memset (r_dlights, 0, sizeof(r_dlights));

	for (i=0 ; i<r_numdlights ; i++)
	{
		dl = &r_dlights[i];

		r = 64 * ( (i%4) - 1.5 );
		f = 64 * (i/4) + 128;

		for (j=0; j<3; j++)
			dl->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j]*f +
			cl.v_right[j]*r;
		dl->color[0] = ((i%6)+1) & 1;
		dl->color[1] = (((i%6)+1) & 2)>>1;
		dl->color[2] = (((i%6)+1) & 4)>>2;
		dl->intensity = 200;
		dl->spotlight = false;
	}
}

//===================================================================

/*
=================
CL_PrepRefresh

Call before entering a new level, or after changing dlls
=================
*/
qboolean needLoadingPlaque (void);
void CL_PrepRefresh (void)
{
	char		mapname[32];
	int			i, max;
	char		pname[MAX_QPATH];
	float		rotate;
	vec3_t		axis;
	qboolean	newPlaque = needLoadingPlaque();

	if (!cl.configstrings[CS_MODELS+1][0])
		return;		// no map loaded

	if (newPlaque)
		SCR_BeginLoadingPlaque();

	// Knightmare- for Psychospaz's map loading screen
	loadingMessage = true;
	Com_sprintf (loadingMessages, sizeof(loadingMessages), S_COLOR_ALT"loading %s", cl.configstrings[CS_MODELS+1]);
	loadingPercent = 0;
	// end Knightmare

	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width-1, viddef.height-1);

	// let the render dll load the map
	strcpy (mapname, cl.configstrings[CS_MODELS+1] + 5);	// skip "maps/"
	mapname[strlen(mapname)-4] = 0;		// cut off ".bsp"

	// register models, pics, and skins
	Com_Printf ("Map: %s\r", mapname); 
	SCR_UpdateScreen ();
	R_BeginRegistration (mapname);
	Com_Printf ("                                     \r");

	// Knightmare- for Psychospaz's map loading screen
	Com_sprintf (loadingMessages, sizeof(loadingMessages), S_COLOR_ALT"loading models...");
	loadingPercent += 20;
	// end Knightmare

	// precache status bar pics
	Com_Printf ("pics\r"); 
	SCR_UpdateScreen ();
	SCR_TouchPics ();
	Com_Printf ("                                     \r");

	CL_RegisterTEntModels ();

	num_cl_weaponmodels = 1;
	strcpy(cl_weaponmodels[0], "weapon.md2");

	// Knightmare- for Psychospaz's map loading screen
	for (i=1, max=0 ; i<MAX_MODELS && cl.configstrings[CS_MODELS+i][0] ; i++)
		max++;

	for (i=1; i<MAX_MODELS && cl.configstrings[CS_MODELS+i][0]; i++)
	{
		strcpy (pname, cl.configstrings[CS_MODELS+i]);
		pname[37] = 0;	// never go beyond one line
		if (pname[0] != '*')
		{
			Com_Printf ("%s\r", pname); 
			// Knightmare- for Psychospaz's map loading screen
			//only make max of 40 chars long
			if (i > 1)
				Com_sprintf (loadingMessages, sizeof(loadingMessages),
					S_COLOR_ALT"loading %s", (strlen(pname)>40)? &pname[strlen(pname)-40]: pname);
		}

		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		if (pname[0] == '#')
		{
			// special player weapon model
			if (num_cl_weaponmodels < MAX_CLIENTWEAPONMODELS)
			{
				strncpy(cl_weaponmodels[num_cl_weaponmodels], cl.configstrings[CS_MODELS+i]+1,
					sizeof(cl_weaponmodels[num_cl_weaponmodels]) - 1);
				num_cl_weaponmodels++;
			}
		} 
		else
		{
			cl.model_draw[i] = R_RegisterModel (cl.configstrings[CS_MODELS+i]);
			if (pname[0] == '*')
				cl.model_clip[i] = CM_InlineModel (cl.configstrings[CS_MODELS+i]);
			else
				cl.model_clip[i] = NULL;
		}
		if (pname[0] != '*')
			Com_Printf ("                                     \r");
		// Knightmare- for Psychospaz's map loading screen
		loadingPercent += 40.0f/(float)max;
	}
	// Knightmare- for Psychospaz's map loading screen
	Com_sprintf (loadingMessages, sizeof(loadingMessages), S_COLOR_ALT"loading pics...");

	Com_Printf ("images\r", i); 
	SCR_UpdateScreen ();

	// Knightmare- BIG UGLY HACK for connected to server using old protocol
	// Changed configstrings require different parsing
	if (LegacyProtocol() )
	{	// Knightmare- for Psychospaz's map loading screen
		for (i=1, max=0; i<OLD_MAX_IMAGES && cl.configstrings[OLD_CS_IMAGES+i][0]; i++)
			max++;
		for (i=1; i<OLD_MAX_IMAGES && cl.configstrings[OLD_CS_IMAGES+i][0]; i++)
		{
			cl.image_precache[i] = R_DrawFindPic (cl.configstrings[OLD_CS_IMAGES+i]);
			Sys_SendKeyEvents ();	// pump message loop
			// Knightmare- for Psychospaz's map loading screen
			loadingPercent += 20.0f/(float)max;
		}
	}
	else
	{	// Knightmare- for Psychospaz's map loading screen
		for (i=1, max=0; i<MAX_IMAGES && cl.configstrings[CS_IMAGES+i][0]; i++)
			max++;
		for (i=1; i<MAX_IMAGES && cl.configstrings[CS_IMAGES+i][0]; i++)
		{
			cl.image_precache[i] = R_DrawFindPic (cl.configstrings[CS_IMAGES+i]);
			Sys_SendKeyEvents ();	// pump message loop
			// Knightmare- for Psychospaz's map loading screen
			loadingPercent += 20.0f/(float)max;
		}
	}
	// Knightmare- for Psychospaz's map loading screen
	Com_sprintf (loadingMessages, sizeof(loadingMessages), S_COLOR_ALT"loading players...");

	Com_Printf ("                                     \r");

	// Knightmare- for Psychospaz's map loading screen
	for (i=1, max=0 ; i<MAX_CLIENTS ; i++)
		if ( LegacyProtocol() ) {
			if (cl.configstrings[OLD_CS_PLAYERSKINS+i][0])
				max++;
		} else {
			if (cl.configstrings[CS_PLAYERSKINS+i][0])
				max++;
		}

	for (i=0; i < MAX_CLIENTS; i++)
	{
		// Knightmare- BIG UGLY HACK for old connected to server using old protocol
		// Changed configstrings require different parsing
		if ( LegacyProtocol() ) {
			if (!cl.configstrings[OLD_CS_PLAYERSKINS+i][0])
				continue;
		} else {
			if (!cl.configstrings[CS_PLAYERSKINS+i][0])
				continue;
		}
		Com_Printf ("client %i\r", i); 
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		CL_ParseClientinfo (i);
		Com_Printf ("                                     \r");

		// Knightmare- for Psychospaz's map loading screen
		loadingPercent += 20.0f/(float)max;
	}
	// Knightmare- for Psychospaz's map loading screen
	Com_sprintf (loadingMessages, sizeof(loadingMessages), S_COLOR_ALT"loading players...done");
	//hack hack hack - psychospaz
	loadingPercent = 100;

	// Knightmare - Vics fix to get rid of male/grunt flicker
	// CL_LoadClientinfo (&cl.baseclientinfo, "unnamed\\male/grunt");
	CL_LoadClientinfo (&cl.baseclientinfo, va("unnamed\\%s", skin->string));

	// Knightmare- refresh the player model/skin info
	userinfo_modified = true;

	// set sky textures and speed
	Com_Printf ("sky\r", i); 
	SCR_UpdateScreen ();
	rotate = atof (cl.configstrings[CS_SKYROTATE]);
	sscanf (cl.configstrings[CS_SKYAXIS], "%f %f %f", 
		&axis[0], &axis[1], &axis[2]);
	R_SetSky (cl.configstrings[CS_SKY], rotate, axis);
	Com_Printf ("                                     \r");

	// the renderer can now free unneeded stuff
	R_EndRegistration ();

	// clear any lines of console text
	Con_ClearNotify ();

	SCR_UpdateScreen ();
	cl.refresh_prepped = true;
	cl.force_refdef = true;	// make sure we have a valid refdef

	// start the cd track
	CL_PlayBackgroundTrack ();

	// Knightmare- for Psychospaz's map loading screen
	loadingMessage = false;
	// Knightmare- close loading screen as soon as done
	cls.disable_screen = false;

	// Knightmare- don't start map with game paused
	if (cls.key_dest != key_menu)
		Cvar_Set ("paused", "0");
}

/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179)
		Com_Error (ERR_DROP, "Bad fov: %f", fov_x);

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
}

//============================================================================

// gun frame debugging functions
void V_Gun_Next_f (void)
{
	gun_frame++;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Prev_f (void)
{
	gun_frame--;
	if (gun_frame < 0)
		gun_frame = 0;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Model_f (void)
{
	char	name[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		gun_model = NULL;
		return;
	}
	Com_sprintf (name, sizeof(name), "models/%s/tris.md2", Cmd_Argv(1));
	gun_model = R_RegisterModel (name);
}

//============================================================================




char *SCR_FindKey (char *key)
{
	static char	mapmsg[16];
	int			k;
	qboolean	found_bind=false;

	for (k=0 ; k<256 ; k++)
	{
		if (keybindings[k] && keybindings[k][0])
		{						
			if (!Q_stricmp(keybindings[k], key))
			{							
				Com_sprintf (mapmsg, sizeof(mapmsg), "%s", Key_KeynumToString(k));
				found_bind = true;
			}
		}
	}

	if (!found_bind)
	{
		//no key bound to action. Give you question marks!
		Com_sprintf (mapmsg, sizeof(mapmsg), "^1?");
	}

	if (mapmsg)
	{
		//return the final result:					
		return mapmsg;		
	}
}


/*
=================
SCR_DrawCrosshair
=================
*/
#define DIV640 0.0015625


//Knightmare- Psychospaz's new crosshair code
void SCR_DrawCrosshair (void)
{	
	float scale;
	float alpha;



	if (cl.frame.playerstate.stats[STAT_HINTTITLE] > 0)
		alpha = 0;
	else
		alpha = 1;

	//DOn't draw xhair if player is dead!
	if ((!crosshair->value) || (cl.frame.playerstate.stats[STAT_HEALTH] <= 0) || (cl.frame.playerstate.stats[STAT_HUDMSG] == 1) ||  (cl.frame.playerstate.stats[STAT_SELECTED_ITEM] == 12))
		return;




	if (crosshair->modified)
	{
		crosshair->modified = false;
		SCR_TouchPics ();

		if (!strcmp("dday",Cvar_Get ("game", "0", CVAR_ARCHIVE)->string)) //dday has no crosshair (FORCED)
			Cvar_SetValue("crosshair", 0);
	}

	if (crosshair_scale->modified)
	{
		crosshair_scale->modified=false;
		if (crosshair_scale->value>5)
			Cvar_SetValue("crosshair_scale", 5);
		else if (crosshair_scale->value<0.25)
			Cvar_SetValue("crosshair_scale", 0.25);
	}

	if (!crosshair_pic[0])
		return;

	scale = crosshair_scale->value * (viddef.width*DIV640);

	
	if (cl.frame.playerstate.stats[STAT_USEABLE] < 1)
	{
		R_DrawScaledPic (scr_vrect.x + ((scr_vrect.width - scale*crosshair_width)/2), // width
			scr_vrect.y + ((scr_vrect.height - scale*crosshair_height)/2),	// height
			scale,	// scale
			alpha,
			//0.75 + 0.25*sin(anglemod(cl.time*0.005)),	// alpha
			crosshair_pic); // pic
	}
	else
	{
		int x,y;
		const char *keymsg;

		x = scr_vrect.width * 0.035f;
		y = scr_vrect.height * 0.08f;

		R_DrawStretchPic ((scr_vrect.width * 0.5f)-(x*0.5f), (scr_vrect.height * 0.5f)-(y*0.5f), x, y, "/gfx/m_cur_hover.tga", 1.0);


		
		/*
		x = scr_vrect.width * 0.03f;
		
		R_DrawStretchPic (
			(scr_vrect.width * 0.5f)-(x*0.5f),
			(scr_vrect.height * 0.52f)-(y*0.5f),
			x, x,
			"/pics/letter_e.tga", 1.0);
		*/

				
		
		keymsg = SCR_FindKey("+use",true);

		//Com_Printf ("%s\n",keymsg);				

		if (keymsg)				
		{
			/*
			if (!strcmp(keymsg, "MOUSE2"))
			{
				x = scr_vrect.width * 0.03f;

				R_DrawStretchPic (
					scr_vrect.width * 0.49f,
					scr_vrect.height * 0.535f,
					x, x,
					"/pics/letter_mouse2.tga", 1.0);
			}
			else*/ if (!strcmp(keymsg, "e"))
			{
				Com_sprintf (keymsg, sizeof(keymsg), "E");

				Menu_DrawString(
					318,
					257, 
					keymsg, 255);
			}
			else
			{
				Menu_DrawString(
					318,
					257, 
					keymsg, 255);
			}

		}
	}
}




/*
==================
V_RenderView

==================
*/
void V_RenderView (float stereo_separation)
{
	extern int entitycmpfnc( const entity_t *, const entity_t * );
	float f; // Barnes added

	if (cls.state != ca_active)
		return;

	if (!cl.refresh_prepped)
		return;			// still loading

	if (cl_timedemo->value)
	{
		if (!cl.timedemo_start)
			cl.timedemo_start = Sys_Milliseconds ();
		cl.timedemo_frames++;
	}

	// an invalid frame will just use the exact previous refdef
	// we can't use the old frame if the video mode has changed, though...
	if ( cl.frame.valid && (cl.force_refdef || !cl_paused->value) )
	{
		cl.force_refdef = false;

		V_ClearScene ();

		// build a refresh entity list and calc cl.sim*
		// this also calls CL_CalcViewValues which loads
		// v_forward, etc.
		CL_AddEntities ();

		if (cl_testparticles->value)
			V_TestParticles ();
		if (cl_testentities->value)
			V_TestEntities ();
		if (cl_testlights->value)
			V_TestLights ();
		if (cl_testblend->value)
		{
			cl.refdef.blend[0] = 1;
			cl.refdef.blend[1] = 0.5;
			cl.refdef.blend[2] = 0.25;
			cl.refdef.blend[3] = 0.5;
		}

		// offset vieworg appropriately if we're doing stereo separation
		if ( stereo_separation != 0 )
		{
			vec3_t tmp;

			VectorScale( cl.v_right, stereo_separation, tmp );
			VectorAdd( cl.refdef.vieworg, tmp, cl.refdef.vieworg );
		}

		// never let it sit exactly on a node line, because a water plane can
		// dissapear when viewed with the eye exactly on it.
		// the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
		cl.refdef.vieworg[0] += 1.0/16;
		cl.refdef.vieworg[1] += 1.0/16;
		cl.refdef.vieworg[2] += 1.0/16;

		cl.refdef.x = scr_vrect.x;
		cl.refdef.y = scr_vrect.y;
		cl.refdef.width = scr_vrect.width;
		cl.refdef.height = scr_vrect.height;

		// adjust fov for wide aspect ratio
		if (cl_widescreen_fov->value)
		{
			float standardRatio, currentRatio;
			standardRatio = (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT;
			currentRatio = (float)cl.refdef.width/(float)cl.refdef.height;
			if (currentRatio > standardRatio)
				cl.refdef.fov_x *= (currentRatio / standardRatio);
		}

		cl.refdef.fov_y = CalcFov (cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);
		cl.refdef.time = cl.time*0.001;

		// Barnes- Warp if underwater ala q3a :-)
		if (cl.refdef.rdflags & RDF_UNDERWATER) {
			f = sin(cl.time * 0.001 * 0.4 * (M_PI*2.7));
			cl.refdef.fov_x += f * (cl.refdef.fov_x/90.0); // Knightmare- scale to fov
			cl.refdef.fov_y -= f * (cl.refdef.fov_y/90.0); // Knightmare- scale to fov
		} // end Barnes

		cl.refdef.areabits = cl.frame.areabits;

		if (!cl_add_entities->value)
			r_numentities = 0;
		if (!cl_add_particles->value)
			r_numparticles = 0;
		if (!cl_add_lights->value)
			r_numdlights = 0;
		if (!cl_add_blend->value)
		{
			VectorClear (cl.refdef.blend);
		}

		cl.refdef.num_entities = r_numentities;
		cl.refdef.entities = r_entities;
		cl.refdef.num_particles = r_numparticles;
		cl.refdef.particles = r_particles;

	#ifdef DECALS
		cl.refdef.num_decals = r_numdecalfrags;
		cl.refdef.decals = r_decalfrags;
	#endif

		cl.refdef.num_dlights = r_numdlights;
		cl.refdef.dlights = r_dlights;
		cl.refdef.lightstyles = r_lightstyles;

		cl.refdef.rdflags = cl.frame.playerstate.rdflags;
        qsort( cl.refdef.entities, cl.refdef.num_entities, sizeof( cl.refdef.entities[0] ), (int (*)(const void *, const void *))entitycmpfnc );
	}

#ifdef LIGHT_BLOOMS
	cl.refdef.rdflags |= RDF_BLOOM;   //BLOOMS
#endif

	//bc uggghh hack
	cl.refdef.bloomintensity = cl.frame.playerstate.stats[STAT_BLOOMINTENSITY] / 100.0;

	cl.refdef.bloomalpha = cl.frame.playerstate.stats[STAT_BLOOMALPHA] / 1000.0;
	cl.refdef.bloomdarken = cl.frame.playerstate.stats[STAT_BLOOMDARKEN];
	

	//Com_Printf("redner %f   Stat:  %f\n",cl.refdef.bloomintensity ,  cl.frame.playerstate.stats[STAT_BLOOMINTENSITY]);

	//Com_Printf("redner %f   Stat:  %f\n", cl.refdef.bloomdarken ,  cl.frame.playerstate.stats[STAT_BLOOMDARKEN]);



	

	R_RenderFrame (&cl.refdef);
	if (cl_stats->value)
		Com_Printf ("ent:%i  lt:%i  part:%i\n", r_numentities, r_numdlights, r_numparticles);
	if ( log_stats->value && ( log_stats_file != 0 ) )
		fprintf( log_stats_file, "%i,%i,%i,",r_numentities, r_numdlights, r_numparticles);


	SCR_AddDirtyPoint (scr_vrect.x, scr_vrect.y);
	SCR_AddDirtyPoint (scr_vrect.x+scr_vrect.width-1,
		scr_vrect.y+scr_vrect.height-1);

	// don't draw crosshair while in menu
	if (cls.key_dest != key_menu) 
		SCR_DrawCrosshair ();
}


/*
=============
V_Viewpos_f
=============
*/
void V_Viewpos_f (void)
{
	Com_Printf ("(%i %i %i) : %i\n", (int)cl.refdef.vieworg[0],
		(int)cl.refdef.vieworg[1], (int)cl.refdef.vieworg[2], 
		(int)cl.refdef.viewangles[YAW]);
}

// Knightmare- diagnostic commands from Lazarus
/*
=============
V_Texture_f
=============
*/
void V_Texture_f (void)
{
	trace_t	tr;
	vec3_t	forward, start, end;

	if (!developer->value) // only works in developer mode
		return;

	VectorCopy(cl.refdef.vieworg, start);
	AngleVectors(cl.refdef.viewangles, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = CL_PMSurfaceTrace(cl.playernum+1, start,NULL,NULL,end,MASK_ALL);
	if (!tr.ent)
		Com_Printf("Nothing hit?\n");
	else {
		if (!tr.surface)
			Com_Printf("Not a brush\n");
		else
			Com_Printf("Texture=%s, surface=0x%08x, value=%d\n",tr.surface->name,tr.surface->flags,tr.surface->value);
	}
}

/*
=============
V_Surf_f
=============
*/
void V_Surf_f (void)
{
	trace_t	tr;
	vec3_t	forward, start, end;
	int		s;

	if (!developer->value) // only works in developer mode
		return;

	// Disable this in multiplayer
	if ( cl.configstrings[CS_MAXCLIENTS][0] &&
		strcmp(cl.configstrings[CS_MAXCLIENTS], "1") )
		return;

	if (Cmd_Argc() < 2)
	{
		Com_Printf("Syntax: surf <value>\n");
		return;
	}
	else
		s = atoi(Cmd_Argv(1));

	VectorCopy(cl.refdef.vieworg, start);
	AngleVectors(cl.refdef.viewangles, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = CL_PMSurfaceTrace(cl.playernum+1, start,NULL,NULL,end,MASK_ALL);
	if (!tr.ent)
		Com_Printf("Nothing hit?\n");
	else
	{
		if (!tr.surface)
			Com_Printf("Not a brush\n");
		else
			tr.surface->flags = s;
	}
}


/*
=============
V_Init
=============
*/
void V_Init (void)
{
	Cmd_AddCommand ("gun_next", V_Gun_Next_f);
	Cmd_AddCommand ("gun_prev", V_Gun_Prev_f);
	Cmd_AddCommand ("gun_model", V_Gun_Model_f);

	Cmd_AddCommand ("viewpos", V_Viewpos_f);

	// Knightmare- diagnostic commands from Lazarus
	Cmd_AddCommand ("texture", V_Texture_f);
	Cmd_AddCommand ("surf", V_Surf_f);
	//Cmd_AddCommand ("bbox", V_BBox_f);

	hand = Cvar_Get ("hand", "0", CVAR_ARCHIVE);

	crosshair = Cvar_Get ("crosshair", "1", CVAR_ARCHIVE);
	crosshair_scale = Cvar_Get ("crosshair_scale", "0.5", CVAR_ARCHIVE); //Knightmare added

	cl_testblend = Cvar_Get ("cl_testblend", "0", 0);
	cl_testparticles = Cvar_Get ("cl_testparticles", "0", 0);
	cl_testentities = Cvar_Get ("cl_testentities", "0", 0);
	cl_testlights = Cvar_Get ("cl_testlights", "0", CVAR_CHEAT);

	cl_stats = Cvar_Get ("cl_stats", "0", 0);
}
