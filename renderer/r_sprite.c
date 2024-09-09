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

// r_sprite.c -- sprite rendering
// moved from r_main.c

#include "r_local.h"

/*
=============================================================

	SPRITE MODELS

=============================================================
*/

vec2_t	r_spriteTexCoord[4] =
{
	{0, 1},
	{0, 0},
	{1, 0},
	{1, 1},
};

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	float			alpha = 1.0f;
	vec3_t			point[8], v_forward, v_right, v_up;
	mspritemodel_t	*pSprite;
	mspriteframe_t	*pFrame;
	float			*up, *right, scale = 1.0f;
	int				i, nVerts = 4, framenum;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	pSprite = (mspritemodel_t *)currentmodel->extradata;

	e->frame %= pSprite->num_frames;

	pFrame = &pSprite->frames[e->frame];

	if ( !pFrame ) return;

	if (e->flags & RF_SPRITE_ORIENTED)
	{	// angle-locked sprite
		c_alias_polys += 4;
		AngleVectors (e->angles, v_forward, v_right, v_up);
		up = v_up;
		right = v_right;

		GL_Disable (GL_CULL_FACE);
	}
	else {
		// normal sprite
		c_alias_polys += 2;
		up = vup;
		right = vright;
	}

	if (e->scale > 0.0f)
		scale = e->scale;

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;

	R_SetVertexRGBScale (true);

	// Psychospaz's additive transparency
	if ( (currententity->flags & RF_TRANS_ADDITIVE) && (alpha != 1.0f) )
	{ 
		GL_Enable (GL_BLEND);
		GL_TexEnv (GL_MODULATE);
		GL_Disable (GL_ALPHA_TEST);
		GL_DepthMask (false);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		GL_TexEnv (GL_MODULATE);
		if (alpha == 1.0f) {
			GL_Enable (GL_ALPHA_TEST);
			GL_DepthMask (true);
		}
		else {
			GL_Enable (GL_BLEND);
			GL_Disable (GL_ALPHA_TEST);
			GL_DepthMask (false);
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	// catch out of bounds framenum
	framenum = e->frame;
	if (( framenum >= pSprite->num_frames) || (framenum < 0) )
		framenum = 0;
	GL_Bind (currentmodel->skins[0][framenum]->texnum);

	VectorMA (e->origin, -(pFrame->origin_y) * scale, up, point[0]);
	VectorMA (point[0], -(pFrame->origin_x) * scale, right, point[0]);

	VectorMA (e->origin, (pFrame->height - pFrame->origin_y) * scale, up, point[1]);
	VectorMA (point[1], -(pFrame->origin_x) * scale, right, point[1]);

	VectorMA (e->origin, (pFrame->height - pFrame->origin_y) * scale, up, point[2]);
	VectorMA (point[2], (pFrame->width - pFrame->origin_x) * scale, right, point[2]);

	VectorMA (e->origin, -(pFrame->origin_y) * scale, up, point[3]);
	VectorMA (point[3], (pFrame->width - pFrame->origin_x) * scale, right, point[3]);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;

	if (e->flags & RF_SPRITE_ORIENTED)
	{
		nVerts = 8;

		VectorMA (e->origin, -(pFrame->origin_y) * scale, up, point[4]);
		VectorMA (point[4], -(pFrame->origin_x) * scale, v_forward, point[4]);

		VectorMA (e->origin, (pFrame->height - pFrame->origin_y) * scale, up, point[5]);
		VectorMA (point[5], -(pFrame->origin_x) * scale, v_forward, point[5]);

		VectorMA (e->origin, (pFrame->height - pFrame->origin_y) * scale, up, point[6]);
		VectorMA (point[6], (pFrame->width - pFrame->origin_x) * scale, v_forward, point[6]);

		VectorMA (e->origin, -(pFrame->origin_y) * scale, up, point[7]);
		VectorMA (point[7], (pFrame->width - pFrame->origin_x) * scale, v_forward, point[7]);

		indexArray[rb_index++] = rb_vertex+4;
		indexArray[rb_index++] = rb_vertex+5;
		indexArray[rb_index++] = rb_vertex+6;
		indexArray[rb_index++] = rb_vertex+4;
		indexArray[rb_index++] = rb_vertex+6;
		indexArray[rb_index++] = rb_vertex+7;
	}

	for (i = 0; i < nVerts; i++) {
		VA_SetElem2v(texCoordArray[0][rb_vertex], r_spriteTexCoord[i%4]);
		VA_SetElem3v(vertexArray[rb_vertex], point[i]);
		VA_SetElem4(colorArray[rb_vertex], 1.0f, 1.0f, 1.0f, alpha);
		rb_vertex++;
	}
	RB_DrawArrays ();

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexEnv (GL_REPLACE);
	GL_DepthMask (true);
	GL_Disable (GL_ALPHA_TEST);
	GL_Disable (GL_BLEND);

	R_SetVertexRGBScale (false);

	RB_DrawMeshTris ();

	if (e->flags & RF_SPRITE_ORIENTED) {
		GL_Enable (GL_CULL_FACE);
	}

	rb_vertex = rb_index = 0;
}

/*
=============================================================

	KEX FLARES

=============================================================
*/

vec3_t	r_flareOcclusionBBox[8] = 
{
	{3.0f, 3.0f, 3.0f},
	{-3.0f, 3.0f, 3.0f},
	{3.0f, -3.0f, 3.0f},
	{-3.0f, -3.0f, 3.0f},
	{3.0f, 3.0f, -3.0f},
	{-3.0f, 3.0f, -3.0f},
	{3.0f, -3.0f, -3.0f},
	{-3.0f, -3.0f, -3.0f},
};

vec2_t	r_flareTexCoord[5] =
{
	{0, 1},
	{0, 0},
	{1, 0},
	{1, 1},
	{0.5, 0.5},
};


/*
=================
R_OccludeTestFlare
=================
*/
void R_OccludeTestFlare (entity_t *e)
{
	int				i, j;
	int				mask, aggregateMask = ~0;
	unsigned int	sampleCount = 0;
	float			dp;
	vec3_t			bbox[8];

	if ( !glConfig.occlusionQuery || !r_occlusion_test->integer )	// occlusion query support required
		return;

	if ( (e->edict_num < 0) || (e->edict_num >= MAX_EDICTS) )	// bad edict num
		return;

	// setup bbox for this flare
	for (i = 0; i < 8; i++) {
		VectorMA (e->origin, e->scale, r_flareOcclusionBBox[i], bbox[i]);
	}

	// check if query for this ent is pending from the last frame
	// if it was pending and is available now, update sample list and bail
	if ( r_occlusionQueryPending[e->edict_num] )
	{
		qglGetQueryObjectuiv (r_occlusionQueries[e->edict_num], GL_QUERY_RESULT, &sampleCount);
		r_occlusionQuerySamples[e->edict_num] = sampleCount;
		r_occlusionQueryPending[e->edict_num] = 0;
	}

	// skip occlusion test if bbox is outside view frustum
	for (i=0; i<8; i++)
	{
		mask = 0;
		for (j=0; j<4; j++)
		{
			dp = DotProduct(frustum[j].normal, bbox[i]);
			if ( ( dp - frustum[j].dist ) < 0 )
				mask |= (1<<j);
		}
		aggregateMask &= mask;
	}
	if ( aggregateMask )
		return;

	qglBeginQuery (GL_SAMPLES_PASSED_ARB, r_occlusionQueries[e->edict_num]);

	// render bbox
	GL_DisableTexture (0);
	qglColorMask (0, 0, 0, 0);
	GL_DepthMask (0);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;

	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+5;
	indexArray[rb_index++] = rb_vertex+4; 
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+0;

	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+6;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+6;

	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+6;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+3;

	indexArray[rb_index++] = rb_vertex+3;
	indexArray[rb_index++] = rb_vertex+5;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+3;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+5;

	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+5;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+4;
	indexArray[rb_index++] = rb_vertex+7;
	indexArray[rb_index++] = rb_vertex+6;
	for (i=0; i<8; i++) {
		VA_SetElem3v(vertexArray[rb_vertex], bbox[i]);
		VA_SetElem4(colorArray[rb_vertex], 1.0f, 1.0f, 1.0f, 1.0f);
		rb_vertex++;
	}
	RB_DrawArrays ();
	rb_vertex = rb_index = 0;
	
	GL_EnableTexture (0);
	qglColorMask (1, 1, 1, 1);
	GL_DepthMask (1);

	qglEndQuery (GL_SAMPLES_PASSED_ARB);
	r_occlusionQueryPending[e->edict_num] = 1;	// mark as pending
}


/*
=================
R_DrawFlare
=================
*/
void R_DrawFlare (entity_t *e)
{
	int				i, j;
	int				mask, aggregateMask = ~0;
	float			dp, viewDist, fadeStart, fadeEnd, fadeMult, alpha;
//	float			fadeDist, fadeRange;
	float			distFrac, minDistFrac;
	float			*up, *right;
	vec3_t			bbox[8], cornerOfs[4], point[5], viewOfs;
	vec3_t			vup_lock_angle = {0.0f, 0.0f, 1.0f};
	qboolean		useOcclusionQuery = ( glConfig.occlusionQuery && r_occlusion_test->integer &&
										(e->edict_num >= 0) && (e->edict_num < MAX_EDICTS) );
	// check if occlusion query passed
	if ( useOcclusionQuery ) {
		if (r_occlusionQuerySamples[e->edict_num] == 0)
			return;
	}
	else	// old-fashioned frustum culling
	{	// cull if bbox is outside view frustum
		for (i = 0; i < 8; i++) {
			VectorMA (e->origin, e->scale, r_flareOcclusionBBox[i], bbox[i]);
		}	

		for (i=0; i<8; i++)
		{
			mask = 0;
			for (j=0; j<4; j++)
			{
				dp = DotProduct(frustum[j].normal, bbox[i]);
				if ( ( dp - frustum[j].dist ) < 0 )
					mask |= (1<<j);
			}
			aggregateMask &= mask;
		}

		if ( aggregateMask )	// culled
			return;
	}

	c_alias_polys += 4;

	if (e->flags & RF_FLARE_LOCK_ANGLE)	// vertical rotation disabled
		up = vup_lock_angle;
	else	// normal sprite
		up = vup;
	right = vright;

	VectorSubtract (e->origin, r_newrefdef.vieworg, viewOfs);
	viewDist = VectorLength (viewOfs);
	fadeStart = e->fade_start_dist;
	fadeEnd = e->fade_end_dist;

	if (fadeEnd == 0.0f)		// prevent divide by 0
		fadeEnd = 384.0f;
	if (fadeEnd <= fadeStart)	// make sure fadeEnd > fadeStart
		fadeEnd = fadeStart + 288.0f;

	// simpler distance alpha fade- maybe this fades in/out too fast?
/*	fadeDist = viewDist - fadeStart;
	fadeRange = fadeEnd - fadeStart;
	fadeMult = R_ClampValue((fadeDist / fadeRange), 0.0f, 1.0f);*/

	distFrac = R_ClampValue((viewDist / fadeEnd), 0.0f, 1.0f);
	minDistFrac = fadeStart / fadeEnd;
	fadeMult = R_SmoothStep(distFrac, minDistFrac, 1.0f);
//	VID_Printf (PRINT_DEVELOPER, "R_DrawFlare: fade_start_dist= %d fade_end_dist= %d distFrac= %f minDistFrac= %f fadeMult= %f\n",
//				e->fade_start_dist, e->fade_end_dist, distFrac, minDistFrac, fadeMult);

	if (fadeMult == 0.0f)	// faded out
		return;

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha * fadeMult * 0.5f;
	else
		alpha = fadeMult * 0.5f;

	R_SetVertexRGBScale (true);

	GL_Enable (GL_BLEND);
	GL_TexEnv (GL_MODULATE);
	GL_Disable (GL_ALPHA_TEST);
	GL_DepthMask (false);
	if ( useOcclusionQuery ) {
		GL_Disable (GL_DEPTH_TEST);
	}

	// use additive transparency if set
//	if ( (currententity->flags & RF_TRANS_ADDITIVE) && (alpha != 1.0f) )
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE);
//	else
//		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Need replacements for Q2 Remaster alternate flare images
	if ( (e->flags & RF_CUSTOMSKIN) && (e->skin != NULL) )
		GL_Bind (e->skin->texnum);
	else
		GL_Bind (glMedia.flareTexture->texnum);

	// bottom left
	VectorScale (up, -1.0f, cornerOfs[0]);
	VectorMA (cornerOfs[0], -1.0f, right, cornerOfs[0]);
	VectorNormalize (cornerOfs[0]);
	VectorMA (e->origin, e->sprite_radius, cornerOfs[0], point[0]);

	// top left
	VectorCopy (up, cornerOfs[1]);
	VectorMA (cornerOfs[1], -1.0f, right, cornerOfs[1]);
	VectorNormalize (cornerOfs[1]);
	VectorMA (e->origin, e->sprite_radius, cornerOfs[1], point[1]);

	// top right
	VectorCopy (up, cornerOfs[2]);
	VectorAdd (cornerOfs[2], right, cornerOfs[2]);
	VectorNormalize (cornerOfs[2]);
	VectorMA (e->origin, e->sprite_radius, cornerOfs[2], point[2]);

	// bottom right
	VectorScale (up, -1.0f, cornerOfs[3]);
	VectorAdd (cornerOfs[3], right, cornerOfs[3]);
	VectorNormalize (cornerOfs[3]);
	VectorMA (e->origin, e->sprite_radius, cornerOfs[3], point[3]);

	// center
	VectorCopy (e->origin, point[4]);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+4;

	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+4;

	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	indexArray[rb_index++] = rb_vertex+4;

	indexArray[rb_index++] = rb_vertex+3;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+4;
	for (i=0; i<5; i++) {
		VA_SetElem2v(texCoordArray[0][rb_vertex], r_flareTexCoord[i]);
		VA_SetElem3v(vertexArray[rb_vertex], point[i]);
		if (i == 5)
			VA_SetElem4(colorArray[rb_vertex], e->color[0][0], e->color[0][1], e->color[0][2], alpha);
		else
			VA_SetElem4(colorArray[rb_vertex], e->color[1][0], e->color[1][1], e->color[1][2], alpha);
		rb_vertex++;
	}
	RB_DrawArrays ();

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexEnv (GL_REPLACE);
	GL_DepthMask (true);
	GL_Enable (GL_DEPTH_TEST);
	GL_Disable (GL_ALPHA_TEST);
	GL_Disable (GL_BLEND);

	R_SetVertexRGBScale (false);

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}
