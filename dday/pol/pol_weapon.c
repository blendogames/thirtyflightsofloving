/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/pol/pol_weapon.c,v $
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

#include "pol_main.h"

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
// Vis
/////////////////////////////////////////////////

void Weapon_Vis (edict_t *ent)
{
	static int	pause_frames[]	= {53};//{13, 32,42};
	static int	fire_frames[1];	//= {4,59,0};

	fire_frames[0]=(ent->client->aim)?71:4;

	ent->client->p_fract= &ent->client->mags[pol_index].pistol_fract;
	ent->client->p_rnd= &ent->client->mags[pol_index].pistol_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?73:6)
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
		58, 61, 65, 
		70, 73, 84, 
		
		pause_frames, fire_frames, Weapon_Pistol_Fire);
}


void Weapon_SVT (edict_t *ent)
{
	static int	pause_frames[]	= {55};//{47, 80};
	static int	fire_frames[1];
	
	fire_frames[0] = (ent->client->aim)?76:4;

	// Wheaty: Uncomment next line to allow topping off
	//ent->client->p_fract = &ent->client->mags[usa_index].rifle_fract
	ent->client->p_rnd= &ent->client->mags[pol_index].rifle_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?78:6)
			//gotta do it this way for both firing modes
)
		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
		//	ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work



//faf: bayonet code
	if (ent->client->ps.gunframe == 93)
	{
		ent->client->ps.gunframe = 10;
		return;
	}

	if (ent->client->ps.gunframe > 89)
	{
		ent->client->ps.gunframe++;
		return;
	}
//end bayonet




	Weapon_Generic (ent, 
		 3,  6, 47, 
		64, 67, 72, 
		75, 78, 89, 
		
		pause_frames, fire_frames, Weapon_Rifle_Fire);

}
/*
/////////////////////////////////////////////////
// Sten
/////////////////////////////////////////////////

void Weapon_Sten_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;


	int randnum; //faf

	srand(rand());
	randnum=rand()%300;

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].submg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (ptrlevel->time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = ptrlevel->time + 1;
		}
		
		
		return;
	}

	if (!ent->client->aim)
	{
		for (i=0 ; i<3 ; i++)
		{
			// Thompson extra-kick (1.7 to 1.9)
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = (crandom() * 0.5)-1.5;
			//ent->client->kick_angles[i] = (crandom() * 1.9)-1.5;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.38;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.8;

	}
	else
	{
		for (i=1 ; i<3 ; i++)
		{	
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = crandom() * 0.35;
			//ent->client->kick_angles[i] = crandom() * 0.7;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	}

	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	ent->client->machinegun_shots++;
	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;
//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		gi.dprintf("*** Firing System Error\n");

//faf for testing	safe_cprintf(ent, PRINT_HIGH, "%i randnum\n", randnum);

//	jamchance = rand() % 100;
	if (randnum == 5)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		safe_centerprintf(ent, "Your gun jammed!\n");
		ent->client->mags[mag_index].submg_rnd= 0;
		return;
	}


	// rezmoth - cosmetic recoil
	if (ptrlevel->framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;
	ent->client->kick_angles[1] = ent->client->machinegun_shots * .3;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);

	if(ent->client->mags[mag_index].submg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	
	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);

  
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].submg_rnd--;
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;
}

void Weapon_Sten (edict_t *ent)
{

	static int	pause_frames[]	= {0};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;

	ent->client->p_fract= &ent->client->mags[pol_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[pol_index].submg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 45, 
		71, 71, 75,
		78, 80, 92, 
		
		pause_frames, fire_frames, Weapon_Sten_Fire);

}

*/

/////////////////////////////////////////////////
// RKM
/////////////////////////////////////////////////

void Weapon_K43_Fire (edict_t *ent); //old lmg code
void Weapon_RKM (edict_t *ent)
{
	static int	pause_frames[]	= {54};//{23, 45, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?77:4;
//	fire_frames[1]=(ent->client->aim)?78:5;

	ent->client->p_fract= &ent->client->mags[pol_index].lmg_fract;
	ent->client->p_rnd= &ent->client->mags[pol_index].lmg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 44,
		69, 69, 73,
		76, 78, 89,
		
		pause_frames, fire_frames, Weapon_LMG_Fire);
}


