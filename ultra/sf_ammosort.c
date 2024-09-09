#include "g_local.h"

void sortammo (edict_t *ent, int table2 [6][2])
{

	int ammotable1 [12][2];
	int tempindex, tempcount;
	int i, j;

	for (i = 0; i < 12; i++)  //put ammo indices in table
		ammotable1 [i][0] = i;

	//assign relative ammo values to table
	ammotable1 [0][1] = ent->client->pers.inventory[ITEM_INDEX(item_shells)] /2;
	ammotable1 [1][1] = ent->client->pers.inventory[ITEM_INDEX(item_bullets)] /4;
	ammotable1 [2][1] = ent->client->pers.inventory[ITEM_INDEX(item_grenades)];
	ammotable1 [3][1] = ent->client->pers.inventory[ITEM_INDEX(item_rockets)];
	ammotable1 [4][1] = ent->client->pers.inventory[ITEM_INDEX(item_cells)] /4;
	ammotable1 [5][1] = ent->client->pers.inventory[ITEM_INDEX(item_slugs)];
	ammotable1 [6][1] = ent->client->pers.inventory[ITEM_INDEX(item_magslugs)];
	ammotable1 [7][1] = ent->client->pers.inventory[ITEM_INDEX(item_flechettes)] /4;
	ammotable1 [8][1] = ent->client->pers.inventory[ITEM_INDEX(item_rounds)];
	ammotable1 [9][1] = ent->client->pers.inventory[ITEM_INDEX(item_prox)];
	ammotable1 [10][1] = ent->client->pers.inventory[ITEM_INDEX(item_tesla)];
	ammotable1 [11][1] = ent->client->pers.inventory[ITEM_INDEX(item_trap)];

	j = 11;
	while (j > 0) //sort descending by relative ammo count
	{
		qboolean swapped = false;
		for (i = 0; i < j; i++)
			if (ammotable1[i][1] < ammotable1[i + 1][1])
			{
				tempindex = ammotable1[i][0];
				tempcount = ammotable1[i][1];
				ammotable1[i][0] = ammotable1[i + 1][0];
				ammotable1[i][1] = ammotable1[i + 1][1];
				ammotable1[i + 1][0] = tempindex;
				ammotable1[i + 1][1] = tempcount;
				swapped = true;
			}
		if (!swapped) //early termination
			break;
		j--;
	}
	
	for (i = 0; i < 6; i++) //copy top 6 rows
	{
		table2[i][0] = ammotable1[i][0];
		table2[i][1] = ammotable1[i][1];
	}

	j = 5;
	while (j > 0) //sort ascending by ammo index
	{
		qboolean swapped = false;
		for (i = 0; i < j; i++)
			if (table2[i][0] > table2[i + 1][0])
			{
				tempindex = table2[i][0];
				tempcount = table2[i][1];
				table2[i][0] = table2[i + 1][0];
				table2[i][1] = table2[i + 1][1];
				table2[i + 1][0] = tempindex;
				table2[i + 1][1] = tempcount;
				swapped = true;
			}	
		if (!swapped) //early termination
			break;
		j--;
	}


}

int getammocount (edict_t *ent, int index)
{
	switch (index)
	{
		case 0: return ent->client->pers.inventory[ITEM_INDEX(item_shells)];
		case 1: return ent->client->pers.inventory[ITEM_INDEX(item_bullets)];
		case 2: return ent->client->pers.inventory[ITEM_INDEX(item_grenades)];
		case 3: return ent->client->pers.inventory[ITEM_INDEX(item_rockets)];
		case 4: return ent->client->pers.inventory[ITEM_INDEX(item_cells)];
		case 5: return ent->client->pers.inventory[ITEM_INDEX(item_slugs)];
		case 6: return ent->client->pers.inventory[ITEM_INDEX(item_magslugs)];
		case 7: return ent->client->pers.inventory[ITEM_INDEX(item_flechettes)];
		case 8: return ent->client->pers.inventory[ITEM_INDEX(item_rounds)];
		case 9: return ent->client->pers.inventory[ITEM_INDEX(item_prox)];
		case 10: return ent->client->pers.inventory[ITEM_INDEX(item_tesla)];
		case 11: return ent->client->pers.inventory[ITEM_INDEX(item_trap)];
	}
	return 0;
}

int getammoicon (int index)
{
	switch (index)
	{
		case 0: return gi.imageindex ("a_shells");
		case 1: return gi.imageindex ("a_bullets");
		case 2: return gi.imageindex ("a_grenades");
		case 3: return gi.imageindex ("a_rockets");
		case 4: return gi.imageindex ("a_cells");
		case 5: return gi.imageindex ("a_slugs");
		case 6: return gi.imageindex ("a_mslugs");
		case 7: return gi.imageindex ("a_flechettes");
		case 8: return gi.imageindex ("a_disruptor");
		case 9: return gi.imageindex ("a_prox");
		case 10: return gi.imageindex ("a_tesla");
		case 11: return gi.imageindex ("a_trap");
	}
	return 0;
}

