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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// r_entity.c -- entity handling and null model rendering
// moved from r_main.c

#include "r_local.h"


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
R_DrawNullModel
=================
*/
void R_DrawNullModel (void)
{	/*
	vec3_t	shadelight;
	int		i;

	if (currententity->flags & RF_FULLBRIGHT)
		shadelight[0] = shadelight[1] = shadelight[2] = 1.0f;
	else
		R_LightPoint (currententity->origin, shadelight, false);
	*/
    qglPushMatrix ();
	R_RotateForEntity (currententity, true);
	qglDisable (GL_TEXTURE_2D);

	qglBegin(GL_LINES);

	qglColor4ub(255, 0, 0, 255);
	qglVertex3f(0, 0, 0);
	qglVertex3f(16, 0, 0);

	qglColor4ub(0, 255, 0, 255);
	qglVertex3f(0, 0, 0);
	qglVertex3f(0, 16, 0);

	qglColor4ub(0, 0, 255, 255);
	qglVertex3f(0, 0, 0);
	qglVertex3f(0, 0, 16);

	qglEnd();
	/*
	qglColor3fv (shadelight);

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, -16);
	for (i=0 ; i<=4 ; i++)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, 16);
	for (i=4 ; i>=0 ; i--)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();
	*/
	qglEnable (GL_TEXTURE_2D);
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
sortedelement_t *ents_viewweaps;
sortedelement_t *ents_viewweaps_trans;

void resetEntSortList (void)
{
	entstosort = 0;
	//ents_prerender = NULL;
	ents_trans = NULL;
	ents_viewweaps = NULL;
	ents_viewweaps_trans = NULL;
}


sortedelement_t *NewSortEnt (entity_t *ent)
{
	qboolean is_weapmodel = false;
	//qboolean entinwater;
	vec3_t distance;
	sortedelement_t *element;
	//mleaf_t *point_in;

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

//==================================================================================

/*
=================
ParseRenderEntity
=================
*/
void ParseRenderEntity (entity_t *ent)
{
	currententity = ent;

	if (currententity->flags & RF_BEAM)
	{
		R_DrawBeam (currententity);
	}
	else
	{
		currentmodel = currententity->model;
		if (!currentmodel)
		{
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
		//Harven MD3 ++
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;
		//Harven MD3 --
		case mod_brush:
			R_DrawBrushModel (currententity);
			break;
		case mod_sprite:
			R_DrawSpriteModel (currententity);
			break;
		default:
			VID_Printf(PRINT_ALL, S_COLOR_YELLOW"Warning: ParseRenderEntity: %s: Bad modeltype (%i)\n", currentmodel->name, currentmodel->type);
			//VID_Error (ERR_DROP, "Bad modeltype");
			break;
		}
	}
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

	RenderEntTree(element->left);

	if (element->data)
		ParseRenderEntity(element->data);

	RenderEntTree(element->right);
}


/*
=================
R_DrawAllEntities
=================
*/
void R_DrawAllEntities (qboolean addViewWeaps)
{
	qboolean alpha;
	int i;
	
	if (!r_drawentities->value)
		return;

	resetEntSortList();

	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];

		alpha = false;
		if (currententity->flags & RF_TRANSLUCENT)
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

		if (alpha)
			continue;

		ParseRenderEntity(currententity);
	}

	GL_DepthMask (0);
	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];

		alpha = false;
		if (currententity->flags & RF_TRANSLUCENT)
			alpha = true;

		// check for md3 mesh transparency
		if (!(currententity->flags & RF_BEAM) && currententity->model) {
			if ( (currententity->model->type == mod_alias) && currententity->model->hasAlpha)
				alpha = true;
		}

		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (!alpha)
			continue;

		ParseRenderEntity(currententity);
	}
	GL_DepthMask (1);
	
}


/*
=================
R_DrawSolidEntities
=================
*/
void R_DrawSolidEntities ()
{
	qboolean alpha;
	int		i;

	if (!r_drawentities->value)
		return;

	resetEntSortList();

	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		alpha = false;

		if (currententity->flags & RF_TRANSLUCENT)
			alpha = true;

		// check for md3 mesh transparency
		if (!(currententity->flags & RF_BEAM) && currententity->model) {
			if ( (currententity->model->type == mod_alias) && currententity->model->hasAlpha)
				alpha = true;
		}

		if (currententity->flags & RF_WEAPONMODEL) {
			AddEntViewWeapTree(currententity, alpha);
			continue;
		}

		if (alpha) {
			AddEntTransTree(currententity);
			continue;
		}

		ParseRenderEntity(currententity);
	}
}


/*
=================
R_DrawEntitiesOnList
=================
*/
void R_DrawEntitiesOnList (sortedelement_t *list)
{
	if (!r_drawentities->value)
		return;

	RenderEntTree(list);
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
	
	if (!r_drawentities->value)
		return;
	//if (!r_shadows->value)
	if (r_shadows->value != 3)
		return;

	for (i=0;i<r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		ParseRenderEntityShadow(currententity);
	}
}
#endif
