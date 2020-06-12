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

// r_gl_state.c - OpenGL state manager from Q2E

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
		if (gl_state.cullFace)
			return;
		gl_state.cullFace = true;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (gl_state.polygonOffsetFill)
			return;
		gl_state.polygonOffsetFill = true;
		break;
	/*case GL_VERTEX_PROGRAM_ARB:
		if (!glConfig.vertexProgram || gl_state.vertexProgram)
			return;
		gl_state.vertexProgram = true;
		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		if (!glConfig.fragmentProgram || gl_state.fragmentProgram)
			return;
		gl_state.fragmentProgram = true;
		break;*/
	case GL_ALPHA_TEST:
		if (gl_state.alphaTest)
			return;
		gl_state.alphaTest = true;
		break;
	case GL_BLEND:
		if (gl_state.blend)
			return;
		gl_state.blend = true;
		break;
	case GL_DEPTH_TEST:
		if (gl_state.depthTest)
			return;
		gl_state.depthTest = true;
		break;
	case GL_STENCIL_TEST:
		if (gl_state.stencilTest)
			return;
		gl_state.stencilTest = true;
		break;
	case GL_SCISSOR_TEST:
		if (gl_state.scissorTest)
			return;
		gl_state.scissorTest = true;
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
		if (!gl_state.cullFace)
			return;
		gl_state.cullFace = false;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (!gl_state.polygonOffsetFill)
			return;
		gl_state.polygonOffsetFill = false;
		break;
	/*case GL_VERTEX_PROGRAM_ARB:
		if (!glConfig.vertexProgram || !gl_state.vertexProgram)
			return;
		gl_state.vertexProgram = false;

		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		if (!glConfig.fragmentProgram || !gl_state.fragmentProgram)
			return;
		gl_state.fragmentProgram = false;
		break;*/
	case GL_ALPHA_TEST:
		if (!gl_state.alphaTest)
			return;
		gl_state.alphaTest = false;
		break;
	case GL_BLEND:
		if (!gl_state.blend)
			return;
		gl_state.blend = false;
		break;
	case GL_DEPTH_TEST:
		if (!gl_state.depthTest)
			return;
		gl_state.depthTest = false;
		break;
	case GL_STENCIL_TEST:
		if (!gl_state.stencilTest)
			return;
		gl_state.stencilTest = false;
		break;
	case GL_SCISSOR_TEST:
		if (!gl_state.scissorTest)
			return;
		gl_state.scissorTest = false;
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
	if (!gl_config.have_stencil || !r_stencil->value) 
		return;

	if (enable)
	{
		if (shell || r_shadows->value == 3) {
			qglPushAttrib(GL_STENCIL_BUFFER_BIT);
			if ( r_shadows->value == 3)
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
		if (shell || r_shadows->value == 3)
			qglPopAttrib();
	}
}

qboolean GL_HasStencil (void)
{
	return (gl_config.have_stencil && r_stencil->value);
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
	if (!gl_config.have_stencil || !r_particle_overdraw->value) 
		return;

	if (passnum == 1) // write area of trans surfaces to stencil buffer
	{
		qglPushAttrib(GL_STENCIL_BUFFER_BIT); // save stencil buffer
		qglClearStencil(1);
		qglClear(GL_STENCIL_BUFFER_BIT);

		GL_Enable(GL_STENCIL_TEST);
		qglStencilFunc( GL_ALWAYS, 1, 0xFF);
		qglStencilOp(GL_KEEP, GL_INCR, GL_INCR);
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
#define GLSTATE_DISABLE_TEXGEN		if (gl_state.texgen) { qglDisable(GL_TEXTURE_GEN_S); qglDisable(GL_TEXTURE_GEN_T); qglDisable(GL_TEXTURE_GEN_R); gl_state.texgen=false; }
#define GLSTATE_ENABLE_TEXGEN		if (!gl_state.texgen) { qglEnable(GL_TEXTURE_GEN_S); qglEnable(GL_TEXTURE_GEN_T); qglEnable(GL_TEXTURE_GEN_R); gl_state.texgen=true; }
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
	if (gl_state.shadeModelMode == mode)
		return;
	gl_state.shadeModelMode = mode;
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

	if ( mode != lastmodes[gl_state.currenttmu] )
	{
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode );
		lastmodes[gl_state.currenttmu] = mode;
	}
}


