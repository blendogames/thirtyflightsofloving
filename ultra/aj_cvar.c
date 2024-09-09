#include "g_local.h"

/******************* Cvar's *******************/

// enable/disable options
cvar_t	*use_runes;          // enables runes
cvar_t	*use_hook;           // enables grappling hook

cvar_t	*use_vwep;           // enables view weapons
cvar_t	*use_mapqueue;       // enables map queueing
cvar_t	*use_fastswitch;     // enables fast weapon switching (ala Rocket Arena 2)

cvar_t	*mapqueue;           // sets plain text file to read for map cycling
cvar_t	*map_random;         // enables random map queue order
cvar_t	*motd;               // sets plain text file to read for MOTD
cvar_t	*def_bestweapon;     // sets default client bestweapon on/off
cvar_t	*def_id;             // sets default client player id on/off
cvar_t	*def_hud;            // sets default client HUD
//                              0 = none, 1 = normal, 2 = lithium, 3 = ammo
cvar_t	*def_scores;         // sets default client scoreboard layout
//                                   0 = old, 1 = by frags, 2 = by FPH
cvar_t	*mega_gibs;			 // whether to spawn extra gibs, default to 0
cvar_t	*player_gib_health;  //when to gib


cvar_t	*intermission_time;  // minimum time the intermission will last
cvar_t	*intermission_sound; // sound to play during intermission

cvar_t	*sk_fall_damagemod;     // fall damage modifier (fall damage multiplied by this)
cvar_t	*sk_jacket_armor;       // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*sk_combat_armor;       // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*sk_body_armor;         // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*sk_shard_armor;        // sets amount of armor gained from armor shards
cvar_t	*sk_power_armor_screen; // sets damage saved per cell
cvar_t	*sk_power_armor_shield; // sets damage saved per cell
cvar_t	*sk_dropweapammo;       // percentage of default ammo gained from a dropped weapon

cvar_t	*allow_func_explosives;
cvar_t	*allow_disabled_func_explosives;

// sets damage the weapon does per projectile
cvar_t	*sk_blaster_damage;
//cvar_t	*sk_blaster_count;
//cvar_t	*sk_blaster_hspread;
//cvar_t	*sk_blaster_vspread;
//cvar_t	*sk_blaster_damage2;
//cvar_t	*sk_blaster_rdamage;
cvar_t	*sk_blaster_speed;
cvar_t	*sk_blaster_color;			// 1=orange, 2=green, 3=blue, 4=red

cvar_t	*sk_shotgun_damage;
cvar_t	*sk_shotgun_count;
cvar_t	*sk_shotgun_hspread;
cvar_t	*sk_shotgun_vspread;
//cvar_t	*sk_shotgun_damage2;
//cvar_t	*sk_shotgun_rdamage;
//cvar_t	*sk_shotgun_speed;

cvar_t	*sk_sshotgun_damage;
cvar_t	*sk_sshotgun_count;
cvar_t	*sk_sshotgun_hspread;
cvar_t	*sk_sshotgun_vspread;
//cvar_t	*sk_sshotgun_damage2;
//cvar_t	*sk_sshotgun_rdamage;
//cvar_t	*sk_sshotgun_speed;

cvar_t	*sk_machinegun_damage;
//cvar_t	*sk_machinegun_count;
cvar_t	*sk_machinegun_hspread;
cvar_t	*sk_machinegun_vspread;
//cvar_t	*sk_machinegun_damage2;
//cvar_t	*sk_machinegun_rdamage;
//cvar_t	*sk_machinegun_speed;

cvar_t	*sk_chaingun_damage;
//cvar_t	*sk_chaingun_count;
cvar_t	*sk_chaingun_hspread;
cvar_t	*sk_chaingun_vspread;
//cvar_t	*sk_chaingun_damage2;
//cvar_t	*sk_chaingun_rdamage;
//cvar_t	*sk_chaingun_speed;

cvar_t	*sk_grenade_damage;
//cvar_t	*sk_grenade_count;
//cvar_t	*sk_grenade_hspread;
//cvar_t	*sk_grenade_vspread;
//cvar_t	*sk_grenade_damage2;
//cvar_t	*sk_grenade_rdamage;
cvar_t	*sk_grenade_radius;
cvar_t	*sk_grenade_speed;

cvar_t	*sk_rocket_damage;
//cvar_t	*sk_rocket_count;
//cvar_t	*sk_rocket_hspread;
//cvar_t	*sk_rocket_vspread;
cvar_t	*sk_rocket_damage2;
cvar_t	*sk_rocket_rdamage;
cvar_t	*sk_rocket_radius;
cvar_t	*sk_rocket_speed;

cvar_t	*sk_hyperblaster_damage;
//cvar_t	*sk_hyperblaster_count;
//cvar_t	*sk_hyperblaster_hspread;
//cvar_t	*sk_hyperblaster_vspread;
//cvar_t	*sk_hyperblaster_damage2;
//cvar_t	*sk_hyperblaster_rdamage;
cvar_t	*sk_hyperblaster_speed;
cvar_t	*sk_hyperblaster_color;		// 1=orange, 2=green, 3=blue, 4=red

cvar_t	*sk_railgun_damage;
//cvar_t	*sk_railgun_count;
//cvar_t	*sk_railgun_hspread;
//cvar_t	*sk_railgun_vspread;
//cvar_t	*sk_railgun_damage2;
//cvar_t	*sk_railgun_rdamage;
//cvar_t	*sk_railgun_speed;
cvar_t	*sk_railgun_skin;

cvar_t	*sk_bfg_damage;
//cvar_t	*sk_bfg_count;
//cvar_t	*sk_bfg_hspread;
//cvar_t	*sk_bfg_vspread;
//cvar_t	*sk_bfg_damage2;
//cvar_t	*sk_bfg_rdamage;
cvar_t	*sk_bfg_radius;
cvar_t	*sk_bfg_speed;

// ScarFace
cvar_t	*sk_ionripper_damage;
cvar_t	*sk_ionripper_speed;
cvar_t	*sk_ionripper_extra_sounds;	// enables extra sounds for ionripper

cvar_t	*sk_phalanx_damage;
cvar_t	*sk_phalanx_damage2;
cvar_t	*sk_phalanx_radius_damage;
cvar_t	*sk_phalanx_radius;
cvar_t	*sk_phalanx_speed;

cvar_t	*sk_trap_life;
cvar_t	*sk_trap_health;

cvar_t	*sk_etf_rifle_damage;
cvar_t	*sk_etf_rifle_radius_damage;
cvar_t	*sk_etf_rifle_radius;
cvar_t	*sk_etf_rifle_speed;

cvar_t	*sk_prox_damage;
cvar_t	*sk_prox_radius;
cvar_t	*sk_prox_speed;
cvar_t	*sk_prox_life;
cvar_t	*sk_prox_health;

