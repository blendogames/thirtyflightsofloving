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

// r_alias_md2.c: MD2-specific triangle model functions (no longer used)

#include "r_local.h"
#include "vlights.h"

#ifndef MD2_AS_MD3

/*
=============================================================

  MD2 MODELS

=============================================================
*/

static	vec4_t	s_lerped[MAX_VERTS];

vec3_t	lightdir_md2;
float	shadowalpha_md2;


/*
=================
R_LerpMD2Verts
=================
*/
void R_LerpMD2Verts (int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3], float normalscale)
{
	int i;

	for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4 )
	{
		float *normal = r_avertexnormals[verts[i].lightnormalindex];

		lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0] + normal[0] * normalscale;
		lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1] + normal[1] * normalscale;
		lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2] + normal[2] * normalscale; 
	}
}


/*
=================
R_LightAliasMD2Model
=================
*/
void R_LightAliasMD2Model (vec3_t baselight, dtrivertx_t *verts, dtrivertx_t *ov, float backlerp, vec3_t lightOut)
{
	int		i;
	float	l; // tmp;

	if (r_model_shading->integer)
	{
		//l = 2.0 * VLight_LerpLight (verts->lightnormalindex, ov->lightnormalindex,
		//						backlerp, lightdir_md2, currententity->angles, false);
		//tmp = shadedots[verts->lightnormalindex] + (shadedots[ov->lightnormalindex] - shadedots[verts->lightnormalindex]) * backlerp;
		if (r_model_shading->integer == 3)
			l = 2.0 * shadedots[verts->lightnormalindex] - 1;
		else if (r_model_shading->integer == 2)
			l = 1.5 * shadedots[verts->lightnormalindex] - 0.5;
		else
			l = shadedots[verts->lightnormalindex];
		VectorScale(baselight, l, lightOut);

		if (model_dlights_num)
			for (i=0; i<model_dlights_num; i++)
			{
				l = 2.0 * VLight_LerpLight (verts->lightnormalindex, ov->lightnormalindex,
					backlerp, model_dlights[i].direction, currententity->angles, true );
				VectorMA(lightOut, l, model_dlights[i].color, lightOut);
			}
	}
	else
	{
		l = 2.0 * VLight_LerpLight (verts->lightnormalindex, ov->lightnormalindex,
								backlerp, lightdir_md2, currententity->angles, false);
		VectorScale(baselight, l, lightOut);
	}

	for (i=0; i<3; i++)
		lightOut[i] = max(min(lightOut[i], 1.0f), 0.0f);
}


