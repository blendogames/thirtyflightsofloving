/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

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

//#define DISABLE_FOG

/*#ifdef KMQUAKE2_ENGINE_MOD
#define NEW_FOGSYS
#endif*/

#ifdef DISABLE_FOG
void Fog_Init()
{
}
void Fog(vec3_t viewpoint)
{
}
void Fog_Off()
{
}
void SP_trigger_fog(edict_t *self)
{
	G_FreeEdict(self);
}
void SP_trigger_fog_bbox(edict_t *self)
{
	G_FreeEdict(self);
}
void SP_target_fog(edict_t *self)
{
	G_FreeEdict(self);
}
void Cmd_Fog_f(edict_t *ent)
{
}
#else // DISABLE_FOG


/*
=================================================
NEW FOG SYSTEM
=================================================
*/
#ifdef NEW_FOGSYS
// new multi-client fog code here

fog_t		gfogs[MAX_FOGS];

fog_t		trig_fade_fog[MAX_CLIENTS];
fog_t		fade_fog[MAX_CLIENTS];
fog_t		*pfog[MAX_CLIENTS];

qboolean	InTriggerFog[MAX_CLIENTS];

int GLModels[3] = {GL_LINEAR, GL_EXP, GL_EXP2};

#define FOG_ON       1
#define FOG_TOGGLE   2
#define FOG_TURNOFF  4
#define FOG_STARTOFF 8

void fog_fade (edict_t *self);

// Fog is sent like this:
// gi.WriteByte (svc_fog); // svc_fog = 21
// gi.WriteByte (fog_enable); // 1 = on, 0 = off
// gi.WriteByte (fog_model); // 0, 1, or 2
// gi.WriteByte (fog_density); // 1-100
// gi.WriteShort (fog_near); // >0, <fog_far
// gi.WriteShort (fog_far); // >fog_near-64, < 5000
// gi.WriteByte (fog_red); // 0-255
// gi.WriteByte (fog_green); // 0-255
// gi.WriteByte (fog_blue); // 0-255
// gi.unicast (player_ent, true); 

void Fog_Off (edict_t *player_ent)
{
	if (!player_ent->client || player_ent->is_bot)
		return;
	
	gi.WriteByte (svc_fog); // svc_fog = 21
	gi.WriteByte (0); // disable message, remaining paramaters are ignored
	gi.WriteByte (0); // 0, 1, or 2
	gi.WriteByte (0); // 1-100
	gi.WriteShort (0); // >0, <fog_far
	gi.WriteShort (0); // >fog_near-64, < 5000
	gi.WriteByte (0); // 0-255
	gi.WriteByte (0); // 0-255
	gi.WriteByte (0); // 0-255
	gi.unicast (player_ent, true); 
}

#else // NEW_FOGSYS 

/*
=================================================
OLD FOG SYSTEM
=================================================
*/
#ifdef _WIN32
#include <windows.h>
#else
#define WINAPI
#include <SDL2/SDL.h>
#endif
#define __MSC__
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

fog_t		gfogs[MAX_FOGS];

fog_t		trig_fade_fog;
fog_t		fade_fog;
fog_t		*pfog;

#ifdef _WIN32
HMODULE		hOpenGL;
#else
int		hOpenGL = 1;
#endif

qboolean	InTriggerFog;
float		last_software_frame;
float		last_opengl_frame;

int GLModels[3] = {GL_LINEAR, GL_EXP, GL_EXP2};


typedef void (WINAPI *GLCLEARCOLOR) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (WINAPI *GLDISABLE) (GLenum cap);
typedef void (WINAPI *GLENABLE) (GLenum cap);
typedef void (WINAPI *GLFOGF) (GLenum pname, GLfloat param);
typedef void (WINAPI *GLFOGFV) (GLenum pname, const GLfloat *params);
typedef void (WINAPI *GLFOGI) (GLenum pname, GLint param);
typedef void (WINAPI *GLHINT) (GLenum target, GLenum mode);
GLCLEARCOLOR GL_glClearColor;
GLDISABLE GL_glDisable;
GLENABLE GL_glEnable;
GLFOGF GL_glFogf;
GLFOGFV GL_glFogfv;
GLFOGI GL_glFogi;
GLHINT GL_glHint;

#define FOG_ON       1
#define FOG_TOGGLE   2
#define FOG_TURNOFF  4
#define FOG_STARTOFF 8

#define COLOR(r,g,b) ((((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

