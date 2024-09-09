/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_weapon.c,v $
 *   $Revision: 1.12 $
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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, gbr.

*/

#include "gbr_main.h"


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
// Webley
/////////////////////////////////////////////////

void Weapon_Webley (edict_t *ent)
{
	static int	pause_frames[]	= {0};//58};//{13, 32,42};
	static int	fire_frames[1];	//= {4,59,0};
	
	fire_frames[0]=(ent->client->aim)?82:4;  //75

	ent->client->p_fract= &ent->client->mags[gbr_index].pistol_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].pistol_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?84:6)
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
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

		// Nick 21/12/2002 - Stop the 'nil ammo' idle -> reload animation.
		// This bug can be seen on any other Teams' pistol.  Empty a clip, and
		// then go T/S, fire a 'blank'.  Let the model go into idle, sit back and
		// watch as the model cycles thru' idle -> reload frames -> back to T/S!.

		//if ((ent->client->ps.gunframe == 47) & (!ent->client->mags[gbr_index].pistol_rnd)) {
		//	ent->client->ps.gunframe = 7;
		//}

		// See the pistol_fire2 routine.  Can leave that as is and use above code
		// or as it is now to get the gun to animate firing with no ammo.
		// End Nick

//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work


	Weapon_Generic (ent, 
		 3,  6, 47, 
		69, 72, 76,
		81, 84, 95, 
		
		pause_frames, fire_frames, Weapon_Pistol_Fire);//_GBR);
}



/////////////////////////////////////////////////
//Lee Enfield
/////////////////////////////////////////////////
//(based on mauser code)
void Weapon_Enfield (edict_t *ent)
{
	static int	pause_frames[]	= {65};//{4,25, 50, 0};
	static int	fire_frames[1];// = {4};

	fire_frames[0]=(ent->client->aim)?89:4;
	
	ent->client->p_fract = &ent->client->mags[gbr_index].rifle_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].rifle_rnd;

	ent->client->crosshair = false;


	if (ent->client->ps.gunframe == 96)
	{
		ent->client->crosshair_offset_x = random() * 4 - 2;
		ent->client->crosshair_offset_y = random() * 2;
	}


    

	if (ent->client->weaponstate != WEAPON_RELOADING &&
		ent->client->ps.gunframe > 98 &&
		ent->client->ps.gunframe < 107 &&
		!ent->client->aim)
	{
		//gi.dprintf ("%i\n",ent->client->weaponstate);
		ent->client->ps.gunframe = 87;
		ent->client->weaponstate = WEAPON_LOWER;
//		ent->client->ps.gunframe = 18;
//		ent->client->weaponstate = WEAPON_READY;
		return;
	}

if ((ent->client->weaponstate == WEAPON_FIRING || 
	 ent->client->weaponstate == WEAPON_READY)
		&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?88:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?89:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?90:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?91:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?92:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?93:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?94:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?95:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?96:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?97:12)			
			&& ent->client->ps.gunframe!=((ent->client->aim)?98:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?99:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?100:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?101:16)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?0:17)
			
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
		ent->client->ps.gunframe = (ent->client->aim)?101:16;
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

/*	if (( *ent->client->p_rnd == 0 ) && 
		(ptrlevel->time >= ent->pain_debounce_time) &&
		(ent->client->weaponstate == WEAPON_READY))
	{
		Weapon_Generic (ent,
			100,  102, 105, 
			106,  108, 110,
			112,  114, 116,
			
			pause_frames, fire_frames, Weapon_Bayonet_Fire);

	}
	else
	{ */



//faf: bayonet code
	if (ent->client->ps.gunframe == 111)
	{
		ent->client->ps.gunframe = 18;
		return;
	}

	if (ent->client->ps.gunframe > 107)
	{
		ent->client->ps.gunframe++;
		return;
	}
//end bayonet

	Weapon_Generic (ent, 
		3,  16, 56, 
		81, 81, 85, 
		88, 101, 106, 
		
		pause_frames, fire_frames, Weapon_Rifle_Fire);
	//}
}

/////////////////////////////////////////////////
// Sten
/////////////////////////////////////////////////

void Weapon_Sten (edict_t *ent)
{

	static int	pause_frames[]	= {56};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;

	ent->client->p_fract= &ent->client->mags[gbr_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].submg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 45, 
		71, 71, 75,
		78, 80, 92, 
		
		pause_frames, fire_frames, Weapon_Sten_Fire);

}

/////////////////////////////////////////////////
// Bren (LMG)
/////////////////////////////////////////////////

void Weapon_Bren (edict_t *ent)
{
	static int	pause_frames[]	= {54};//{23, 45, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?77:4;
	fire_frames[1]=(ent->client->aim)?78:5;

	ent->client->p_fract= &ent->client->mags[gbr_index].lmg_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].lmg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 44,
		69, 69, 73,
		76, 78, 89,
		
		pause_frames, fire_frames, Weapon_Bren_Fire);
}