/*
=================
R_DrawAliasMD2FrameLerp
=================
*/
void R_DrawAliasMD2FrameLerp (dmdl_t *paliashdr, float backlerp)
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov, *verts;
	int				*order; 
	int				i, count, index_xyz, vertcount, baseindex;
	float			*lerp;
	float			frontlerp, alpha, thisalpha;
	GLenum			mode;
	vec3_t			move, delta, vectors[3];
	vec3_t			frontv, backv, lightcolor;
	vec2_t			tempSkin;
	qboolean		shellModel = e->flags & RF_MASK_SHELL;

	if (currententity->flags & RF_VIEWERMODEL)
		return;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;


	frontlerp = 1.0 - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i=0 ; i<3 ; i++)
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];

	for (i=0 ; i<3 ; i++) {
		frontv[i] = frontlerp*frame->scale[i];
		backv[i] = backlerp*oldframe->scale[i];
	}

	lerp = s_lerped[0];

	if (shellModel)
	{
		if (currententity->flags & RF_WEAPONMODEL)
			R_LerpMD2Verts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, WEAPON_SHELL_SCALE);
		else
			R_LerpMD2Verts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, POWERSUIT_SCALE);
		if (FlowingShell())	alpha = 0.7;
	}
	else
		R_LerpMD2Verts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, 0);

	R_SetVertexRGBScale (true); // added
	R_SetShellBlend (true);

	rb_vertex = rb_index = 0;
	while (1)
	{	// get the vertex count and primitive type
		count = *order++;

		if (!count)
			break;		// done
		if (count < 0) {
			count = -count;
			mode = GL_TRIANGLE_FAN;
		}
		else
			mode = GL_TRIANGLE_STRIP;

		vertcount = count;
		baseindex = rb_vertex;
	
		do {
			// texture coordinates come from the draw list
			// normals and vertexes come from the frame list
			index_xyz = order[2];

			if (shellModel)
				VectorCopy(shadelight, lightcolor);
			else
				R_LightAliasMD2Model (shadelight, &verts[index_xyz], &ov[index_xyz], backlerp, lightcolor);
			//thisalpha = R_CalcEntAlpha(alpha, s_lerped[index_xyz]);
			thisalpha = alpha;
			
			if (shellModel && FlowingShell()) {
				tempSkin[0] = (s_lerped[index_xyz][1] + s_lerped[index_xyz][0]) / 40.0 + shellFlowH;
				tempSkin[1] = s_lerped[index_xyz][2] / 40.0 + shellFlowV;
			}
			else {
				tempSkin[0] = ((float *)order)[0];
				tempSkin[1] = ((float *)order)[1];
			}
			
			VA_SetElem2(texCoordArray[0][rb_vertex], tempSkin[0], tempSkin[1]);
			VA_SetElem3(vertexArray[rb_vertex], s_lerped[index_xyz][0], s_lerped[index_xyz][1], s_lerped[index_xyz][2]);
			VA_SetElem4(colorArray[rb_vertex], lightcolor[0], lightcolor[1], lightcolor[2], thisalpha);
			rb_vertex++;
			order += 3;
		} while (--count);

		// add triangle fan or strip indices to array
		if (mode == GL_TRIANGLE_FAN)
			for (i = 0; i < vertcount-2; i++) {
				indexArray[rb_index++] = baseindex;
				indexArray[rb_index++] = baseindex+i+1;
				indexArray[rb_index++] = baseindex+i+2;
			}
		else // GL_TRIANGLE_STRIP
			for (i = 0; i < vertcount-2; i++) {
				if (i%2 == 0) {
					indexArray[rb_index++] = baseindex+i;
					indexArray[rb_index++] = baseindex+i+1;
					indexArray[rb_index++] = baseindex+i+2;
				}
				else { // backwards order
					indexArray[rb_index++] = baseindex+i+2;
					indexArray[rb_index++] = baseindex+i+1;
					indexArray[rb_index++] = baseindex+i;
				}
			}
	}
	RB_DrawArrays ();

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;

	R_SetShellBlend (false);
	R_SetVertexRGBScale (false); // added
}