void fog_fade (edict_t *self);

void Fog_Off()
{
	if (deathmatch->value || coop->value)
		return;

/*#ifdef KMQUAKE2_ENGINE_MOD // engine fog
	{
		edict_t	*player_ent = &g_edicts[1];

		if (!player_ent->client || player_ent->is_bot)
			return;

		gi.WriteByte (svc_fog); // svc_fog = 21
		gi.WriteByte (0); // disable message, remaining paramaters are ignored
		gi.WriteByte (0); // 0, 1, or 2
		gi.WriteByte (0); // 1-100
		gi.WriteShort (0); // >0, <fog_far
		gi.WriteShort (0); // >fog_near-64, < 5000
		gi.WriteByte (0); // 0-255
		gi.WriteByte (0); // 0-255
		gi.WriteByte (0); // 0-255
		gi.unicast (player_ent, true);
	}

#else // old fog
*/
	if (gl_driver && vid_ref)
	{
		//Knightmare- also ref_kmgl.dll
		if (!strcmp(vid_ref->string,"gl") || !strcmp(vid_ref->string,"kmgl"))
		{
			if (hOpenGL) GL_glDisable (GL_FOG);
		}
		else
		{
			edict_t	*player;

			player = &g_edicts[1];
			player->client->fadein = 0;
		}
	}

//#endif
}

void Fog_ConsoleFog()
{
	// This routine is ONLY called for console fog commands

	if(deathmatch->value || coop->value)
		return;

	if(!level.active_fog) return;

	memcpy(&level.fog,&gfogs[0],sizeof(fog_t));
	pfog = &level.fog;

	// Force sensible values for linear:
	if(pfog->Model==0 && pfog->Near==0. && pfog->Far==0.)
	{
		pfog->Near=64;
		pfog->Far=1024;
	}
}

void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0);
void Cmd_Fog_f(edict_t *ent)
{
	char	*cmd;
	char	*parm;
	fog_t	*fog;

	fog = &gfogs[0];
	cmd = gi.argv(0);
	if(gi.argc() < 2)
		parm = NULL;
	else
		parm = gi.argv(1);

	if (Q_stricmp (cmd, "fog_help") == 0 )
	{
		gi.dprintf("Fog parameters for console only.\n"
			       "Use fog_active to see parameters for currently active fog.\n");

		gi.dprintf("\nUse \"fog [0/1]\" to turn fog off/on (currently %s)\n"
			"Current GL driver is %s\n",(level.active_fog > 0 ? "on" : "off"), gl_driver->string);

		gi.dprintf("Fog_Red     = red component   (0-1)\n"
				   "Fog_Grn     = green component (0-1)\n"
				   "Fog_Blu     = blue component  (0-1)\n"
				   "Fog_Model     0=linear, 1=exponential, 2=exponential squared\n\n"
				   "Linear parameters:\n"
				   "Fog_Near    = fog start distance (>0 and < Fog_Far)\n"
				   "Fog_Far     = distance where objects are completely obscured\n"
				   "              (<5000 and > Fog_Near)\n"
				   "Exponential parameters:\n"
				   "Fog_Density   Best results with values < 100\n\n"
				   "Command without a value will show current setting\n");
	}
	else if(Q_stricmp (cmd, "fog_active") == 0 )
	{
		if(level.active_fog)
		{
			gi.dprintf("Active fog:\n"
				"  Color: %f, %f, %f\n"
				"  Model: %s\n",
				level.fog.Color[0],level.fog.Color[1],level.fog.Color[2],
				(level.fog.Model==1 ? "Exp" : (level.fog.Model==2 ? "Exp2" : "Linear")));
			if(level.fog.Model)
				gi.dprintf("Density: %f\n",level.fog.Density);
			else
			{
				gi.dprintf("   Near: %f\n",level.fog.Near);
				gi.dprintf("    Far: %f\n",level.fog.Far);
			}
		}
		else
			gi.dprintf("No fogs currently active\n");
	}
	else if(Q_stricmp (cmd, "fog_stuff") == 0 )
	{
		gi.dprintf("active_fog=%d, last_active_fog=%d\n",level.active_fog,level.last_active_fog);
	}
	else if(Q_stricmp (cmd, "fog") == 0 )
	{
		if(parm)
		{
			int on;
			on = atoi(parm);
			level.active_fog = level.active_target_fog = (on ? 1 : 0);
			Fog_ConsoleFog();
		}
		gi.dprintf("fog is %s\n",(level.active_fog ? "on" : "off"));
	}
	else if(Q_stricmp (cmd, "Fog_Red") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Color[0]);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Color[0] = max(0.0f,min(1.0f,(float)atof(parm)));
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Grn") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Color[1]);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Color[1] = max(0.0f,min(1.0f,(float)atof(parm)));
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Blu") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Color[2]);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Color[2] = max(0.0f,min(1.0f,(float)atof(parm)));
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Near") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Near);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Near = (float)atof(parm);
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Far") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Far);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Far = (float)atof(parm);
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Model") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %d\n0=Linear\n1=Exp\n2=Exp2\n",cmd,fog->Model);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			//fog->Model = max(0,min(2,atoi(parm)));
			fog->Model = max(0,min(3,atoi(parm)));
			if(fog->Model == 3)
				fog->GL_Model = GLModels[2];
			else
				fog->GL_Model = GLModels[fog->Model];
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_Density") == 0 )
	{
		if(!parm)
			gi.dprintf("%s = %f\n",cmd,fog->Density);
		else
		{
			level.active_fog = level.active_target_fog = 1;
			fog->Density = fog->Density1 = fog->Density2 = (float)atof(parm);
			Fog_ConsoleFog();
		}
	}
	else if(Q_stricmp (cmd, "Fog_List") == 0 )
	{
		int	i;

		gi.dprintf("level.fogs=%d\n",level.fogs);
		gi.dprintf("level.trigger_fogs=%d\n",level.trigger_fogs);
		for(i=0; i<level.fogs; i++)
		{
			pfog = &gfogs[i];
			gi.dprintf("Fog #%d\n",i+1);
			gi.dprintf("Trigger=%s\n",(pfog->Trigger ? "true" : "false"));
			gi.dprintf("Model=%d, Near=%g, Far=%g, Density=%g\n",
				pfog->Model, pfog->Near, pfog->Far, pfog->Density);
			gi.dprintf("Color=%g,%g,%g\n",pfog->Color[0],pfog->Color[1],pfog->Color[2]);
			gi.dprintf("Targetname=%s\n",(pfog->ent ? pfog->ent->targetname : "no ent"));
		}
	}
	else
		Cmd_Say_f (ent, false, true);
}

