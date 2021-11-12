/* Knightmare's cvar header file */

extern	cvar_t	*mega_gibs;					// whether to spawn extra gibs, default to 0
extern	cvar_t	*mp_monster_replace;		// whether to replace monsters with mission pack modified variants
extern	cvar_t	*mp_monster_ammo_replace;	// whether to replace monsters' ammo items if above is enabled
extern	cvar_t	*kamikaze_flyer_replace;	// whether to replace some flyers with kamikaze flyers, probability 0-1
extern	cvar_t	*allow_player_use_abandoned_turret;	// whether to allow player to use turrets in exisiting maps
extern	cvar_t	*turn_rider;				// whether to turn player on rotating object
extern	cvar_t	*adjust_train_corners;		// whether to subtract (1,1,1) from train path corners to fix misalignments

extern	cvar_t	*add_velocity_throw;		// whether to add player's velocity to thrown objects
extern	cvar_t	*falling_armor_damage;		// whether player's armor absorbs damage from falling
extern	cvar_t	*player_jump_sounds;		// whether to play that STUPID grunting sound when the player jumps
extern	cvar_t	*tpp_auto;					// whether to automatically go into third-person when pushing a pushable
extern	cvar_t	*use_vwep;

// Server-side speed control stuff
extern	cvar_t	*player_max_speed;
extern	cvar_t	*player_crouch_speed;
extern	cvar_t	*player_accel;
extern	cvar_t	*player_stopspeed;

// weapon balancing
extern	cvar_t	*sk_blaster_damage;
extern	cvar_t	*sk_blaster_damage_dm;
extern	cvar_t	*sk_blaster_speed;
extern	cvar_t	*sk_blaster_color;

extern	cvar_t	*sk_shotgun_damage;
extern	cvar_t	*sk_shotgun_count;
extern	cvar_t	*sk_shotgun_hspread;
extern	cvar_t	*sk_shotgun_vspread;

extern	cvar_t	*sk_sshotgun_damage;
extern	cvar_t	*sk_sshotgun_count;
extern	cvar_t	*sk_sshotgun_hspread;
extern	cvar_t	*sk_sshotgun_vspread;

extern	cvar_t	*sk_machinegun_damage;
extern	cvar_t	*sk_machinegun_hspread;
extern	cvar_t	*sk_machinegun_vspread;

extern	cvar_t	*sk_chaingun_damage;
extern	cvar_t	*sk_chaingun_damage_dm;
extern	cvar_t	*sk_chaingun_hspread;
extern	cvar_t	*sk_chaingun_vspread;

extern	cvar_t	*sk_grenade_damage;
extern	cvar_t	*sk_grenade_radius;
extern	cvar_t	*sk_grenade_speed;

extern	cvar_t	*sk_hand_grenade_damage;
extern	cvar_t	*sk_hand_grenade_radius;

extern	cvar_t	*sk_rocket_damage;
extern	cvar_t	*sk_rocket_damage2;
extern	cvar_t	*sk_rocket_rdamage;
extern	cvar_t	*sk_rocket_radius;
extern	cvar_t	*sk_rocket_speed;

extern	cvar_t	*sk_hyperblaster_damage;
extern	cvar_t	*sk_hyperblaster_damage_dm;
extern	cvar_t	*sk_hyperblaster_speed;
extern	cvar_t	*sk_hyperblaster_color;

extern	cvar_t	*sk_railgun_damage;
extern	cvar_t	*sk_railgun_damage_dm;
extern	cvar_t	*sk_railgun_skin;

extern	cvar_t	*sk_bfg_damage;
extern	cvar_t	*sk_bfg_damage_dm;
extern	cvar_t	*sk_bfg_damage2;
extern	cvar_t	*sk_bfg_damage2_dm;
extern	cvar_t	*sk_bfg_rdamage;
extern	cvar_t	*sk_bfg_radius;
extern	cvar_t	*sk_bfg_speed;

extern	cvar_t	*sk_jump_kick_damage;

extern	cvar_t	*sk_ionripper_damage;
extern	cvar_t	*sk_ionripper_damage_dm;
extern	cvar_t	*sk_ionripper_kick;
extern	cvar_t	*sk_ionripper_kick_dm;
extern	cvar_t	*sk_ionripper_speed;
extern	cvar_t	*sk_ionripper_extra_sounds;

