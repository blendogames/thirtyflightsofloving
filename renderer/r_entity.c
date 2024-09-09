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

// r_entity.c -- entity handling and null model rendering
// moved from r_main.c

#include "r_local.h"

unsigned int	r_occlusionQueries[MAX_EDICTS];
unsigned int	r_occlusionQuerySamples[MAX_EDICTS];
byte			r_occlusionQueryPending[MAX_EDICTS];


/*
=================
R_ClearOcclusionQuerySampleList
=================
*/
void R_ClearOcclusionQuerySampleList (void)
{
	if ( !glConfig.occlusionQuery )
		return;

	qglDeleteQueries (MAX_EDICTS, r_occlusionQueries);
	qglGenQueries (MAX_EDICTS, r_occlusionQueries);

	memset (&r_occlusionQuerySamples, 0, sizeof(r_occlusionQuerySamples));
	memset (&r_occlusionQueryPending, 0, sizeof(r_occlusionQueryPending));
}


/*
=================
R_ShutdownOcclusionQueries
=================
*/
void R_ShutdownOcclusionQueries (void)
{
	if ( !glConfig.occlusionQuery )
		return;

	qglDeleteQueries (MAX_EDICTS, r_occlusionQueries);
}


/*
=================
R_RotateForEntity
=================
*/
void R_RotateForEntity (entity_t *e, qboolean full)
{
    qglTranslatef (e->origin[0],  e->origin[1],  e->origin[2]);

    qglRotatef (e->angles[1],  0, 0, 1);
	if (full==true)
	{
		qglRotatef (e->angles[0],  0, 1, 0);
		qglRotatef (e->angles[2],  1, 0, 0);
	}
}


/*
=================
R_RollMult
=================
*/
int R_RollMult (void)
{
	if (r_entity_fliproll->integer)
		return -1;
	else
		return 1;
}


/*
=================
R_DrawNullModel
=================
*/
void R_DrawNullModel (void)
{
	vec3_t	shadelight;
	vec_t	modelview_lightscale;

	qglPushMatrix ();
	R_RotateForEntity (currententity, true);
	GL_DisableTexture (0);

	if (r_old_nullmodel->integer)
	{
		if (currententity->flags & RF_FULLBRIGHT)
			VectorSet (shadelight, 1.0f, 1.0f, 1.0f);
		else if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {
			// shading for model views based on inverse intensity
			VectorSet (shadelight,  glState.inverse_intensity,  glState.inverse_intensity,  glState.inverse_intensity);
			modelview_lightscale = min(max(r_modelview_lightscale->value, 0.0f), 1.0f);
			VectorScale (shadelight, modelview_lightscale, shadelight);
		}
		else
			R_LightPoint (currententity->origin, shadelight, false);
		qglColor3fv (shadelight);
		qglCallList (glMedia.displayLists[DL_NULLMODEL1]);
	}
	else
		qglCallList (glMedia.displayLists[DL_NULLMODEL2]);

	GL_EnableTexture (0);
	qglPopMatrix ();
	qglColor4f (1,1,1,1);
}

/*
==================================================================================

	TREE BUILDING AND USAGE

==================================================================================
*/

int entstosort;
sortedelement_t theents[MAX_ENTITIES];
// Is this really used at all?
//sortedelement_t *ents_prerender;
sortedelement_t *ents_trans;
sortedelement_t *ents_flares;
sortedelement_t *ents_viewweaps;
sortedelement_t *ents_viewweaps_trans;

void resetEntSortList (void)
{
	entstosort = 0;
//	ents_prerender = NULL;
	ents_trans = NULL;
	ents_flares = NULL;
	ents_viewweaps = NULL;
	ents_viewweaps_trans = NULL;
}


sortedelement_t *NewSortEnt (entity_t *ent)
{
//	qboolean is_weapmodel = false;
//	qboolean entinwater;
	vec3_t distance;
	sortedelement_t *element;
//	mleaf_t *point_in;

	element = &theents[entstosort];

	VectorSubtract(ent->origin, r_origin, distance);
	VectorCopy(ent->origin, element->org);

	element->data = (entity_t *)ent;
	element->len = (vec_t)VectorLength(distance);
	element->left = NULL;
	element->right = NULL;

	return element;
}


/*
=================
ElementAddNode
=================
*/
// This is dirty, but it's gotta be fast...
void ElementAddNode (sortedelement_t *base, sortedelement_t *thisElement)
{
again:
	if (thisElement->len > base->len)
	{
		if (base->left) {
			base = base->left;
			goto again;
		}
		else
			base->left = thisElement;
	}
	else
	{
		if (base->right) {
			base = base->right;
			goto again;
		}
		else
			base->right = thisElement;
	}
}
/*void ElementAddNode (sortedelement_t *base, sortedelement_t *thisElement)
{
	if (thisElement->len > base->len)
	{
		if (base->left)
			ElementAddNode(base->left, thisElement);
		else
			base->left = thisElement;
	}
	else
	{
		if (base->right)
			ElementAddNode(base->right, thisElement);
		else
			base->right = thisElement;
	}
}*/