/////////////////////////////////////////////////
// Heavy Machine Gun (Vickers 'K')
/////////////////////////////////////////////////

void Weapon_Vickers (edict_t *ent)
{
	static int	pause_frames[]	= {74};//{38, 61, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?104:20;
	fire_frames[1]=(ent->client->aim)?105:21;
//	fire_frames[2]=(ent->client->aim)?91:0;

	ent->client->p_rnd= &ent->client->mags[gbr_index].hmg_rnd;

	ent->client->crosshair = false;

	// Nick 23/11/2002 - Parts 7th frame bolt.wav (added bolt.wav to weapon cache preload also).
	if (ent->client->ps.gunframe == 7)
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gbr/vickers/bolt.wav"), 1, ATTN_NORM, 0);

	Weapon_Generic (ent, 
		19, 21,  61, 
		93, 96,  99,
		104, 105, 116,
		
		pause_frames, fire_frames, Weapon_Vickers_Fire);
}



/////////////////////////////////////////////////
// PIAT
/////////////////////////////////////////////////


void Weapon_PIAT (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[1];

	// Nick 25/11/2002 added next declare for empty barrel hack.
	int mag_index;

	mag_index = ent->client->pers.weapon->mag_index;

	fire_frames[0]=(ent->client->aim)?83:4;

	ent->client->p_rnd = &ent->client->mags[gbr_index].antitank_rnd;

	ent->client->crosshair = false;


	//faf
	if (ent->client->ps.gunframe == 85)//(last fire)
	{
		ent->client->weaponstate = WEAPON_LOWER;
		ent->client->aim = false;
	}


	// Nick 25/11/2002 - Hack to make the 'barrel' empty after firing a shell.
	// I added 10 extra 'empty' frames in the idle cycle, and if PIAT is not
	// loaded it will cycle through that as opposed to the 'loaded' frames.
	//  If this is used as a 'if else' logic test, the frames go wonky on reload animation.

		if (ent->client->mags[mag_index].antitank_rnd < 1) {

	Weapon_Generic (ent, 
		 3,  45, 56, 
		75, 75, 79, 
		82, 85, 96,
					
		pause_frames, fire_frames, Weapon_PIAT_Fire);
		} 

		if (ent->client->mags[mag_index].antitank_rnd > 0) {

		Weapon_Generic (ent, 
		 3,  5, 45, 
		75, 75, 79, 
		82, 85, 96,
					
		pause_frames, fire_frames, Weapon_PIAT_Fire);
	}
}
// End Nick


/////////////////////////////////////////////////
// Lee Enfield Sniper Springfield Sniper Rifle
/////////////////////////////////////////////////

void Weapon_Enfields (edict_t *ent)
{
	static int	pause_frames[]	= {33};
	static int	fire_frames[4];

	fire_frames[0]=(ent->client->aim)?62:4;//fire here
	fire_frames[1]=(ent->client->aim)?71:0;//sniper bolt
	fire_frames[2]=(ent->client->aim)?86:0;//sniper start zoom
	fire_frames[3]=(ent->client->aim)?91:0;//sniper end zoom

	ent->client->p_fract= &ent->client->mags[gbr_index].sniper_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].sniper_rnd;

	//skip delay
	if (ent->client->ps.gunframe == 73)
		ent->client->ps.gunframe = 76;

	if (ent->client->ps.gunframe == 78)
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gbr/303s/bolt.wav"), 1, ATTN_NORM, 0);


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
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:12)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?73:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?74:16)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:17)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:18)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:19)
			&& ent->client->ps.gunframe!=((ent->client->aim)?78:20)
			&& ent->client->ps.gunframe!=((ent->client->aim)?79:21)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:22)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:23)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:24)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:25)
			&& ent->client->ps.gunframe!=((ent->client->aim)?84:26)
			&& ent->client->ps.gunframe!=((ent->client->aim)?85:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?86:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?87:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?88:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?89:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?90:8)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?81:9)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?82:10)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?83:11)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?84:12)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?85:13)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?86:14)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?87:15)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?88:16)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?89:17)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?90:18)

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

	// Nick 25/12/2002 - Terrible, terrible hack to get the zoomout right
	// after firing a shot, rather than the gun suddenly appearing idle.
	// Two new 'remove from eye' frames added/replaced on model.
	if (ent->client->ps.gunframe >=69 && ent->client->ps.gunframe <= 71) {
		ent->client->crosshair = false;
		ent->client->ps.fov = STANDARD_FOV;
		// Nick 27/12/2002 - Need the next bit to stop reload animation
		// half starting the zoom-in again then going back to idle if reload 
		// (use weapon) button is hit immediately after firing a shot.
		// Had me beat for 2 hours 8=)
		ent->client->aim = false;
	}

	Weapon_Generic (ent, 
		 3, 26, 26, 
		53, 53, 58, 
		61, 92, 92, 
		
		pause_frames, fire_frames, Weapon_Sniper_Fire);
}