/*
void SoftwareFog()
{
	edict_t	*player = &g_edicts[1];

	player->client->fadein       = level.framenum - 1;
	player->client->fadehold     = level.framenum + 2;
	player->client->fadeout      = 0;
	player->client->fadecolor[0] = pfog->Color[0];
	player->client->fadecolor[1] = pfog->Color[1];
	player->client->fadecolor[2] = pfog->Color[2];

	if(pfog->Model == 0)
		player->client->fadealpha = pfog->Far/256.;
	else if(pfog->Model == 1)
		player->client->fadealpha = pfog->Density/30;
	else
		player->client->fadealpha = pfog->Density/15;

	if(player->client->fadealpha > 0.9)
		player->client->fadealpha = 0.9;

	last_software_frame = level.framenum;

}
*/


void GLFog()
{
/*#ifdef KMQUAKE2_ENGINE_MOD // engine fog
	edict_t	*player_ent = &g_edicts[1];
	int fog_model, fog_density, fog_near, fog_far;
	int fog_red, fog_green, fog_blue;
	
	if (!player_ent->client || player_ent->is_bot)
		return;

	if (pfog->GL_Model == GL_EXP)
		fog_model = 1;
	else if (pfog->GL_Model == GL_EXP2)
		fog_model = 2;
	else // GL_LINEAR
		fog_model = 0;

	fog_density = (int)(pfog->Density);
	fog_near = (int)(pfog->Near);
	fog_far = (int)(pfog->Far);
	fog_red = (int)(pfog->Color[0]*255);
	fog_green = (int)(pfog->Color[1]*255);
	fog_blue = (int)(pfog->Color[2]*255);

	gi.WriteByte (svc_fog);			// svc_fog = 21
	gi.WriteByte (1);				// enable message
	gi.WriteByte (fog_model);	// model 0, 1, or 2
	gi.WriteByte (fog_density);	// density 1-100
	gi.WriteShort (fog_near);		// near >0, <fog_far
	gi.WriteShort (fog_far);		// far >fog_near-64, < 5000
	gi.WriteByte (fog_red);	// red	0-255
	gi.WriteByte (fog_green);	// green	0-255
	gi.WriteByte (fog_blue);	// blue	0-255
	gi.unicast (player_ent, true); 

#else // old fog
*/
	GLfloat fogColor[4];
	if(!hOpenGL) return;
	fogColor[0] = pfog->Color[0];
	fogColor[1] = pfog->Color[1];
	fogColor[2] = pfog->Color[2];
	fogColor[3] = 1.0;
	GL_glEnable (GL_FOG);                               // turn on fog, otherwise you won't see any
	GL_glClearColor ( fogColor[0], fogColor[1], fogColor[2], fogColor[3]); // Clear the background color to the fog color
	GL_glFogi (GL_FOG_MODE, pfog->GL_Model);
	GL_glFogfv (GL_FOG_COLOR, fogColor);
	if(pfog->GL_Model == GL_LINEAR)
	{
		GL_glFogf (GL_FOG_START, pfog->Near);
		GL_glFogf (GL_FOG_END, pfog->Far);
	}
	else
		GL_glFogf (GL_FOG_DENSITY, pfog->Density/10000.f);
    GL_glHint (GL_FOG_HINT, GL_NICEST);
//#endif

	last_opengl_frame = level.framenum;
}