/*
=================
AddEntViewWeapTree
=================
*/
void AddEntViewWeapTree (entity_t *ent, qboolean trans)
{
	int closer = 0;
	sortedelement_t *thisEnt;

	thisEnt = NewSortEnt(ent);

	if (!thisEnt)
		return;

	if (!trans)
	{
		if (ents_viewweaps)
			ElementAddNode(ents_viewweaps, thisEnt);
		else
			ents_viewweaps = thisEnt;
	}
	else
	{		
		if (ents_viewweaps_trans)
			ElementAddNode(ents_viewweaps_trans, thisEnt);
		else
			ents_viewweaps_trans = thisEnt;	
	}
	entstosort++;
}


/*
=================
AddEntTransTree
=================
*/
void AddEntTransTree (entity_t *ent)
{
	int closer = 0;
	sortedelement_t *thisEnt;

	thisEnt = NewSortEnt(ent);

	if (!thisEnt)
		return;

	if (ents_trans)
		ElementAddNode(ents_trans, thisEnt);
	else
		ents_trans = thisEnt;

	entstosort++;
}


/*
=================
AddEntFlareTree
=================
*/
void AddEntFlareTree (entity_t *ent)
{
	sortedelement_t *thisEnt;

	thisEnt = NewSortEnt(ent);

	if (!thisEnt)
		return;

	if (ents_flares)
		ElementAddNode (ents_flares, thisEnt);
	else
		ents_flares = thisEnt;

	entstosort++;
}

//==================================================================================

/*
=================
ParseOccludeTestForEntity
=================
*/
void ParseOccludeTestForEntity (entity_t *ent)
{
	currententity = ent;

	if (currententity->flags & RF_BEAM) {
		// Do nothing for now
	}
	else if (currententity->flags & RF_FLARE) {
		R_OccludeTestFlare (currententity);
	}
	else
	{
		// Do nothing for now
	/*	currentmodel = currententity->model;
		if (!currentmodel) {
			return;
		}
		switch (currentmodel->type)
		{
		case mod_alias:
			break;
		case mod_brush:
			break;
		case mod_sprite:
			break;
		default:
			break;
		} */
	}
}


/*
=================
ParseRenderEntity
=================
*/
void ParseRenderEntity (entity_t *ent)
{
	currententity = ent;

	if (currententity->flags & RF_BEAM) {
		R_DrawBeam (currententity);
	}
	else if (currententity->flags & RF_FLARE) {
		R_DrawFlare (currententity);
	}
	else
	{
		currentmodel = currententity->model;
		if (!currentmodel) {
			R_DrawNullModel ();
			return;
		}
		switch (currentmodel->type)
		{
#ifndef MD2_AS_MD3
		case mod_md2:
			R_DrawAliasMD2Model (currententity);
			break;
#endif // MD2_AS_MD3
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;
		case mod_brush:
			R_DrawBrushModel (currententity);
			break;
		case mod_sprite:
			R_DrawSpriteModel (currententity);
			break;
		default:
			VID_Printf (PRINT_ALL, S_COLOR_YELLOW"Warning: ParseRenderEntity: %s: Bad modeltype (%i)\n", currentmodel->name, currentmodel->type);
		//	VID_Error (ERR_DROP, "Bad modeltype");
			break;
		}

		// draw entity bbox
		if (currentmodel->type != mod_brush)
			R_DrawEntityBBox (currententity, 0.0f, 1.0f, 0.0f, 1.0f); 
	}
}


/*
=================
OccludeTestEntTree
=================
*/
void OccludeTestEntTree (sortedelement_t *element)
{
	if (!element)
		return;

	OccludeTestEntTree (element->left);

	if (element->data)
		ParseOccludeTestForEntity (element->data);

	OccludeTestEntTree (element->right);
}


/*
=================
RenderEntTree
=================
*/
void RenderEntTree (sortedelement_t *element)
{
	if (!element)
		return;

	RenderEntTree (element->left);

	if (element->data)
		ParseRenderEntity (element->data);

	RenderEntTree (element->right);
}


