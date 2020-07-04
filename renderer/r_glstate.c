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

// r_glstate.c - OpenGL state manager from Q2E

#include "r_local.h"


/*
=================
GL_Enable
=================
*/
void GL_Enable (GLenum cap)
{
	switch (cap)
	{
	case GL_CULL_FACE:
		if (glState.cullFace)
			return;
		glState.cullFace = true;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (glState.polygonOffsetFill)
			return;
		glState.polygonOffsetFill = true;
		break;
	case GL_TEXTURE_SHADER_NV: 
		if (!glConfig.NV_texshaders || glState.TexShaderNV)
			return;
		glState.TexShaderNV = true;
		break;
	case GL_VERTEX_PROGRAM_ARB:
		if (!glConfig.arb_vertex_program || glState.vertexProgram)
			return;
		glState.vertexProgram = true;
		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		if (!glConfig.arb_fragment_program || glState.fragmentProgram)
			return;
		glState.fragmentProgram = true;
		break;
	case GL_ALPHA_TEST:
		if (glState.alphaTest)
			return;
		glState.alphaTest = true;
		break;
	case GL_BLEND:
		if (glState.blend)
			return;
		glState.blend = true;
		break;
	case GL_DEPTH_TEST:
		if (glState.depthTest)
			return;
		glState.depthTest = true;
		break;
	case GL_STENCIL_TEST:
		if (glState.stencilTest)
			return;
		glState.stencilTest = true;
		break;
	case GL_SCISSOR_TEST:
		if (glState.scissorTest)
			return;
		glState.scissorTest = true;
	}
	qglEnable(cap);
}


/*
=================
GL_Disable
=================
*/
void GL_Disable (GLenum cap)
{
	switch (cap)
	{
	case GL_CULL_FACE:
		if (!glState.cullFace)
			return;
		glState.cullFace = false;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (!glState.polygonOffsetFill)
			return;
		glState.polygonOffsetFill = false;
		break;
	case GL_TEXTURE_SHADER_NV: 
		if (!glConfig.NV_texshaders || !glState.TexShaderNV)
			return;
		glState.TexShaderNV = false;
		break;
	case GL_VERTEX_PROGRAM_ARB:
		if (!glConfig.arb_vertex_program || !glState.vertexProgram)
			return;
		glState.vertexProgram = false;
		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		if (!glConfig.arb_fragment_program || !glState.fragmentProgram)
			return;
		glState.fragmentProgram = false;
		break;
	case GL_ALPHA_TEST:
		if (!glState.alphaTest)
			return;
		glState.alphaTest = false;
		break;
	case GL_BLEND:
		if (!glState.blend)
			return;
		glState.blend = false;
		break;
	case GL_DEPTH_TEST:
		if (!glState.depthTest)
			return;
		glState.depthTest = false;
		break;
	case GL_STENCIL_TEST:
		if (!glState.stencilTest)
			return;
		glState.stencilTest = false;
		break;
	case GL_SCISSOR_TEST:
		if (!glState.scissorTest)
			return;
		glState.scissorTest = false;
	}
	qglDisable(cap);
}


/*
=================
GL_Stencil
setting stencil buffer
stenciling for shadows & color shells
=================
*/
void GL_Stencil (qboolean enable, qboolean shell)
{
	if (!glConfig.have_stencil || !r_stencil->integer) 
		return;

	if (enable)
	{
		if (shell || r_shadows->integer == 3) {
			qglPushAttrib(GL_STENCIL_BUFFER_BIT);
			if ( r_shadows->integer == 3)
				qglClearStencil(1);
			qglClear(GL_STENCIL_BUFFER_BIT);
		}

		GL_Enable(GL_STENCIL_TEST);
		qglStencilFunc(GL_EQUAL, 1, 2);
		qglStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	}
	else
	{
		GL_Disable(GL_STENCIL_TEST);
		if (shell || r_shadows->integer == 3)
			qglPopAttrib();
	}
}

qboolean GL_HasStencil (void)
{
	return (glConfig.have_stencil && r_stencil->integer);
}

/*
=================
R_ParticleStencil
uses stencil buffer to redraw
particles only over trans surfaces
=================
*/
extern	cvar_t	*r_particle_overdraw;
void R_ParticleStencil (int passnum)
{
	if (!glConfig.have_stencil || !r_particle_overdraw->integer) 
		return;

	if (passnum == 1) // write area of trans surfaces to stencil buffer
	{
		qglPushAttrib(GL_STENCIL_BUFFER_BIT); // save stencil buffer
		qglClearStencil(1);
		qglClear(GL_STENCIL_BUFFER_BIT);

		GL_Enable(GL_STENCIL_TEST);
		qglStencilFunc( GL_ALWAYS, 1, 0xFF);
		qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	}
	else if (passnum == 2) // turn off writing
	{
		GL_Disable(GL_STENCIL_TEST);
	}
	else if (passnum == 3) // enable drawing only to affected area
	{
		GL_Enable(GL_STENCIL_TEST);
		qglStencilFunc( GL_NOTEQUAL, 1, 0xFF);
		qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}
	else if (passnum == 4) // turn off and restore
	{
		GL_Disable(GL_STENCIL_TEST);
		qglPopAttrib(); // restore stencil buffer
	}
}