void trig_fog_fade (edict_t *self)
{
	float	frames;
	int		index;

	if(!InTriggerFog)
	{
		self->nextthink = 0;
		return;
	}
	if(level.framenum <= self->goal_frame)
	{
		index = self->fog_index-1;
		frames = self->goal_frame - level.framenum + 1;
		if(trig_fade_fog.Model == 0)
		{
			trig_fade_fog.Near += (gfogs[index].Near - trig_fade_fog.Near)/frames;
			trig_fade_fog.Far  += (gfogs[index].Far  - trig_fade_fog.Far )/frames;
		}
		else
		{
			trig_fade_fog.Density  += (gfogs[index].Density  - trig_fade_fog.Density) /frames;
			trig_fade_fog.Density1 += (gfogs[index].Density1 - trig_fade_fog.Density1)/frames;
			trig_fade_fog.Density2 += (gfogs[index].Density2 - trig_fade_fog.Density2)/frames;
		}
		trig_fade_fog.Color[0] += (gfogs[index].Color[0] - trig_fade_fog.Color[0])/frames;
		trig_fade_fog.Color[1] += (gfogs[index].Color[1] - trig_fade_fog.Color[1])/frames;
		trig_fade_fog.Color[2] += (gfogs[index].Color[2] - trig_fade_fog.Color[2])/frames;
		trig_fade_fog.GL_Model = GLModels[trig_fade_fog.Model];
		self->nextthink = level.time + FRAMETIME;
		gi.linkentity(self);
	}
}

void init_trigger_fog_delay (edict_t *self)
{
	int		i;
	int		index;
	edict_t *e;

	index = self->fog_index-1;

	// scan for other trigger_fog's that are currently "thinking", iow
	// the trigger_fog has a delay and is ramping. If found, stop the ramp for those fogs
	for(i=1, e=g_edicts+i; i<globals.num_edicts; i++, e++)
	{
		if(!e->inuse) continue;
		if(e == self) continue;
		if(e->think == trig_fog_fade || e->think == fog_fade)
		{
			e->think = NULL;
			e->nextthink = 0;
			gi.linkentity(e);
		}
	}

	self->spawnflags |= FOG_ON;
	if(!level.active_fog)
	{
		// Fog isn't currently on
		memcpy(&level.fog,&gfogs[index],sizeof(fog_t));
		level.fog.Near     = 4999.;
		level.fog.Far      = 5000.;
		level.fog.Density  = 0.0;
		level.fog.Density1 = 0.0;
		level.fog.Density2 = 0.0;
	}
	VectorCopy(self->fog_color,gfogs[index].Color);
	gfogs[index].Near      = self->fog_near;
	gfogs[index].Far       = self->fog_far;
	gfogs[index].Density   = self->fog_density;
	gfogs[index].Density1  = self->fog_density;
	gfogs[index].Density2  = self->density;
	self->goal_frame = level.framenum + self->delay*10 + 1;
	self->think      = trig_fog_fade;
	self->nextthink  = level.time + FRAMETIME;
	memcpy(&trig_fade_fog,&level.fog,sizeof(fog_t));
	level.active_fog = self->fog_index;
}

