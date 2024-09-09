/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_weapon.c,v $
 *   $Revision: 1.13 $
 *   $Date: 2002/07/23 07:26:15 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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

#include "jpn_main.h"

// p_weapon.c
// D-Day: Normandy Player Weapon Code


/*
void Weapon_Generic (edict_t *ent, 
 int FRAME_ACTIVATE_LAST,	int FRAME_LFIRE_LAST,	int FRAME_LIDLE_LAST, 
 int FRAME_RELOAD_LAST,		int FRAME_LASTRD_LAST,	int FRAME_DEACTIVATE_LAST,
 int FRAME_RAISE_LAST,		int FRAME_AFIRE_LAST,	int FRAME_AIDLE_LAST,
 int *pause_frames,			int *fire_frames,		void (*fire)(edict_t *ent))
*/


/////////////////////////////////////////////////
// Nambu Pistol
/////////////////////////////////////////////////

void Weapon_Nambu (edict_t *ent)
{
	static int	pause_frames[]	= {54};//{13, 32,42};
	static int	fire_frames[1];	//= {4,59,0};

	fire_frames[0]=(ent->client->aim)?81:4;

	ent->client->p_fract= &ent->client->mags[jpn_index].pistol_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].pistol_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:6)
			//gotta do it this way for both firing modes
)
		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		3,  6,  47, 
		70, 73, 76, 
		80, 83, 92, 
		
		pause_frames, fire_frames, Weapon_Pistol_Fire);
}


/////////////////////////////////////////////////
// arisaka rifle
/////////////////////////////////////////////////

void Weapon_Arisaka (edict_t *ent)
{
	static int	pause_frames[]	= {39};//{4,25, 50, 0};
	static int	fire_frames[1];// = {4};

	fire_frames[0]=(ent->client->aim)?61:4;
	
	ent->client->p_fract = &ent->client->mags[jpn_index].rifle_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].rifle_rnd;

	ent->client->crosshair = false;
	if (ent->client->ps.gunframe == 69)
	{
		ent->client->crosshair_offset_x = random() * 4 - 2;
		ent->client->crosshair_offset_y = random() * 2;
	}

	//faf
	if (ent->client->ps.gunframe == 9 ||
		ent->client->ps.gunframe == 63)
		gi.sound(ent, CHAN_VOICE, gi.soundindex("jpn/arisaka/bolt.wav"), 1, ATTN_NORM, 0);





//faf: bayonet code
	if (ent->client->ps.gunframe == 104)
	{
		ent->client->ps.gunframe = 19;
		return;
	}

	if (ent->client->ps.gunframe > 99)
	{
		ent->client->ps.gunframe++;
		return;
	}
//end bayonet




	if (ent->client->weaponstate != WEAPON_RELOADING &&
		ent->client->ps.gunframe > 70 &&
		!ent->client->aim)
	{
		//gi.dprintf ("%i\n",ent->client->weaponstate);
		ent->client->ps.gunframe = 58;
		ent->client->weaponstate = WEAPON_LOWER;
		return;
	}

	if ((ent->client->weaponstate == WEAPON_FIRING || 
	 ent->client->weaponstate == WEAPON_READY)
		&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?60:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:12)			
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:15)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?73:16)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?0:17)
			
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
		ent->client->ps.gunframe = (ent->client->aim)?72:17;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work




	Weapon_Generic (ent, 
		3,  17, 29,//56, 
		53, 53, 56,
		60, 72, 78, 
		
		pause_frames, fire_frames, Weapon_Rifle_Fire);
}


/////////////////////////////////////////////////
// Type 100 SMG
/////////////////////////////////////////////////

void Weapon_Type_100 (edict_t *ent)
{

	static int	pause_frames[]	= {50};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;


	ent->client->p_fract= &ent->client->mags[jpn_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].submg_rnd;

	ent->client->crosshair = false;
 
	Weapon_Generic (ent, 
		 3,  5, 38, 
		70, 70, 74, 
		78, 80, 88, 
		
		pause_frames, fire_frames, Weapon_Submachinegun_Fire);

}

/////////////////////////////////////////////////
// Type 99 LMG
/////////////////////////////////////////////////

void Weapon_Type_99 (edict_t *ent)
{
	static int	pause_frames[]	= {54};//{38};//{23, 45, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?63:4;
	fire_frames[1]=(ent->client->aim)?64:5;

	ent->client->p_fract= &ent->client->mags[jpn_index].lmg_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].lmg_rnd;

	ent->client->crosshair = false;


	if (ent->client->ps.gunframe == 63)
		ent->client->ps.gunframe= 64;

	if (ent->client->next_fire_frame == 63)
		ent->client->ps.gunframe= 64;
	Weapon_Generic (ent, 
		 3,  5, 25, 
		53, 53, 57, 
		62, 64, 70, 
		
		pause_frames, fire_frames, Weapon_Bren_Fire);
	//realized this should be changed in guninfo.c but screw it, leaving this hack in
	if (ent->client->ps.gunframe == 80){
		ent->client->ps.gunframe = 63 + ent->client->next_fire_frame%2;
		ent->client->next_fire_frame++;
	}
	else
		ent->client->next_fire_frame =0;


}




