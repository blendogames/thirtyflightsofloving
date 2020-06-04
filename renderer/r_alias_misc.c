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

// r_alias_misc.c: shared model rendering functions

#include "r_local.h"


float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

// precalculated dot products for quantized angles
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anormtab.h"
;

float		*shadedots = r_avertexnormal_dots[0];

vec3_t		shadelight;

m_dlight_t	model_dlights[MAX_MODEL_DLIGHTS];
int			model_dlights_num;

float		shellFlowH, shellFlowV;

/*
=================
mirrorValue
=================
*/
float  mirrorValue (float value, qboolean mirrormodel)
{
	if (mirrormodel)
	{
		if (value>1)
			return 0;
		else if (value<0)
			return 1;
		else
			return 1-value;
	}
	return value;
}


#if 0
/*
=================
R_CalcEntAlpha
=================
*/
float R_CalcEntAlpha (float alpha, vec3_t point)
{
	float	baseDist, newAlpha;
	vec3_t	vert_len;

	newAlpha = alpha;

	if (!(currententity->renderfx & RF2_CAMERAMODEL) || !(currententity->flags & RF_TRANSLUCENT))
	{
		newAlpha = max(min(newAlpha, 1.0f), 0.0f);
		return newAlpha;
	}

	baseDist = Cvar_VariableValue("cg_thirdperson_dist");
	if (baseDist < 1)	baseDist = 50;
	VectorSubtract(r_newrefdef.vieworg, point, vert_len);
	newAlpha *= VectorLength(vert_len) / baseDist;
	if (newAlpha > alpha)	newAlpha = alpha;

	newAlpha = max(min(newAlpha, 1.0f), 0.0f);

	return newAlpha;
}
#endif


/*
=================
R_CalcShadowAlpha
=================
*/
#define SHADOW_FADE_DIST 128
float R_CalcShadowAlpha (entity_t *e)
{
	vec3_t	vec;
	float	dist, minRange, maxRange, outAlpha;

	VectorSubtract(e->origin, r_origin, vec);
	dist = VectorLength(vec);

	if (r_newrefdef.fov_y >= 90)
		minRange = r_shadowrange->value;
	else // reduced FOV means longer range
		minRange = r_shadowrange->value * (90/r_newrefdef.fov_y); // this can't be zero
	maxRange = minRange + SHADOW_FADE_DIST;

	if (dist <= minRange) // in range
		outAlpha = r_shadowalpha->value;
	else if (dist >= maxRange) // out of range
		outAlpha = 0.0f;
	else // fade based on distance
		outAlpha = r_shadowalpha->value * (fabs(dist-maxRange)/SHADOW_FADE_DIST);

	return outAlpha;
}


/*
==============
R_ShadowBlend
Draws projection shadow(s)
from stenciled volume
==============
*/
void R_ShadowBlend (float shadowalpha)
{
	if (r_shadows->integer != 3)
		return;

	qglPushMatrix();
    qglLoadIdentity ();

	// FIXME: get rid of these
    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up

	GL_Disable (GL_ALPHA_TEST);
	GL_Enable (GL_BLEND);
	GL_Disable (GL_DEPTH_TEST);
	GL_DisableTexture(0);

	GL_Enable(GL_STENCIL_TEST);
	qglStencilFunc(GL_NOTEQUAL, 0, 255);
	qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	VA_SetElem3(vertexArray[rb_vertex], 10, 100, 100);
	VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, shadowalpha);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, -100, 100);
	VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, shadowalpha);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, -100, -100);
	VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, shadowalpha);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, 100, -100);
	VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, shadowalpha);
	rb_vertex++;
	RB_RenderMeshGeneric (false);

	qglPopMatrix();

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_Disable (GL_BLEND);
	GL_EnableTexture(0);
	GL_Enable (GL_DEPTH_TEST);
	GL_Disable(GL_STENCIL_TEST);
	//GL_Enable (GL_ALPHA_TEST);

	qglColor4f(1,1,1,1);
}


/*
=================
capColorVec
=================
*/
void capColorVec (vec3_t color)
{
	int i;

	for (i=0;i<3;i++)
		color[i] = max(min(color[i], 1.0f), 0.0f);
}

/*
=================
R_SetVertexRGBScale
=================
*/
void R_SetVertexRGBScale (qboolean toggle)
{
	if (!r_rgbscale->integer || !glConfig.mtexcombine)
		return;

	if (toggle) // turn on
	{
#if 1
		qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, r_rgbscale->integer);
		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
		
		GL_TexEnv(GL_COMBINE_ARB);
#else
		qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, r_rgbscale->integer);
		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);

		GL_TexEnv(GL_COMBINE_EXT);