void Fog (edict_t *ent) //vec3_t viewpoint)
{
	edict_t	*triggerfog;
	edict_t	*player = ent; //&g_edicts[1];
	vec3_t	viewpoint;

	if (!gl_driver || !vid_ref)
		return;

	if (deathmatch->value || coop->value)
		return;

	if (!player->client || player->is_bot)
		return;

	VectorCopy(player->s.origin, viewpoint);
	viewpoint[2] += ent->viewheight;

	//Knightmare- also ref_kmgl.dll
	if(stricmp(vid_ref->string,"gl") && stricmp(vid_ref->string,"kmgl"))
	{
		last_software_frame = level.framenum;
		level.active_fog = 0;
		return;
	}

	InTriggerFog = false;
	if(level.trigger_fogs)
	{
		int i;
		int trigger;
		trigger=0;
		for(i=1; i<level.fogs; i++)
		{
			if(!gfogs[i].Trigger) continue;
			if(!gfogs[i].ent->inuse) continue;
			if(!(gfogs[i].ent->spawnflags & FOG_ON)) continue;
			if(viewpoint[0] < gfogs[i].ent->absmin[0]) continue;
			if(viewpoint[0] > gfogs[i].ent->absmax[0]) continue;
			if(viewpoint[1] < gfogs[i].ent->absmin[1]) continue;
			if(viewpoint[1] > gfogs[i].ent->absmax[1]) continue;
			if(viewpoint[2] < gfogs[i].ent->absmin[2]) continue;
			if(viewpoint[2] > gfogs[i].ent->absmax[2]) continue;
			trigger = i;
			break;
		}
		if(trigger)
		{
			InTriggerFog = true;
			triggerfog = gfogs[trigger].ent;

			if(level.last_active_fog != trigger+1)
			{
				if(triggerfog->delay)
					init_trigger_fog_delay(triggerfog);
				else
					memcpy(&level.fog,&gfogs[trigger],sizeof(fog_t));
				level.active_fog = trigger+1;
			}
			else if(triggerfog->delay)
				memcpy(&level.fog,&trig_fade_fog,sizeof(fog_t));
		}
		else
		{
			InTriggerFog = false;
			level.active_fog = level.active_target_fog;
			// if we are just coming out of a trigger_fog, force
			// level.fog to last active target_fog values
			if(level.active_fog && level.last_active_fog && gfogs[level.last_active_fog-1].Trigger)
			{
				edict_t	*ent = gfogs[level.active_fog-1].ent;
				if(ent && (ent->think == fog_fade))
					ent->think(ent);
				else
					memcpy(&level.fog,&gfogs[level.active_fog-1],sizeof(fog_t));
			}
		}
	}
	
	if (!level.active_fog)
	{
		if (level.last_active_fog)
			Fog_Off();
		level.last_active_fog = 0;
		return;
	}
	
	pfog = &level.fog;
	if((pfog->Density1 != pfog->Density2) && (game.maxclients == 1) && (pfog->Model))
	{
		float	density;
		float	dp;
		vec3_t	vp;

		AngleVectors(player->client->ps.viewangles,vp,0,0);
		dp = DotProduct(pfog->Dir,vp) + 1.0;
		density = ((pfog->Density1*dp) + (pfog->Density2*(2.0-dp)))/2.;
		if(pfog->Density != density)
		{
			pfog->Density = density;
		}
	}
	GLFog();
	level.last_active_fog = level.active_fog;
}