cvar_t	*sk_plasmabeam_damage;

cvar_t	*sk_disruptor_damage;
cvar_t	*sk_disruptor_speed;

cvar_t	*sk_chainfist_damage;

cvar_t	*sk_tesla_damage;
cvar_t	*sk_tesla_radius;
cvar_t	*sk_tesla_life;
cvar_t	*sk_tesla_health;

cvar_t	*sk_nuke_delay;
cvar_t	*sk_nuke_life;
cvar_t	*sk_nuke_radius;
cvar_t	*sk_defender_blaster_damage;
cvar_t	*sk_defender_blaster_speed;
cvar_t	*sk_vengeance_health_threshold;

cvar_t	*ctf_blastercolors;			// enable different blaster colors for each team
cvar_t	*ctf_railcolors;			// enable different railtrail colors for each team
// end ScarFace

// Knightmare- LMCTF Plasma Rifle
cvar_t	*sk_plasma_rifle_damage_bounce;
cvar_t	*sk_plasma_rifle_damage_bounce_dm;
cvar_t	*sk_plasma_rifle_damage_spread;
cvar_t	*sk_plasma_rifle_damage_spread_dm;
cvar_t	*sk_plasma_rifle_speed_bounce;
cvar_t	*sk_plasma_rifle_speed_spread;
cvar_t	*sk_plasma_rifle_radius;
cvar_t	*sk_plasma_rifle_life_bounce;
cvar_t	*sk_plasma_rifle_life_spread;

cvar_t	*sk_shockwave_bounces;
cvar_t	*sk_shockwave_damage;
cvar_t	*sk_shockwave_damage2;
cvar_t	*sk_shockwave_rdamage;
cvar_t	*sk_shockwave_radius;
cvar_t	*sk_shockwave_speed;
cvar_t	*sk_shockwave_effect_damage;
cvar_t	*sk_shockwave_effect_radius;
// end ScarFace


// sets how many projectiles are fired

// sets the horizontal spread radius of projectiles

// sets the vertical spread radius of projectiles

// sets max random extra damage

// sets radius damage


// starting values
cvar_t	*sk_start_weapon;

cvar_t	*sk_start_health;
cvar_t	*sk_start_armor;	// what sort of armor????
cvar_t	*sk_start_bullets;
cvar_t	*sk_start_shells;
cvar_t	*sk_start_rockets;
cvar_t	*sk_start_grenades;
cvar_t	*sk_start_cells;
cvar_t	*sk_start_slugs;
cvar_t	*sk_start_magslugs;
cvar_t	*sk_start_traps;
cvar_t	*sk_start_rounds;
cvar_t	*sk_start_flechettes;
cvar_t	*sk_start_prox;
cvar_t	*sk_start_tesla;

cvar_t	*sk_start_blaster;
cvar_t	*sk_start_shotgun;
cvar_t	*sk_start_sshotgun;
cvar_t	*sk_start_machinegun;
cvar_t	*sk_start_chaingun;
cvar_t	*sk_start_grenadelauncher;
cvar_t	*sk_start_rocketlauncher;
cvar_t	*sk_start_hyperblaster;
cvar_t	*sk_start_railgun;
cvar_t	*sk_start_bfg;
cvar_t	*sk_start_phalanx;
cvar_t	*sk_start_ionripper;
cvar_t	*sk_start_etfrifle;
cvar_t	*sk_start_disruptor;
cvar_t	*sk_start_proxlauncher;
cvar_t	*sk_start_plasmabeam;
cvar_t	*sk_start_chainfist;
cvar_t	*sk_start_shockwave;
cvar_t	*sk_start_plasmarifle;
cvar_t	*sk_start_rune;

// weapon banning
cvar_t	*sk_no_shotgun;
cvar_t	*sk_no_sshotgun;
cvar_t	*sk_no_machinegun;
cvar_t	*sk_no_chaingun;
cvar_t	*sk_no_grenadelauncher;
cvar_t	*sk_no_rocketlauncher;
cvar_t	*sk_no_hyperblaster;
cvar_t	*sk_no_railgun;
cvar_t	*sk_no_bfg;
cvar_t	*sk_no_ionripper;
cvar_t	*sk_no_phalanx;
cvar_t	*sk_no_traps;
cvar_t	*sk_no_etfrifle;
cvar_t	*sk_no_proxlauncher;
cvar_t	*sk_no_plasmabeam;
cvar_t	*sk_no_disruptor;
cvar_t	*sk_no_chainfist;
cvar_t	*sk_no_plasmarifle;
cvar_t	*sk_no_quad;
cvar_t	*sk_no_pent;

cvar_t	*sk_no_nukes; // ScarFace added
cvar_t	*sk_no_mines;
cvar_t	*sk_no_spheres;

// ScarFace
cvar_t	*sk_armor_bonus_value;	// value of armor shards
cvar_t	*sk_health_bonus_value;	// value of stimpacks
cvar_t	*sk_powerup_max;
cvar_t	*sk_nuke_max;
cvar_t	*sk_doppleganger_max;
cvar_t	*sk_defender_time;
cvar_t	*sk_vengeance_time;
cvar_t	*sk_hunter_time;
cvar_t	*sk_doppleganger_time;
cvar_t	*sk_doppleganger_health;
cvar_t	*sk_quad_time;
cvar_t	*sk_inv_time;
cvar_t	*sk_breather_time;
cvar_t	*sk_enviro_time;
cvar_t	*sk_silencer_shots;
cvar_t	*sk_ir_time;
cvar_t	*sk_double_time;
cvar_t	*sk_quad_fire_time;
// end ScarFace

// maximum values
cvar_t	*sk_max_health;
cvar_t	*sk_max_armor;
cvar_t	*sk_max_bullets;
cvar_t	*sk_max_shells;
cvar_t	*sk_max_rockets;
cvar_t	*sk_max_grenades;
cvar_t	*sk_max_cells;
cvar_t	*sk_max_slugs;
cvar_t	*sk_max_magslugs;
cvar_t	*sk_max_traps;
cvar_t	*sk_max_prox;
cvar_t	*sk_max_tesla;
cvar_t	*sk_max_flechettes;
cvar_t	*sk_max_rounds;

// maximum settings if a player gets a bandolier
cvar_t	*sk_bando_bullets;
cvar_t	*sk_bando_shells;
cvar_t	*sk_bando_cells;
cvar_t	*sk_bando_slugs;
cvar_t	*sk_bando_magslugs;
cvar_t	*sk_bando_flechettes;
cvar_t	*sk_bando_rounds;