#endif
	}
	else // turn off
	{
		GL_TexEnv(GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
	}
}


/*
=================
EnvMapShell
=================
*/
qboolean EnvMapShell (void)
{
	return ( (r_shelltype->integer == 2)
		|| (r_shelltype->integer == 1 && currententity->alpha == 1.0f) );
}


/*
=================
FlowingShell
=================
*/
qboolean FlowingShell (void)
{
	return (r_shelltype->integer == 1 && currententity->alpha != 1.0f);
}


/*
=================
R_SetShellBlend
=================
*/
void R_SetShellBlend (qboolean toggle)
{
	// shells only
	if ( !(currententity->flags & RF_MASK_SHELL) )
		return;

	if (toggle) //turn on
	{
		// Psychospaz's envmapping
		if (EnvMapShell())
		{
			qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

			GL_Bind(glMedia.spheremappic->texnum);

			qglEnable(GL_TEXTURE_GEN_S);
			qglEnable(GL_TEXTURE_GEN_T);
		}
		else if (FlowingShell())
			GL_Bind(glMedia.shelltexture->texnum);
		else
			GL_DisableTexture(0);

		GL_Stencil(true, true);

		shellFlowH =  0.25 * sin(r_newrefdef.time * 0.5 * M_PI);
		shellFlowV = -(r_newrefdef.time / 2.0); 
	}
	else // turn off
	{
		// Psychospaz's envmapping
		if (EnvMapShell())
		{
			qglDisable(GL_TEXTURE_GEN_S);
			qglDisable(GL_TEXTURE_GEN_T);
		}
		else if (FlowingShell())
		{ /*nothing*/ }
		else
			GL_EnableTexture(0);

		GL_Stencil(false, true);
	}
}

/*
=================
R_FlipModel
=================
*/
void R_FlipModel (qboolean on, qboolean cullOnly)
{
	extern void MYgluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar );

	if (on)
	{
		if (!cullOnly)
		{
			qglMatrixMode( GL_PROJECTION );
			qglPushMatrix();
			qglLoadIdentity();
			qglScalef( -1, 1, 1 );
			MYgluPerspective( r_newrefdef.fov_y, ( float ) r_newrefdef.width / r_newrefdef.height,  4,  r_farz);	// Knightmare- was 4096
			qglMatrixMode( GL_MODELVIEW );
		}
		GL_CullFace( GL_BACK );
	}
	else
	{
		if (!cullOnly) {
			qglMatrixMode( GL_PROJECTION );
			qglPopMatrix();
			qglMatrixMode( GL_MODELVIEW );
		}
		GL_CullFace( GL_FRONT );
	}
}

/*
=================
R_SetBlendModeOn
=================
*/
void R_SetBlendModeOn (image_t *skin)
{
	GL_TexEnv( GL_MODULATE );

	if (skin)
		GL_Bind(skin->texnum);

	GL_ShadeModel (GL_SMOOTH);

	if (currententity->flags & RF_TRANSLUCENT)
	{
		GL_DepthMask (false);

		if (currententity->flags & RF_TRANS_ADDITIVE)
		{ 
			GL_BlendFunc   (GL_SRC_ALPHA, GL_ONE);	
			qglColor4ub(255, 255, 255, 255);
			GL_ShadeModel (GL_FLAT);
		}
		else
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		GL_Enable (GL_BLEND);
	}
}

