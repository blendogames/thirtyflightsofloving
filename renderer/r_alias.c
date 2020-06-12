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

// r_alias.c: alias triangle model functions

#include "r_local.h"
#include "vlights.h"
#include "r_normals.h"

/*
=============================================================

  ALIAS MODELS

=============================================================
*/

vec3_t	tempVertexArray[MD3_MAX_MESHES][MD3_MAX_VERTS];

vec3_t	aliasLightDir;
float	aliasShadowAlpha;

/*
=================
R_LightAliasModel
=================
*/
void R_LightAliasModel (vec3_t baselight, vec3_t normal, vec3_t lightOut, byte normalindex, qboolean shaded) //byte oldnormalindex, float backlerp)
{
	int		i;
	float	l;

	if (r_model_shading->value)
	{
		if (shaded)
		{
			if (r_model_shading->value == 3)
				l = 2.0 * shadedots[normalindex] - 1;
			else if (r_model_shading->value == 2)
				l = 1.5 * shadedots[normalindex] - 0.5;
			else
				l = shadedots[normalindex];
			VectorScale(baselight, l, lightOut);
		}
		else
			VectorCopy(baselight, lightOut);

		if (model_dlights_num)
			for (i=0; i<model_dlights_num; i++)
			{
				l = 2.0 * VLight_GetLightValue (normal, model_dlights[i].direction,
					currententity->angles[PITCH], currententity->angles[YAW], true);
				VectorMA(lightOut, l, model_dlights[i].color, lightOut);
			}
	}
	else
	{
		l = 2.0 * VLight_GetLightValue (normal, aliasLightDir, currententity->angles[PITCH],
			currententity->angles[YAW], false);

		VectorScale(baselight, l, lightOut);
	}

	for (i=0; i<3; i++)
		lightOut[i] = max(min(lightOut[i], 1.0f), 0.0f);
}