// maximum settings if a player gets a pack
cvar_t	*sk_pack_health;
cvar_t	*sk_pack_armor;
cvar_t	*sk_pack_bullets;  // 300
cvar_t	*sk_pack_shells;   // 200
cvar_t	*sk_pack_rockets;  // 100
cvar_t	*sk_pack_grenades; // 100
cvar_t	*sk_pack_cells;    // 300
cvar_t	*sk_pack_slugs;    // 100
cvar_t	*sk_pack_magslugs;
cvar_t	*sk_pack_flechettes;
cvar_t	*sk_pack_rounds;
cvar_t	*sk_pack_prox;
cvar_t	*sk_pack_tesla;
cvar_t	*sk_pack_traps;

cvar_t	*use_safety;         // enables safety spawn period
cvar_t	*safety_time;        // time the safety spawn shield lasts
cvar_t	*use_observer;       // enables clients to use observer/chasecam modes
cvar_t	*use_hitfactor;      // enables aim to be factored into damage inflicted
cvar_t	*fast_respawn;       // sets how much faster items respawn, as more players join
cvar_t	*fast_pbound;        // respawning gets faster up to this many players

cvar_t	*use_lithiumrunes;   // determines whether to use Runes or Techs
cvar_t	*rune_flags;         // determines which runes will show in the game, add these:
//                              1 = resist, 2 = strength, 4 = haste, 8 = regen, 16 = vampire, 32 = ammogen
cvar_t	*rune_spawn;         // chance a rune will spawn from another item respawning
cvar_t	*rune_perplayer;     // sets runes per player that will appear in map
cvar_t	*rune_life;          // seconds a rune will stay around before disappearing
cvar_t	*rune_min;           // sets minimum number of runes to be in the game
cvar_t	*rune_max;           // sets maximum number of runes to be in the game

/// customisable names for runes
cvar_t	*rune_resist_name;
cvar_t	*rune_strength_name;
cvar_t	*rune_regen_name;
cvar_t	*rune_haste_name;
cvar_t	*rune_vampire_name;
cvar_t	*rune_ammogen_name;

cvar_t	*rune_resist_shortname;
cvar_t	*rune_strength_shortname;
cvar_t	*rune_regen_shortname;
cvar_t	*rune_haste_shortname;
cvar_t	*rune_vampire_shortname;
cvar_t	*rune_ammogen_shortname;

// customisable md2's for runes
cvar_t	*rune_resist_mdl;
cvar_t	*rune_strength_mdl;
cvar_t	*rune_regen_mdl;
cvar_t	*rune_haste_mdl;
cvar_t	*rune_vampire_mdl;
cvar_t	*rune_ammogen_mdl;

//customisable icons for runes- ScarFace
cvar_t	*rune_resist_icon;
cvar_t	*rune_strength_icon;
cvar_t	*rune_regen_icon;
cvar_t	*rune_haste_icon;
cvar_t	*rune_vampire_icon;
cvar_t	*rune_ammogen_icon;

// customisable sounds for runes
cvar_t	*rune_resist_sound;
cvar_t	*rune_strength_sound;
cvar_t	*rune_regen_sound;
cvar_t	*rune_haste_sound;
cvar_t	*rune_vampire_sound;
cvar_t	*rune_ammogen_sound;

cvar_t	*rune_haste; // what should I use this for?

cvar_t	*rune_resist;        // sets how much damage is divided by with resist rune
cvar_t	*rune_strength;      // sets how much damage is multiplied by with strength rune
cvar_t	*rune_regen;         // sets how fast health is gained back
cvar_t	*rune_regen_armor;	 // sets whether armor should be regenerated (CTF style) or not (Lithium)
cvar_t	*rune_regen_health_max;      // sets maximum health that can be gained from regen rune
cvar_t	*rune_regen_armor_max;      // sets maximum armor that can be gained from regen rune
cvar_t	*rune_regen_armor_always;      // sets whether armor should be regened regardless of if currently held

cvar_t	*rune_vampire;       // sets percentage of health gained from damage inflicted
cvar_t	*rune_vampiremax;    // sets maximum health that can be gained from vampire rune
//cvar_t	*rune_*_sound;       sets sounds for runes
cvar_t	*use_masskillkick;   // enables kicking anyone who mass kills
cvar_t	*use_chatfloodkick;  // enables kicking anyone who chat floods
cvar_t	*use_nocamp;         // enables no camping check
cvar_t	*banlist;            // sets plain text file to read for IP banning

cvar_t	*sk_hook_offhand;    // enable off-hand hook - otherwise standard CTF grapple
cvar_t	*sk_new_hand_hook;   // enable the new hand-grapple sounds
cvar_t	*sk_hook_speed;      // sets how fast the hook shoots out
cvar_t	*sk_hook_pullspeed;  // sets how fast the hook pulls a player
cvar_t	*sk_hook_sky;        // enables hooking to the sky
cvar_t	*sk_hook_maxtime;    // sets max time you can stay hooked
cvar_t	*sk_hook_damage;     // sets damage hook does to other players
cvar_t	*sk_hook_style;

cvar_t	*camp_threshold;     // sets movement required to not be camping
cvar_t	*camp_time;          // seconds a player has before getting a camping warning
cvar_t	*camp_warn;          // seconds a player has after warned, or be killed
cvar_t	*ping_watch;         // seconds until a clients ping (average so far) is checked
cvar_t	*ping_min;           // mi nimum ping allowed
cvar_t	*ping_max;           // maximum ping allowed
//cvar_t	*no_*               inhibit (disable) items, only takes effect after map change
cvar_t	*admin_code;         // sets the admin code, 0 to disable (max 5 digits)
cvar_t	*use_packs;          // enables packs
cvar_t	*use_gslog;          // enables GSLog frag logging (Gibstats Standard Log)
cvar_t	*gslog;              // sets GSLog output file
cvar_t	*gslog_flush;        // enables instant logging
cvar_t	*sk_knockback_adjust;   // knockback adjustment (impact knockbacks multiplied by this)
cvar_t	*sk_knockback_self;     // knockback adjustment for self inflicted impacts
//                                  (use this to modify rocket jumping height)
cvar_t	*sk_pack_spawn;		// chance a pack will spawn from another item respawning
cvar_t	*sk_pack_life;		// seconds a pack will stay around before disappearing

cvar_t	*ctffraglimit;

cvar_t	*lmctf;

cvar_t	*rune_spawn_dmstart;

cvar_t	*use_onegun;