/*
=================
GL_Envmap
setting up envmap
=================
*/
#define GLSTATE_DISABLE_TEXGEN		if (glState.texgen) { qglDisable(GL_TEXTURE_GEN_S); qglDisable(GL_TEXTURE_GEN_T); qglDisable(GL_TEXTURE_GEN_R); glState.texgen=false; }
#define GLSTATE_ENABLE_TEXGEN		if (!glState.texgen) { qglEnable(GL_TEXTURE_GEN_S); qglEnable(GL_TEXTURE_GEN_T); qglEnable(GL_TEXTURE_GEN_R); glState.texgen=true; }
void GL_Envmap (qboolean enable)
{

	if (enable)
	{
		qglTexGenf(GL_S, GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
		qglTexGenf(GL_T, GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
		GLSTATE_ENABLE_TEXGEN
	}
	else
	{
		GLSTATE_DISABLE_TEXGEN
	}
}


/*
=================
GL_ShadeModel
=================
*/
void GL_ShadeModel (GLenum mode)
{
	if (glState.shadeModelMode == mode)
		return;
	glState.shadeModelMode = mode;
	qglShadeModel(mode);
}


/*
=================
GL_TexEnv
=================
*/
void GL_TexEnv (GLenum mode)
{
	static int lastmodes[2] = { -1, -1 };

	if ( mode != lastmodes[glState.currenttmu] )
	{
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode );
		lastmodes[glState.currenttmu] = mode;
	}
}


/*
=================
GL_CullFace
=================
*/
void GL_CullFace (GLenum mode)
{
	if (glState.cullMode == mode)
		return;
	glState.cullMode = mode;
	qglCullFace(mode);
}


/*
=================
GL_PolygonOffset
=================
*/
void GL_PolygonOffset (GLfloat factor, GLfloat units)
{
	if (glState.offsetFactor == factor && glState.offsetUnits == units)
		return;
	glState.offsetFactor = factor;
	glState.offsetUnits = units;
	qglPolygonOffset(factor, units);
}


/*
=================
GL_AlphaFunc
=================
*/
void GL_AlphaFunc (GLenum func, GLclampf ref)
{
	if (glState.alphaFunc == func && glState.alphaRef == ref)
		return;
	glState.alphaFunc = func;
	glState.alphaRef = ref;
	qglAlphaFunc(func, ref);
}


/*
=================
GL_BlendFunc
=================
*/
void GL_BlendFunc (GLenum src, GLenum dst)
{
	if (glState.blendSrc == src && glState.blendDst == dst)
		return;
	glState.blendSrc = src;
	glState.blendDst = dst;
	qglBlendFunc(src, dst);
}


/*
=================
GL_DepthFunc
=================
*/
void GL_DepthFunc (GLenum func)
{
	if (glState.depthFunc == func)
		return;
	glState.depthFunc = func;
	qglDepthFunc(func);
}


/*
=================
GL_DepthMask
=================
*/
void GL_DepthMask (GLboolean mask)
{
	if (glState.depthMask == mask)
		return;
	glState.depthMask = mask;
	qglDepthMask(mask);
}

/*
=================
GL_DepthRange
=================
*/
void GL_DepthRange (GLfloat rMin, GLfloat rMax)
{
	if (glState.depthMin == rMin && glState.depthMax == rMax)
		return;
	glState.depthMin = rMin;
	glState.depthMax = rMax;
	qglDepthRange (rMin, rMax);
}


/*
=============
GL_LockArrays
=============
*/
void GL_LockArrays (int numVerts)
{
	if (!glConfig.extCompiledVertArray)
		return;
	if (glState.arraysLocked)
		return;

	qglLockArraysEXT (0, numVerts);
	glState.arraysLocked = true;
}


/*
=============
GL_UnlockArrays
=============
*/
void GL_UnlockArrays (void)
{
	if (!glConfig.extCompiledVertArray)
		return;
	if (!glState.arraysLocked)
		return;

	qglUnlockArraysEXT ();
	glState.arraysLocked = false;
}

/*
=================
GL_EnableTexture
=================
*/
void GL_EnableTexture (unsigned tmu)
{
	if (tmu > 0 && !glConfig.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= glConfig.max_texunits)
		return;

	GL_SelectTexture(tmu);
	qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, sizeof(texCoordArray[tmu][0]), texCoordArray[tmu][0]);
	glState.activetmu[tmu] = true;
}

/*
=================
GL_DisableTexture
=================
*/
void GL_DisableTexture (unsigned tmu)
{
	if (tmu > 0 && !glConfig.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= glConfig.max_texunits)
		return;

	GL_SelectTexture(tmu);
	qglDisable(GL_TEXTURE_2D);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glState.activetmu[tmu] = false;
}