/*
=================
R_DrawAliasFrameLerp
=================
*/
void R_DrawAliasFrameLerp (maliasmodel_t *paliashdr, entity_t *e)
{
	int			i, j, k, meshnum;
	maliasframe_t	*frame, *oldframe;
	maliasmesh_t	mesh;
	maliasvertex_t	*v, *ov;
	vec3_t		move, delta, vectors[3];
	vec3_t		curScale, oldScale, curNormal, oldNormal;
	vec3_t		tempNormalsArray[MD3_MAX_VERTS];
	vec2_t		tempSkinCoord;
	vec3_t		meshlight, lightcolor;
	float		alpha, meshalpha, thisalpha, shellscale, frontlerp, backlerp = e->backlerp;
	image_t		*skin;
	renderparms_t	skinParms;
	qboolean	shellModel = e->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM);

	frontlerp = 1.0 - backlerp;

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;
	else
		alpha = 1.0;

	frame = paliashdr->frames + e->frame;
	oldframe = paliashdr->frames + e->oldframe;

	VectorScale(frame->scale, frontlerp, curScale);
	VectorScale(oldframe->scale, backlerp, oldScale);

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (e->oldorigin, e->origin, delta);
	AngleVectors (e->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i=0 ; i<3 ; i++)
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];

	R_SetVertexOverbrights(true);
	R_SetShellBlend (true);

	// new outer loop for whole model
	for (k=0, meshnum=0; k < paliashdr->num_meshes; k++, meshnum++)
	{
		mesh = paliashdr->meshes[k];
		skinParms = mesh.skins[e->skinnum].renderparms;

		// select skin
		if (e->skin)
			skin = e->skin;	// custom player skin
		else
			skin = currentmodel->skins[k][e->skinnum];
		if (!skin)
			skin = r_notexture;
		if ( !shellModel )
			GL_Bind(skin->texnum);
		else if (FlowingShell())
			alpha = 0.7;

		// md3 skin scripting
		if (skinParms.nodraw) 
			continue; // skip this mesh for this skin

		if (skinParms.twosided)
			GL_Disable(GL_CULL_FACE);

		if (skinParms.alphatest && !shellModel)
			GL_Enable(GL_ALPHA_TEST);

		if (skinParms.fullbright)
			VectorSet(meshlight, 1.0f, 1.0f, 1.0f);
		else
			VectorCopy(shadelight, meshlight);

		meshalpha = alpha * skinParms.basealpha;

		if (meshalpha < 1.0f || skinParms.blend)
			GL_Enable (GL_BLEND);
		else
			GL_Disable (GL_BLEND);

		if (skinParms.blend && !shellModel)
			GL_BlendFunc (skinParms.blendfunc_src, skinParms.blendfunc_dst);
		else
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// md3 skin scripting

		v = mesh.vertexes + e->frame*mesh.num_verts;
		ov = mesh.vertexes + e->oldframe*mesh.num_verts;
		rb_vertex = 0;

		for (i=0; i<mesh.num_verts; i++, v++, ov++)
		{
			// lerp verts
			curNormal[0] = r_sinTable[v->normal[0]] * r_cosTable[v->normal[1]];
			curNormal[1] = r_sinTable[v->normal[0]] * r_sinTable[v->normal[1]];
			curNormal[2] = r_cosTable[v->normal[0]];

			oldNormal[0] = r_sinTable[ov->normal[0]] * r_cosTable[ov->normal[1]];
			oldNormal[1] = r_sinTable[ov->normal[0]] * r_sinTable[ov->normal[1]];
			oldNormal[2] = r_cosTable[ov->normal[0]];

			VectorSet ( tempNormalsArray[i],
					curNormal[0] + (oldNormal[0] - curNormal[0])*backlerp,
					curNormal[1] + (oldNormal[1] - curNormal[1])*backlerp,
					curNormal[2] + (oldNormal[2] - curNormal[2])*backlerp );

			if (shellModel) 
				shellscale = (e->flags & RF_WEAPONMODEL) ? WEAPON_SHELL_SCALE: POWERSUIT_SCALE;
			else
				shellscale = 0.0;

			VectorSet ( tempVertexArray[meshnum][i], 
					move[0] + ov->point[0]*oldScale[0] + v->point[0]*curScale[0] + tempNormalsArray[i][0]*shellscale,
					move[1] + ov->point[1]*oldScale[1] + v->point[1]*curScale[1] + tempNormalsArray[i][1]*shellscale,
					move[2] + ov->point[2]*oldScale[2] + v->point[2]*curScale[2] + tempNormalsArray[i][2]*shellscale );

			// calc lighting and alpha
			if (shellModel)
				VectorCopy(meshlight, lightcolor);
			else
				R_LightAliasModel(meshlight, tempNormalsArray[i], lightcolor, v->lightnormalindex, !skinParms.nodiffuse);
			//thisalpha = R_CalcEntAlpha(meshalpha, tempVertexArray[meshnum][i]);
			thisalpha = meshalpha;

			// get tex coords
			if (shellModel && FlowingShell()) {
				tempSkinCoord[0] = (tempVertexArray[meshnum][i][0] + tempVertexArray[meshnum][i][1]) / 40.0 + shellFlowH;
				tempSkinCoord[1] = tempVertexArray[meshnum][i][2] / 40.0 + shellFlowV;
			} else {
				tempSkinCoord[0] = mesh.stcoords[i].st[0];
				tempSkinCoord[1] = mesh.stcoords[i].st[1];
			}

			// add to arrays
			VA_SetElem2(texCoordArray[0][rb_vertex], tempSkinCoord[0], tempSkinCoord[1]);
			VA_SetElem3(vertexArray[rb_vertex], tempVertexArray[meshnum][i][0], tempVertexArray[meshnum][i][1], tempVertexArray[meshnum][i][2]);
			VA_SetElem4(colorArray[rb_vertex], lightcolor[0], lightcolor[1], lightcolor[2], thisalpha);
			rb_vertex++;
		}
		if (!shellModel)
			RB_ModifyTextureCoords (&texCoordArray[0][0][0], &vertexArray[0][0], rb_vertex, skinParms);

		// set indices for each triangle and draw
		rb_index = 0;
		for (j=0; j < mesh.num_tris; j++)
		{
			indexArray[rb_index++] = mesh.indexes[3*j+0];
			indexArray[rb_index++] = mesh.indexes[3*j+1];
			indexArray[rb_index++] = mesh.indexes[3*j+2];
		}
		RB_DrawArrays (GL_TRIANGLES);

		// glow pass
		if (mesh.skins[e->skinnum].glowimage && !shellModel)
		{
			float	glowcolor;
			if (skinParms.glow.type > -1)
				glowcolor = RB_CalcGlowColor (skinParms);
			else
				glowcolor = 1.0;
			qglDisableClientState (GL_COLOR_ARRAY);
			qglColor4f(glowcolor, glowcolor, glowcolor, 1.0);

			GL_Enable (GL_BLEND);
			GL_BlendFunc (GL_ONE, GL_ONE);

			GL_Bind(mesh.skins[e->skinnum].glowimage->texnum);

			RB_DrawArrays (GL_TRIANGLES);

			qglEnableClientState (GL_COLOR_ARRAY);
			qglColor4f(1.0, 1.0, 1.0, 1.0);
		}

		// envmap pass
		if (skinParms.envmap > 0.0f && !shellModel)
		{
			GL_Enable (GL_BLEND);
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			// apply alpha to array
			for (i=0; i<mesh.num_verts; i++) 
				//colorArray[i][3] = R_CalcEntAlpha(meshalpha*skinParms.envmap, tempVertexArray[meshnum][i]);
				colorArray[i][3] = meshalpha*skinParms.envmap;

			GL_Bind(r_envmappic->texnum);

			qglEnable(GL_TEXTURE_GEN_S);
			qglEnable(GL_TEXTURE_GEN_T);

			RB_DrawArrays (GL_TRIANGLES);

			qglDisable(GL_TEXTURE_GEN_S);
			qglDisable(GL_TEXTURE_GEN_T);
		}

		RB_DrawMeshTris (GL_TRIANGLES, 1);

		// md3 skin scripting
		if (skinParms.twosided)
			GL_Enable(GL_CULL_FACE);
		if (skinParms.alphatest && !shellModel)
			GL_Disable(GL_ALPHA_TEST);
		GL_Disable (GL_BLEND);
		// md3 skin scripting

	} // end new outer loop
	
	R_SetShellBlend (false);
	R_SetVertexOverbrights(false);
}