/////////////////////////////////////////////////
// arisaka Sniper Rifle
/////////////////////////////////////////////////

void Weapon_Arisakas (edict_t *ent)
{
	static int	pause_frames[]	= {26};
	static int	fire_frames[4];

	fire_frames[0]=(ent->client->aim)?52:4;//fire here
	fire_frames[1]=(ent->client->aim)?59:0;//faf 60:0;//sniper bolt
	fire_frames[2]=(ent->client->aim)?75:0;//sniper start zoom
	fire_frames[3]=(ent->client->aim)?80:0;//sniper end zoom

	ent->client->p_fract= &ent->client->mags[jpn_index].sniper_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].sniper_rnd;

	if (ent->client->ps.gunframe == 61)
		gi.sound(ent, CHAN_AUTO, gi.soundindex(ent->client->pers.weapon->guninfo->sniper_bolt_wav), 1, ATTN_NORM, 0);

	if (ent->client->aim) 
	{
		if ( (ent->client->ps.gunframe >= fire_frames[0] && ent->client->ps.gunframe <= fire_frames[1]) ||
			  ent->client->ps.gunframe >= fire_frames[3])
			ent->client->crosshair = true;
		else
			ent->client->crosshair = false;
	} 
	else
		ent->client->crosshair = false;

	if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?51:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?52:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?53:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?54:6)

)
		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		 3, 8, 18,//26,//19,//25, 27,//faf 26, 26, 
		43, 43, 48, 
		51, 80, 80, 
		
		pause_frames, fire_frames, Weapon_Sniper_Fire);
}








void Weapon_Katana (edict_t *ent)
{
	static int      pause_frames[]  = {0};//{19, 32, 0};
    int				fire_frames[] = {9,10};

	if (ent->client->aim)
		ent->client->aim = false;

	ent->client->crosshair = false;

	//ent->client->aim=false;
	//fire_frames[0]=(ent->client->aim)?54:4;
	ent->client->p_rnd=NULL;
	
	//faf
		fire_frames[0]=9;

		fire_frames[1] = 0;

	if(ent->client->ps.gunframe == 1)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("jpn/katana/draw.wav"), 1, ATTN_NORM, 0);//faf




	if (ent->client->ps.gunframe == 9)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		if (ent->stanceflags == STANCE_STAND)
			{
				ent->s.frame = 66;//FRAME_pain304+1;
				ent->client->anim_end = 62;//FRAME_pain301;            
			}
			else if (ent->stanceflags == STANCE_DUCK)
			{
				ent->s.frame = 173;// FRAME_crpain4+1;
				ent->client->anim_end = 169;//FRAME_crpain1;
			}
			else if (ent->stanceflags == STANCE_CRAWL)
			{
				ent->s.frame = 234;// FRAME_crawlpain04+1;
				ent->client->anim_end = 230;//FRAME_crawlpain01;
			}
	}



	if(ent->client->ps.gunframe == 9)
	{
		if (random() < .333)	
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("jpn/katana/swing1.wav"), 1, ATTN_NORM, 0);
		}
		else if (random() < .5)	
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("jpn/katana/swing2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("jpn/katana/swing3.wav"), 1, ATTN_NORM, 0);
		}
	}







	Weapon_Generic (ent, 
		7, 14, 29, 
		29,29,36,
		36,36,36, 
		pause_frames, fire_frames, Weapon_Katana_Fire);
}




//molotov cocktail




void Weapon_Molotov (edict_t *ent)
{
	static int      pause_frames[]  = {0};//{19, 32, 0};
    int				fire_frames[] = {7, 9};

	ent->client->p_rnd= NULL;//&ent->client->mags[jpn_index].antitank_rnd;


	ent->client->crosshair = false;

	if (ent->client->aim)
		ent->client->aim = false;

	
	//faf
	fire_frames[0]=19;//faf54:7;

	fire_frames[1] = 0;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/light.wav"), 1, ATTN_NORM, 0);

	if (ent->client->ps.gunframe == 16)
		if (ent->client->buttons & BUTTON_ATTACK)
			return;



	Weapon_Generic (ent, 
		3,  20, 54, 
		54, 54, 57,
		57, 57, 57, 
		pause_frames, fire_frames, Weapon_Molotov_Fire);
}

/////////////////////////////////////////////////
// MG42
/////////////////////////////////////////////////

void JPN_Weapon_MG42 (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{38, 61, 0};
	static int	fire_frames[2];

//	fire_frames[0]=(ent->client->aim)?99:20;
	fire_frames[0]=(ent->client->aim)?86:21;
	fire_frames[1]=(ent->client->aim)?87:22;

	ent->client->p_rnd= &ent->client->mags[jpn_index].hmg_rnd;

	ent->client->crosshair = false;
	if (ent->client->weaponstate == WEAPON_RELOADING)
	{
		if (ent->client->ps.gunframe == 14)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("grm/mg42/unload.wav"), 1, ATTN_NORM, 0);
		
		else if (ent->client->ps.gunframe == 72)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("grm/mg42/reload.wav"), 1, ATTN_NORM, 0);
	}
	Weapon_Generic (ent, 
		20, 22, 62, 
		79, 79, 82, 
		85, 87, 99, 
		
		pause_frames, fire_frames, Weapon_HMG_Fire);
}