extern	cvar_t	*sk_phalanx_damage;
extern	cvar_t	*sk_phalanx_damage2;
extern	cvar_t	*sk_phalanx_radius_damage;
extern	cvar_t	*sk_phalanx_radius;
extern	cvar_t	*sk_phalanx_speed;

extern	cvar_t	*sk_trap_life;
extern	cvar_t	*sk_trap_health;

extern	cvar_t	*sk_etf_rifle_damage;
extern	cvar_t	*sk_etf_rifle_damage_dm;
extern	cvar_t	*sk_etf_rifle_radius_damage;
extern	cvar_t	*sk_etf_rifle_radius;
extern	cvar_t	*sk_etf_rifle_speed;

extern	cvar_t	*sk_plasmabeam_damage;
extern	cvar_t	*sk_plasmabeam_damage_dm;
extern	cvar_t	*sk_plasmabeam_kick;
extern	cvar_t	*sk_plasmabeam_kick_dm;

extern	cvar_t	*sk_disruptor_damage;
extern	cvar_t	*sk_disruptor_damage_dm;
extern	cvar_t	*sk_disruptor_speed;

extern	cvar_t	*sk_prox_damage;
extern	cvar_t	*sk_prox_radius;
extern	cvar_t	*sk_prox_speed;
extern	cvar_t	*sk_prox_life;
extern	cvar_t	*sk_prox_health;

extern	cvar_t	*sk_tesla_damage;
extern	cvar_t	*sk_tesla_radius;
extern	cvar_t	*sk_tesla_life;
extern	cvar_t	*sk_tesla_health;

extern	cvar_t	*sk_chainfist_damage;
extern	cvar_t	*sk_chainfist_damage_dm;

extern	cvar_t	*sk_plasma_rifle_damage_bounce;
extern	cvar_t	*sk_plasma_rifle_damage_bounce_dm;
extern	cvar_t	*sk_plasma_rifle_damage_spread;
extern	cvar_t	*sk_plasma_rifle_damage_spread_dm;
extern	cvar_t	*sk_plasma_rifle_speed_bounce;
extern	cvar_t	*sk_plasma_rifle_speed_spread;
extern	cvar_t	*sk_plasma_rifle_radius;
extern	cvar_t	*sk_plasma_rifle_life_bounce;
extern	cvar_t	*sk_plasma_rifle_life_spread;

extern	cvar_t	*sk_nuke_delay;
extern	cvar_t	*sk_nuke_life;
extern	cvar_t	*sk_nuke_radius;

extern	cvar_t	*sk_nbomb_delay;
extern	cvar_t	*sk_nbomb_life;
extern	cvar_t	*sk_nbomb_radius;
extern	cvar_t	*sk_nbomb_damage;

extern	cvar_t	*sk_shockwave_bounces;
extern	cvar_t	*sk_shockwave_damage;
extern	cvar_t	*sk_shockwave_damage2;
extern	cvar_t	*sk_shockwave_rdamage;
extern	cvar_t	*sk_shockwave_speed;
extern	cvar_t	*sk_shockwave_radius;
extern	cvar_t	*sk_shockwave_effect_damage;
extern	cvar_t	*sk_shockwave_effect_radius;

extern	cvar_t	*sk_flaregun_damage;
extern	cvar_t	*sk_flaregun_rdamage;
extern	cvar_t	*sk_flaregun_radius;
extern	cvar_t	*sk_flaregun_speed;
extern	cvar_t	*sk_flare_life;

extern	cvar_t	*sk_tbomb_damage;
extern	cvar_t	*sk_tbomb_radius;
extern	cvar_t	*sk_tbomb_shrapnel;
extern	cvar_t	*sk_tbomb_shrapnel_damage;
extern	cvar_t	*sk_tbomb_life;
extern	cvar_t	*sk_tbomb_health;

/*
extern	cvar_t	*sk_sniperrifle_damage;
extern	cvar_t	*sk_sniperrifle_damage_dm;
extern	cvar_t	*sk_sniperrifle_kick;
extern	cvar_t	*sk_sniperrifle_kick_dm;

extern	cvar_t	*sk_soniccannon_damage;
extern	cvar_t	*sk_soniccannon_damage2;
extern	cvar_t	*sk_soniccannon_radius;
extern	cvar_t	*sk_soniccannon_maxfiretime;
extern	cvar_t	*sk_soniccannon_maxcells;
*/
extern	cvar_t	*sk_empnuke_radius;
/*
extern	cvar_t	*sk_a2k_damage;
extern	cvar_t	*sk_a2k_radius;
*/
extern	cvar_t	*sk_plasmashield_health;
extern	cvar_t	*sk_plasmashield_life;