// Bot weapon balancing cvars
cvar_t	*sk_bot_blaster_damage;
cvar_t	*sk_bot_blaster_speed;
cvar_t	*sk_bot_shotgun_damage;
cvar_t	*sk_bot_shotgun_count;
cvar_t	*sk_bot_shotgun_hspread;
cvar_t	*sk_bot_shotgun_vspread;
cvar_t	*sk_bot_sshotgun_damage;
cvar_t	*sk_bot_sshotgun_count;
cvar_t	*sk_bot_sshotgun_hspread;
cvar_t	*sk_bot_sshotgun_vspread;
cvar_t	*sk_bot_machinegun_damage;
cvar_t	*sk_bot_machinegun_hspread;
cvar_t	*sk_bot_machinegun_vspread;
cvar_t	*sk_bot_chaingun_damage;
cvar_t	*sk_bot_chaingun_hspread;
cvar_t	*sk_bot_chaingun_vspread;
cvar_t	*sk_bot_hyperblaster_damage;
cvar_t	*sk_bot_hyperblaster_speed;
cvar_t	*sk_bot_rocket_damage;
cvar_t	*sk_bot_rocket_damage2;
cvar_t	*sk_bot_rocket_rdamage;
cvar_t	*sk_bot_rocket_radius;
cvar_t	*sk_bot_rocket_speed;
cvar_t	*sk_bot_grenade_damage;
cvar_t	*sk_bot_grenade_radius;
cvar_t	*sk_bot_grenade_speed;
cvar_t	*sk_bot_railgun_damage;
cvar_t	*sk_bot_bfg_damage;
cvar_t	*sk_bot_bfg_radius;
cvar_t	*sk_bot_bfg_speed;

cvar_t	*use_replacelist;
cvar_t	*replace_file;

cvar_t	*use_lasers;

cvar_t	*lava_mod;
cvar_t	*slime_mod;
cvar_t	*bot_lava_mod;
cvar_t	*bot_slime_mod;
cvar_t	*bot_aimangle;
cvar_t	*bot_railtime;

cvar_t	*allow_flagdrop;
cvar_t	*allow_pickuprune;
cvar_t	*allow_runedrop;

cvar_t	*config_file;
cvar_t	*use_configlist;

cvar_t *zk_pinglimit;
cvar_t *zk_logonly;
cvar_t *zk_retry;

cvar_t	*ttctf;

cvar_t	*use_iplogging;

cvar_t	*showbotping;

cvar_t	*use_rogueplats;

cvar_t	*grapple_mdl;
cvar_t	*grapple_spin;

//cvar_t	*strong_mines;

cvar_t	*invisible_dmstarts;