/*
=================
GL_CullFace
=================
*/
void GL_CullFace (GLenum mode)
{
	if (gl_state.cullMode == mode)
		return;
	gl_state.cullMode = mode;
	qglCullFace(mode);
}


/*
=================
GL_PolygonOffset
=================
*/
void GL_PolygonOffset (GLfloat factor, GLfloat units)
{
	if (gl_state.offsetFactor == factor && gl_state.offsetUnits == units)
		return;
	gl_state.offsetFactor = factor;
	gl_state.offsetUnits = units;
	qglPolygonOffset(factor, units);
}


/*
=================
GL_AlphaFunc
=================
*/
void GL_AlphaFunc (GLenum func, GLclampf ref)
{
	if (gl_state.alphaFunc == func && gl_state.alphaRef == ref)
		return;
	gl_state.alphaFunc = func;
	gl_state.alphaRef = ref;
	qglAlphaFunc(func, ref);
}


/*
=================
GL_BlendFunc
=================
*/
void GL_BlendFunc (GLenum src, GLenum dst)
{
	if (gl_state.blendSrc == src && gl_state.blendDst == dst)
		return;
	gl_state.blendSrc = src;
	gl_state.blendDst = dst;
	qglBlendFunc(src, dst);
}


/*
=================
GL_DepthFunc
=================
*/
void GL_DepthFunc (GLenum func)
{
	if (gl_state.depthFunc == func)
		return;
	gl_state.depthFunc = func;
	qglDepthFunc(func);
}


/*
=================
GL_DepthMask
=================
*/
void GL_DepthMask (GLboolean mask)
{
	if (gl_state.depthMask == mask)
		return;
	gl_state.depthMask = mask;
	qglDepthMask(mask);
}

/*
=================
GL_DepthRange
=================
*/
void GL_DepthRange (GLfloat rMin, GLfloat rMax)
{
	if (gl_state.depthMin == rMin && gl_state.depthMax == rMax)
		return;
	gl_state.depthMin = rMin;
	gl_state.depthMax = rMax;
	qglDepthRange (rMin, rMax);
}


/*
=============
GL_LockArrays
=============
*/
void GL_LockArrays (int numVerts)
{
	if (!gl_config.extCompiledVertArray)
		return;
	if (gl_state.arraysLocked)
		return;

	qglLockArraysEXT (0, numVerts);
	gl_state.arraysLocked = true;
}


/*
=============
GL_UnlockArrays
=============
*/
void GL_UnlockArrays (void)
{
	if (!gl_config.extCompiledVertArray)
		return;
	if (!gl_state.arraysLocked)
		return;

	qglUnlockArraysEXT ();
	gl_state.arraysLocked = false;
}

/*
=================
GL_EnableTexture
=================
*/
void GL_EnableTexture (unsigned tmu)
{
	if (tmu > 0 && !gl_config.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= gl_config.max_texunits)
		return;

	GL_SelectTexture(tmu);
	qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, sizeof(texCoordArray[tmu][0]), texCoordArray[tmu][0]);
	//gl_state.activetmu[tmu] = true;
}

/*
=================
GL_DisableTexture
=================
*/
void GL_DisableTexture (unsigned tmu)
{
	if (tmu > 0 && !gl_config.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= gl_config.max_texunits)
		return;

	GL_SelectTexture(tmu);
	qglDisable(GL_TEXTURE_2D);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//gl_state.activetmu[tmu] = false;
}