int		shadow_va, shadow_index;
/*
=============
R_BuildShadowVolume
based on code from BeefQuake R6
=============
*/
void R_BuildShadowVolume (maliasmodel_t *hdr, int meshnum, vec3_t light, float projectdistance, qboolean nocap)
{
	int				i, j, baseindex;
	BOOL			trianglefacinglight[MD3_MAX_TRIANGLES];
	vec3_t			v0, v1, v2, v3;
	float			thisAlpha;
	maliasmesh_t	mesh;
	maliasvertex_t	*verts;

	mesh = hdr->meshes[meshnum];

	if (mesh.skins[currententity->skinnum].renderparms.nodraw
		|| mesh.skins[currententity->skinnum].renderparms.alphatest
		|| mesh.skins[currententity->skinnum].renderparms.noshadow)
		return;

	verts = mesh.vertexes;

	thisAlpha = aliasShadowAlpha; // was r_shadowalpha->value

	for (i=0; i<mesh.num_tris; i++)
	{
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+0]], v0);
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+1]], v1);
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+2]], v2);

		trianglefacinglight[i] =
			(light[0] - v0[0]) * ((v0[1] - v1[1]) * (v2[2] - v1[2]) - (v0[2] - v1[2]) * (v2[1] - v1[1]))
			+ (light[1] - v0[1]) * ((v0[2] - v1[2]) * (v2[0] - v1[0]) - (v0[0] - v1[0]) * (v2[2] - v1[2]))
			+ (light[2] - v0[2]) * ((v0[0] - v1[0]) * (v2[1] - v1[1]) - (v0[1] - v1[1]) * (v2[0] - v1[0])) > 0;
	}

	shadow_va = shadow_index = 0;
	for (i=0; i<mesh.num_tris; i++)
	{
		if (!trianglefacinglight[i])
			continue;

		if (mesh.trneighbors[i*3+0] < 0 || !trianglefacinglight[mesh.trneighbors[i*3+0]])
		{
			baseindex = shadow_va;
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;

			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+1;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+3;
		}

		if (mesh.trneighbors[i*3+1] < 0 || !trianglefacinglight[mesh.trneighbors[i*3+1]])
		{
			baseindex = shadow_va;
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;

			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+1;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+3;
		}

		if (mesh.trneighbors[i*3+2] < 0 || !trianglefacinglight[mesh.trneighbors[i*3+2]])
		{
			baseindex = shadow_va;
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			shadow_va++;

			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+1;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+0;
			indexArray[shadow_index++] = baseindex+2;
			indexArray[shadow_index++] = baseindex+3;
		}
	}

	if (nocap)	return;

	// cap the volume
	for (i=0; i<mesh.num_tris; i++)
	{
		if (trianglefacinglight[i])
		{
			VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+0]], v0);
			VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+1]], v1);
			VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+2]], v2);

			VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			indexArray[shadow_index++] = shadow_va;
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			indexArray[shadow_index++] = shadow_va;
			shadow_va++;
			VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
			indexArray[shadow_index++] = shadow_va;
			shadow_va++;
			continue;
		}

		for (j=0; j<3; j++)
		{
			v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
			v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
			v2[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];

			v0[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			v1[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
			v2[j]=v2[j]+((v2[j]-light[j]) * projectdistance);
		}
		VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
		VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
		indexArray[shadow_index++] = shadow_va;
		shadow_va++;
		VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
		VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
		indexArray[shadow_index++] = shadow_va;
		shadow_va++;
		VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
		VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
		indexArray[shadow_index++] = shadow_va;
		shadow_va++;
	}
}


