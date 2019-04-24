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

vec3_t	aliasLightDir = {0, 0, 0};
float	aliasShadowAlpha;

/*
=================
R_LightAliasModel
=================
*/
void R_LightAliasModel (vec3_t baselight, vec3_t normal, vec3_t lightOut, byte normalindex, qboolean shaded)
{
	int		i;
	float	l;

	if (r_fullbright->value != 0) {
		VectorSet (lightOut, 1.0f, 1.0f, 1.0f);
		return;
	}

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
R_AliasMeshesAreBatchable
=================
*/
qboolean R_AliasMeshesAreBatchable (maliasmodel_t *paliashdr, unsigned meshnum1, unsigned meshnum2, unsigned skinnum)
{
	maliasmesh_t	*mesh1, *mesh2;
	renderparms_t	*skinParms1, *skinParms2;
	int				skinnum1, skinnum2;

	if (!paliashdr)
		return false;

	mesh1 = &paliashdr->meshes[meshnum1];
	mesh2 = &paliashdr->meshes[meshnum2];
	skinnum1 = (skinnum<mesh1->num_skins)?skinnum:0;
	skinnum2 = (skinnum<mesh2->num_skins)?skinnum:0;
	skinParms1 = &mesh1->skins[skinnum1].renderparms;
	skinParms2 = &mesh2->skins[skinnum2].renderparms;

	if (!mesh1 || !mesh2 || !skinParms1 || !skinParms2)
		return false;

	if (currentmodel->skins[meshnum1][skinnum1] != currentmodel->skins[meshnum2][skinnum2])
		return false;
	if (mesh1->skins[skinnum1].glowimage != mesh2->skins[skinnum2].glowimage)
		return false;
	if (skinParms1->alphatest != skinParms2->alphatest)
		return false;
	if (skinParms1->basealpha != skinParms2->basealpha)
		return false;
	if (skinParms1->blend != skinParms2->blend)
		return false;
	if (skinParms1->blendfunc_src != skinParms2->blendfunc_src)
		return false;
	if (skinParms1->blendfunc_dst != skinParms2->blendfunc_dst)
		return false;
	if (skinParms1->envmap != skinParms2->envmap)
		return false;
	if ( (skinParms1->glow.type != skinParms2->glow.type)
		|| (skinParms1->glow.params[0] != skinParms2->glow.params[0])
		|| (skinParms1->glow.params[1] != skinParms2->glow.params[1])
		|| (skinParms1->glow.params[2] != skinParms2->glow.params[2])
		|| (skinParms1->glow.params[3] != skinParms2->glow.params[3]) )
		return false;
	if (skinParms1->nodraw != skinParms2->nodraw)
		return false;
	if (skinParms1->twosided != skinParms2->twosided)
		return false;

	return true;
}


/*
=================
RB_RenderAliasMesh

Backend for R_DrawAliasMeshes
=================
*/
void RB_RenderAliasMesh (maliasmodel_t *paliashdr, unsigned meshnum, unsigned skinnum, image_t *skin)
{
	entity_t		*e = currententity;
	maliasmesh_t	*mesh;
	renderparms_t	skinParms;
	int				i;
	float			thisalpha = colorArray[0][3];
	qboolean		shellModel = e->flags & RF_MASK_SHELL;

	if (!paliashdr)
		return;

	mesh = &paliashdr->meshes[meshnum];

	if (!shellModel)
		GL_Bind(skin->texnum);

	// md3 skin scripting
	skinParms = mesh->skins[skinnum].renderparms;

	if (skinParms.twosided)
		GL_Disable (GL_CULL_FACE);
	else
		GL_Enable (GL_CULL_FACE);

	if (skinParms.alphatest && !shellModel)
		GL_Enable (GL_ALPHA_TEST);
	else
		GL_Disable (GL_ALPHA_TEST);

	if (thisalpha < 1.0f || skinParms.blend)
		GL_Enable (GL_BLEND);
	else
		GL_Disable (GL_BLEND);

	if (skinParms.blend && !shellModel)
		GL_BlendFunc (skinParms.blendfunc_src, skinParms.blendfunc_dst);
	else if (shellModel)
		GL_BlendFunc (GL_ONE, GL_ONE);
	else
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// md3 skin scripting

	// draw
	RB_DrawArrays ();

	// glow pass
	if (mesh->skins[skinnum].glowimage && !shellModel)
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

		GL_Bind(mesh->skins[skinnum].glowimage->texnum);

		RB_DrawArrays ();

		qglColor4f(1.0, 1.0, 1.0, 1.0);
		qglEnableClientState (GL_COLOR_ARRAY);
	}

	// envmap pass
	if (skinParms.envmap > 0.0f && !shellModel)
	{
		GL_Enable (GL_BLEND);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		// apply alpha to array
		for (i=0; i<rb_vertex; i++)
			colorArray[i][3] = thisalpha*skinParms.envmap;

		GL_Bind(glMedia.envmappic->texnum);

		qglEnable(GL_TEXTURE_GEN_S);
		qglEnable(GL_TEXTURE_GEN_T);

		RB_DrawArrays ();

		qglDisable(GL_TEXTURE_GEN_S);
		qglDisable(GL_TEXTURE_GEN_T);
	}

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;

	// restore state
	GL_Enable (GL_CULL_FACE);
	GL_Disable (GL_ALPHA_TEST);
	GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/*
=================
R_DrawAliasMeshes
=================
*/
void R_DrawAliasMeshes (maliasmodel_t *paliashdr, entity_t *e, qboolean lerpOnly, qboolean mirrored)
{
	int				i, k, meshnum, skinnum, baseindex;	// numCalls
	maliasframe_t	*frame, *oldframe;
	maliasmesh_t	mesh;
	maliasvertex_t	*v, *ov;
	vec3_t			move, delta, vectors[3];
	vec3_t			curScale, oldScale, curNormal, oldNormal;
	vec3_t			tempNormalsArray[MD3_MAX_VERTS];
	vec2_t			tempSkinCoord;
	vec3_t			meshlight, lightcolor;
	float			alpha, meshalpha, thisalpha, shellscale, frontlerp, backlerp = e->backlerp, mirrormult;
	image_t			*skin;
	renderparms_t	skinParms;
	qboolean		shellModel = e->flags & RF_MASK_SHELL;

	frontlerp = 1.0 - backlerp;

	if (shellModel && FlowingShell())
		alpha = 0.7;
	else if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;
	else
		alpha = 1.0;

	frame = paliashdr->frames + e->frame;
	oldframe = paliashdr->frames + e->oldframe;

	VectorScale(frame->scale, frontlerp, curScale);
	VectorScale(oldframe->scale, backlerp, oldScale);

	mirrormult = (mirrored) ? -1.0f : 1.0f;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (e->oldorigin, e->origin, delta);
	AngleVectors (e->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i=0 ; i<3 ; i++)
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];

	GL_ShadeModel (GL_SMOOTH);
	GL_TexEnv (GL_MODULATE);
	R_SetVertexRGBScale(true);
	R_SetShellBlend (true);

	rb_vertex = rb_index = 0;
//	numCalls = 0;

	// new outer loop for whole model
	for (k=0, meshnum=0; k < paliashdr->num_meshes; k++, meshnum++)
	{
		mesh = paliashdr->meshes[k];

		// select skin
		if (e->skin) {	// custom player skin
			skinnum = 0;
			skin = e->skin;
		}
		else {
			skinnum = (e->skinnum<mesh.num_skins)?e->skinnum:0; // catch bad skinnums
			skin = currentmodel->skins[k][skinnum];
			if (!skin) {
				skinnum = 0;
				skin = currentmodel->skins[k][0];
			}
		}
		if (!skin) {
			skinnum = 0;
			skin = glMedia.notexture;
		}

		// md3 skin scripting
		skinParms = mesh.skins[skinnum].renderparms;

		if (skinParms.nodraw) 
			continue; // skip this mesh for this skin

		if (skinParms.fullbright)
			VectorSet(meshlight, 1.0f, 1.0f, 1.0f);
		else
			VectorCopy(shadelight, meshlight);

		meshalpha = alpha * skinParms.basealpha;
		// md3 skin scripting

		v = mesh.vertexes + e->frame * mesh.num_verts;
		ov = mesh.vertexes + e->oldframe * mesh.num_verts;
		baseindex = rb_vertex;

		// set indices for each triangle
		for (i=0; i<mesh.num_tris; i++)
		{
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*i+0];
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*i+1];
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*i+2];
		}

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
					move[0] + ov->xyz[0]*oldScale[0] + v->xyz[0]*curScale[0] + tempNormalsArray[i][0]*shellscale,
					mirrormult * (move[1] + ov->xyz[1]*oldScale[1] + v->xyz[1]*curScale[1] + tempNormalsArray[i][1]*shellscale),
					move[2] + ov->xyz[2]*oldScale[2] + v->xyz[2]*curScale[2] + tempNormalsArray[i][2]*shellscale );

			// skip drawing if we're only lerping the verts for a shadow-only rendering pass
			if (lerpOnly)	continue;

			tempNormalsArray[i][1] *= mirrormult;

			// calc lighting and alpha
			if (shellModel)
				VectorCopy(meshlight, lightcolor);
			else
				R_LightAliasModel (meshlight, tempNormalsArray[i], lightcolor, v->lightnormalindex, !skinParms.nodiffuse);
			//thisalpha = R_CalcEntAlpha(meshalpha, tempVertexArray[meshnum][i]);
			thisalpha = meshalpha;

			// get tex coords
			if (shellModel && FlowingShell()) {
				tempSkinCoord[0] = (tempVertexArray[meshnum][i][0] + tempVertexArray[meshnum][i][1]) * DIV40 + shellFlowH;
				tempSkinCoord[1] = tempVertexArray[meshnum][i][2] * DIV40 + shellFlowV; // was / 40
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
			RB_ModifyTextureCoords (&texCoordArray[0][baseindex][0], &vertexArray[baseindex][0], mesh.num_verts, skinParms);

		// compare renderparms for next mesh and check for overflow
		if ( k < (paliashdr->num_meshes-1) ) {
			if ( ( shellModel || R_AliasMeshesAreBatchable (paliashdr, k, k+1, e->skinnum) )
				&& !RB_CheckArrayOverflow (paliashdr->meshes[k+1].num_verts, paliashdr->meshes[k+1].num_tris*3) )
				continue;
		}

		RB_RenderAliasMesh (paliashdr, meshnum, skinnum, skin);

	//	numCalls++;
	} // end new outer loop