/*
=================
GL_EnableMultitexture
Only used for world drawing
=================
*/
void GL_EnableMultitexture (qboolean enable)
{
	if (!gl_config.multitexture)
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
	if (!gl_config.multitexture)
		return;

	if (tmu >= MAX_TEXTURE_UNITS || tmu >= gl_config.max_texunits)
		return;

	if (tmu == gl_state.currenttmu)
		return;

	gl_state.currenttmu = tmu;

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

	if (r_nobind->value && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;

	gl_state.currenttextures[gl_state.currenttmu] = texnum;
	qglBindTexture (GL_TEXTURE_2D, texnum);
}

/*
=================
GL_MBind
=================
*/
void GL_MBind (unsigned tmu, int texnum)
{
	if (tmu >= MAX_TEXTURE_UNITS || tmu >= gl_config.max_texunits)
		return;

	GL_SelectTexture(tmu);

	if (gl_state.currenttextures[tmu] == texnum)
		return;

	GL_Bind(texnum);
}

#if 1
/*
=================
GL_SetDefaultState
=================
*/
void GL_SetDefaultState (void)
{
	// Reset the state manager
	gl_state.texgen = false;
	gl_state.cullFace = false;
	gl_state.polygonOffsetFill = false;
	gl_state.vertexProgram = false;
	gl_state.fragmentProgram = false;
	gl_state.alphaTest = false;
	gl_state.blend = false;
	gl_state.stencilTest = false;
	gl_state.depthTest = false;
	gl_state.scissorTest = false;
	gl_state.arraysLocked = false;

	gl_state.cullMode = GL_FRONT;
	gl_state.shadeModelMode = GL_FLAT;
	gl_state.depthMin = gldepthmin;
	gl_state.depthMax = gldepthmax;
	gl_state.offsetFactor = -1;
	gl_state.offsetUnits = -2;
	gl_state.alphaFunc = GL_GREATER;
	gl_state.alphaRef = 0.666;
	gl_state.blendSrc = GL_SRC_ALPHA;
	gl_state.blendDst = GL_ONE_MINUS_SRC_ALPHA;
	gl_state.depthFunc = GL_LEQUAL;
	gl_state.depthMask = GL_TRUE;

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

	qglClearColor (1,0, 0.5, 0.5);
	qglClearDepth(1.0);
	qglClearStencil(128);

	qglColor4f (1,1,1,1);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GL_TextureMode( r_texturemode->string );
	GL_TextureAlphaMode( r_texturealphamode->string );
	GL_TextureSolidMode( r_texturesolidmode->string );

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
	//gl_state.activetmu[0] = true;
	// end vertex arrays

	GL_TexEnv (GL_REPLACE);

	GL_UpdateSwapInterval();
}

#else
/*
=================
GL_SetDefaultState
=================
*/
void GL_SetDefaultState ( void )
{
	qglClearColor (1,0, 0.5, 0.5);
	qglCullFace(GL_FRONT);
	qglEnable(GL_TEXTURE_2D);

	qglEnable(GL_ALPHA_TEST);
	qglAlphaFunc(GL_GREATER, 0.666);

	qglDisable (GL_DEPTH_TEST);
	qglDisable (GL_CULL_FACE);
	qglDisable (GL_BLEND);

	qglColor4f (1,1,1,1);

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	GL_ShadeModel (GL_FLAT);

	GL_TextureMode( r_texturemode->string );
	GL_TextureAlphaMode( r_texturealphamode->string );
	GL_TextureSolidMode( r_texturesolidmode->string );

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_TexEnv( GL_REPLACE );

	/*if ( qglPointParameterfEXT )
	{
		float attenuations[3];

		attenuations[0] = gl_particle_att_a->value;
		attenuations[1] = gl_particle_att_b->value;
		attenuations[2] = gl_particle_att_c->value;

		qglEnable( GL_POINT_SMOOTH );
		qglPointParameterfEXT( GL_POINT_SIZE_MIN_EXT, r_particle_min_size->value );
		qglPointParameterfEXT( GL_POINT_SIZE_MAX_EXT, r_particle_max_size->value );
		qglPointParameterfvEXT( GL_DISTANCE_ATTENUATION_EXT, attenuations );
	}*/

	/*if ( qglColorTableEXT && gl_ext_palettedtexture->value )
	{
		qglEnable( GL_SHARED_TEXTURE_PALETTE_EXT );

		GL_SetTexturePalette( d_8to24table );
	}*/

	GL_UpdateSwapInterval();
}
#endif