void Fog_Init()
{
#ifdef _WIN32
	char	GL_Lib[64];

	if(gl_driver_fog && strlen(gl_driver_fog->string))
		strcpy(GL_Lib,gl_driver_fog->string);
	else
		strcpy(GL_Lib,"opengl32");
	strcat(GL_Lib,".dll");
	hOpenGL = LoadLibrary(GL_Lib);
	if(hOpenGL)
	{
		GL_glClearColor = (GLCLEARCOLOR)GetProcAddress(hOpenGL,"glClearColor");
		GL_glDisable    = (GLDISABLE)GetProcAddress(hOpenGL,"glDisable");
		GL_glEnable     = (GLENABLE)GetProcAddress(hOpenGL,"glEnable");
		GL_glFogf       = (GLFOGF)GetProcAddress(hOpenGL,"glFogf");
		GL_glFogfv      = (GLFOGFV)GetProcAddress(hOpenGL,"glFogfv");
		GL_glFogi       = (GLFOGI)GetProcAddress(hOpenGL,"glFogi");
		GL_glHint       = (GLHINT)GetProcAddress(hOpenGL,"glHint");
	}
#else
	GL_glClearColor = (GLCLEARCOLOR)SDL_GL_GetProcAddress("glClearColor");
	GL_glDisable    = (GLDISABLE)SDL_GL_GetProcAddress("glDisable");
	GL_glEnable     = (GLENABLE)SDL_GL_GetProcAddress("glEnable");
	GL_glFogf       = (GLFOGF)SDL_GL_GetProcAddress("glFogf");
	GL_glFogfv      = (GLFOGFV)SDL_GL_GetProcAddress("glFogfv");
	GL_glFogi       = (GLFOGI)SDL_GL_GetProcAddress("glFogi");
	GL_glHint       = (GLHINT)SDL_GL_GetProcAddress("glHint");
#endif

	gfogs[0].Color[0] = gfogs[0].Color[1] = gfogs[0].Color[2] = 0.5;
	gfogs[0].Model    = 1;
	gfogs[0].GL_Model = GLModels[1];
	gfogs[0].Density  = 20.;
	gfogs[0].Trigger  = false;
}


void fog_fade (edict_t *self)
{
	float	frames;
	int		index;

	if(level.framenum <= self->goal_frame)
	{
		index = self->fog_index-1;
		frames = self->goal_frame - level.framenum + 1;
		if(fade_fog.Model == 0)
		{
			fade_fog.Near += (gfogs[index].Near - fade_fog.Near)/frames;
			fade_fog.Far  += (gfogs[index].Far  - fade_fog.Far )/frames;
		}
		else
		{
			fade_fog.Density  += (gfogs[index].Density  - fade_fog.Density) /frames;
			fade_fog.Density1 += (gfogs[index].Density1 - fade_fog.Density1)/frames;
			fade_fog.Density2 += (gfogs[index].Density2 - fade_fog.Density2)/frames;
		}
		fade_fog.Color[0] += (gfogs[index].Color[0] - fade_fog.Color[0])/frames;
		fade_fog.Color[1] += (gfogs[index].Color[1] - fade_fog.Color[1])/frames;
		fade_fog.Color[2] += (gfogs[index].Color[2] - fade_fog.Color[2])/frames;
		fade_fog.GL_Model = GLModels[fade_fog.Model];
		self->nextthink = level.time + FRAMETIME;
		if(!InTriggerFog)
			memcpy(&level.fog,&fade_fog,sizeof(fog_t));
		gi.linkentity(self);
	}
	else
	{
//		if(!(self->spawnflags & FOG_ON))
		if(self->spawnflags & FOG_TURNOFF)
			level.active_fog = level.active_target_fog = 0;
	}
}

#endif // NEW_FOGSYS

// The fog entity code works for both systems

/*QUAKED target_fog (1 0 0) (-8 -8 -8) (8 8 8) ADDITIVE NEGATIVE
Change the fog effects.

ADDITIVE	: adds the target_fog settings to the current settings
NEGATIVE	: subtracts the target_fog settings from the current settings

fog_color   : The colour of the fog, the colour picker box can (and should IMO) be used.
fog_density	: The density of the fog, dens*10K (exp&exp2) Default=20
fog_model   : Choices are:
   0 :Linear (Default)
   1 :Exponential
   2 :Exponential2
fog_near    : How close the player must get before he sees the fog. Default=64
fog_far     : How far the player can see into the fog. Default=1024
density     : Specifies how player see fog. Direction is in degrees. Default=0
delay       : Ramp time in seconds
count       : Number of times it can be used
*/