void lithium_defaults (void)
{
	use_runes = gi.cvar("use_runes", "1", 0);
	use_hook = gi.cvar("use_hook", "1", 0);
	use_vwep = gi.cvar("use_vwep", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	mapqueue = gi.cvar("mapqueue", "maps.lst", 0);
	map_random = gi.cvar("map_random", "1", 0);         // overide ridah's one?

	motd = gi.cvar("motd", "motd.txt", 0);
	def_bestweapon = gi.cvar("def_bestweapon", "0", 0);
	def_id = gi.cvar("def_id", "1", 0);
	def_hud = gi.cvar("def_hud", "2", 0);
	def_scores = gi.cvar("def_scores", "1", 0);
	mega_gibs = gi.cvar("mega_gibs", "0", 0);

	player_gib_health = gi.cvar("player_gib_health", "-40", 0);
	showbotping = gi.cvar("showbotping", "0", 0);

	intermission_time = gi.cvar("intermission_time", "8.5", 0);
	intermission_sound = gi.cvar("intermission_sound", "world/xian1.wav", 0);

	sk_fall_damagemod = gi.cvar("sk_fall_damagemod", "1.0", 0);
	sk_jacket_armor = gi.cvar("sk_jacket_armor", "50 100 .30 .00", 0);
	sk_combat_armor = gi.cvar("sk_combat_armor", "75 150 .60 .30", 0);
	sk_body_armor = gi.cvar("sk_body_armor", "100 200 .80 .60", 0);
	sk_shard_armor = gi.cvar("sk_shard_armor", "5", 0);
	sk_power_armor_screen = gi.cvar("sk_power_armor_screen", "0.25", 0);
	sk_power_armor_shield = gi.cvar("sk_power_armor_shield", "0.50", 0);
	sk_dropweapammo = gi.cvar("sk_dropweapammo", "0.5", 0);

	allow_func_explosives = gi.cvar("allow_func_explosives", "0", CVAR_LATCH);
	allow_disabled_func_explosives = gi.cvar("allow_disabled_func_explosives", "0", CVAR_LATCH);

	use_safety = gi.cvar("use_safety", "1", 0);
	safety_time = gi.cvar("safety_time", "3.0", 0);
	use_observer = gi.cvar("use_observer", "1", 0);
	use_hitfactor = gi.cvar("use_hitfactor", "0", 0);
	fast_respawn = gi.cvar("fast_respawn", "0.75", 0);
	fast_pbound = gi.cvar("rast_pbound", "16", 0);

	rune_spawn_dmstart = gi.cvar("rune_spawn_dmstart", "0", 0); // new

	rune_flags = gi.cvar("rune_flags", "31", 0);
	rune_spawn = gi.cvar("rune_spawn", "0.10", 0);
	rune_perplayer = gi.cvar("rune_perplayer", "0.7", 0);
	rune_life = gi.cvar("rune_life", "20", 0);
	rune_min = gi.cvar("rune_min", "2", 0);
	rune_max = gi.cvar("rune_max", "10", 0);

	rune_resist = gi.cvar("rune_resist", "2.0", 0);
	rune_resist_sound = gi.cvar("rune_resist_sound", "world/force2.wav", 0);
	rune_resist_mdl = gi.cvar("rune_resist_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_resist_icon = gi.cvar("rune_resist_icon", "k_pyramid", 0);
	rune_resist_name = gi.cvar("rune_resist_name", "resist rune", 0);
	rune_resist_shortname = gi.cvar("rune_resist_shortname", "Resist", 0);

	rune_strength = gi.cvar("rune_strength", "2.0", 0);
	rune_strength_sound = gi.cvar("rune_strength_sound", "items/damage3.wav", 0);
	rune_strength_mdl = gi.cvar("rune_strength_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_strength_icon = gi.cvar("rune_strength_icon", "k_pyramid", 0);
	rune_strength_name = gi.cvar("rune_strength_name", "strength rune", 0);
	rune_strength_shortname = gi.cvar("rune_strength_shortname", "Strength", 0);

	rune_haste = gi.cvar("rune_haste", "", 0);
	rune_haste_sound = gi.cvar("rune_haste_sound", "items/s_health.wav", 0);
	rune_haste_mdl = gi.cvar("rune_haste_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_haste_icon = gi.cvar("rune_haste_icon", "k_pyramid", 0);
	rune_haste_name = gi.cvar("rune_haste_name", "Time Accel", 0);
	rune_haste_shortname = gi.cvar("rune_haste_shortname", "Haste", 0);

	rune_regen = gi.cvar("rune_regen", "0.25", 0);
	rune_regen_armor = gi.cvar("rune_regen_armor", "1", 0);
	rune_regen_health_max = gi.cvar("rune_regen_health_max", "150", 0);
	rune_regen_armor_max = gi.cvar("rune_regen_armor_max", "150", 0);
	rune_regen_sound = gi.cvar("rune_regen_sound", "items/s_health.wav", 0);
	rune_regen_mdl = gi.cvar("rune_regen_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_regen_icon = gi.cvar("rune_regen_icon", "k_pyramid", 0);
	rune_regen_name = gi.cvar("rune_regen_name", "regen rune", 0);
	rune_regen_shortname = gi.cvar("rune_regen_shortname", "Regen", 0);
	rune_regen_armor_always = gi.cvar("rune_regen_armor_always", "0", 0);

	rune_vampire = gi.cvar("rune_vampire", "0.5", 0);
	rune_vampiremax = gi.cvar("rune_vampiremax", "200", 0);
	rune_vampire_sound = gi.cvar("rune_vampire_sound", "makron/pain2.wav", 0);
	rune_vampire_mdl = gi.cvar("rune_vampire_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_vampire_icon = gi.cvar("rune_vampire_icon", "k_pyramid", 0);
	rune_vampire_name = gi.cvar("rune_vampire_name", "vampire rune", 0);
	rune_vampire_shortname = gi.cvar("rune_vampire_shortname", "Vampire", 0);

	rune_ammogen_sound = gi.cvar("rune_ammogen_sound", "misc/spawn1.wav", 0);
	rune_ammogen_mdl = gi.cvar("rune_ammogen_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_ammogen_icon = gi.cvar("rune_ammogen_icon", "k_pyramid", 0);
	rune_ammogen_name = gi.cvar("rune_ammogen_name", "ammogen rune", 0);
	rune_ammogen_shortname = gi.cvar("rune_ammogen_shortname", "AmmoGen", 0);

	use_masskillkick = gi.cvar("use_masskillkick", "1", 0);
	use_chatfloodkick = gi.cvar("use_chatfloodkick", "1", 0);
	use_nocamp = gi.cvar("use_nocamp", "0", 0);
	banlist = gi.cvar("banlist", "banlist.txt", 0);

	sk_hook_offhand = gi.cvar("sk_hook_offhand", "1", 0);
	sk_new_hand_hook = gi.cvar("sk_new_hand_hook", "0", 0);
	sk_hook_speed = gi.cvar("sk_hook_speed", "900", 0);
	sk_hook_pullspeed = gi.cvar("sk_hook_pullspeed", "700", 0);
	sk_hook_sky = gi.cvar("sk_hook_sky", "0", 0);
	sk_hook_maxtime = gi.cvar("sk_hook_maxtime", "5", 0);
	sk_hook_damage = gi.cvar("sk_hook_damage", "2", 0);
	sk_hook_style =  gi.cvar("sk_hook_style", "1", 0);

	camp_threshold = gi.cvar("camp_threshold", "250", 0);
	camp_time = gi.cvar("camp_time", "8.0", 0);
	camp_warn = gi.cvar("camp_warn", "5.0", 0);

	ping_watch = gi.cvar("ping_watch", "10", 0);
	ping_min = gi.cvar("ping_min", "0", 0);
	ping_max = gi.cvar("ping_max", "999", 0);

	admin_code = gi.cvar("admin_code", "0", 0);
	use_packs = gi.cvar("use_packs", "1", 0);

	use_gslog = gi.cvar("use_gslog", "0", 0);
	gslog = gi.cvar("gslog", "gslog.log", 0);
	gslog_flush = gi.cvar("gslog_flush", "0", 0);

	sk_knockback_adjust = gi.cvar("sk_knockback_adjust", "1.0", 0);
	sk_knockback_self = gi.cvar("sk_knockback_self", "3.2", 0);

// ScarFace
	sk_armor_bonus_value = gi.cvar("sk_armor_bonus_value", "2", 0);
	sk_health_bonus_value = gi.cvar("sk_health_bonus_value", "2", 0);
	sk_powerup_max = gi.cvar("sk_powerup_max", "2", 0);
	sk_doppleganger_max = gi.cvar("sk_doppleganger_max", "1", 0);
	sk_nuke_max = gi.cvar("sk_nuke_max", "1", 0);
	sk_defender_time = gi.cvar("sk_defender_time", "60", 0);
	sk_vengeance_time = gi.cvar("sk_vengeance_time", "60", 0);
	sk_hunter_time = gi.cvar("sk_hunter_time", "60", 0);
	sk_doppleganger_time = gi.cvar("sk_doppleganger_time", "30", 0);
	sk_doppleganger_health = gi.cvar("sk_doppleganger_health", "30", 0);
	sk_quad_time = gi.cvar("sk_quad_time", "30", 0);
	sk_inv_time = gi.cvar("sk_inv_time", "30", 0);
	sk_breather_time = gi.cvar("sk_breather_time", "30", 0);
	sk_enviro_time = gi.cvar("sk_enviro_time", "30", 0);
	sk_silencer_shots = gi.cvar("sk_silencer_shots", "30", 0);
	sk_ir_time = gi.cvar("sk_ir_time", "30", 0);
	sk_double_time = gi.cvar("sk_double_time", "30", 0);
	sk_quad_fire_time = gi.cvar("sk_quad_fire_time", "30", 0);
// end ScarFace
	sk_pack_health = gi.cvar("sk_pack_health", "120", 0);
	sk_pack_armor = gi.cvar("sk_pack_armor", "250", 0);
	sk_pack_bullets = gi.cvar("sk_pack_bullets", "360", 0);
	sk_pack_shells = gi.cvar("sk_pack_shells", "200", 0);
	sk_pack_rockets = gi.cvar("sk_pack_rockets", "100", 0);
	sk_pack_grenades = gi.cvar("sk_pack_grenades", "100", 0);
	sk_pack_cells = gi.cvar("sk_pack_cells", "360", 0);
	sk_pack_slugs = gi.cvar("sk_pack_slugs", "100", 0);
	sk_pack_magslugs = gi.cvar("sk_pack_magslugs", "100", 0);
	sk_pack_flechettes = gi.cvar("sk_pack_flechettes", "360", 0);
	sk_pack_rounds = gi.cvar("sk_pack_rounds", "300", 0);
	sk_pack_prox = gi.cvar("sk_pack_prox", "100", 0);
	sk_pack_tesla = gi.cvar("sk_pack_tesla", "100", 0);
	sk_pack_traps = gi.cvar("sk_pack_traps", "50", 0);

	sk_pack_spawn = gi.cvar("sk_pack_spawn", "0.03", 0);
	sk_pack_life = gi.cvar("sk_pack_life", "25", 0);

	sk_bando_bullets = gi.cvar("sk_bando_bullets", "280", 0);
	sk_bando_shells = gi.cvar("sk_bando_shells", "150", 0);
	sk_bando_cells = gi.cvar("sk_bando_cells", "280", 0);
	sk_bando_slugs = gi.cvar("sk_bando_slugs", "75", 0);
	sk_bando_magslugs = gi.cvar("sk_bando_magslugs", "75", 0);
	sk_bando_flechettes = gi.cvar("sk_bando_flechettes", "280", 0);
	sk_bando_rounds = gi.cvar("sk_bando_rounds", "225", 0);

	sk_max_health = gi.cvar("sk_max_health", "120", 0);
	sk_max_armor = gi.cvar("sk_max_armor", "200", 0);
	sk_max_bullets = gi.cvar("sk_max_bullets", "200", 0);
	sk_max_shells = gi.cvar("sk_max_shells", "100", 0);
	sk_max_rockets = gi.cvar("sk_max_rockets", "50", 0);
	sk_max_grenades = gi.cvar("sk_max_grenades", "50", 0);
	sk_max_cells = gi.cvar("sk_max_cells", "200", 0);
	sk_max_slugs = gi.cvar("sk_max_slugs", "50", 0);
	sk_max_magslugs = gi.cvar("sk_max_magslugs", "50", 0);
	sk_max_traps = gi.cvar("sk_max_traps", "20", 0);
	sk_max_prox = gi.cvar("sk_max_prox", "50", 0);
	sk_max_tesla = gi.cvar("sk_max_tesla", "50", 0);
	sk_max_flechettes = gi.cvar("sk_max_flechettes", "200", 0);
	sk_max_rounds = gi.cvar("sk_max_rounds", "150", 0);

	sk_start_weapon = gi.cvar("sk_start_weapon", "0", 0);

	sk_start_health = gi.cvar("sk_start_health", "100", 0);
	sk_start_armor = gi.cvar("sk_start_armor", "0", 0);
	sk_start_bullets = gi.cvar("sk_start_bullets", "50", 0);
	sk_start_shells = gi.cvar("sk_start_shells", "10", 0);
	sk_start_rockets = gi.cvar("sk_start_rockets", "5", 0);
	sk_start_grenades = gi.cvar("sk_start_grenades", "1", 0);
	sk_start_cells = gi.cvar("sk_start_cells", "50", 0);
	sk_start_slugs = gi.cvar("sk_start_slugs", "5", 0);
	sk_start_magslugs = gi.cvar("sk_start_magslugs", "10", 0);
	sk_start_traps = gi.cvar("sk_start_traps", "0", 0);
	sk_start_rounds = gi.cvar("sk_start_rounds", "5", 0);
	sk_start_flechettes = gi.cvar("sk_start_flechettes", "25", 0);
	sk_start_prox = gi.cvar("sk_start_prox", "0", 0);
	sk_start_tesla = gi.cvar("sk_start_tesla", "0", 0);

	sk_start_blaster = gi.cvar("sk_start_blaster", "1", 0);
	sk_start_shotgun = gi.cvar("sk_start_shotgun", "0", 0);
	sk_start_sshotgun = gi.cvar("sk_start_sshotgun", "0", 0);
	sk_start_machinegun = gi.cvar("sk_start_machinegun", "1", 0);
	sk_start_chaingun = gi.cvar("sk_start_chaingun", "0", 0);
	sk_start_grenadelauncher = gi.cvar("sk_start_grenadelauncher", "0", 0);
	sk_start_rocketlauncher = gi.cvar("sk_start_rocketlauncher", "0", 0);
	sk_start_hyperblaster = gi.cvar("sk_start_hyperblaster", "0", 0);
	sk_start_railgun = gi.cvar("sk_start_railgun", "0", 0);
	sk_start_bfg = gi.cvar("sk_start_bfg", "0", 0);
	sk_start_phalanx = gi.cvar("sk_start_phalanx", "0", 0);
	sk_start_ionripper = gi.cvar("sk_start_ionripper", "0", 0);
	sk_start_etfrifle = gi.cvar("sk_start_etfrifle", "0", 0);
	sk_start_disruptor = gi.cvar("sk_start_disruptor", "0", 0);
	sk_start_proxlauncher = gi.cvar("sk_start_proxlauncher", "0", 0);
	sk_start_plasmabeam = gi.cvar("sk_start_plasmabeam", "0", 0);
	sk_start_chainfist = gi.cvar("sk_start_chainfist", "0", 0);
	sk_start_shockwave = gi.cvar("sk_start_shockwave", "0", 0);
	sk_start_plasmarifle = gi.cvar("sk_start_plasmarifle", "0", 0);
	sk_start_rune = gi.cvar("sk_start_rune", "0", 0);

	sk_no_shotgun = gi.cvar("sk_no_shotgun", "0", 0);
	sk_no_sshotgun = gi.cvar("sk_no_sshotgun", "0", 0);
	sk_no_machinegun = gi.cvar("sk_no_machinegun", "0", 0);
	sk_no_chaingun = gi.cvar("sk_no_chaingun", "0", 0);
	sk_no_grenadelauncher = gi.cvar("sk_no_grenadelauncher", "0", 0);
	sk_no_rocketlauncher = gi.cvar("sk_no_rocketlauncher", "0", 0);
	sk_no_hyperblaster = gi.cvar("sk_no_hyperblaster", "0", 0);
	sk_no_railgun = gi.cvar("sk_no_railgun", "0", 0);
	sk_no_bfg = gi.cvar("sk_no_bfg", "1", 0);
	sk_no_ionripper = gi.cvar("sk_no_ionripper", "0", 0);
	sk_no_phalanx = gi.cvar("sk_no_phalanx", "0", 0);
	sk_no_traps = gi.cvar("sk_no_traps", "0", 0);
	sk_no_etfrifle = gi.cvar("sk_no_etfrifle", "0", 0);
	sk_no_proxlauncher = gi.cvar("sk_no_proxlauncher", "0", 0);
	sk_no_plasmabeam = gi.cvar("sk_no_plasmabeam", "0", 0);
	sk_no_disruptor = gi.cvar("sk_no_disruptor", "0", 0);
	sk_no_chainfist = gi.cvar("sk_no_chainfist", "0", 0);
	sk_no_plasmarifle = gi.cvar("sk_no_plasmarifle", "0", 0);

	sk_no_quad = gi.cvar("sk_no_quad", "0", 0);
	sk_no_pent = gi.cvar("sk_no_pent", "1", 0);

	sk_no_nukes = gi.cvar("sk_no_nukes", "0", 0);
	sk_no_mines = gi.cvar("sk_no_mines", "0", 0);
	sk_no_spheres = gi.cvar("sk_no_spheres", "0", 0);

	use_lithiumrunes = gi.cvar("use_lithiumrunes", "1", 0);

	ctffraglimit = gi.cvar("ctffraglimit", "0", 0);

	lmctf = gi.cvar("lmctf", "0", 0);

	ctf_blastercolors = gi.cvar("ctf_blastercolors", "0", 0);
	ctf_railcolors = gi.cvar("ctf_railcolors", "0", 0);

	sk_blaster_damage = gi.cvar("sk_blaster_damage", "17", 0);
	sk_blaster_speed = gi.cvar("sk_blaster_speed", "1000", 0);
	sk_blaster_color = gi.cvar("sk_blaster_color", "1", 0);

	sk_shotgun_damage = gi.cvar("sk_shotgun_damage", "6", 0);
	sk_shotgun_count = gi.cvar("sk_shotgun_count", "12", 0);
	sk_shotgun_hspread = gi.cvar("sk_shotgun_hspread", "500", 0);
	sk_shotgun_vspread = gi.cvar("sk_shotgun_vspread", "500", 0);

	sk_sshotgun_damage = gi.cvar("sk_sshotgun_damage", "6", 0);
	sk_sshotgun_count = gi.cvar("sk_sshotgun_count", "22", 0);
	sk_sshotgun_hspread = gi.cvar("sk_sshotgun_hspread", "1000", 0);
	sk_sshotgun_vspread = gi.cvar("sk_sshotgun_vspread", "500", 0);

	sk_machinegun_damage = gi.cvar("sk_machinegun_damage", "8", 0);
	sk_machinegun_hspread = gi.cvar("sk_machinegun_hspread", "300", 0);
	sk_machinegun_vspread = gi.cvar("sk_machinegun_vspread", "500", 0);

	sk_chaingun_damage = gi.cvar("sk_chaingun_damage", "8", 0);
	sk_chaingun_hspread = gi.cvar("sk_chaingun_hspread", "300", 0);
	sk_chaingun_vspread = gi.cvar("sk_chaingun_vspread", "500", 0);

	sk_grenade_damage = gi.cvar("sk_grenade_damage", "100", 0);
	sk_grenade_radius = gi.cvar("sk_grenade_radius", "140", 0);
	sk_grenade_speed = gi.cvar("sk_grenade_speed", "600", 0);

	sk_rocket_damage = gi.cvar("sk_rocket_damage", "100", 0);
	sk_rocket_damage2 = gi.cvar("sk_rocket_damage2", "20", 0);
	sk_rocket_rdamage = gi.cvar("sk_rocket_rdamage", "120", 0);
	sk_rocket_radius = gi.cvar("sk_rocket_radius", "140", 0);
	sk_rocket_speed = gi.cvar("sk_rocket_speed", "650", 0);

	sk_hyperblaster_damage = gi.cvar("sk_hyperblaster_damage", "17", 0);
	sk_hyperblaster_speed = gi.cvar("sk_hyperblaster_speed", "1000", 0);
	sk_hyperblaster_color = gi.cvar("sk_hyperblaster_color", "1", 0);

	sk_railgun_damage = gi.cvar("sk_railgun_damage", "120", 0);
	sk_railgun_skin = gi.cvar("sk_railgun_skin", "0", 0);

	sk_bfg_damage = gi.cvar("sk_bfg_damage", "200", 0);
	sk_bfg_radius = gi.cvar("sk_bfg_radius", "1000", 0);
	sk_bfg_speed = gi.cvar("sk_bfg_speed", "600", 0);

// ScarFace
	sk_ionripper_damage = gi.cvar("sk_ionripper_damage", "30", 0);
	sk_ionripper_speed = gi.cvar("sk_ionripper_speed", "500", 0);
	sk_ionripper_extra_sounds = gi.cvar("sk_ionripper_extra_sounds", "0", 0);

	sk_phalanx_damage = gi.cvar("sk_phalanx_damage", "70", 0);
	sk_phalanx_damage2 = gi.cvar("sk_phalanx_damage2", "10", 0);
	sk_phalanx_radius_damage = gi.cvar("sk_phalanx_radius_damage", "120", 0);
	sk_phalanx_radius = gi.cvar("sk_phalanx_radius", "120", 0);
	sk_phalanx_speed = gi.cvar("sk_phalanx_speed", "725", 0);

	sk_trap_life = gi.cvar("sk_trap_life", "30", 0);
	sk_trap_health = gi.cvar("sk_trap_health", "30", 0);

	sk_etf_rifle_damage = gi.cvar("sk_etf_rifle_damage", "10", 0);
	sk_etf_rifle_radius_damage = gi.cvar("sk_etf_rifle_radius_damage", "10", 0);
	sk_etf_rifle_radius = gi.cvar("sk_etf_rifle_radius", "100", 0);
	sk_etf_rifle_speed = gi.cvar("sk_etf_rifle_speed", "750", 0);

	sk_prox_damage = gi.cvar("sk_prox_damage", "90", 0);
	sk_prox_radius = gi.cvar("sk_prox_radius", "192", 0);
	sk_prox_speed = gi.cvar("sk_prox_speed", "600", 0);
	sk_prox_life = gi.cvar("sk_prox_life", "60", 0);
	sk_prox_health = gi.cvar("sk_prox_health", "20", 0);

	sk_plasmabeam_damage = gi.cvar("sk_plasmabeam_damage", "15", 0);

	sk_disruptor_damage = gi.cvar("sk_disruptor_damage", "30", 0);
	sk_disruptor_speed = gi.cvar("sk_disruptor_speed", "1000", 0);

	sk_chainfist_damage = gi.cvar("sk_chainfist_damage", "30", 0);

	sk_tesla_damage = gi.cvar("sk_tesla_damage", "3", 0);
	sk_tesla_radius = gi.cvar("sk_tesla_radius", "128", 0);
	sk_tesla_life = gi.cvar("sk_tesla_life", "30", 0);
	sk_tesla_health = gi.cvar("sk_tesla_health", "20", 0);

	sk_nuke_delay = gi.cvar("sk_nuke_delay", "4", 0);
	sk_nuke_life = gi.cvar("sk_nuke_life", "6", 0);
	sk_nuke_radius = gi.cvar("sk_nuke_radius", "512", 0);
	sk_defender_blaster_damage = gi.cvar("sk_defender_blaster_damage", "10", 0);
	sk_defender_blaster_speed = gi.cvar("sk_defender_blaster_speed", "1000", 0);
	sk_vengeance_health_threshold = gi.cvar("sk_vengeance_health_threshold", "25", 0);
// end ScarFace

	sk_plasma_rifle_damage_bounce = gi.cvar("sk_plasma_rifle_damage_bounce", "60", 0);
	sk_plasma_rifle_damage_bounce_dm = gi.cvar("sk_plasma_rifle_damage_bounce_dm", "39", 0);
	sk_plasma_rifle_damage_spread = gi.cvar("sk_plasma_rifle_damage_spread", "43", 0);
	sk_plasma_rifle_damage_spread_dm = gi.cvar("sk_plasma_rifle_damage_spread_dm", "28", 0);
	sk_plasma_rifle_speed_bounce = gi.cvar("sk_plasma_rifle_speed_bounce", "1200", 0);
	sk_plasma_rifle_speed_spread = gi.cvar("sk_plasma_rifle_speed_spread", "1200", 0);
	sk_plasma_rifle_radius = gi.cvar("sk_plasma_rifle_radius", "70", 0);
	sk_plasma_rifle_life_bounce = gi.cvar("sk_plasma_rifle_life_bounce", "3", 0);
	sk_plasma_rifle_life_spread = gi.cvar("sk_plasma_rifle_life_spread", "3", 0);

	sk_shockwave_bounces = gi.cvar("sk_shockwave_bounces", "6", 0);
	sk_shockwave_damage = gi.cvar("sk_shockwave_damage", "80", 0);
	sk_shockwave_damage2 = gi.cvar("sk_shockwave_damage2", "40", 0);
	sk_shockwave_rdamage = gi.cvar("sk_shockwave_rdamage", "120", 0);
	sk_shockwave_radius = gi.cvar("sk_shockwave_radius", "300", 0);
	sk_shockwave_speed = gi.cvar("sk_shockwave_speed", "650", 0);
	sk_shockwave_effect_damage = gi.cvar("sk_shockwave_effect_damage", "100", 0);
	sk_shockwave_effect_radius = gi.cvar("sk_shockwave_effect_radius", "340", 0);

	sk_bot_blaster_damage = gi.cvar("sk_bot_blaster_damage", "17", 0);
	sk_bot_blaster_speed = gi.cvar("sk_bot_blaster_speed", "1000", 0);
	sk_bot_shotgun_damage = gi.cvar("sk_bot_shotgun_damage", "6", 0);
	sk_bot_shotgun_count = gi.cvar("sk_bot_shotgun_count", "12", 0);
	sk_bot_shotgun_hspread = gi.cvar("sk_bot_shotgun_hspread", "500", 0);
	sk_bot_shotgun_vspread = gi.cvar("sk_bot_shotgun_vspread", "500", 0);
	sk_bot_sshotgun_damage = gi.cvar("sk_bot_sshotgun_damage", "6", 0);
	sk_bot_sshotgun_count = gi.cvar("sk_bot_sshotgun_count", "22", 0);
	sk_bot_sshotgun_hspread = gi.cvar("sk_bot_sshotgun_hspread", "1000", 0);
	sk_bot_sshotgun_vspread = gi.cvar("sk_bot_sshotgun_vspread", "500", 0);
	sk_bot_machinegun_damage = gi.cvar("sk_bot_machinegun_damage", "8", 0);
	sk_bot_machinegun_hspread = gi.cvar("sk_bot_machinegun_hspread", "300", 0);
	sk_bot_machinegun_vspread = gi.cvar("sk_bot_machinegun_vspread", "500", 0);
	sk_bot_chaingun_damage = gi.cvar("sk_bot_chaingun_damage", "8", 0);
	sk_bot_chaingun_hspread = gi.cvar("sk_bot_chaingun_hspread", "300", 0);
	sk_bot_chaingun_vspread = gi.cvar("sk_bot_chaingun_vspread", "500", 0);
	sk_bot_hyperblaster_damage = gi.cvar("sk_bot_hyperblaster_damage", "17", 0);
	sk_bot_hyperblaster_speed = gi.cvar("sk_bot_hyperblaster_speed", "1000", 0);
	sk_bot_rocket_damage = gi.cvar("sk_bot_rocket_damage", "100", 0);
	sk_bot_rocket_damage2 = gi.cvar("sk_bot_rocket_damage2", "20", 0);
	sk_bot_rocket_rdamage = gi.cvar("sk_bot_rocket_rdamage", "120", 0);
	sk_bot_rocket_radius = gi.cvar("sk_bot_rocket_radius", "140", 0);
	sk_bot_rocket_speed = gi.cvar("sk_bot_rocket_speed", "650", 0);
	sk_bot_grenade_damage = gi.cvar("sk_bot_grenade_damage", "100", 0);
	sk_bot_grenade_radius = gi.cvar("sk_bot_grenade_radius", "140", 0);
	sk_bot_grenade_speed = gi.cvar("sk_bot_grenade_speed", "600", 0);
	sk_bot_railgun_damage = gi.cvar("sk_bot_railgun_damage", "120", 0);
	sk_bot_bfg_damage = gi.cvar("sk_bot_bfg_damage", "200", 0);
	sk_bot_bfg_radius = gi.cvar("sk_bot_bfg_radius", "1000", 0);
	sk_bot_bfg_speed = gi.cvar("sk_bot_bfg_speed", "400", 0);

	use_replacelist = gi.cvar("use_replacelist", "1", 0);
	replace_file = gi.cvar("replace_file", "replace.lst", 0);

	use_onegun = gi.cvar("use_onegun", "0", 0);

	use_lasers = gi.cvar("use_lasers", "1", 0);
	bot_lava_mod = gi.cvar("bot_lava_mod", "1.0", 0);
	bot_slime_mod = gi.cvar("bot_slime_mod", "1.0", 0);

	lava_mod = gi.cvar("lava_mod", "1.0", 0);
	slime_mod = gi.cvar("slime_mod", "1.0", 0);

	bot_aimangle = gi.cvar("bot_aimangle", "15", 0);
	bot_railtime = gi.cvar("bot_railtime", "0.2", 0);

	allow_flagdrop = gi.cvar("allow_flagdrop", "0", 0);
	allow_runedrop = gi.cvar("allow_runedrop", "1", 0);
	allow_pickuprune = gi.cvar("allow_pickuprune", "1", 0);

	config_file = gi.cvar("config_file", "config.lst", CVAR_LATCH);
	use_configlist = gi.cvar("use_configlist", "0", 0);

	zk_pinglimit = gi.cvar("zk_pinglimit", "600", CVAR_ARCHIVE);
	zk_logonly = gi.cvar("zk_logonly", "0", CVAR_ARCHIVE);
	zk_retry = gi.cvar("zk_retry", "0", CVAR_ARCHIVE);

	ttctf = gi.cvar("ttctf", "0", CVAR_LATCH);
	use_iplogging = gi.cvar("use_iplogging", "0", 0);

	use_rogueplats = gi.cvar("userogueplats", "1", 0);

	grapple_mdl = gi.cvar("grapple_mdl", "models/objects/grenade2/tris.md2", 0);
	grapple_spin = gi.cvar("grapple_spin", "0", 0);

// rogue
//	strong_mines = gi.cvar ("strong_mines", "0", 0);

	invisible_dmstarts = gi.cvar("invisible_dmstarts","0", CVAR_LATCH);
}