/*
=============
R_DrawShadowVolume 
=============
*/
void R_DrawShadowVolume (void)
{
	if (gl_config.drawRangeElements)
		qglDrawRangeElementsEXT(GL_TRIANGLES, 0, shadow_va, shadow_index, GL_UNSIGNED_INT, indexArray);
	else
		qglDrawElements(GL_TRIANGLES, shadow_index, GL_UNSIGNED_INT, indexArray);
}


/*
=============
R_DrawAliasVolumeShadow
based on code from BeefQuake R6
=============
*/
void R_DrawAliasVolumeShadow (maliasmodel_t *paliashdr, vec3_t bbox[8])
{
	vec3_t		light, temp, vecAdd;
	float		dist, highest, lowest, projected_distance;
	float		angle, cosp, sinp, cosy, siny, cosr, sinr, ix, iy, iz;
	int			i, lnum;
	dlight_t	*dl;

	dl = r_newrefdef.dlights;

	VectorSet(vecAdd, 680,0,1024); // set base vector, was 576,0,1024

	// compute average light vector from dlights
	for (i=0, lnum=0; i<r_newrefdef.num_dlights; i++, dl++)
	{
		if (VectorCompare(dl->origin, currententity->origin))
			continue;
		
		VectorSubtract(dl->origin, currententity->origin, temp);
		dist = dl->intensity - VectorLength(temp);
		if (dist <= 0)
			continue;
		
		lnum++;
		// Factor in the intensity of a dlight
		VectorScale (temp, dist*0.25, temp);
		VectorAdd (vecAdd, temp, vecAdd);
	}
	VectorNormalize(vecAdd);
	VectorScale(vecAdd, 1024, vecAdd);

	// get projection distance from lightspot height
	highest = lowest = bbox[0][2];
	for (i=0; i<8; i++) {
		if (bbox[i][2] > highest) highest = bbox[i][2];
		if (bbox[i][2] < lowest) lowest = bbox[i][2];
	}
	projected_distance = (fabs(highest - lightspot[2]) + (highest-lowest)) / vecAdd[2];

	VectorCopy(vecAdd, light);
	
	/*cosy = cos(-currententity->angles[YAW] / 180 * M_PI);
	siny = sin(-currententity->angles[YAW] / 180 * M_PI);

	ix = light[0], iy = light[1];
	light[0] = (cosy * (ix - 0) + siny * (0 - iy) + 0);
	light[1] = (cosy * (iy - 0) + siny * (ix - 0) + 0);
	light[2] += 8;*/

	// reverse-rotate light vector based on angles
	angle = -currententity->angles[PITCH] / 180 * M_PI;
	cosp = cos(angle), sinp = sin(angle);
	angle = -currententity->angles[YAW] / 180 * M_PI;
	cosy = cos(angle), siny = sin(angle);
	angle = currententity->angles[ROLL] / 180 * M_PI; // roll is backwards
	cosr = cos(angle), sinr = sin(angle);

	// rotate for yaw (z axis)
	ix = light[0], iy = light[1];
	light[0] = cosy * ix - siny * iy + 0;
	light[1] = siny * ix + cosy * iy + 0;

	// rotate for pitch (y axis)
	ix = light[0], iz = light[2];
	light[0] = cosp * ix + 0 + sinp * iz;
	light[2] = -sinp * ix + 0 + cosp * iz;

	// rotate for roll (x axis)
	iy = light[1], iz = light[2];
	light[1] = 0 + cosr * iy - sinr * iz;
	light[2] = 0 + sinr * iy + cosr * iz;


	// set up stenciling
	if (!r_shadowvolumes->value)
	{
		qglPushAttrib(GL_STENCIL_BUFFER_BIT); // save stencil buffer
		qglClear(GL_STENCIL_BUFFER_BIT);

		qglColorMask(0,0,0,0);
		GL_DepthMask(0);
		GL_DepthFunc(GL_LESS);

		GL_Enable(GL_STENCIL_TEST);
		qglStencilFunc(GL_ALWAYS, 0, 255);
	//	qglStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
	//	qglStencilMask (255);
	}

	// build shadow volumes and render each to stencil buffer
	for (i=0; i<paliashdr->num_meshes; i++)
	{
		if (paliashdr->meshes[i].skins[currententity->skinnum].renderparms.nodraw
			|| paliashdr->meshes[i].skins[currententity->skinnum].renderparms.alphatest
			|| paliashdr->meshes[i].skins[currententity->skinnum].renderparms.noshadow)
			continue;

		R_BuildShadowVolume (paliashdr, i, light, projected_distance, r_shadowvolumes->value);
		GL_LockArrays (shadow_va);

		if (!r_shadowvolumes->value)
		{
			if (gl_config.atiSeparateStencil && gl_config.extStencilWrap) // Barnes ATI stenciling
			{
				GL_Disable(GL_CULL_FACE);

				qglStencilOpSeparateATI (GL_BACK, GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP); 
				qglStencilOpSeparateATI (GL_FRONT, GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);

				R_DrawShadowVolume ();

				GL_Enable(GL_CULL_FACE);
			}
			else if (gl_config.extStencilTwoSide && gl_config.extStencilWrap) // Echon's two-sided stenciling
			{
				GL_Disable(GL_CULL_FACE);
				qglEnable (GL_STENCIL_TEST_TWO_SIDE_EXT);

				qglActiveStencilFaceEXT (GL_BACK);
				qglStencilOp (GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
				qglActiveStencilFaceEXT (GL_FRONT);
				qglStencilOp (GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);

				R_DrawShadowVolume ();

				qglDisable (GL_STENCIL_TEST_TWO_SIDE_EXT);
				GL_Enable(GL_CULL_FACE);
			}
			else
			{	// increment stencil if backface is behind depthbuffer
				GL_CullFace(GL_BACK); // quake is backwards, this culls front faces
				qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
				R_DrawShadowVolume ();

				// decrement stencil if frontface is behind depthbuffer
				GL_CullFace(GL_FRONT); // quake is backwards, this culls back faces
				qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
				R_DrawShadowVolume ();
			}
		}
		else
			R_DrawShadowVolume ();

		GL_UnlockArrays ();
	}

	// end stenciling and draw stenciled volume
	if (!r_shadowvolumes->value)
	{
		GL_CullFace(GL_FRONT);
		GL_Disable(GL_STENCIL_TEST);
		
		GL_DepthFunc(GL_LEQUAL);
		GL_DepthMask(1);
		qglColorMask(1,1,1,1);
		
		// draw shadows for this model now
		R_ShadowBlend (aliasShadowAlpha * currententity->alpha); // was r_shadowalpha->value
		qglPopAttrib(); // restore stencil buffer
	}
}


/*
=================
R_DrawAliasPlanarShadow
=================
*/
void R_DrawAliasPlanarShadow (maliasmodel_t *paliashdr)
{
	maliasmesh_t	mesh;
	float	height, lheight, thisAlpha;
	vec3_t	point, shadevector;
	int		i, j;

	R_ShadowLight (currententity->origin, shadevector);

	lheight = currententity->origin[2] - lightspot[2];
	height = -lheight + 0.1f;
	if (currententity->flags & RF_TRANSLUCENT)
		thisAlpha = aliasShadowAlpha * currententity->alpha; // was r_shadowalpha->value
	else
		thisAlpha = aliasShadowAlpha; // was r_shadowalpha->value

	// don't draw shadows above view origin, thnx to MrG
	if (r_newrefdef.vieworg[2] < (currententity->origin[2] + height))
		return;

	GL_Stencil (true, false);
	GL_BlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);

	rb_vertex = rb_index = 0;
	for (i=0; i<paliashdr->num_meshes; i++) 
	{
		mesh = paliashdr->meshes[i];

		if (mesh.skins[currententity->skinnum].renderparms.nodraw
			|| mesh.skins[currententity->skinnum].renderparms.alphatest
			|| mesh.skins[currententity->skinnum].renderparms.noshadow)
			continue;

		for (j=0; j<mesh.num_verts; j++)
		{
			VectorCopy(tempVertexArray[i][j], point);
			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
			VA_SetElem3(vertexArray[rb_vertex], point[0], point[1], point[2]);
			VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, thisAlpha);
			rb_vertex++;
		}

		for (j=0; j < mesh.num_tris; j++)
		{
			indexArray[rb_index++] = mesh.indexes[3*j+0];
			indexArray[rb_index++] = mesh.indexes[3*j+1];
			indexArray[rb_index++] = mesh.indexes[3*j+2];
		}
	}
	RB_DrawArrays (GL_TRIANGLES);

	GL_Stencil (false, false);
}