//	if (paliashdr->num_meshes > numCalls)
//		VID_Printf (PRINT_DEVELOPER, "%s: rendered %i  meshes in %i pass(es)\n", currentmodel->name, paliashdr->num_meshes, numCalls);
	
	R_SetShellBlend (false);
	R_SetVertexRGBScale(false);
	GL_TexEnv (GL_REPLACE);
	GL_ShadeModel (GL_FLAT);
}


unsigned	shadow_va, shadow_index;
/*
=============
R_BuildShadowVolume
based on code from BeefQuake R6
=============
*/
void R_BuildShadowVolume (maliasmodel_t *hdr, int meshnum, vec3_t light, float projectdistance, qboolean nocap)
{
	int				i, j;
	BOOL			triangleFacingLight[MD3_MAX_TRIANGLES];
	vec3_t			v0, v1, v2, v3;
	float			thisAlpha;
	maliasmesh_t	mesh;
	maliasvertex_t	*verts;

	mesh = hdr->meshes[meshnum];

	verts = mesh.vertexes;

	thisAlpha = aliasShadowAlpha; // was r_shadowalpha->value

	for (i=0; i<mesh.num_tris; i++)
	{
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+0]], v0);
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+1]], v1);
		VectorCopy(tempVertexArray[meshnum][mesh.indexes[3*i+2]], v2);

		triangleFacingLight[i] =
			(light[0] - v0[0]) * ((v0[1] - v1[1]) * (v2[2] - v1[2]) - (v0[2] - v1[2]) * (v2[1] - v1[1]))
			+ (light[1] - v0[1]) * ((v0[2] - v1[2]) * (v2[0] - v1[0]) - (v0[0] - v1[0]) * (v2[2] - v1[2]))
			+ (light[2] - v0[2]) * ((v0[0] - v1[0]) * (v2[1] - v1[1]) - (v0[1] - v1[1]) * (v2[0] - v1[0])) > 0;
	}

	shadow_va = shadow_index = 0;
	for (i=0; i<mesh.num_tris; i++)
	{
		if (!triangleFacingLight[i])
			continue;

		if (mesh.trneighbors[i*3+0] < 0 || !triangleFacingLight[mesh.trneighbors[i*3+0]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+1;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+3;

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
		}

		if (mesh.trneighbors[i*3+1] < 0 || !triangleFacingLight[mesh.trneighbors[i*3+1]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+1;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+3;

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
		}

		if (mesh.trneighbors[i*3+2] < 0 || !triangleFacingLight[mesh.trneighbors[i*3+2]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
				v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];
				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+1;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+0;
			indexArray[shadow_index++] = shadow_va+2;
			indexArray[shadow_index++] = shadow_va+3;

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
		}
	}

	if (nocap)	return;

	// cap the volume
	for (i=0; i<mesh.num_tris; i++)
	{
		if (!triangleFacingLight[i]) // changed to draw only front facing polys- thanx to Kirk Barnes
			continue;

	//	if (triangleFacingLight[i])
	//	{
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
	//		continue;
	//	}

		// rear with reverse order
		for (j=0; j<3; j++)
		{
			v0[j]=tempVertexArray[meshnum][mesh.indexes[3*i+0]][j];
			v1[j]=tempVertexArray[meshnum][mesh.indexes[3*i+1]][j];
			v2[j]=tempVertexArray[meshnum][mesh.indexes[3*i+2]][j];

			v0[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			v1[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
			v2[j]=v2[j]+((v2[j]-light[j]) * projectdistance);
		}
		VA_SetElem3(vertexArray[shadow_va], v2[0], v2[1], v2[2]);
		VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
		indexArray[shadow_index++] = shadow_va;
		shadow_va++;
		VA_SetElem3(vertexArray[shadow_va], v1[0], v1[1], v1[2]);
		VA_SetElem4(colorArray[shadow_va], 0, 0, 0, thisAlpha);
		indexArray[shadow_index++] = shadow_va;
		shadow_va++;
		VA_SetElem3(vertexArray[shadow_va], v0[0], v0[1], v0[2]);
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
	if (glConfig.drawRangeElements)
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
	int			i, lnum, skinnum;
	//GLenum		incr, decr;
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
	
	// reverse-rotate light vector based on angles
	angle = -currententity->angles[PITCH] / 180 * M_PI;
	cosp = cos(angle), sinp = sin(angle);
	angle = -currententity->angles[YAW] / 180 * M_PI;
	cosy = cos(angle), siny = sin(angle);
	angle = -currententity->angles[ROLL] / 180 * M_PI * R_RollMult(); // roll is backwards
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
		/*if (glConfig.extStencilWrap)
		{	incr = GL_INCR_WRAP_EXT;	decr = GL_DECR_WRAP_EXT;	}
		else
		{	incr = GL_INCR;				decr = GL_DECR;	}*/

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
		skinnum = (currententity->skinnum<paliashdr->meshes[i].num_skins)?currententity->skinnum:0;
		if (paliashdr->meshes[i].skins[skinnum].renderparms.noshadow)
			continue;

		R_BuildShadowVolume (paliashdr, i, light, projected_distance, r_shadowvolumes->value);
		GL_LockArrays (shadow_va);

		if (!r_shadowvolumes->value)
		{
			if (glConfig.atiSeparateStencil && glConfig.extStencilWrap) // Barnes ATI stenciling
			{
				GL_Disable(GL_CULL_FACE);

				qglStencilOpSeparateATI (GL_BACK, GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP); 
				qglStencilOpSeparateATI (GL_FRONT, GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);

				R_DrawShadowVolume ();

				GL_Enable(GL_CULL_FACE);
			}
			else if (glConfig.extStencilTwoSide && glConfig.extStencilWrap) // Echon's two-sided stenciling
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
	int		i, j, skinnum;

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

		skinnum = (currententity->skinnum<mesh.num_skins)?currententity->skinnum:0;
		if (mesh.skins[skinnum].renderparms.noshadow)
			continue;

		for (j=0; j < mesh.num_tris; j++)
		{
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*j+0];
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*j+1];
			indexArray[rb_index++] = rb_vertex + mesh.indexes[3*j+2];
		}

		for (j=0; j < mesh.num_verts; j++)
		{
			VectorCopy(tempVertexArray[i][j], point);
			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
			VA_SetElem3(vertexArray[rb_vertex], point[0], point[1], point[2]);
			VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, thisAlpha);
			rb_vertex++;
		}
	}
	RB_DrawArrays ();
	rb_vertex = rb_index = 0;

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
	qboolean	mirrorview = false, mirrormodel = false;
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
			mirrorview = true;
	//		mirrormodel = true;
	}
	else if (e->renderfx & RF2_CAMERAMODEL)
	{
		if (r_lefthand->value == 1)
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
//	if (mirrormodel)
//		R_FlipModel (true);
	if (mirrorview || mirrormodel)
		R_FlipModel (true, mirrormodel);

	for (i=0; i < paliashdr->num_meshes; i++)
		c_alias_polys += paliashdr->meshes[i].num_tris;

	qglPushMatrix ();
	e->angles[ROLL] = e->angles[ROLL] * R_RollMult();		// roll is backwards
	R_RotateForEntity (e, true);
	e->angles[ROLL] = e->angles[ROLL] * R_RollMult();		// roll is backwards

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

	R_DrawAliasMeshes (paliashdr, e, false, mirrormodel);

	qglPopMatrix ();

	// mirroring support
//	if (mirrormodel)
//		R_FlipModel (false);
	if (mirrorview || mirrormodel)
		R_FlipModel (false, mirrormodel);

	// show model bounding box
	R_DrawAliasModelBBox (bbox, e, 1.0f, 1.0f, 1.0f, 1.0f);

	if (e->flags & RF_DEPTHHACK)
		GL_DepthRange (gldepthmin, gldepthmax);

	aliasShadowAlpha = R_CalcShadowAlpha(e);

	if ( !(e->flags & (RF_WEAPONMODEL | RF_NOSHADOW))
		// no shadows from shells
		&& !( (e->flags & RF_MASK_SHELL) && (e->flags & RF_TRANSLUCENT) )
		&& r_shadows->value >= 1 && aliasShadowAlpha >= DIV255)
	{
 		qglPushMatrix ();
		GL_DisableTexture(0);
		GL_Enable (GL_BLEND);

		if (r_shadows->value == 3) {
			e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards
			R_RotateForEntity (e, true);
			e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards
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
	if ( (e->flags & RF_MASK_SHELL) && (e->flags & RF_TRANSLUCENT) )
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
	
	R_DrawAliasMeshes (paliashdr, e, true, mirrormodel);

	qglPushMatrix ();
	GL_DisableTexture(0);
	GL_Enable (GL_BLEND);
				
	if (r_shadows->value == 3) {
		e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards
		R_RotateForEntity (e, true);
		e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards
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