// DM start values
extern	cvar_t	*sk_dm_start_shells;
extern	cvar_t	*sk_dm_start_bullets;
extern	cvar_t	*sk_dm_start_rockets;
extern	cvar_t	*sk_dm_start_homing;
extern	cvar_t	*sk_dm_start_grenades;
extern	cvar_t	*sk_dm_start_cells;
extern	cvar_t	*sk_dm_start_slugs;
extern	cvar_t	*sk_dm_start_magslugs;
extern	cvar_t	*sk_dm_start_traps;
extern	cvar_t	*sk_dm_start_flechettes;
extern	cvar_t	*sk_dm_start_rounds;
extern	cvar_t	*sk_dm_start_prox;
extern	cvar_t	*sk_dm_start_tesla;
extern	cvar_t	*sk_dm_start_shocksphere;
extern	cvar_t	*sk_dm_start_flares;
extern	cvar_t	*sk_dm_start_tbombs;
extern	cvar_t	*sk_dm_start_empnuke;
extern	cvar_t	*sk_dm_start_plasmashield;

extern	cvar_t	*sk_dm_start_shotgun;
extern	cvar_t	*sk_dm_start_sshotgun;
extern	cvar_t	*sk_dm_start_machinegun;
extern	cvar_t	*sk_dm_start_chaingun;
extern	cvar_t	*sk_dm_start_grenadelauncher;
extern	cvar_t	*sk_dm_start_rocketlauncher;
extern	cvar_t	*sk_dm_start_hyperblaster;
extern	cvar_t	*sk_dm_start_railgun;
extern	cvar_t	*sk_dm_start_bfg;
extern	cvar_t	*sk_dm_start_ionripper;
extern	cvar_t	*sk_dm_start_phalanx;
extern	cvar_t	*sk_dm_start_etfrifle;
extern	cvar_t	*sk_dm_start_proxlauncher;
extern	cvar_t	*sk_dm_start_plasmabeam;
extern	cvar_t	*sk_dm_start_disruptor;
extern	cvar_t	*sk_dm_start_chainfist;
extern	cvar_t	*sk_dm_start_plasmarifle;
extern	cvar_t	*sk_dm_start_shockwave;
extern	cvar_t	*sk_dm_start_flaregun;
//extern	cvar_t	*sk_dm_start_sniperrifle;
//extern	cvar_t	*sk_dm_start_soniccannon;

// maximum values
extern	cvar_t	*sk_max_health;
extern	cvar_t	*sk_max_health_dm;
extern	cvar_t	*sk_max_foodcube_health;
extern	cvar_t	*sk_max_armor;
extern	cvar_t	*sk_max_armor_jacket;
extern	cvar_t	*sk_max_armor_combat;
extern	cvar_t	*sk_max_armor_body;
extern	cvar_t	*sk_max_bullets;
extern	cvar_t	*sk_max_shells;
extern	cvar_t	*sk_max_rockets;
extern	cvar_t	*sk_max_grenades;
extern	cvar_t	*sk_max_cells;
extern	cvar_t	*sk_max_slugs;
extern	cvar_t	*sk_max_magslugs;
extern	cvar_t	*sk_max_traps;
extern	cvar_t	*sk_max_flechettes;
extern	cvar_t	*sk_max_rounds;
extern	cvar_t	*sk_max_prox;
extern	cvar_t	*sk_max_tesla;
extern	cvar_t	*sk_max_shocksphere;
extern	cvar_t	*sk_max_fuel;
extern	cvar_t	*sk_max_flares;
extern	cvar_t	*sk_max_tbombs;
extern	cvar_t	*sk_max_empnuke;
extern	cvar_t	*sk_max_plasmashield;
//extern	cvar_t	*sk_max_a2k;