/*
=================
R_DrawAllEntities
=================
*/
void R_DrawAllEntities (qboolean addViewWeaps)
{
	int			i;
	qboolean	flare, alpha;
	
	if ( !r_drawentities->integer )
		return;

	resetEntSortList ();

	for (i=0; i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		flare = false;
		alpha = false;

		if ( (currententity->flags & RF_FLARE) && !(currententity->flags & RF_BEAM) )
			flare = true;
		else if (currententity->flags & RF_TRANSLUCENT)
			alpha = true;

		// check for md3 mesh transparency
		if (!(currententity->flags & RF_BEAM) && currententity->model) {
			if ( (currententity->model->type == mod_alias) && currententity->model->hasAlpha)
				alpha = true;
		}

		if (currententity->flags & RF_WEAPONMODEL)
		{
			if (addViewWeaps)
				AddEntViewWeapTree(currententity, alpha);
			continue;
		}

		if (alpha || flare)
			continue;

		ParseRenderEntity (currententity);
	}

	// occlusion test flares
	if ( r_drawflares->integer && glConfig.occlusionQuery && r_occlusion_test->integer )
	{
		for (i=0; i<r_newrefdef.num_entities; i++)
		{
			currententity = &r_newrefdef.entities[i];
			if ( !(currententity->flags & RF_FLARE) )
				continue;
			if (currententity->flags & RF_BEAM)
				continue;
			if (currententity->flags & RF_WEAPONMODEL)
				continue;

			ParseOccludeTestForEntity (currententity);
		}
		qglFlush ();
	}

	GL_DepthMask (0);
	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		alpha = false;

		if (currententity->flags & RF_FLARE)	// flares get their own pass
			continue;
		if (currententity->flags & RF_TRANSLUCENT)
			alpha = true;

		// check for md3 mesh transparency
		if ( !(currententity->flags & RF_BEAM) && currententity->model ) {
			if ( (currententity->model->type == mod_alias) && currententity->model->hasAlpha )
				alpha = true;
		}

		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (!alpha)
			continue;

		ParseRenderEntity (currententity);
	}
	// draw flares
	if (r_drawflares->integer)
	{
		for (i=0; i<r_newrefdef.num_entities; i++)
		{
			currententity = &r_newrefdef.entities[i];
			if ( !(currententity->flags & RF_FLARE) )
				continue;
			if (currententity->flags & RF_BEAM)
				continue;
			if (currententity->flags & RF_WEAPONMODEL)
				continue;

			ParseRenderEntity (currententity);
		}
	}
	GL_DepthMask (1);
}


/*
=================
R_DrawSolidEntities
=================
*/
void R_DrawSolidEntities (void)
{
	int			i;
	qboolean	flare, alpha;

	if ( !r_drawentities->integer )
		return;

	resetEntSortList();

	for (i=0; i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		flare = false;
		alpha = false;

		if ( (currententity->flags & RF_FLARE) && !(currententity->flags & RF_BEAM) )
			flare = true;
		else if (currententity->flags & RF_TRANSLUCENT)
			alpha = true;

		// check for md3 mesh transparency
		if ( !(currententity->flags & RF_BEAM) && currententity->model ) {
			if ( (currententity->model->type == mod_alias) && currententity->model->hasAlpha)
				alpha = true;
		}

		if (currententity->flags & RF_WEAPONMODEL) {
			AddEntViewWeapTree (currententity, alpha);
			continue;
		}

		if (flare)
		{
			if (r_drawflares->integer) {
				AddEntFlareTree (currententity);
			}
			continue;
		}
		else if (alpha) {
			AddEntTransTree (currententity);
			continue;
		}

		ParseRenderEntity (currententity);
	}

	if ( r_drawflares->integer ) {
		R_OccludeTestEntitiesOnList (ents_flares);
	}
}


/*
=================
R_OccludeTestEntitiesOnList
=================
*/
void R_OccludeTestEntitiesOnList (sortedelement_t *list)
{
	if ( !r_drawentities->integer )
		return;
	if ( !glConfig.occlusionQuery )
		return;
	if ( !r_occlusion_test->integer )
		return;

	OccludeTestEntTree (list);

	qglFlush ();
}


/*
=================
R_DrawEntitiesOnList
=================
*/
void R_DrawEntitiesOnList (sortedelement_t *list)
{
	if ( !r_drawentities->integer )
		return;

	RenderEntTree (list);
}

/*
==================================================
	SHADOW-ONLY RENDERING (UNUSED)
==================================================
*/
#if 0
void ParseRenderEntityShadow (entity_t *ent)
{
	currententity = ent;
	if ( currententity->flags & RF_BEAM )
		return;
	currentmodel = currententity->model;
	if (!currentmodel)
		return;
	switch (currentmodel->type)
	{
#ifndef MD2_AS_MD3
	case mod_md2:
		R_DrawAliasMD2ModelShadow (currententity);
		break;
#endif // MD2_AS_MD3
	case mod_alias:
		R_DrawAliasModelShadow (currententity);
		break;
	default:
		break;
	}
}

void R_DrawAllEntityShadows (void)
{
	int i;
	
	if (!r_drawentities->integer)
		return;
//	if (!r_shadows->integer)
	if (r_shadows->integer != 3)
		return;

	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		ParseRenderEntityShadow(currententity);
	}
}
#endif