void target_fog_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int		i;
	int		index;
	edict_t *e;

	self->count--;
	if(self->count == 0)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + self->delay + 1;
	}

	if((self->spawnflags & FOG_ON) && (self->spawnflags & FOG_TOGGLE))
	{
		self->spawnflags &= ~FOG_ON;
		return;
	}
	else
		self->spawnflags |= FOG_ON;

	index = self->fog_index-1;

	InTriggerFog = false;

	// scan for other target_fog's that are currently "thinking", iow
	// the target_fog has a delay and is ramping. If found, stop the ramp for those fogs
	for(i=1, e=g_edicts+i; i<globals.num_edicts; i++, e++)
	{
		if(!e->inuse) continue;
		if(e->think == fog_fade)
		{
			e->nextthink = 0;
			gi.linkentity(e);
		}
	}

	if(self->spawnflags & FOG_TURNOFF)
	{
		// Fog is "turn off" only

		if( self->delay && level.active_fog )
		{
			gfogs[index].Far       = 5000.0;
			gfogs[index].Near      = 4999.0;
			gfogs[index].Density   = 0.0;
			gfogs[index].Density1  = 0.0;
			gfogs[index].Density2  = 0.0;
			VectorCopy(level.fog.Color,gfogs[index].Color);
			self->goal_frame  = level.framenum + self->delay*10 + 1;
			self->think       = fog_fade;
			self->nextthink   = level.time + FRAMETIME;
			level.active_fog = level.active_target_fog = self->fog_index;
			memcpy(&fade_fog,&level.fog,sizeof(fog_t));
		}
		else
			level.active_fog = level.active_target_fog = 0;
	}
	else
	{
		if(self->delay)
		{
			if(!level.active_fog)
			{
				// Fog isn't currently on
				memcpy(&level.fog,&gfogs[index],sizeof(fog_t));
				level.fog.Near     = 4999.;
				level.fog.Far      = 5000.;
				level.fog.Density  = 0.0;
				level.fog.Density1 = 0.0;
				level.fog.Density2 = 0.0;
			}
			VectorCopy(self->fog_color,gfogs[index].Color);
			gfogs[index].Near      = self->fog_near;
			gfogs[index].Far       = self->fog_far;
			gfogs[index].Density   = self->fog_density;
			gfogs[index].Density1  = self->fog_density;
			gfogs[index].Density2  = self->density;
			self->goal_frame = level.framenum + self->delay*10 + 1;
			self->think      = fog_fade;
			self->nextthink  = level.time + FRAMETIME;
			memcpy(&fade_fog,&level.fog,sizeof(fog_t));
		} else {
			memcpy(&level.fog,&gfogs[index],sizeof(fog_t));
		}
		level.active_fog = level.active_target_fog = self->fog_index;
	}
}

void SP_target_fog (edict_t *self)
{
	fog_t *fog;

	if ( !allow_fog->value )
	{
		G_FreeEdict(self);
		return;
	}
	if (deathmatch->value || coop->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_FOG;

	if(!level.fogs) level.fogs = 1;   // 1st fog reserved for console commands

	if(level.fogs >= MAX_FOGS)
	{
		gi.dprintf("Maximum number of fogs exceeded!\n");
		G_FreeEdict(self);
		return;
	}

	if( self->delay < 0.)
		self->delay = 0.;

	self->fog_index = level.fogs+1;
	fog = &gfogs[level.fogs];
	fog->Trigger = false;
	fog->Model   = self->fog_model;
	if(fog->Model < 0 || fog->Model > 2) fog->Model = 0;
	fog->GL_Model = GLModels[fog->Model];
	VectorCopy(self->fog_color,fog->Color);
	if(self->spawnflags & FOG_TURNOFF)
	{
		fog->Near     = 4999;
		fog->Far      = 5000;
		fog->Density  = 0;
		fog->Density1 = 0;
		fog->Density2 = 0;
	}
	else
	{
		fog->Near     = self->fog_near;
		fog->Far      = self->fog_far;
		fog->Density  = self->fog_density;
		fog->Density1 = self->fog_density;
		if(self->density == 0.)
			self->density = self->fog_density;
		else if(self->density < 0.)
			self->density = 0.;
		fog->Density2= self->density;
	}
	AngleVectors(self->s.angles,fog->Dir,0,0);
	fog->ent = self;
	level.fogs++;
	self->use = target_fog_use;
	gi.linkentity(self);

	if(self->spawnflags & FOG_ON)
	{
		self->spawnflags &= ~FOG_ON;
		target_fog_use(self,NULL,NULL);
	}
}

/*QUAKED trigger_fog (1 0 0) ? x Toggle x StartOff 
Fog field

"fog_color" specify an RGB color: Default = .5 .5 .5
"fog_model" default = 1
   0 :Linear
   1 :Exp
   2 :Exp2

"fog_near" Starting distance from player. Default =  64
"fog_far" How far the player can see into the fog. Default = 1024
"fog_density" Default = 20
"density" at 180 degrees; Default=0
"delay" ramp time in seconds
"count" number of times it can be used
*/

void trigger_fog_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if((self->spawnflags & FOG_ON) && (self->spawnflags & FOG_TOGGLE))
	{
		self->spawnflags &= ~FOG_ON;
		self->count--;
		if(self->count == 0)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + FRAMETIME;
		}
	}
	else
	{
		self->spawnflags |= FOG_ON;
	}
}