/*
=================
R_SetBlendModeOff
=================
*/
void R_SetBlendModeOff (void)
{
	if ( currententity->flags & RF_TRANSLUCENT )
	{
		GL_DepthMask (true);
		GL_Disable(GL_BLEND);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

/*
=================
R_SetShadeLight
=================
*/
void R_SetShadeLight (void)
{
	int		i;

	if (currententity->flags & RF_MASK_SHELL)
	{
		VectorClear (shadelight);
		if (currententity->flags & RF_SHELL_HALF_DAM)
		{
			shadelight[0] = 0.56;
			shadelight[1] = 0.59;
			shadelight[2] = 0.45;
		}
		if ( currententity->flags & RF_SHELL_DOUBLE )
		{
			shadelight[0] = 0.9;
			shadelight[1] = 0.7;
		}
		if ( currententity->flags & RF_SHELL_RED )
			shadelight[0] = 1.0;
		if ( currententity->flags & RF_SHELL_GREEN )
			shadelight[1] = 1.0;
		if ( currententity->flags & RF_SHELL_BLUE )
			shadelight[2] = 1.0;
	}
	else if ( currententity->flags & RF_FULLBRIGHT )
	{
		for (i=0 ; i<3 ; i++)
			shadelight[i] = 1.0;
	}
	else
	{
		// Set up basic lighting...
		if (r_model_shading->integer && r_model_dlights->integer)
		{
			int max = r_model_dlights->integer;

			if (max<0) max=0;
			if (max>MAX_MODEL_DLIGHTS) max=MAX_MODEL_DLIGHTS;

			R_LightPointDynamics (currententity->origin, shadelight, model_dlights, 
				&model_dlights_num, max);
		}
		else
		{
			R_LightPoint (currententity->origin, shadelight, false);//true);
			model_dlights_num = 0;
		}

		// player lighting hack for communication back to server
		// big hack!
		if ( currententity->flags & RF_WEAPONMODEL )
		{
			// pick the greatest component, which should be the same
			// as the mono value returned by software
			if (shadelight[0] > shadelight[1])
			{
				if (shadelight[0] > shadelight[2]) {
				//	r_lightlevel->value = 150*shadelight[0];
					Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[0]);
				}
				else {
				//	r_lightlevel->value = 150*shadelight[2];
					Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[2]);
				}
			}
			else
			{
				if (shadelight[1] > shadelight[2]) {
				//	r_lightlevel->value = 150*shadelight[1];
					Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[1]);
				}
				else {
				//	r_lightlevel->value = 150*shadelight[2];
					Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[2]);
				}
			}

		}
		
		if ( r_monolightmap->string[0] != '0' )
		{
			float s = shadelight[0];

			if ( s < shadelight[1] )
				s = shadelight[1];
			if ( s < shadelight[2] )
				s = shadelight[2];

			shadelight[0] = s;
			shadelight[1] = s;
			shadelight[2] = s;
		}
	}

	if ( currententity->flags & RF_MINLIGHT )
	{
		for (i=0; i<3; i++)
			if (shadelight[i] > r_model_minlight->value) // 0.02
				break;
		if (i == 3)
		{
			shadelight[0] = r_model_minlight->value; // 0.02
			shadelight[1] = r_model_minlight->value; // 0.02
			shadelight[2] = r_model_minlight->value; // 0.02
		}
	}

	if ( currententity->flags & RF_GLOW )
	{	// bonus items will pulse with time
		float	scale;
		float	min;

		scale = 0.2 * sin(r_newrefdef.time*7);
		for (i=0 ; i<3 ; i++)
		{
			min = shadelight[i] * 0.8;
			shadelight[i] += scale;
			if (shadelight[i] < min)
				shadelight[i] = min;
		}
	}

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
	{
		if (currententity->flags & RF_IR_VISIBLE)
		{
			shadelight[0] = 1.0;
			shadelight[1] = 0.0;
			shadelight[2] = 0.0;
		}
	}
/*	if (r_newrefdef.rdflags & RDF_UVGOGGLES)
	{
		if (currententity->flags & RF_IR_VISIBLE)
		{
			shadelight[0] = 0.5;
			shadelight[1] = 1.0;
			shadelight[2] = 0.5;
		}
		else
		{
			shadelight[0] = 0.0;
			shadelight[1] = 1.0;
			shadelight[2] = 0.0;
		}
	}*/

	shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
}

/*
=================
R_DrawAliasModelBBox
=================
*/
void R_DrawAliasModelBBox (vec3_t bbox[8], entity_t *e, float red, float green, float blue, float alpha)
{
	int	i;

	if (!r_showbbox->integer)
		return;

	if (e->flags & RF_WEAPONMODEL || e->flags & RF_VIEWERMODEL || e->flags & RF_BEAM || e->renderfx & RF2_CAMERAMODEL)
		return;

	GL_Disable (GL_CULL_FACE);
	qglPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	qglDisableClientState (GL_COLOR_ARRAY);
	qglColor4f(red, green, blue, alpha);
	GL_DisableTexture (0);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;

	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+5;

	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+6;

	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+5;
	indexArray[rb_index++] = rb_vertex+7;

	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+6;
	indexArray[rb_index++] = rb_vertex+7;

	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+5;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+6;
	indexArray[rb_index++] = rb_vertex+7;
	for (i=0; i<8; i++) {
		VA_SetElem3v(vertexArray[rb_vertex], bbox[i]);
		rb_vertex++;
	}
	RB_DrawArrays ();
	rb_vertex = rb_index = 0;

	GL_EnableTexture (0);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	qglEnableClientState (GL_COLOR_ARRAY);
	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	GL_Enable (GL_CULL_FACE);
}