/////////////////////////////////////////////////
// MG34
/////////////////////////////////////////////////

void Weapon_MG34 (edict_t *ent)
{
	static int	pause_frames[]	= {69};//{38, 61, 0};
	static int	fire_frames[2];

//	fire_frames[0]=(ent->client->aim)?99:20;
	fire_frames[0]=(ent->client->aim)?86:21;
	fire_frames[1]=(ent->client->aim)?87:22;

	ent->client->p_rnd= &ent->client->mags[pol_index].hmg_rnd;

	ent->client->crosshair = false;

	if (ent->client->ps.gunframe == 7)
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/vickers/bolt.wav"), 1, ATTN_NORM, 0);

	Weapon_Generic (ent, 
		20, 22, 62, 
		79, 79, 82, 
		85, 87, 99, 
		
		pause_frames, fire_frames, Weapon_MG34_Fire);
}



/////////////////////////////////////////////////
// PIAT
/////////////////////////////////////////////////



void POL_Weapon_PIAT (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[1];

	// Nick 25/11/2002 added next declare for empty barrel hack.
	int mag_index;

	mag_index = ent->client->pers.weapon->mag_index;

	fire_frames[0]=(ent->client->aim)?83:4;

	ent->client->p_rnd = &ent->client->mags[pol_index].antitank_rnd;

	ent->client->crosshair = false;


	//faf
	if (ent->client->ps.gunframe == 85)//(last fire)
	{
		ent->client->weaponstate = WEAPON_LOWER;
		ent->client->aim = false;
	}

// gi.dprintf("%i\n",ent->client->ps.gunframe);

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

 
/////////////////////////////////////////////////
// Mauser 98(ks) Sniper Rifle
/////////////////////////////////////////////////

void Weapon_POL_m98ks (edict_t *ent)
{
	static int	pause_frames[]	= {34};
	static int	fire_frames[4];

	fire_frames[0]=(ent->client->aim)?52:4;//fire here
	fire_frames[1]=(ent->client->aim)?59:0;//faf 60:0;//sniper bolt
	fire_frames[2]=(ent->client->aim)?75:0;//sniper start zoom
	fire_frames[3]=(ent->client->aim)?80:0;//sniper end zoom

	ent->client->p_fract= &ent->client->mags[pol_index].sniper_fract;
	ent->client->p_rnd= &ent->client->mags[pol_index].sniper_rnd;

	if (ent->client->ps.gunframe == 61)
		ent->client->ps.gunframe = 65;

	if (ent->client->ps.gunframe == 67)
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
/*			&& ent->client->ps.gunframe!=((ent->client->aim)?55:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?56:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?57:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?58:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?59:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?60:12)
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:16)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:17)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:18)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:19)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:20)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:21)
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:22)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:23)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:24)
			&& ent->client->ps.gunframe!=((ent->client->aim)?73:25)
			&& ent->client->ps.gunframe!=((ent->client->aim)?74:26)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?78:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?79:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:8)*/
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

	Weapon_Generic (ent, 
		 3, 8, 26,//25, 27,//faf 26, 26, 
		43, 43, 48, 
		51, 80, 80, 
		
		pause_frames, fire_frames, Weapon_Sniper_Fire);
}


void Weapon_Mors (edict_t *ent)
{

	static int	pause_frames[]	= {52};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;

	ent->client->p_fract= &ent->client->mags[pol_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[pol_index].submg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 45, 
		71, 71, 75, 
		78, 80, 92, 
		
		pause_frames, fire_frames, Weapon_Submachinegun_Fire);

}



void Weapon_Sabre (edict_t *ent)
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
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/sabre/draw.wav"), 1, ATTN_NORM, 0);//faf




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
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/sabre/swing1.wav"), 1, ATTN_NORM, 0);
		}
		else if (random() < .5)	
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/sabre/swing2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("pol/sabre/swing3.wav"), 1, ATTN_NORM, 0);
		}
	}







	Weapon_Generic (ent, 
		7, 14, 29, 
		29,29,36,
		36,36,36, 
		pause_frames, fire_frames, Weapon_Sabre_Fire);
}