/*
=================
R_CullAliasModel
=================
*/
static qboolean R_CullAliasModel ( vec3_t bbox[8], entity_t *e )
{
	int			i, j;
	vec3_t		mins, maxs, tmp; //angles;
	vec3_t		vectors[3];
	maliasmodel_t	*paliashdr;
	maliasframe_t	*pframe, *poldframe;
	int			mask, aggregatemask = ~0;			

	paliashdr = (maliasmodel_t *)currentmodel->extradata;

	if ( ( e->frame >= paliashdr->num_frames ) || ( e->frame < 0 ) )
	{
		VID_Printf (PRINT_ALL, "R_CullAliasModel %s: no such frame %d\n", 
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ( ( e->oldframe >= paliashdr->num_frames ) || ( e->oldframe < 0 ) )
	{
		VID_Printf (PRINT_ALL, "R_CullAliasModel %s: no such oldframe %d\n", 
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = paliashdr->frames + e->frame;
	poldframe = paliashdr->frames + e->oldframe;

	// compute axially aligned mins and maxs
	if ( pframe == poldframe )
	{
		VectorCopy(pframe->mins, mins);
		VectorCopy(pframe->maxs, maxs);
	}
	else
	{
		for ( i = 0; i < 3; i++ )
		{
			if (pframe->mins[i] < poldframe->mins[i])
				mins[i] = pframe->mins[i];
			else
				mins[i] = poldframe->mins[i];

			if (pframe->maxs[i] > poldframe->maxs[i])
				maxs[i] = pframe->maxs[i];
			else
				maxs[i] = poldframe->maxs[i];
		}
	}

	// jitspoe's bbox rotation fix
	// compute and rotate bonding box
	e->angles[ROLL] = -e->angles[ROLL]; // roll is backwards
	AngleVectors(e->angles, vectors[0], vectors[1], vectors[2]);
	e->angles[ROLL] = -e->angles[ROLL]; // roll is backwards
	VectorSubtract(vec3_origin, vectors[1], vectors[1]); // AngleVectors returns "right" instead of "left"
	for (i = 0; i < 8; i++)
	{
		tmp[0] = ((i & 1) ? mins[0] : maxs[0]);
		tmp[1] = ((i & 2) ? mins[1] : maxs[1]);
		tmp[2] = ((i & 4) ? mins[2] : maxs[2]);

		bbox[i][0] = vectors[0][0] * tmp[0] + vectors[1][0] * tmp[1] + vectors[2][0] * tmp[2] + e->origin[0];
		bbox[i][1] = vectors[0][1] * tmp[0] + vectors[1][1] * tmp[1] + vectors[2][1] * tmp[2] + e->origin[1];
		bbox[i][2] = vectors[0][2] * tmp[0] + vectors[1][2] * tmp[1] + vectors[2][2] * tmp[2] + e->origin[2];
	}

	// cull
	for (i=0; i<8; i++)
	{
		mask = 0;
		for (j=0; j<4; j++)
		{
			float dp = DotProduct(frustum[j].normal, bbox[i]);
			if ( ( dp - frustum[j].dist ) < 0 )
				mask |= (1<<j);
		}

		aggregatemask &= mask;
	}

	if ( aggregatemask )
		return true;

	return false;
}


/*
=================
R_DrawAliasModel
=================
*/
void R_DrawAliasModel (entity_t *e)
{
	maliasmodel_t	*paliashdr;
	vec3_t		bbox[8];
	qboolean	mirrormodel = false;
	int			i;

	// also skip this for viewermodels and cameramodels
	if ( !(e->flags & RF_WEAPONMODEL || e->flags & RF_VIEWERMODEL || e->renderfx & RF2_CAMERAMODEL) )
	{
		if (R_CullAliasModel(bbox, e))
			return;
	}

	// mirroring support
	if (e->flags & RF_WEAPONMODEL)
	{
		if (r_lefthand->value == 2)
			return;
		else if (r_lefthand->value == 1)
			mirrormodel = true;
	}
	else if (e->renderfx & RF2_CAMERAMODEL)
	{
		if (r_lefthand->value==1)
			mirrormodel = true;
	}
	else if (e->flags & RF_MIRRORMODEL)
		mirrormodel = true;
	// end mirroring support

	paliashdr = (maliasmodel_t *)currentmodel->extradata;

	R_SetShadeLight ();

	if (e->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
	{
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			GL_DepthRange (gldepthmin, gldepthmin + 0.01*(gldepthmax-gldepthmin));
		else
			GL_DepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
	}

	// mirroring support
	if (mirrormodel)
		R_FlipModel(true);

	for (i=0; i < paliashdr->num_meshes; i++)
		c_alias_polys += paliashdr->meshes[i].num_tris;

	qglPushMatrix ();
	e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards
	R_RotateForEntity (e, true);
	e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards

	GL_ShadeModel (GL_SMOOTH);

	GL_TexEnv(GL_MODULATE);

	if ( (e->frame >= paliashdr->num_frames) || (e->frame < 0) )
	{
		VID_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such frame %d\n", currentmodel->name, e->frame);
		e->frame = 0;
		e->oldframe = 0;
	}

	if ( (e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0))
	{
		VID_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, e->oldframe);
		e->frame = 0;
		e->oldframe = 0;
	}

	if (!r_lerpmodels->value)
		e->backlerp = 0;

	R_DrawAliasFrameLerp (paliashdr, e);

	GL_TexEnv(GL_REPLACE);
	GL_ShadeModel (GL_FLAT);

	qglPopMatrix ();

	// mirroring support
	if (mirrormodel)
		R_FlipModel(false);

	// show model bounding box
	R_DrawAliasModelBBox (bbox, e);

	if (e->flags & RF_DEPTHHACK)
		GL_DepthRange (gldepthmin, gldepthmax);

	aliasShadowAlpha = R_CalcShadowAlpha(e);

	if (!(e->flags & (RF_WEAPONMODEL | RF_NOSHADOW))
		// no shadows from shells
		&& !( (e->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM)) && (e->flags & RF_TRANSLUCENT) )
		&& r_shadows->value >= 1 && aliasShadowAlpha >= DIV255)
	{
 		qglPushMatrix ();
		GL_DisableTexture(0);
		GL_Enable (GL_BLEND);

		if (r_shadows->value == 3) {
			e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards
			R_RotateForEntity (e, true);
			e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards
			R_DrawAliasVolumeShadow (paliashdr, bbox);
		}
		else {
			R_RotateForEntity (e, false);
			R_DrawAliasPlanarShadow (paliashdr);
		}

		GL_Disable (GL_BLEND);
		GL_EnableTexture(0);
		qglPopMatrix ();
	}
}

#if 0
/*
=================
R_DrawAliasModelShadow
Just draws the shadow for a model
=================
*/
void R_DrawAliasModelShadow (entity_t *e)
{
	maliasmodel_t	*paliashdr;
	vec3_t		bbox[8];
	qboolean	mirrormodel = false;

	if (!r_shadows->value)
		return;
	if (e->flags & (RF_WEAPONMODEL | RF_NOSHADOW))
		return;
	// no shadows from shells
	if ( (e->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM))
		&& (e->flags & RF_TRANSLUCENT) )
		return;

	// also skip this for viewermodels and cameramodels
	if ( !(e->flags & RF_WEAPONMODEL || e->flags & RF_VIEWERMODEL || e->renderfx & RF2_CAMERAMODEL) )
	{
		if (R_CullAliasModel(bbox, e))
			return;
	}

	aliasShadowAlpha = R_CalcShadowAlpha(e);
	if (aliasShadowAlpha < DIV255) // out of range
		return;

	if (e->renderfx & RF2_CAMERAMODEL)
	{
		if (r_lefthand->value==1)
			mirrormodel = true;
	}
	else if (e->flags & RF_MIRRORMODEL)
		mirrormodel = true;

	paliashdr = (maliasmodel_t *)currentmodel->extradata;

	// mirroring support
//	if (mirrormodel)
//		R_FlipModel(true);

	if ( (e->frame >= paliashdr->num_frames) || (e->frame < 0) )
	{
		e->frame = 0;
		e->oldframe = 0;
	}

	if ( (e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0))
	{
		e->frame = 0;
		e->oldframe = 0;
	}

	//if ( !r_lerpmodels->value )
	//	e->backlerp = 0;
	
	qglPushMatrix ();
	GL_DisableTexture(0);
	GL_Enable (GL_BLEND);
				
	if (r_shadows->value == 3) {
		e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards
		R_RotateForEntity (e, true);
		e->angles[ROLL] = -e->angles[ROLL];		// roll is backwards
		R_DrawAliasVolumeShadow (paliashdr, bbox);
	}
	else {
		R_RotateForEntity (e, false);
		R_DrawAliasPlanarShadow (paliashdr);
	}

	GL_Disable (GL_BLEND);
	GL_EnableTexture(0);
	qglPopMatrix ();

	// mirroring support
//	if (mirrormodel)
//		R_FlipModel(false);
}
#endif