unsigned	md2shadow_va, md2shadow_index;
/*
=============
R_BuildMD2ShadowVolume
projection shadows from BeefQuake R6
=============
*/
void R_BuildMD2ShadowVolume (dmdl_t *hdr, vec3_t light, float projectdistance, qboolean nocap)
{
	int				i, j;
	BOOL			trianglefacinglight[MAX_TRIANGLES];
	vec3_t			v0, v1, v2, v3;
	float			thisAlpha;
	dtriangle_t		*ot, *tris;
	daliasframe_t	*frame;
	dtrivertx_t		*verts;

	if (!currentmodel->edge_tri) // paranoia
		return;

	frame = (daliasframe_t *)((byte *)hdr + hdr->ofs_frames 
		+ currententity->frame * hdr->framesize);
	verts = frame->verts;

	ot = tris = (dtriangle_t *)((unsigned char*)hdr + hdr->ofs_tris);

	thisAlpha = shadowalpha_md2; // was r_shadowalpha->value

	for (i=0; i<hdr->num_tris; i++, tris++)
	{
		VectorCopy(s_lerped[tris->index_xyz[0]], v0);
		VectorCopy(s_lerped[tris->index_xyz[1]], v1);
		VectorCopy(s_lerped[tris->index_xyz[2]], v2);

		trianglefacinglight[i] =
			(light[0] - v0[0]) * ((v0[1] - v1[1]) * (v2[2] - v1[2]) - (v0[2] - v1[2]) * (v2[1] - v1[1]))
			+ (light[1] - v0[1]) * ((v0[2] - v1[2]) * (v2[0] - v1[0]) - (v0[0] - v1[0]) * (v2[2] - v1[2]))
			+ (light[2] - v0[2]) * ((v0[0] - v1[0]) * (v2[1] - v1[1]) - (v0[1] - v1[1]) * (v2[0] - v1[0])) > 0;
	}
	
	md2shadow_va = md2shadow_index = 0;
	for (i=0, tris=ot; i<hdr->num_tris; i++, tris++)
	{
		if (!trianglefacinglight[i])
			continue;

		if (currentmodel->edge_tri[i*3+0] < 0 || !trianglefacinglight[currentmodel->edge_tri[i*3+0]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=s_lerped[tris->index_xyz[1]][j];
				v1[j]=s_lerped[tris->index_xyz[0]][j];

				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+1;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+3;

			VA_SetElem3(vertexArray[md2shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
		}

		if (currentmodel->edge_tri[i*3+1] < 0 || !trianglefacinglight[currentmodel->edge_tri[i*3+1]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=s_lerped[tris->index_xyz[2]][j];
				v1[j]=s_lerped[tris->index_xyz[1]][j];

				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+1;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+3;

			VA_SetElem3(vertexArray[md2shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
		}

		if (currentmodel->edge_tri[i*3+2] < 0 || !trianglefacinglight[currentmodel->edge_tri[i*3+2]])
		{
			for (j=0; j<3; j++)
			{
				v0[j]=s_lerped[tris->index_xyz[0]][j];
				v1[j]=s_lerped[tris->index_xyz[2]][j];

				v2[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
				v3[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			}
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+1;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+0;
			indexArray[md2shadow_index++] = md2shadow_va+2;
			indexArray[md2shadow_index++] = md2shadow_va+3;

			VA_SetElem3(vertexArray[md2shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v3[0], v3[1], v3[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			md2shadow_va++;
		}
	}

	if (nocap) {
		//RB_DrawArrays (md2shadow_va, md2shadow_index);
		return;
	}

	// cap the volume
	for (i=0, tris=ot; i<hdr->num_tris; i++, tris++)
	{
		if (trianglefacinglight[i])
		{
			VectorCopy(s_lerped[tris->index_xyz[0]], v0);
			VectorCopy(s_lerped[tris->index_xyz[1]], v1);
			VectorCopy(s_lerped[tris->index_xyz[2]], v2);

			VA_SetElem3(vertexArray[md2shadow_va], v0[0], v0[1], v0[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			indexArray[md2shadow_index++] = md2shadow_va;
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v1[0], v1[1], v1[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			indexArray[md2shadow_index++] = md2shadow_va;
			md2shadow_va++;
			VA_SetElem3(vertexArray[md2shadow_va], v2[0], v2[1], v2[2]);
			VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
			indexArray[md2shadow_index++] = md2shadow_va;
			md2shadow_va++;
			continue;
		}

		for (j=0; j<3; j++)
		{
			v0[j]=s_lerped[tris->index_xyz[0]][j];
			v1[j]=s_lerped[tris->index_xyz[1]][j];
			v2[j]=s_lerped[tris->index_xyz[2]][j];

			v0[j]=v0[j]+((v0[j]-light[j]) * projectdistance);
			v1[j]=v1[j]+((v1[j]-light[j]) * projectdistance);
			v2[j]=v2[j]+((v2[j]-light[j]) * projectdistance);
		}
		VA_SetElem3(vertexArray[md2shadow_va], v0[0], v0[1], v0[2]);
		VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
		indexArray[md2shadow_index++] = md2shadow_va;
		md2shadow_va++;
		VA_SetElem3(vertexArray[md2shadow_va], v1[0], v1[1], v1[2]);
		VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
		indexArray[md2shadow_index++] = md2shadow_va;
		md2shadow_va++;
		VA_SetElem3(vertexArray[md2shadow_va], v2[0], v2[1], v2[2]);
		VA_SetElem4(colorArray[md2shadow_va], 0, 0, 0, thisAlpha);
		indexArray[md2shadow_index++] = md2shadow_va;
		md2shadow_va++;
	}
	//RB_DrawArrays (md2shadow_va, md2shadow_index);
}


/*
=============
R_DrawAliasMD2VolumeShadow
projection shadows from BeefQuake R6
=============
*/
void R_DrawAliasMD2VolumeShadow (dmdl_t *paliashdr, vec3_t bbox[8])
{
	vec3_t		light, temp, vecAdd;//, static_offset;
	int			i, lnum;
	float		projected_distance = 1;
	float		cost, sint, is, it, dist, highest, lowest;//, maxdist = 384;
	qboolean	zfail = true;
	dlight_t	*dl;

	if (!currentmodel->edge_tri) // paranoia
		return;

	dl = r_newrefdef.dlights;

	//VectorSet(static_offset, 576,0,1024); // set static vector, was 144,0,256
	VectorSet(vecAdd, 680,0,1024); // set base vector, was 576,0,1024

	//VectorClear(vecAdd);
	for (i=0, lnum=0; i<r_newrefdef.num_dlights; i++, dl++)
	{
		if (VectorCompare(dl->origin, currententity->origin))
			continue;
		
		VectorSubtract(dl->origin, currententity->origin, temp);
		//dist = sqrt(DotProduct(temp,temp));
		dist = dl->intensity - VectorLength(temp);
		//if (dist > maxdist)
		if (dist <= 0)
			continue;
		
		lnum++;
		// Factor in the intensity of a dlight
		//VectorScale(temp, (dl->intensity * (DIV256)), temp);
		//VectorScale(temp, dl->intensity*0.05*-(dist-maxdist)/maxdist, temp);
		//VectorScale (temp, (dl->intensity - VectorLength(temp))*0.25, temp);
		VectorScale (temp, dist*0.25, temp);
		VectorAdd (vecAdd, temp, vecAdd);
	}
	VectorNormalize(vecAdd);
	VectorScale(vecAdd, 1024, vecAdd);

	highest = lowest = bbox[0][2];
	for (i=0; i<8; i++) {
		if (bbox[i][2] > highest) highest = bbox[i][2];
		if (bbox[i][2] < lowest) lowest = bbox[i][2];
	}
	//VectorSubtract(bbox[0], bbox[4], temp);
	//projected_distance = (fabs(bbox[0][2] - lightspot[2]) + VectorLength(temp)) / vecAdd[2];
	projected_distance = (fabs(highest - lightspot[2]) + (highest-lowest)) / vecAdd[2];

	//if (lnum > 0)
		VectorCopy(vecAdd, light);
	//else
	//	VectorCopy(static_offset, light);

	// adjust vector based on angles
	cost = cos(-currententity->angles[1] / 180 * M_PI);
	sint = sin(-currententity->angles[1] / 180 * M_PI);

	is = light[0], it = light[1];
	light[0] = (cost * (is - 0) + sint * (0 - it) + 0);
	light[1] = (cost * (it - 0) + sint * (is - 0) + 0);
	light[2] += 8;

	if (!r_shadowvolumes->integer)
	{
		qglColorMask(0,0,0,0);
		qglPushAttrib(GL_STENCIL_BUFFER_BIT); // save stencil buffer
		qglClear(GL_STENCIL_BUFFER_BIT);
		GL_Enable(GL_STENCIL_TEST);
		
		GL_DepthMask(0);
		GL_DepthFunc( GL_LESS );
		qglStencilFunc( GL_ALWAYS, 0, 0xFF);
	}

	R_BuildMD2ShadowVolume(paliashdr, light, projected_distance, r_shadowvolumes->integer||!zfail);
	GL_LockArrays(md2shadow_va);

	if (!r_shadowvolumes->integer)
	{	// increment stencil if backface is behind depthbuffer
		if (zfail) { // Carmack reverse
			GL_CullFace(GL_BACK); // quake is backwards, this culls front faces
			qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
		}
		else { // Z-Pass
			GL_CullFace(GL_FRONT); // quake is backwards, this culls back faces
			qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		}
		if (glConfig.drawRangeElements)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, md2shadow_va, md2shadow_index, GL_UNSIGNED_INT, indexArray);
		else
			qglDrawElements(GL_TRIANGLES, md2shadow_index, GL_UNSIGNED_INT, indexArray);

		// decrement stencil if frontface is behind depthbuffer
		if (zfail) { // Carmack reverse
			GL_CullFace(GL_FRONT); // quake is backwards, this culls back faces
			qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		}
		else { // Z-Pass
			GL_CullFace(GL_BACK); // quake is backwards, this culls front faces
			qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		}
	}

	if (glConfig.drawRangeElements)
		qglDrawRangeElementsEXT(GL_TRIANGLES, 0, md2shadow_va, md2shadow_index, GL_UNSIGNED_INT, indexArray);
	else
		qglDrawElements(GL_TRIANGLES, md2shadow_index, GL_UNSIGNED_INT, indexArray);
	GL_UnlockArrays();

	/*for (i=-1; i<r_newrefdef.num_dlights; i++) //, dl++)
	{
		if ((dl->origin[0] == currententity->origin[0]) &&
			(dl->origin[1] == currententity->origin[1]) &&
			(dl->origin[2] == currententity->origin[2]))
			continue;
		
		// -1 pass only for no dynamic lights
		if (i == -1 && r_newrefdef.num_dlights > 0)
			continue;
		
		//	VectorSubtract(a, b, temp);
		//	dist = sqrt(DotProduct(temp, temp));
		
		if (i == -1) // static vector
			VectorAdd(currententity->origin, static_offset, l_origin);
		else // light origin
			VectorCopy(dl->origin, l_origin);
		
		if (i >= 0)
		{
			VectorSubtract(currententity->origin, l_origin, temp); // was l->origin
			dist = sqrt(DotProduct(temp,temp));
			if (dist > 384)
				continue;
		}
		//	projected_distance = (384 - dist) / (384 / 4);
		
		for (o=0; o<3; o++)
			light[o] = -currententity->origin[o] + l_origin[o];	// lights origin in relation to the entity, was l->origin
		
		is = light[0], it = light[1];
		light[0] = (cost * (is - 0) + sint * (0 - it) + 0);
		light[1] = (cost * (it - 0) + sint * (is - 0) + 0);
		light[2] += 8;
		
		// increment stencil if backface is behind depthbuffer
		GL_CullFace(GL_BACK); // quake is backwards, this culls front faces
		qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
		CastMD2VolumeShadow(paliashdr, light, projected_distance);
		
		// decrement stencil if frontface is behind depthbuffer
		GL_CullFace(GL_FRONT); // quake is backwards, this culls back faces
		qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		CastMD2VolumeShadow(paliashdr, light, projected_distance);
		
		dl++; // increment dl 
	}*/

	if (!r_shadowvolumes->integer)
	{
		GL_CullFace(GL_FRONT);
		GL_Disable(GL_STENCIL_TEST);
		qglColorMask(1,1,1,1);
		
		GL_DepthMask(1);
		GL_DepthFunc(GL_LEQUAL);

		// draw shadows for this model now
		R_ShadowBlend (shadowalpha_md2 * currententity->alpha); // was r_shadowalpha->value
		qglPopAttrib(); // restore stencil buffer
	}
}


/*
=============
R_DrawAliasMD2PlanarShadow
=============
*/
void R_DrawAliasMD2PlanarShadow (dmdl_t *paliashdr, qboolean mirrored)
{
	int		*order;
	vec3_t	point, shadevector;
	float	height, lheight, thisAlpha;//, an;
	GLenum	mode;
	int		i, count, /*va, index,*/ vertcount, baseindex;

	//if (r_shadows->integer == 2) // dynamic lighted shadows - psychospaz
		R_ShadowLight (currententity->origin, shadevector);
	/*else {
		an = currententity->angles[1]/180*M_PI;
		shadevector[0] = cos(-an);
		shadevector[1] = sin(-an);
		shadevector[2] = 1;
		VectorNormalize (shadevector);
	}*/

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);
	lheight = currententity->origin[2] - lightspot[2];
	height = -lheight + 0.1f; // 12/24/2001- lowered shadows to ground
	if (currententity->flags & RF_TRANSLUCENT)
		thisAlpha = shadowalpha_md2 * currententity->alpha; // was r_shadowalpha->value
	else
		thisAlpha = shadowalpha_md2; // was r_shadowalpha->value

	// if above entity's origin, skip
	//if ((currententity->origin[2]+height) > currententity->origin[2])
	//	return;

	// don't draw shadows above view origin, thnx to MrG
	if (r_newrefdef.vieworg[2] < (currententity->origin[2] + height))
		return;

	GL_Stencil(true, false);
	GL_BlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);

	rb_vertex = rb_index = 0;
	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;

		if (!count)
			break;		// done
		if (count < 0) {
			count = -count;
			mode = GL_TRIANGLE_FAN;
		}
		else
			mode = GL_TRIANGLE_STRIP;

		vertcount = count;
		baseindex = rb_vertex;

		do {
			// normals and vertexes come from the frame list
			memcpy( point, s_lerped[order[2]], sizeof( point )  );

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
			VA_SetElem3(vertexArray[rb_vertex], point[0], point[1], point[2]);
			VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, thisAlpha);
			rb_vertex++;
			order += 3;
		} while (--count);

		// add triangle fan or strip indices to array
		if (mode == GL_TRIANGLE_FAN)
			for (i = 0; i < vertcount-2; i++) {
				indexArray[rb_index++] = baseindex;
				indexArray[rb_index++] = baseindex+i+1;
				indexArray[rb_index++] = baseindex+i+2;
			}
		else // GL_TRIANGLE_STRIP
			for (i = 0; i < vertcount-2; i++) {
				if (i%2 == 0) {
					indexArray[rb_index++] = baseindex+i;
					indexArray[rb_index++] = baseindex+i+1;
					indexArray[rb_index++] = baseindex+i+2;
				}
				else { // backwards order
					indexArray[rb_index++] = baseindex+i+2;
					indexArray[rb_index++] = baseindex+i+1;
					indexArray[rb_index++] = baseindex+i;
				}
			}
	}
	RB_DrawArrays ();
	rb_vertex = rb_index = 0;

	GL_Stencil(false, false);
}


/*
=================
R_CullAliasMD2Model
=================
*/
static qboolean R_CullAliasMD2Model (vec3_t bbox[8], entity_t *e)
{
	int			i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		tmp, thismins, oldmins, thismaxs, oldmaxs;//, angles;
	daliasframe_t *pframe, *poldframe;
	int			p, f, mask, aggregatemask = ~0;

	paliashdr = (dmdl_t *)currentmodel->extradata;

	if ( (e->frame >= paliashdr->num_frames) || (e->frame < 0) )
	{
		VID_Printf (PRINT_ALL, "R_CullAliasMD2Model %s: no such frame %d\n", 
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ( (e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0) )
	{
		VID_Printf (PRINT_ALL, "R_CullAliasMD2Model %s: no such oldframe %d\n", 
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = (daliasframe_t *) ( (byte *) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->frame * paliashdr->framesize);

	poldframe = (daliasframe_t *) ( (byte *) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->oldframe * paliashdr->framesize);

	// compute axially aligned mins and maxs
	if (pframe == poldframe)
	{
		for (i=0; i<3; i++)
		{
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i]*255;
		}
	}
	else
	{
		for (i=0; i<3; i++)
		{
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i]*255;

			oldmins[i]  = poldframe->translate[i];
			oldmaxs[i]  = oldmins[i] + poldframe->scale[i]*255;

			mins[i] = (thismins[i] < oldmins[i]) ? thismins[i] : oldmins[i];
			maxs[i] = (thismaxs[i] > oldmaxs[i]) ? thismaxs[i] : oldmaxs[i];
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
	for (p=0; p<8; p++)
	{
		mask = 0;

		for (f=0; f<4; f++)
		{
			float dp = DotProduct( frustum[f].normal, bbox[p] );

			if ( ( dp - frustum[f].dist ) < 0 )
				mask |= ( 1 << f );
		}
		aggregatemask &= mask;
	}

	if ( aggregatemask )
		return true;

	return false;
}


/*
=================
R_DrawAliasMD2Model
=================
*/
void R_DrawAliasMD2Model (entity_t *e)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
	image_t		*skin;
	qboolean	mirrormodel = false;
	qboolean	shadowonly = false;

	// also skip this for viewermodels and cameramodels
	if ( !(e->flags & RF_WEAPONMODEL || e->flags & RF_VIEWERMODEL || e->renderfx & RF2_CAMERAMODEL) )
	{
		if (R_CullAliasMD2Model(bbox, e))
			/*if (r_shadows->integer == 3)
				shadowonly = true;
			else*/
				return;
	}

	if (e->flags & RF_WEAPONMODEL)
	{
		if (r_lefthand->integer == 2)
			return;
		else if (r_lefthand->integer == 1)
			mirrormodel = true;
	}
	else if (e->renderfx & RF2_CAMERAMODEL)
	{
		if (r_lefthand->integer==1)
			mirrormodel = true;
	}
	else if (e->flags & RF_MIRRORMODEL)
		mirrormodel = true;

	paliashdr = (dmdl_t *)currentmodel->extradata;

	//
	// get lighting information
	//
	// PMM - rewrote, reordered to handle new shells & mixing
	// PMM - 3.20 code .. replaced with original way of doing it to keep mod authors happy
	//

	R_SetShadeLight();

	c_alias_polys += paliashdr->num_tris;

	//
	// draw all the triangles
	//
	if (e->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
	{
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			GL_DepthRange (gldepthmin, gldepthmin + 0.01*(gldepthmax-gldepthmin));
		else
			GL_DepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
	}

	if (mirrormodel)
		R_FlipModel (true, false);

    qglPushMatrix ();
	e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards
	R_RotateForEntity (e, true);
	e->angles[ROLL] = e->angles[ROLL] * R_RollMult();	// roll is backwards

	// select skin
	if (e->skin)
		skin = e->skin;	// custom player skin
	else
	{
		if (currententity->skinnum >= MAX_MD2SKINS)
			skin = currentmodel->skins[0][0];
		else
		{
			skin = currentmodel->skins[0][currententity->skinnum];
			if (!skin)
				skin = currentmodel->skins[0][0];
		}
	}
	if (!skin)
		skin = glMedia.notexture;	// fallback...

	if ( (e->frame >= paliashdr->num_frames) 
		|| (e->frame < 0) )
	{
		VID_Printf (PRINT_ALL, "R_DrawAliasMD2Model %s: no such frame %d\n",
			currentmodel->name, e->frame);
		e->frame = 0;
		e->oldframe = 0;
	}

	if ( (e->oldframe >= paliashdr->num_frames)
		|| (e->oldframe < 0))
	{
		VID_Printf (PRINT_ALL, "R_DrawAliasMD2Model %s: no such oldframe %d\n",
			currentmodel->name, e->oldframe);
		e->frame = 0;
		e->oldframe = 0;
	}

	if ( !r_lerpmodels->integer )
		e->backlerp = 0;

	R_SetBlendModeOn (skin); // Q2max add

	R_DrawAliasMD2FrameLerp (paliashdr, e->backlerp);

	GL_TexEnv (GL_REPLACE);
	GL_ShadeModel (GL_FLAT);

	qglPopMatrix ();

	if (mirrormodel)
		R_FlipModel (false, false);

	// show model bounding box
	R_DrawAliasModelBBox (bbox, e, 1.0f, 1.0f, 1.0f, 1.0f);

	R_SetBlendModeOff (); // Q2max add

	if (e->flags & RF_DEPTHHACK)
		GL_DepthRange (gldepthmin, gldepthmax);

	shadowalpha_md2 = R_CalcShadowAlpha(e);

	// added noshadow flag
	if ( !(e->flags & (RF_WEAPONMODEL | RF_NOSHADOW))
		// no shadows from shells
		&& !( (e->flags & RF_MASK_SHELL) && (e->flags & RF_TRANSLUCENT) ) 
		&& r_shadows->integer >= 1 && shadowalpha_md2 >= DIV255)
	{
 		qglPushMatrix ();
		R_RotateForEntity (e, false);
		GL_DisableTexture(0);
		GL_Enable (GL_BLEND);

		if ((r_shadows->integer == 3) && currentmodel->edge_tri)
			R_DrawAliasMD2VolumeShadow (paliashdr, bbox);
		else
			R_DrawAliasMD2PlanarShadow (paliashdr, mirrormodel);

		GL_Disable (GL_BLEND);
		GL_EnableTexture(0);
		qglPopMatrix ();
	}
}

#if 0
/*
=================
R_DrawAliasMD2ModelShadow
Just draws the shadow for a model
=================
*/
void R_DrawAliasMD2ModelShadow (entity_t *e)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
	qboolean	mirrormodel = false;
	//float		an;

	if (!r_shadows->integer)
		return;
	if (e->flags & (RF_WEAPONMODEL | RF_NOSHADOW))
		return;
	// no shadows from shells
	if ( (e->flags & RF_MASK_SHELL) && (e->flags & RF_TRANSLUCENT) )
		return;

	// also skip this for viewermodels and cameramodels
	if ( !(e->flags & RF_WEAPONMODEL || e->flags & RF_VIEWERMODEL || e->renderfx & RF2_CAMERAMODEL) )
	{
		if (R_CullAliasMD2Model(bbox, e))
			return;
	}

	shadowalpha_md2 = R_CalcShadowAlpha(e);
	if (shadowalpha_md2 < DIV255) // out of range
		return;

	if (e->renderfx & RF2_CAMERAMODEL)
	{
		if (r_lefthand->integer==1)
			mirrormodel = true;
	}
	else if (e->flags & RF_MIRRORMODEL)
		mirrormodel = true;

	paliashdr = (dmdl_t *)currentmodel->extradata;

//	if (mirrormodel)
//		R_FlipModel (true, false);

	if ( (e->frame >= paliashdr->num_frames) 
		|| (e->frame < 0) )
	{
		e->frame = 0;
		e->oldframe = 0;
	}

	if ( (e->oldframe >= paliashdr->num_frames)
		|| (e->oldframe < 0))
	{
		e->frame = 0;
		e->oldframe = 0;
	}

//	if ( !r_lerpmodels->integer )
//		e->backlerp = 0;

	/*an = e->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);
	switch ((r_shadows->integer))
	{
	case 0:
		break;
	case 2:
		//dynamic lighted shadows - psychospaz
		R_ShadowLight (e->origin, shadevector);
	default:
		{*/
			qglPushMatrix ();
			R_RotateForEntity (e, false);
			GL_DisableTexture(0);
			GL_Enable (GL_BLEND);
						
			if ((r_shadows->integer == 3) && currentmodel->edge_tri)
				R_DrawAliasMD2VolumeShadow (paliashdr, bbox);
			else
				R_DrawAliasMD2PlanarShadow (paliashdr, mirrormodel);
			
			GL_Disable (GL_BLEND);
			GL_EnableTexture(0);
			qglPopMatrix ();
	/*	}
		break;
	}*/

//	if (mirrormodel)
//		R_FlipModel (false, false);
}
#endif

#endif // MD2_AS_MD3