/*
=================
GL_EnableMultitexture
Only used for world drawing
=================
*/
void GL_EnableMultitexture (qboolean enable)
{
	if (!glConfig.multitexture)
		return;

	if (enable)
	{
		GL_EnableTexture(1);
		GL_TexEnv(GL_REPLACE);
	}
	else
	{
		GL_DisableTexture(1);
		GL_TexEnv(GL_REPLACE);
	}
	GL_SelectTexture(0);
	GL_TexEnv(GL_REPLACE);
}

/*
=================
GL_SelectTexture
=================
*/
void GL_SelectTexture (unsigned tmu)
{
	if (!glConfig.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= glConfig.max_texunits)
		return;

	if (tmu == glState.currenttmu)
		return;

	glState.currenttmu = tmu;

	qglActiveTextureARB(GL_TEXTURE0_ARB+tmu);
	qglClientActiveTextureARB(GL_TEXTURE0_ARB+tmu);
}

/*
=================
GL_Bind
=================
*/
void GL_Bind (int texnum)
{
	extern	image_t	*draw_chars;

	if (r_nobind->integer && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
	if (glState.currenttextures[glState.currenttmu] == texnum)
		return;

	glState.currenttextures[glState.currenttmu] = texnum;
	qglBindTexture (GL_TEXTURE_2D, texnum);
}

/*
=================
GL_MBind
=================
*/
void GL_MBind (unsigned tmu, int texnum)
{
	if (tmu >= MAX_TEXTURE_UNITS || tmu >= glConfig.max_texunits)
		return;

	GL_SelectTexture(tmu);

	if (glState.currenttextures[tmu] == texnum)
		return;

	GL_Bind(texnum);
}

/*
=================
GL_SetDefaultState
=================
*/
void GL_SetDefaultState (void)
{
	int		i;

	// Reset the state manager
	glState.texgen = false;
	glState.cullFace = false;
	glState.polygonOffsetFill = false;
	glState.TexShaderNV = false;
	glState.vertexProgram = false;
	glState.fragmentProgram = false;
	glState.alphaTest = false;
	glState.blend = false;
	glState.stencilTest = false;
	glState.depthTest = false;
	glState.scissorTest = false;
	glState.arraysLocked = false;

	glState.cullMode = GL_FRONT;
	glState.shadeModelMode = GL_FLAT;
	glState.depthMin = gldepthmin;
	glState.depthMax = gldepthmax;
	glState.offsetFactor = -1;
	glState.offsetUnits = -2;
	glState.alphaFunc = GL_GREATER;
	glState.alphaRef = 0.666;
	glState.blendSrc = GL_SRC_ALPHA;
	glState.blendDst = GL_ONE_MINUS_SRC_ALPHA;
	glState.depthFunc = GL_LEQUAL;
	glState.depthMask = GL_TRUE;

	// Set default state
	qglDisable(GL_TEXTURE_GEN_S);
	qglDisable(GL_TEXTURE_GEN_T);
	qglDisable(GL_TEXTURE_GEN_R);
	qglEnable(GL_TEXTURE_2D);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_POLYGON_OFFSET_FILL);
	qglDisable(GL_ALPHA_TEST);
	qglDisable(GL_BLEND);
	qglDisable(GL_STENCIL_TEST);
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_SCISSOR_TEST);

	qglCullFace(GL_FRONT);
	qglShadeModel(GL_FLAT);
	qglPolygonOffset(-1, -2);
	qglAlphaFunc(GL_GREATER, 0.666);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDepthFunc(GL_LEQUAL);
	qglDepthMask(GL_TRUE);

//	qglClearColor (1,0, 0.5, 0.5);
	qglClearColor (r_clearColor[0], r_clearColor[1], r_clearColor[2], r_clearColor[3]);
	qglClearDepth(1.0);
	qglClearStencil(128);

	qglColor4f (1,1,1,1);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GL_TextureMode (r_texturemode->string);

//	GL_TextureAlphaMode (r_texturealphamode->string);
//	GL_TextureSolidMode (r_texturesolidmode->string);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Vertex arrays
	qglEnableClientState (GL_TEXTURE_COORD_ARRAY);
	qglEnableClientState (GL_VERTEX_ARRAY);
	qglEnableClientState (GL_COLOR_ARRAY);

	qglTexCoordPointer (2, GL_FLOAT, sizeof(texCoordArray[0][0]), texCoordArray[0][0]);
	qglVertexPointer (3, GL_FLOAT, sizeof(vertexArray[0]), vertexArray[0]);
	qglColorPointer (4, GL_FLOAT, sizeof(colorArray[0]), colorArray[0]);
	// end vertex arrays

	glState.activetmu[0] = true;
	for (i=1; i<MAX_TEXTURE_UNITS; i++)
		glState.activetmu[i] = false;

	GL_TexEnv (GL_REPLACE);

	GL_UpdateSwapInterval();
}