// maximum settings if a player gets a bandolier
extern	cvar_t	*sk_bando_bullets;
extern	cvar_t	*sk_bando_shells;
extern	cvar_t	*sk_bando_cells;
extern	cvar_t	*sk_bando_slugs;
extern	cvar_t	*sk_bando_magslugs;
extern	cvar_t	*sk_bando_flechettes;
extern	cvar_t	*sk_bando_rounds;
extern	cvar_t	*sk_bando_fuel;
extern	cvar_t	*sk_bando_flares;

// maximum settings if a player gets a pack
extern	cvar_t	*sk_pack_bullets;
extern	cvar_t	*sk_pack_shells;
extern	cvar_t	*sk_pack_rockets;
extern	cvar_t	*sk_pack_grenades;
extern	cvar_t	*sk_pack_cells;
extern	cvar_t	*sk_pack_slugs;
extern	cvar_t	*sk_pack_magslugs;
extern	cvar_t	*sk_pack_traps;
extern	cvar_t	*sk_pack_flechettes;
extern	cvar_t	*sk_pack_rounds;
extern	cvar_t	*sk_pack_prox;
extern	cvar_t	*sk_pack_tesla;
extern	cvar_t	*sk_pack_shocksphere;
extern	cvar_t	*sk_pack_fuel;
extern	cvar_t	*sk_pack_flares;
extern	cvar_t	*sk_pack_tbombs;
extern	cvar_t	*sk_pack_empnuke;
extern	cvar_t	*sk_pack_plasmashield;
//extern	cvar_t	*sk_pack_a2k;

extern	cvar_t	*sk_pack_give_xatrix_ammo;
extern	cvar_t	*sk_pack_give_rogue_ammo;
extern	cvar_t	*sk_pack_give_zaero_ammo;

// pickup values
extern	cvar_t	*sk_box_shells; // value of shells
extern	cvar_t	*sk_box_bullets; // value of bullets
extern	cvar_t	*sk_box_grenades; // value of grenade pack
extern	cvar_t	*sk_box_rockets; // value of rocket pack
extern	cvar_t	*sk_box_cells; // value of cell pack
extern	cvar_t	*sk_box_slugs; // value of slug box
extern	cvar_t	*sk_box_magslugs; // value ofmagslug box
extern	cvar_t	*sk_box_flechettes; // value of flechettes
extern	cvar_t	*sk_box_prox; // value of prox
extern	cvar_t	*sk_box_tesla; // value of tesla pack
extern	cvar_t	*sk_box_disruptors; // value of disruptor pack
extern	cvar_t	*sk_box_shocksphere; // value of shocksphere
extern	cvar_t	*sk_box_trap; // value of trap
extern	cvar_t	*sk_box_fuel; // value of fuel
extern	cvar_t	*sk_box_flares;	// value of flares box
extern	cvar_t	*sk_box_tbombs;	// value of IRED box
extern	cvar_t	*sk_box_empnuke;	// value of EMP nuke
extern	cvar_t	*sk_box_plasmashield;	// value of plasma shield

// items/powerups
extern	cvar_t	*sk_armor_bonus_value; //value of armor shards
extern	cvar_t	*sk_health_bonus_value; //value of stimpacks
extern	cvar_t	*sk_powerup_max;
extern	cvar_t	*sk_nuke_max;
extern	cvar_t	*sk_nbomb_max;
extern	cvar_t	*sk_doppleganger_max;
extern	cvar_t	*sk_defender_time;
extern	cvar_t	*sk_defender_blaster_damage;
extern	cvar_t	*sk_defender_blaster_speed;
extern	cvar_t	*sk_vengeance_time;
extern	cvar_t	*sk_vengeance_health_threshold;
extern	cvar_t	*sk_hunter_time;
extern	cvar_t	*sk_doppleganger_time;
extern	cvar_t	*sk_quad_time;
extern	cvar_t	*sk_inv_time;
extern	cvar_t	*sk_breather_time;
extern	cvar_t	*sk_enviro_time;
extern	cvar_t	*sk_silencer_shots;
extern	cvar_t	*sk_ir_time;
extern	cvar_t	*sk_double_time;
extern	cvar_t	*sk_quad_fire_time;
extern	cvar_t	*sk_stasis_time;
extern	cvar_t	*sk_visor_time;
//extern	cvar_t	*sk_sniper_charge_time;
//extern	cvar_t	*sk_a2k_detonate_time;