void SP_trigger_fog (edict_t *self)
{
	fog_t *fog;

	if( !allow_fog->value )
	{
		G_FreeEdict(self);
		return;
	}
	if(deathmatch->value || coop->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TRIGGER_FOG;

	if(!level.fogs) level.fogs = 1;   // 1st fog reserved for console commands

	if(level.fogs >= MAX_FOGS)
	{
		gi.dprintf("Maximum number of fogs exceeded!\n");
		G_FreeEdict(self);
		return;
	}

	self->fog_index = level.fogs+1;
	fog = &gfogs[level.fogs];
	fog->Trigger = true;
	fog->Model   = self->fog_model;
	if(fog->Model < 0 || fog->Model > 2) fog->Model = 0;
	fog->GL_Model = GLModels[fog->Model];
	VectorCopy(self->fog_color,fog->Color);
	if(self->spawnflags & FOG_TURNOFF)
	{
		fog->Near    = 4999;
		fog->Far     = 5000;
		fog->Density = 0;
		fog->Density1 = 0;
		fog->Density2 = 0;
	}
	else
	{
		fog->Near    = self->fog_near;
		fog->Far     = self->fog_far;
		fog->Density = self->fog_density;
		fog->Density1 = self->fog_density;
		if(self->density == 0.)
			self->density = self->fog_density;
		else if(self->density < 0.)
			self->density = 0.;
		fog->Density2= self->density;
	}
	if(!(self->spawnflags & FOG_STARTOFF))
		self->spawnflags |= FOG_ON;

	AngleVectors(self->s.angles,fog->Dir,0,0);
	VectorClear(self->s.angles);
	fog->ent     = self;
	level.fogs++;
	level.trigger_fogs++;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	gi.setmodel (self, self->model);
	gi.linkentity(self);
}

/*QUAKED trigger_fog_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) x Toggle x StartOff 
Fog field

"fog_color" specify an RGB color: Default = .5 .5 .5
"fog_model" default = 1
   0 :Linear
   1 :Exp
   2 :Exp2

"fog_near" Starting distance from player. Default =  64
"fog_far" How far the player can see into the fog. Default = 1024
"fog_density" Default = 20
"density" at 180 degrees; Default=0
"delay" ramp time in seconds
"count" number of times it can be used

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_fog_bbox (edict_t *self)
{
	fog_t *fog;

	if( !allow_fog->value )
	{
		G_FreeEdict(self);
		return;
	}
	if(deathmatch->value || coop->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TRIGGER_FOG;

	if(!level.fogs) level.fogs = 1;   // 1st fog reserved for console commands

	if(level.fogs >= MAX_FOGS)
	{
		gi.dprintf("Maximum number of fogs exceeded!\n");
		G_FreeEdict(self);
		return;
	}

	self->fog_index = level.fogs+1;
	fog = &gfogs[level.fogs];
	fog->Trigger = true;
	fog->Model   = self->fog_model;
	if(fog->Model < 0 || fog->Model > 2) fog->Model = 0;
	fog->GL_Model = GLModels[fog->Model];
	VectorCopy(self->fog_color,fog->Color);
	if(self->spawnflags & FOG_TURNOFF)
	{
		fog->Near    = 4999;
		fog->Far     = 5000;
		fog->Density = 0;
		fog->Density1 = 0;
		fog->Density2 = 0;
	}
	else
	{
		fog->Near    = self->fog_near;
		fog->Far     = self->fog_far;
		fog->Density = self->fog_density;
		fog->Density1 = self->fog_density;
		if(self->density == 0.)
			self->density = self->fog_density;
		else if(self->density < 0.)
			self->density = 0.;
		fog->Density2= self->density;
	}
	if(!(self->spawnflags & FOG_STARTOFF))
		self->spawnflags |= FOG_ON;

	AngleVectors(self->s.angles,fog->Dir,0,0);
	VectorClear(self->s.angles);
	fog->ent     = self;
	level.fogs++;
	level.trigger_fogs++;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft, -16, -16, -16);
		VectorSet(self->tright, 16, 16, 16);
	}
	VectorCopy (self->bleft, self->mins);
	VectorCopy (self->tright, self->maxs);
	gi.linkentity(self);
}

#endif // DISABLE_FOG
