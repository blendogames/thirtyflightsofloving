
#include "./g_local.h"
#include "pak.h"

/*typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;*/


void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_coop (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_bobbingwater(edict_t *self); //Knightmare added
void SP_func_train (edict_t *ent);
void SP_func_conveyor (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_clock (edict_t *ent);
void SP_func_killbox (edict_t *ent);

void SP_trigger_always (edict_t *ent);
void SP_trigger_once (edict_t *ent);
void SP_trigger_multiple (edict_t *ent);
void SP_trigger_bbox (edict_t *self);
void SP_trigger_relay (edict_t *ent);
void SP_trigger_push (edict_t *ent);
void SP_trigger_push_bbox (edict_t *ent);

void SP_trigger_hurt (edict_t *ent);
void SP_trigger_hurt_bbox (edict_t *ent);
void SP_trigger_key (edict_t *ent);
void SP_trigger_counter (edict_t *ent);
void SP_trigger_elevator (edict_t *ent);
void SP_trigger_gravity (edict_t *ent);
void SP_trigger_gravity_bbox (edict_t *ent);
void SP_trigger_monsterjump (edict_t *ent);
void SP_trigger_monsterjump_bbox (edict_t *ent);

void SP_target_temp_entity (edict_t *ent);
void SP_target_speaker (edict_t *ent);
void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_secret (edict_t *ent);
void SP_target_goal (edict_t *ent);
void SP_target_splash (edict_t *ent);
void SP_target_spawner (edict_t *ent);
void SP_target_blaster (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_laser (edict_t *self);
void SP_target_help (edict_t *ent);
void SP_target_actor (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);
void SP_target_character (edict_t *ent);
void SP_target_string (edict_t *ent);
//Knightmare
void SP_target_monitor (edict_t *ent);
void SP_target_monsterbattle (edict_t *self);
void SP_target_movewith (edict_t *self);
void SP_target_change (edict_t *self);
void SP_target_rotation (edict_t *self);
void SP_target_attractor (edict_t *self);
void SP_target_cd (edict_t *self);
void SP_target_skill (edict_t *self);
void SP_target_sky (edict_t *self);
void SP_target_playback (edict_t *self);
void SP_target_rocks (edict_t *self);
void SP_target_clone (edict_t *self);
void SP_target_text (edict_t *self);
void SP_thing (edict_t *self);

void SP_worldspawn (edict_t *ent);
void SP_viewthing (edict_t *ent);

void SP_light (edict_t *self);
void SP_light_mine1 (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);

void SP_misc_explobox (edict_t *self);
void SP_misc_banner (edict_t *self);
void SP_misc_satellite_dish (edict_t *self);
void SP_misc_actor (edict_t *self);
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
void SP_misc_insane (edict_t *self);
void SP_misc_deadsoldier (edict_t *self);
void SP_misc_viper (edict_t *self);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_bigviper (edict_t *self);
void SP_misc_strogg_ship (edict_t *self);
void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void SP_misc_blackhole (edict_t *self);
void SP_misc_eastertank (edict_t *self);
void SP_misc_easterchick (edict_t *self);
void SP_misc_easterchick2 (edict_t *self);
//Knightmare- patient for the infirmary
void SP_misc_sick_guard (edict_t *self);
//Knightmare- experiment gekk
void SP_misc_gekk_writhe (edict_t *self);

//Knightmare- Coconut Monkey 3 Flame entities
void SP_light_flame1 (edict_t *self);
void SP_light_flame1s (edict_t *self);
void SP_light_flame2 (edict_t *self);
void SP_light_flame2s (edict_t *self);
//Knightmare- Coconut Monkey
void SP_monster_coco_monkey (edict_t *self);

void SP_monster_berserk (edict_t *self);
void SP_monster_gladiator (edict_t *self);
void SP_monster_gunner (edict_t *self);
void SP_monster_infantry (edict_t *self);
void SP_monster_soldier_light (edict_t *self);
void SP_monster_soldier (edict_t *self);
void SP_monster_soldier_ss (edict_t *self);
void SP_monster_tank (edict_t *self);
void SP_monster_medic (edict_t *self);
void SP_monster_flipper (edict_t *self);
void SP_monster_chick (edict_t *self);
void SP_monster_parasite (edict_t *self);
void SP_monster_flyer (edict_t *self);
void SP_monster_gekk (edict_t *self);
void SP_monster_brain (edict_t *self);
void SP_monster_brain_beta (edict_t *self);
void SP_monster_floater (edict_t *self);
void SP_monster_hover (edict_t *self);
void SP_monster_mutant (edict_t *self);
void SP_monster_supertank (edict_t *self);
void SP_monster_boss2 (edict_t *self);
void SP_monster_jorg (edict_t *self);
//Knightmare- added spawn function for monster_makron
void SP_monster_makron (edict_t *self);
void SP_monster_boss3_stand (edict_t *self);

void SP_monster_commander_body (edict_t *self);

void SP_turret_breach (edict_t *self);
void SP_turret_base (edict_t *self);
void SP_turret_driver (edict_t *self);

// RAFAEL 14-APR-98
void SP_monster_soldier_hypergun (edict_t *self);
void SP_monster_soldier_lasergun (edict_t *self);
void SP_monster_soldier_ripper (edict_t *self);
void SP_monster_fixbot (edict_t *self);
void SP_monster_gekk (edict_t *self);
void SP_monster_chick_heat (edict_t *self);
void SP_monster_gladb (edict_t *self);
void SP_monster_boss5 (edict_t *self);

// Zaero
void SP_monster_hound (edict_t *self);
void SP_monster_sentien(edict_t *self);

//Knightmare- the dog from Coconut Monkey 3
void SP_monster_dog (edict_t *self);

void SP_rotating_light (edict_t *self);
void SP_object_repair (edict_t *self);
void SP_misc_crashviper (edict_t *ent);
void SP_misc_viper_missile (edict_t *self);
void SP_misc_amb4 (edict_t *ent);
void SP_target_mal_laser (edict_t *ent);
void SP_misc_transport (edict_t *ent);
// END 14-APR-98
void SP_misc_nuke (edict_t *ent);

//===========
//ROGUE
void SP_func_plat2 (edict_t *ent);
void SP_func_door_secret2(edict_t *ent);
void SP_func_force_wall(edict_t *ent);

//Knightmare
void SP_func_breakaway (edict_t *self);
void SP_func_monitor (edict_t *self);
void SP_func_pendulum (edict_t *self);
void SP_func_pivot (edict_t *self);
void SP_func_pushable (edict_t *self);
void SP_func_reflect (edict_t *self);
void SP_func_rotating_dh (edict_t *self);
void SP_func_door_rot_dh (edict_t *self);
void SP_func_door_swinging (edict_t *self);
void SP_func_trackchange (edict_t *self);
void SP_func_tracktrain (edict_t *self);
void SP_path_track (edict_t *self);
void SP_info_train_start (edict_t *self);
void SP_func_trainbutton (edict_t *self);
void SP_func_vehicle (edict_t *self);
void SP_info_train_start (edict_t *self);
void SP_crane_beam (edict_t *self);
void SP_crane_hoist (edict_t *self);
void SP_crane_hook (edict_t *self);
void SP_crane_control (edict_t *self);
void SP_crane_reset (edict_t *self);
void SP_info_player_coop_lava (edict_t *self);
void SP_info_teleport_destination (edict_t *self);
void SP_trigger_teleport (edict_t *self);
void SP_trigger_teleporter (edict_t *self);
void SP_trigger_teleporter_bbox (edict_t *self);
void SP_trigger_disguise (edict_t *self);
void SP_trigger_inside (edict_t *self);
void SP_trigger_inside_bbox (edict_t *self);
void SP_trigger_mass (edict_t *self);
void SP_trigger_mass_bbox (edict_t *self);
void SP_trigger_scales (edict_t *self);
void SP_trigger_scales_bbox (edict_t *self);
void SP_trigger_switch (edict_t *self);
void SP_trigger_look (edict_t *self);
void SP_trigger_speaker (edict_t *self);

void SP_monster_stalker (edict_t *self);
void SP_monster_turret (edict_t *self);
void SP_target_steam (edict_t *self);
void SP_target_anger (edict_t *self);
void SP_target_killplayers (edict_t *self);
// PMM - still experimental!
void SP_target_blacklight (edict_t *self);
void SP_target_orb (edict_t *self);
// pmm
//void SP_target_spawn (edict_t *self);
void SP_hint_path (edict_t *self);
void SP_monster_carrier (edict_t *self);
void SP_monster_widow (edict_t *self);
void SP_monster_widow2 (edict_t *self);
void SP_dm_tag_token (edict_t *self);
void SP_dm_dball_goal (edict_t *self);
void SP_dm_dball_ball (edict_t *self);
void SP_dm_dball_team1_start (edict_t *self);
void SP_dm_dball_team2_start (edict_t *self);
void SP_dm_dball_ball_start (edict_t *self);
void SP_dm_dball_speed_change (edict_t *self);
void SP_monster_kamikaze (edict_t *self);
//void SP_monster_chick2 (edict_t *self);
void SP_turret_invisible_brain (edict_t *self);
void SP_xatrix_item (edict_t *self);
void SP_misc_nuke_core (edict_t *self);
//ROGUE
//===========

void SP_target_command (edict_t *self);
void SP_target_set_effect (edict_t *self);
void SP_target_effect (edict_t *self);
void SP_target_fade (edict_t *self);
void SP_target_failure (edict_t *self);
void SP_target_fog (edict_t *self);
void SP_trigger_fog (edict_t *self);
void SP_trigger_fog_bbox (edict_t *self);
void SP_target_precipitation (edict_t *self);
void SP_target_fountain (edict_t *self);
void SP_target_lightswitch (edict_t *self);
void SP_target_locator (edict_t *self);
void SP_target_lock (edict_t *self);
void SP_target_lock_clue (edict_t *self);
void SP_target_lock_code (edict_t *self);
void SP_target_lock_digit (edict_t *self);
//void SP_target_ignore_player (edict_t *self);
void SP_target_global_text (edict_t *self);
void SP_light_torch (edict_t *self);
void SP_light_flame (edict_t *self);
void SP_model_spawn (edict_t *self);
void SP_model_train (edict_t *self);
void SP_model_turret (edict_t *self);
void SP_func_dm_wall (edict_t *self);
void SP_trigger_transition (edict_t *self);
void SP_trigger_transition_bbox (edict_t *self);

// Knightmare- entities that use origin-based train pathing
void SP_func_train_origin (edict_t *self);
void SP_model_train_origin (edict_t *self);
void SP_misc_viper_origin (edict_t *ent);
void SP_misc_crashviper_origin (edict_t *ent);
void SP_misc_strogg_ship_origin (edict_t *ent);
void SP_misc_transport_origin (edict_t *ent);

// Knightmare- Q1 monsters/entities
void SP_monster_q1_chthon (edict_t *self);
void SP_monster_q1_dog (edict_t *self);
void SP_monster_q1_enforcer (edict_t *self);
void SP_monster_q1_fiend (edict_t *self);
void SP_monster_q1_grunt (edict_t *self);
void SP_monster_q1_hknight (edict_t *self);
void SP_monster_q1_knight (edict_t *self);
void SP_monster_q1_ogre (edict_t *self);     
void SP_monster_q1_rotfish (edict_t *self);
void SP_monster_q1_scrag (edict_t *self); 
void SP_monster_q1_shalrath (edict_t *self);
void SP_monster_q1_shambler (edict_t *self);
void SP_monster_q1_tarbaby (edict_t *self);
void SP_monster_q1_zombie (edict_t *self);

void SP_misc_q1_zombie_crucified (edict_t *self);
void SP_misc_q1_air_bubbles (edict_t *self);
void SP_misc_q1_large_flame(edict_t *self);
void SP_misc_q1_small_flame (edict_t *self);
void SP_misc_q1_torch(edict_t *self);
void SP_target_q1_trap(edict_t *self);
void SP_misc_q1_explobox(edict_t *self);  
void SP_misc_q1_fireball(edict_t *self);
void SP_misc_q1_globe(edict_t *self);
// end Knightmare

// transition entities
void SP_bolt (edict_t *self);
void SP_bolt2 (edict_t *self);
void SP_gekk_loogie (edict_t *self);
void SP_flechette (edict_t *self);
void SP_tracker (edict_t *self);
void SP_ion (edict_t *self);
void SP_plasma (edict_t *self);
void SP_debris (edict_t *self);
void SP_gib (edict_t *self);
void SP_gibhead (edict_t *self);
void SP_grenade (edict_t *self);
void SP_handgrenade (edict_t *self);
void SP_shocksphere (edict_t *self);
void SP_prox (edict_t *self);
void SP_tesla (edict_t *self);
void SP_trap (edict_t *self);
void SP_rocket (edict_t *self);
void SP_missile (edict_t *self);
//
// end Lazarus


spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_door_secret", SP_func_door_secret},
	{"func_door_rotating", SP_func_door_rotating},
	{"func_rotating", SP_func_rotating},
	{"func_train", SP_func_train},
	{"func_water", SP_func_water},
	{"func_bobbingwater", SP_func_bobbingwater},
	{"func_conveyor", SP_func_conveyor},
	{"func_areaportal", SP_func_areaportal},
	{"func_clock", SP_func_clock},
	{"func_reflect", SP_func_reflect},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	// RAFAEL
	{"func_object_repair", SP_object_repair},
	{"rotating_light", SP_rotating_light},

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_bbox", SP_trigger_bbox},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_push_bbox", SP_trigger_push_bbox},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_hurt_bbox", SP_trigger_hurt_bbox},
	{"trigger_key", SP_trigger_key},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_gravity_bbox", SP_trigger_gravity_bbox},
	{"trigger_monsterjump", SP_trigger_monsterjump},
	{"trigger_monsterjump_bbox", SP_trigger_monsterjump_bbox},

	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_secret", SP_target_secret},
	{"target_goal", SP_target_goal},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
	{"target_actor", SP_target_actor},
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},
	//Knightmare
	{"target_monitor", SP_target_monitor},
	{"target_monsterbattle", SP_target_monsterbattle},
	{"target_movewith", SP_target_movewith},
	{"target_change", SP_target_change},
	{"target_rotation", SP_target_rotation},
	{"target_attractor", SP_target_attractor},
	{"target_cd", SP_target_cd},
	{"target_skill", SP_target_skill},
	{"target_sky", SP_target_sky},
	{"target_playback", SP_target_playback},
	{"target_rocks", SP_target_rocks},
	{"target_bmodel_spawner", SP_target_clone},
	{"target_clone", SP_target_clone},
	{"target_text", SP_target_text},
	{"thing", SP_thing},

	// RAFAEL 15-APR-98
	{"target_mal_laser", SP_target_mal_laser},

	{"worldspawn", SP_worldspawn},
	{"viewthing", SP_viewthing},

	{"light", SP_light},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"point_combat", SP_point_combat},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
	{"misc_satellite_dish", SP_misc_satellite_dish},
	{"misc_actor", SP_misc_actor},
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_insane", SP_misc_insane},
	{"misc_deadsoldier", SP_misc_deadsoldier},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},
//Knightmare- patients for the infirmary
	{"misc_sick_guard", SP_misc_sick_guard},
//Knightmare- experiment gekks
	{"misc_gekk_writhe", SP_misc_gekk_writhe},

	// Knightmare- Coconut Monkey 3 Flame entities
	{"light_flame1", SP_light_flame1}, 
	{"light_flame1s", SP_light_flame1s}, 
	{"light_flame2", SP_light_flame2}, 
	{"light_flame2s", SP_light_flame2s},
	//Knightmare- Coconut Monkey
	{"monster_coco_monkey", SP_monster_coco_monkey}, 

	// RAFAEL
	{"misc_crashviper", SP_misc_crashviper},
	{"misc_viper_missile", SP_misc_viper_missile},
	{"misc_amb4", SP_misc_amb4},
	// RAFAEL 17-APR-98
	{"misc_transport", SP_misc_transport},
	// END 17-APR-98
	// RAFAEL 12-MAY-98
	{"misc_nuke", SP_misc_nuke},

	{"monster_berserk", SP_monster_berserk},
	{"monster_gladiator", SP_monster_gladiator},
	{"monster_gunner", SP_monster_gunner},
	{"monster_infantry", SP_monster_infantry},
	{"monster_soldier_light", SP_monster_soldier_light},
	{"monster_soldier", SP_monster_soldier},
	{"monster_soldier_ss", SP_monster_soldier_ss},
	{"monster_tank", SP_monster_tank},
	{"monster_tank_commander", SP_monster_tank},
	{"monster_medic", SP_monster_medic},
	{"monster_flipper", SP_monster_flipper},
	{"monster_chick", SP_monster_chick},
	{"monster_parasite", SP_monster_parasite},
	{"monster_flyer", SP_monster_flyer},
	{"monster_brain", SP_monster_brain},
	{"monster_brain_beta", SP_monster_brain_beta},
	{"monster_floater", SP_monster_floater},
	{"monster_hover", SP_monster_hover},
	{"monster_mutant", SP_monster_mutant},
	{"monster_supertank", SP_monster_supertank},
	{"monster_boss2", SP_monster_boss2},
	{"monster_boss3_stand", SP_monster_boss3_stand},
	{"monster_jorg", SP_monster_jorg},
	//Knightmare- added spawn function for monster_makron
	{"monster_makron", SP_monster_makron},

	{"monster_commander_body", SP_monster_commander_body},

	// RAFAEL 14-APR-98
	{"monster_soldier_hypergun", SP_monster_soldier_hypergun},
	{"monster_soldier_lasergun", SP_monster_soldier_lasergun},
	{"monster_soldier_ripper",	SP_monster_soldier_ripper},
	{"monster_fixbot", SP_monster_fixbot},
	{"monster_gekk", SP_monster_gekk},
	{"monster_chick_heat", SP_monster_chick_heat},
	{"monster_gladb", SP_monster_gladb},
	{"monster_boss5", SP_monster_boss5},
	// END 14-APR-98
	// RAFAEL 12-MAY-98

	//Rogue monsters
	{"monster_daedalus", SP_monster_hover},
	{"monster_medic_commander", SP_monster_medic},
	{"monster_stalker", SP_monster_stalker},
	{"monster_turret", SP_monster_turret},
	{"monster_carrier", SP_monster_carrier},
	{"monster_kamikaze", SP_monster_kamikaze},
	{"monster_widow", SP_monster_widow},
	{"monster_widow2", SP_monster_widow2},

	// Zaero
	{"monster_hound", SP_monster_hound},
	{"monster_sentien", SP_monster_sentien},

	//Knightmare- the dog from Coconut Monkey 3
	{"monster_dog", SP_monster_dog},

	{"misc_nuke", SP_misc_nuke},

	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},

//==============
//ROGUE
	{"func_plat2", SP_func_plat2},
	{"func_door_secret2", SP_func_door_secret2},
	{"func_force_wall", SP_func_force_wall},
	{"func_monitor", SP_func_monitor},
	{"func_pendulum", SP_func_pendulum},
	{"func_pivot", SP_func_pivot},
	{"func_pushable", SP_func_pushable},
	{"func_rotating_dh", SP_func_rotating_dh},
	{"func_door_rot_dh", SP_func_door_rot_dh},
	{"func_door_swinging", SP_func_door_swinging},
	{"func_trackchange", SP_func_trackchange},
	{"func_tracktrain", SP_func_tracktrain},
	{"path_track", SP_path_track},
	{"info_train_start", SP_info_train_start},
	{"func_trainbutton", SP_func_trainbutton},
	{"func_vehicle", SP_func_vehicle},
	//Knightmare
	{"func_breakaway", SP_func_breakaway},
	// Lazarus
	{"crane_beam",   SP_crane_beam},
	{"crane_hoist",  SP_crane_hoist},
	{"crane_hook",   SP_crane_hook},
	{"crane_control",SP_crane_control},
	{"crane_reset",SP_crane_reset},
	{"trigger_teleport", SP_trigger_teleport},
	{"trigger_teleporter", SP_trigger_teleporter},
	{"trigger_teleporter_bbox", SP_trigger_teleporter_bbox},
	{"trigger_disguise", SP_trigger_disguise},
	{"trigger_inside", SP_trigger_inside},
	{"trigger_inside_bbox", SP_trigger_inside_bbox},
	{"trigger_mass", SP_trigger_mass},
	{"trigger_mass_bbox", SP_trigger_mass_bbox},
	{"trigger_scales", SP_trigger_scales},
	{"trigger_scales_bbox", SP_trigger_scales_bbox},
	{"trigger_look", SP_trigger_look},
	{"trigger_speaker", SP_trigger_speaker},
	{"trigger_switch", SP_trigger_switch},
	{"info_teleport_destination", SP_info_teleport_destination},
	{"info_player_coop_lava", SP_info_player_coop_lava},
	{"target_steam", SP_target_steam},
	{"target_anger", SP_target_anger},
//	{"target_spawn", SP_target_spawn},
	{"target_killplayers", SP_target_killplayers},
	// PMM - experiment
	{"target_blacklight", SP_target_blacklight},
	{"target_orb", SP_target_orb},
	// pmm
	{"hint_path", SP_hint_path},
	{"dm_tag_token", SP_dm_tag_token},
	{"dm_dball_goal", SP_dm_dball_goal},
	{"dm_dball_ball", SP_dm_dball_ball},
	{"dm_dball_team1_start", SP_dm_dball_team1_start},
	{"dm_dball_team2_start", SP_dm_dball_team2_start},
	{"dm_dball_ball_start", SP_dm_dball_ball_start},
	{"dm_dball_speed_change", SP_dm_dball_speed_change},
//	{"monster_chick2", SP_monster_chick2},
	{"turret_invisible_brain", SP_turret_invisible_brain},
	{"misc_nuke_core", SP_misc_nuke_core},
//ROGUE
//==============

	{"target_command", SP_target_command},
	{"target_effect", SP_target_effect},
	{"target_set_effect", SP_target_set_effect},
	{"target_fade", SP_target_fade},
	{"target_failure", SP_target_failure},
	{"target_fog", SP_target_fog},
	{"trigger_fog", SP_trigger_fog},
	{"trigger_fog_bbox", SP_trigger_fog_bbox},
	{"target_precipitation", SP_target_precipitation},
	{"target_fountain", SP_target_fountain},
	{"target_lightswitch", SP_target_lightswitch},
	{"target_locator", SP_target_locator},
	{"target_lock", SP_target_lock},
	{"target_lock_clue", SP_target_lock_clue},
	{"target_lock_code", SP_target_lock_code},
	{"target_lock_digit", SP_target_lock_digit},
//	{"target_ignore_player", SP_target_ignore_player},
	{"target_global_text", SP_target_global_text},
	{"light_torch", SP_light_torch},
	{"light_flame", SP_light_flame},
	{"model_spawn", SP_model_spawn},
	{"model_train", SP_model_train},
	{"model_turret", SP_model_turret},
	{"func_dm_wall", SP_func_dm_wall},
	{"trigger_transition", SP_trigger_transition},
	{"trigger_transition_bbox", SP_trigger_transition_bbox},

// Knightmare- entities that use origin-based train pathing
	{"func_train_origin", SP_func_train_origin},
	{"model_train_origin", SP_model_train_origin},
	{"misc_viper_origin", SP_misc_viper_origin},
	{"misc_crashviper_origin", SP_misc_crashviper_origin},
	{"misc_strogg_ship_origin", SP_misc_strogg_ship_origin},
	{"misc_transport_origin", SP_misc_transport_origin},

// Knightmare- Q1 monsters/entities
	// Generations spawn names
//	{"q1_monster_chton", SP_monster_q1_chthon},	
	{"q1_monster_dog", SP_monster_q1_dog},
	{"q1_monster_enforcer", SP_monster_q1_enforcer},
	{"q1_monster_fiend", SP_monster_q1_fiend},
	{"q1_monster_soldier", SP_monster_q1_grunt},
	{"q1_monster_hknight", SP_monster_q1_hknight},
	{"q1_monster_knight", SP_monster_q1_knight},
	{"q1_monster_ogre", SP_monster_q1_ogre},
	{"q1_monster_fish", SP_monster_q1_rotfish},
	{"q1_monster_scrag", SP_monster_q1_scrag},
	{"q1_monster_shambler", SP_monster_q1_shambler},
	{"q1_monster_shalrath", SP_monster_q1_shalrath},
	{"q1_monster_tarbaby", SP_monster_q1_tarbaby},
	{"q1_monster_zombie", SP_monster_q1_zombie},

	// Q2Infighter spawn names
//	{"monster_q1_chthon", SP_monster_q1_chthon},	
	{"monster_q1_dog", SP_monster_q1_dog},
	{"monster_q1_enforcer", SP_monster_q1_enforcer},
	{"monster_q1_demon", SP_monster_q1_fiend},
	{"monster_q1_army", SP_monster_q1_grunt},
	{"monster_q1_hknight", SP_monster_q1_hknight},
	{"monster_q1_knight", SP_monster_q1_knight},
	{"monster_q1_ogre", SP_monster_q1_ogre},
	{"monster_q1_fish", SP_monster_q1_rotfish},
	{"monster_q1_wizard", SP_monster_q1_scrag},
	{"monster_q1_shambler", SP_monster_q1_shambler},
	{"monster_q1_shalrath", SP_monster_q1_shalrath},
	{"monster_q1_tarbaby", SP_monster_q1_tarbaby},
	{"monster_q1_zombie", SP_monster_q1_zombie},

	// misc Q1 entities
	{"misc_q1_zombie_crucified", SP_misc_q1_zombie_crucified},
	{"misc_q1_air_bubbles", SP_misc_q1_air_bubbles},
	{"misc_q1_large_flame", SP_misc_q1_large_flame},
	{"misc_q1_small_flame", SP_misc_q1_small_flame},
	{"misc_q1_torch", SP_misc_q1_torch},
	{"target_q1_trap", SP_target_q1_trap},
	{"misc_q1_light_globe", SP_misc_q1_globe},
	{"misc_q1_explobox", SP_misc_q1_explobox},
	{"misc_q1_fireball",SP_misc_q1_fireball},
// end Knightmare

// transition entities
	{"bolt", SP_bolt},
	{"bolt2", SP_bolt2},
	{"gekk_loogie", SP_gekk_loogie},
	{"flechette", SP_flechette},
	{"tracker", SP_tracker},
	{"ion", SP_ion},
	{"plasma", SP_plasma},
	{"debris", SP_debris},
	{"gib", SP_gib},
	{"gibhead", SP_gibhead},
	{"grenade", SP_grenade},
	{"hgrenade", SP_handgrenade},
	{"shocksphere", SP_shocksphere},
	{"prox", SP_prox},
	{"tesla", SP_tesla},
	{"trap", SP_trap},
	{"rocket", SP_rocket},
	{"homing rocket", SP_rocket},
	{"missile", SP_missile},
	{"homing rocket", SP_missile},
// end Lazarus

	{NULL, NULL}
};

// Knightmare- global pointer for the entity alias script
// The file should be loaded into memory, because we can't
// re-open and read it for every entity we parse
char	*alias_data;
int		alias_data_size;
#ifndef KMQUAKE2_ENGINE_MOD
qboolean alias_from_pak;
#endif

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;
	cvar_t	*gamedir;

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		return;
	}
//PGM - do this before calling the spawn function so it can be overridden.
#ifdef ROGUE_GRAVITY
	ent->gravityVector[0] =  0.0;
	ent->gravityVector[1] =  0.0;
	ent->gravityVector[2] = -1.0;
#endif
//PGM

	// FIXME - PMM classnames hack
	if (!strcmp(ent->classname, "weapon_nailgun"))
		ent->classname = (FindItem("ETF Rifle"))->classname;
	if (!strcmp(ent->classname, "ammo_nails"))
		ent->classname = (FindItem("Flechettes"))->classname;
	if (!strcmp(ent->classname, "weapon_heatbeam"))
		ent->classname = (FindItem("Plasma Beam"))->classname;
	// PMM

	// replace brains in Reckoning
	gamedir = gi.cvar("game", "", 0);
	if (*gamedir->string && !Q_stricmp(gamedir->string, "xatrix")
		&& IsXatrixMap() && !strcmp(ent->classname, "monster_brain"))
		ent->classname = "monster_brain_beta";

	// Knightmare- mission pack monster replacement
	if (mp_monster_replace->value)
	{	// gladiator
		if (!strcmp(ent->classname, "monster_gladiator"))
		{
			ent->classname = "monster_gladb";
			if (st.item && mp_monster_ammo_replace->value)
				if (!strcmp(st.item, "ammo_slugs")) //replace ammo
				{
					ent->item = FindItemByClassname ("ammo_magslug");
					st.item = NULL;
				}
		}
		// brain
		if (!strcmp(ent->classname, "monster_brain"))
			ent->classname = "monster_brain_beta";
		// soldiers
		if (!strcmp(ent->classname, "monster_soldier_light"))
		{
			ent->classname = "monster_soldier_ripper";
			if (st.item && mp_monster_ammo_replace->value)
				if ( !strcmp(st.item, "ammo_shells") || !strcmp(st.item, "ammo_bullets"))
				{
					ent->item = FindItemByClassname ("ammo_cells");
					st.item = NULL;
				}
		}
		if (!strcmp(ent->classname, "monster_soldier"))
		{
			ent->classname = "monster_soldier_hypergun";
			if (st.item && mp_monster_ammo_replace->value)
				if ( !strcmp(st.item, "ammo_shells") || !strcmp(st.item, "ammo_bullets"))
				{
					ent->item = FindItemByClassname ("ammo_cells");
					st.item = NULL;
				}
		}
		if (!strcmp(ent->classname, "monster_soldier_ss"))
		{
			ent->classname = "monster_soldier_lasergun";
			if (st.item && mp_monster_ammo_replace->value)
				if ( !strcmp(st.item, "ammo_shells") || !strcmp(st.item, "ammo_bullets"))
				{
					ent->item = FindItemByClassname ("ammo_cells");
					st.item = NULL;
				}
		}
		// icarus
		if (!strcmp(ent->classname, "monster_hover"))
			ent->classname = "monster_daedalus";
		// supertank
		if (!strcmp(ent->classname, "monster_supertank"))
			ent->classname = "monster_boss5";
		// chick
		if (!strcmp(ent->classname, "monster_chick"))
			ent->classname = "monster_chick_heat";
		// medic
		if (!strcmp(ent->classname, "monster_medic")
			//don't spawn another medic commander from medic commander
			&& !(ent->monsterinfo.monsterflags & MFL_DO_NOT_COUNT))
			ent->classname = "monster_medic_commander";
	}

	//LM Escape monster replacement
	if (!strcmp(ent->classname, "monster_soldier_plasma_re"))
		ent->classname = "monster_soldier_ripper";
	if (!strcmp(ent->classname, "monster_soldier_plasma_sp"))
		ent->classname = "monster_soldier_hypergun";
	//LM Escape Plasma gun
	if (!strcmp(ent->classname, "weapon_plasma"))
		ent->classname = "weapon_boomer";

	//Knightmare- replace Pierre in Coconut Monkey 3 with Daedalus
	if (!strcmp(ent->classname, "monster_pierre_monkey"))
	{
		ent->classname = "monster_daedalus";
		ent->s.origin[2] += 8;
		ent->dmg = 4;
		ent->health = 1200;
		ent->powerarmor = 500;
	}

	//Knightmare- backup entity's original angles
	VectorCopy (ent->s.angles, ent->org_angles);

	// check item spawn functions
	for (i=0, item=itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, ent->classname))
		{	// found it
			SpawnItem (ent, item);
			return;
		}
	}

	// check normal spawn functions
	for (s=spawns; s->name; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
	gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
}

void ReInitialize_Entity (edict_t *ent)
{
	spawn_t	*s;

	// check normal spawn functions
	for (s=spawns; s->name; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
}

/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*newb, *new_p;
	int		i,l;
	
	l = (int)strlen(string) + 1;

	newb = gi.TagMalloc (l, TAG_LEVEL);

	new_p = newb;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return newb;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void ED_ParseField (char *key, char *value, edict_t *ent)
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for (f=fields ; f->name ; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_stricmp(f->name, key))
		{	// found it
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			switch (f->type)
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = ED_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	gi.dprintf ("%s is not a field\n", key);
}

/*
==============================================================================

ALIAS SCRIPT LOADING

==============================================================================
*/


#ifndef KMQUAKE2_ENGINE_MOD
/*
====================
ReadTextFile

Called from LoadAliasData to try
loading script file from disk.
====================
*/
char *ReadTextFile (char *filename, int *size)
{
    FILE *fp;
    char *filestring = NULL;
    int len;

    fp = fopen(filename, "r");
    if (!fp)
		return NULL;

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    filestring = gi.TagMalloc(len + 1, TAG_LEVEL);
    if (!filestring)
	{
		fclose(fp);
		return NULL;
    }

    len = fread(filestring, 1, len, fp);
	*size = len;
    filestring[len] = 0;

    fclose(fp);

    return filestring;
}

/*
====================
LoadAliasFile

Tries to load script file from
disk, and then from a pak file.
====================
*/
qboolean LoadAliasFile (char *name)
{
	char aliasfilename[MAX_QPATH] = "";

	alias_from_pak = false;

	GameDirRelativePath (name, aliasfilename, sizeof(aliasfilename));
    alias_data = ReadTextFile(aliasfilename, &alias_data_size);

	// If file doesn't exist on hard disk, it must be in a pak file
	if (!alias_data)
	{
		cvar_t			*basedir, *gamedir;
		char			filename[256];
		char			pakfile[256];
		char			textname[128];
		int				i, k, num, numitems;
		qboolean		in_pak = false;
		FILE			*fpak;
		pak_header_t	pakheader;
		pak_item_t		pakitem;

		basedir = gi.cvar("basedir", "", 0);
		gamedir = gi.cvar("gamedir", "", 0);
		Com_strcpy(filename, sizeof(filename), basedir->string);
		Com_sprintf(textname, sizeof(textname), name);

		if (strlen(gamedir->string))
			Com_strcpy(filename, sizeof(filename), gamedir->string);
		// check all pakfiles in current gamedir
		for (i=0; i<10; i++)
		{
			Com_sprintf(pakfile, sizeof(pakfile), "%s/pak%d.pak",filename,i);
			if (NULL != (fpak = fopen(pakfile, "rb")))
			{
				num=fread(&pakheader,1,sizeof(pak_header_t),fpak);
				if(num >= sizeof(pak_header_t))
				{
					if (pakheader.id[0] == 'P' &&
						pakheader.id[1] == 'A' &&
						pakheader.id[2] == 'C' &&
						pakheader.id[3] == 'K'   )
					{
						numitems = pakheader.dsize/sizeof(pak_item_t);
						fseek(fpak,pakheader.dstart,SEEK_SET);
						for(k=0; k<numitems && !in_pak; k++)
						{
							fread(&pakitem,1,sizeof(pak_item_t),fpak);
							if (!Q_stricmp(pakitem.name,textname))
							{
								in_pak = true;
								fseek(fpak,pakitem.start,SEEK_SET);
								alias_data = gi.TagMalloc(pakitem.size + 1, TAG_LEVEL);
								if (!alias_data) {
									fclose(fpak);
									gi.dprintf("LoadAliasData: Memory allocation failure for entalias.dat\n");
									return false;
								}
								alias_data_size = fread(alias_data,1,pakitem.size,fpak);
								alias_data[pakitem.size] = 0; // put end marker
								alias_from_pak = true;
							}
						}
					}
				}
				fclose(fpak);
			}
		}
	}

	if (!alias_data)
		return false;

	return true;
}
#else
// Gotta have these for extractfuncs...
char *ReadTextFile (char *filename, int *size)
{
	return NULL;
}
qboolean LoadAliasFile (char *name)
{
	return false;
}
#endif

/*
====================
LoadAliasData

Loads entity alias file either on disk
or from a pak file in the game dir.
====================
*/
void LoadAliasData (void)
{
#ifdef KMQUAKE2_ENGINE_MOD // use new engine file loading function instead
	// try per-level script file
	alias_data_size = gi.LoadFile(va("ext_data/entalias/%s.alias", level.mapname), (void **)&alias_data);
	if (alias_data_size < 2) // file not found, try global file
		alias_data_size = gi.LoadFile("ext_data/entalias.def", (void **)&alias_data);
	if (alias_data_size < 2) // file still not found, try old filename
		alias_data_size = gi.LoadFile("scripts/entalias.dat", (void **)&alias_data);
#else
	if (!LoadAliasFile(va("ext_data/entalias/%s.alias", level.mapname)))
		if (!LoadAliasFile("ext_data/entalias.def"))
			LoadAliasFile("scripts/entalias.dat");
#endif
}

/*
====================
ED_ParseEntityAlias

Parses an edict, looking for its classname, and then
looks in the entity alias file for an alias, and
loads that if found.
Returns true if an aliws was loaded for the given entity.
====================
*/
qboolean ED_ParseEntityAlias (char *data, edict_t *ent)
{
	qboolean	classname_found, alias_found, alias_loaded;
	char		*search_data;
	char		*search_token;
	char		entclassname[256];
	char		keyname[256];
	int			braceLevel;

	classname_found = false;
	alias_found	= false;
	alias_loaded = false;
	braceLevel = 0;

	if (!alias_data) // If no alias file was loaded, don't bother
		return false;

	search_data = data;  // copy entity data postion
	// go through all the dictionary pairs looking for the classname
	while (1)
	{	//parse keyname
		search_token = COM_Parse (&search_data);
		if (!search_data)
			gi.error ("ED_ParseEntityAlias: end of entity data without closing brace");
		if (search_token[0] == '}')
			break;
		if (!strcmp(search_token, "classname"))
			classname_found = true;

		// parse value	
		search_token = COM_Parse (&search_data);
		if (!search_data)
			gi.error ("ED_ParseEntityAlias: end of entity data without closing brace");
		if (search_token[0] == '}')
			gi.error ("ED_ParseEntityAlias: closing brace without entity data");
		// if we've found the classname, exit loop
		if (classname_found) {
			Com_strcpy (entclassname, sizeof(entclassname), search_token);
			break;
		}
	}
	// then search the entalias.def file for that classname
	if (classname_found)
	{
		search_data = alias_data;	// copy alias data postion
 		while (search_data < (alias_data + alias_data_size))
		{
			search_token = COM_Parse (&search_data);
			if (!search_data)
				return false;
			if (!search_token)
				break;
			// see if we're inside the braces of an alias definition
			if (search_token[0] == '{') braceLevel++;
			else if (search_token[0] == '}') braceLevel--;
			if (braceLevel < 0) {
				gi.dprintf ("ED_ParseEntityAlias: closing brace without matching opening brace\n");
				return false;
			}
			// matching classname must be outside braces
			if (!strcmp(search_token, entclassname) && (braceLevel == 0)) {
				//gi.dprintf ("Alias for %s found in alias script file.\n", search_token);
				alias_found = true;
				break;
			}
		}
		// then if that classname is found, load the fields from that alias
		if (alias_found)
		{	// get the opening curly brace
			search_token = COM_Parse (&search_data);
			if (!search_data) {
				gi.dprintf ("ED_ParseEntityAlias: unexpected EOF\n");
				return false;
			}
			if (search_token[0] != '{') {
				gi.dprintf ("ED_ParseEntityAlias: found %s when expecting {\n", search_token);
				return false;
			}
			// go through all the dictionary pairs
			while (search_data < (alias_data + alias_data_size))
			{	
			// parse key
				search_token = COM_Parse (&search_data);
				if (!search_data) {
					gi.dprintf ("ED_ParseEntityAlias: EOF without closing brace\n");
					return false;
				}
				if (search_token[0] == '}')
					break;
				Com_strcpy (keyname, sizeof(keyname), search_token);
				
			// parse value	
				search_token = COM_Parse (&search_data);
				if (!search_data) {
					gi.dprintf ("ED_ParseEntityAlias: EOF without closing brace\n");
					return false;
				}
				if (search_token[0] == '}') {
					gi.dprintf ("ED_ParseEntityAlias: closing brace without data\n");
					return false;
				}
				ED_ParseField (keyname, search_token, ent);
				alias_loaded = true;
			}
		}
	}
	return alias_loaded;
}


/*
==============================================================================
END ALIAS SCRIPT LOADING
==============================================================================
*/

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;
	//Knightmare added
	qboolean	alias_loaded = false;

	init = false;
	memset (&st, 0, sizeof(st));

	// Knightmare- look for and load an alias for this ent
	alias_loaded = ED_ParseEntityAlias (data, ent);

// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		Com_strcpy (keyname, sizeof(keyname), com_token);
		
	// parse value	
		com_token = COM_Parse (&data);
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error ("ED_ParseEntity: closing brace without data");

		init = true;	

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		// Knightmare- if the classname was replaced by an alias, don't load it back
		if (alias_loaded && !strcmp(keyname, "classname"))
			continue;

		ED_ParseField (keyname, com_token, ent);
	}

	if (!init)
		memset (ent, 0, sizeof(*ent));

	return data;
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FixTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		//Lazarus- ignore bmodel spawner (its team isn't used)
		if (e->classname && !Q_stricmp(e->classname,"target_bmodel_spawner"))
			continue; 
		if (!strcmp(e->classname, "func_train"))
		{
			if(e->flags & FL_TEAMSLAVE)
			{
				chain = e;
				e->teammaster = e;
				e->teamchain = NULL;
				e->flags &= ~FL_TEAMSLAVE;
				c++;
				c2++;
				for (j=1, e2=g_edicts+j ; j < globals.num_edicts ; j++,e2++)
				{
					if (e2 == e)
						continue;
					if (!e2->inuse)
						continue;
					if (!e2->team)
						continue;
					if (!strcmp(e->team, e2->team))
					{
						c2++;
						chain->teamchain = e2;
						e2->teammaster = e;
						e2->teamchain = NULL;
						chain = e2;
						e2->flags |= FL_TEAMSLAVE;
						e2->movetype = MOVETYPE_PUSH;
						e2->speed = e->speed;
					}
				}
			}
		}
	}
	gi.dprintf ("%i teams repaired\n", c);
}

void G_FindTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	G_FixTeams();

	gi.dprintf ("%i teams with %i entities\n", c, c2);
}

void trans_ent_filename (char *filename, size_t filenameSize);
void ReadEdict (FILE *f, edict_t *ent);
void LoadTransitionEnts()
{
	if(developer->value)
		gi.dprintf("==== LoadTransitionEnts ====\n");
	if(game.transition_ents)
	{
		char		t_file[MAX_QPATH];
		int			i, j;
		FILE		*f;
		vec3_t		v_spawn;
		edict_t		*ent;
		edict_t		*spawn;
		
		VectorClear(v_spawn);
		if(strlen(game.spawnpoint))
		{
			spawn = G_Find(NULL,FOFS(targetname),game.spawnpoint);
			while(spawn)
			{
				if(!Q_stricmp(spawn->classname,"info_player_start"))
				{
					VectorCopy(spawn->s.origin,v_spawn);
					break;
				}
				spawn = G_Find(spawn,FOFS(targetname),game.spawnpoint);
			}
		}
		trans_ent_filename (t_file, sizeof(t_file));
		f = fopen(t_file,"rb");
		if(!f)
			gi.error("LoadTransitionEnts: Cannot open %s\n",t_file);
		else
		{
			for(i=0; i<game.transition_ents; i++)
			{
				ent = G_Spawn();
				ReadEdict(f,ent);
				// Correction for monsters with health EXACTLY 0
				// If we don't do this, spawn function will bring
				// 'em back to life
				if(ent->svflags & SVF_MONSTER)
				{
					if(!ent->health)
					{
						ent->health = -1;
						ent->deadflag = DEAD_DEAD;
					}
					else if(ent->deadflag == DEAD_DEAD)
					{
						ent->health = min(ent->health,-1);
					}
				}
				VectorAdd(ent->s.origin,v_spawn,ent->s.origin);
				VectorCopy(ent->s.origin,ent->s.old_origin);
				ED_CallSpawn (ent);
				if(ent->owner_id)
				{
					if(ent->owner_id < 0)
					{
						ent->owner = &g_edicts[-ent->owner_id];
					}
					else
					{
						// We KNOW owners precede owned ents in the 
						// list because of the way it was constructed
						ent->owner = NULL;
						for(j=game.maxclients+1; j<globals.num_edicts && !ent->owner; j++)
						{
							if(ent->owner_id == g_edicts[j].id)
								ent->owner = &g_edicts[j];
						}
					}
					ent->owner_id = 0;
				}
			//Knightmare- is this necessary?  Entity state flags should be saved.
			//	ent->s.renderfx |= RF_IR_VISIBLE;
			}
			fclose(f);
		}
	}
}
/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void movewith_init (edict_t *ent);

void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;
	extern int	max_modelindex;
	extern int	max_soundindex;
	extern int	lastgibframe;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
	if (skill->value != skill_level)
		gi.cvar_forceset("skill", va("%f", skill_level));

	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);

	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	// Lazarus: these are used to track model and sound indices
	//          in g_main.c:
	max_modelindex = 0;
	max_soundindex = 0;

	// Lazarus: last frame a gib was spawned in
	lastgibframe = 0;

	Com_strcpy (level.mapname, sizeof(level.mapname), mapname);
	Com_strcpy (game.spawnpoint, sizeof(game.spawnpoint), spawnpoint);

	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

	// Knightmare- load the entity alias script file
	LoadAliasData();
	//gi.dprintf ("Size of alias data: %i\n", alias_data_size);

// parse ents
	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent);

		// yet another map hack
		if (!Q_stricmp(level.mapname, "command") && !Q_stricmp(ent->classname, "trigger_once") && !Q_stricmp(ent->model, "*27"))
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;
		
		// ROGUE
		//ahh, the joys of map hacks .. 
		if (!Q_stricmp(level.mapname, "rhangar2") && !Q_stricmp(ent->classname, "func_door_rotating") && ent->targetname && !Q_stricmp(ent->targetname, "t265"))
			ent->spawnflags &= ~SPAWNFLAG_NOT_COOP;
		if (!Q_stricmp(level.mapname, "rhangar2") && !Q_stricmp(ent->classname, "trigger_always") && ent->target && !Q_stricmp(ent->target, "t265"))
			ent->spawnflags |= SPAWNFLAG_NOT_COOP;
		if (!Q_stricmp(level.mapname, "rhangar2") && !Q_stricmp(ent->classname, "func_wall") && !Q_stricmp(ent->model, "*15"))
			ent->spawnflags |= SPAWNFLAG_NOT_COOP;
		// rogue
		
		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
				{
					G_FreeEdict (ent);	
					inhibit++;
					continue;
				}
			}
			else if ( coop->value )	
			{
				if ( IsRogueMap() )	// Knightmare- this breaks non-Rogue maps in coop
				{
					if (ent->spawnflags & SPAWNFLAG_NOT_COOP)
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}

					// stuff marked !easy & !med & !hard are coop only, all levels
					if (!((ent->spawnflags & SPAWNFLAG_NOT_EASY) &&
						(ent->spawnflags & SPAWNFLAG_NOT_MEDIUM) &&
						(ent->spawnflags & SPAWNFLAG_NOT_HARD)) )
					{
						if ( ((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
							((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
							((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
						  )
						{
							G_FreeEdict (ent);	
							inhibit++;
							continue;
						}
					}
				}
				else // generic coop
				{
					if ( CheckCoop_MapHacks(ent) )	// FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}
					if ( ((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
						((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
						((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
						)
						{
							G_FreeEdict (ent);	
							inhibit++;
							continue;
						}
				}
			}
			else
			{
				if ( /*((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) || */ 
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

//PGM - do this before calling the spawn function so it can be overridden.
#ifdef ROGUE_GRAVITY
		ent->gravityVector[0] =  0.0;
		ent->gravityVector[1] =  0.0;
		ent->gravityVector[2] = -1.0;
#endif
//PGM
		ED_CallSpawn (ent);

		//Knightmare- remove IR visible attrib from mine lights and banners
		if ( !strcmp(ent->classname, "light_mine1") || !strcmp(ent->classname, "light_mine2")
			|| !strcmp(ent->classname, "misc_banner") )
			ent->s.renderfx &= ~RF_IR_VISIBLE;

		//David Hyde's code for the origin offset
		VectorAdd(ent->absmin, ent->absmax, ent->origin_offset);
		VectorScale(ent->origin_offset, 0.5, ent->origin_offset);
		VectorSubtract(ent->origin_offset, ent->s.origin, ent->origin_offset);
	}	

	// Knightmare- unload the alias script file
	if (alias_data) { // If no alias file was loaded, don't bother
#ifdef KMQUAKE2_ENGINE_MOD // use new engine function instead
		gi.FreeFile(alias_data);
#else
//		if (alias_from_pak)
			gi.TagFree(alias_data);
//		else
//			free(&alias_data);
#endif
	}

	gi.dprintf ("%i entities inhibited\n", inhibit);

#ifdef DEBUG
	i = 1;
	ent = EDICT_NUM(i);
	while (i < globals.num_edicts)
	{
		if (ent->inuse != 0 || ent->inuse != 1)
			Com_DPrintf("Invalid entity %d\n", i);
		i++, ent++;
	}
#endif

	G_FindTeams ();

	// DWH
	G_FindCraneParts();

	PlayerTrail_Init ();

//ROGUE
	if(deathmatch->value)
	{
		if(randomrespawn && randomrespawn->value)
			PrecacheForRandomRespawn();
	}
	else
	{
		InitHintPaths();		// if there aren't hintpaths on this map, enable quick aborts
	}
//ROGUE

// ROGUE	-- allow dm games to do init stuff right before game starts.
	if (deathmatch->value && gamerules && gamerules->value)
	{
		if (DMGame.PostInitSetup)
			DMGame.PostInitSetup ();
	}
// ROGUE

	//Knightmare added
	if (game.transition_ents)
	{
		LoadTransitionEnts();
	}

	actor_files ();
}


//===================================================================

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

char *single_statusbar = 
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	230 " //was xv 262
"	num	4	10 "
"	xv	296 "
"	pic	9 "
"endif "

// help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "

// zoom
"if 23 "
"   yv 0 "
"   xv 0 "
"   pic 23 "
"endif "
;

char *dm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	230 " //was 246
"	num	4	10 "
"	xv	296 "
"	pic	9 "
"endif "

// help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

// frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "
;


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
void SP_worldspawn (edict_t *ent)
{
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

	// reserve some spots for dead player bodies for coop / deathmatch
	InitBodyQue ();

	// set configstrings for items
	SetItemNames ();

	if (st.nextmap)
		Com_strcpy (level.nextmap, sizeof(level.nextmap), st.nextmap);

	// make some data visible to the server

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_NAME, ent->message);
		Com_strcpy (level.level_name, sizeof(level.level_name), ent->message);
	}
	else
		Com_strcpy (level.level_name, sizeof(level.level_name), level.mapname);

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "unit1_");

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	// Knightmare- if a named soundtrack is specified, play it instead of from CD
	if (ent->musictrack && strlen(ent->musictrack))
		gi.configstring (CS_CDTRACK, ent->musictrack);
	else
		gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// status bar program
	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);

	//---------------


	// help icon for statusbar
	gi.imageindex ("i_help");
	level.pic_health = gi.imageindex ("i_health");
	gi.imageindex ("help");
	gi.imageindex ("field_3");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime

	PrecacheItem (FindItem ("Blaster"));

	gi.soundindex ("player/lava1.wav");
	gi.soundindex ("player/lava2.wav");

	gi.soundindex ("misc/pc_up.wav");
	gi.soundindex ("misc/talk1.wav");

	// gibs
	gi.soundindex ("misc/udeath.wav");

	gi.soundindex ("items/respawn1.wav");

	// sexed sounds
	gi.soundindex ("*death1.wav");
	gi.soundindex ("*death2.wav");
	gi.soundindex ("*death3.wav");
	gi.soundindex ("*death4.wav");
	gi.soundindex ("*fall1.wav");
	gi.soundindex ("*fall2.wav");	
	gi.soundindex ("*gurp1.wav");		// drowning damage
	gi.soundindex ("*gurp2.wav");
	gi.soundindex ("*jump1.wav");		// player jump
	gi.soundindex ("*pain25_1.wav");
	gi.soundindex ("*pain25_2.wav");
	gi.soundindex ("*pain50_1.wav");
	gi.soundindex ("*pain50_2.wav");
	gi.soundindex ("*pain75_1.wav");
	gi.soundindex ("*pain75_2.wav");
	gi.soundindex ("*pain100_1.wav");
	gi.soundindex ("*pain100_2.wav");

	// sexed models
	// THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	// you can add more, max 64
	// Knightmare- load these in single player, too
	if (use_vwep->value || deathmatch->value)
	{
		gi.modelindex ("#w_blaster.md2");
		gi.modelindex ("#w_shotgun.md2");
		gi.modelindex ("#w_sshotgun.md2");
		gi.modelindex ("#w_machinegun.md2");
		gi.modelindex ("#w_chaingun.md2");
		gi.modelindex ("#a_grenades.md2");
		gi.modelindex ("#w_glauncher.md2");
		gi.modelindex ("#w_rlauncher.md2");
		gi.modelindex ("#w_hyperblaster.md2");
		gi.modelindex ("#w_railgun.md2");
		gi.modelindex ("#w_bfg.md2");

		gi.modelindex ("#w_disrupt.md2");			// PGM
		gi.modelindex ("#w_etfrifle.md2");			// PGM
		gi.modelindex ("#w_plasma.md2");			// PGM
		gi.modelindex ("#w_plauncher.md2");			// PGM
		gi.modelindex ("#w_chainfist.md2");			// PGM
		gi.modelindex ("#w_phalanx.md2");			// Knightmare added
		gi.modelindex ("#w_ripper.md2");			// Knightmare added
		gi.modelindex ("#w_shockwave.md2");			// Knightmare added
//		gi.modelindex ("#a_trap.md2");				// Knightmare added
//		gi.modelindex ("#a_tesla.md2");				// Knightmare added
//		gi.modelindex ("#w_grapple.md2");
	}
	//-------------------

	gi.soundindex ("player/gasp1.wav");		// gasping for air
	gi.soundindex ("player/gasp2.wav");		// head breaking surface, not gasping

	gi.soundindex ("player/watr_in.wav");	// feet hitting water
	gi.soundindex ("player/watr_out.wav");	// feet leaving water

	gi.soundindex ("player/watr_un.wav");	// head going underwater

	gi.soundindex ("mud/mud_in2.wav");		// feet hitting mud
	gi.soundindex ("mud/mud_out1.wav");		// feet leaving mud
	gi.soundindex ("mud/mud_un1.wav");		// head going under mud
	//gi.soundindex ("mud/wade_mud1.wav");
	//gi.soundindex ("mud/wade_mud2.wav");

	//Knightmare- moved these precaches to item data blocks
	//gi.soundindex ("player/u_breath1.wav");
	//gi.soundindex ("player/u_breath2.wav");

	gi.soundindex ("items/pkup.wav");		// bonus item pickup
	gi.soundindex ("world/land.wav");		// landing thud
	gi.soundindex ("misc/h2ohit1.wav");		// landing splash

	//gi.soundindex ("items/damage.wav");
	// ROGUE - double damage
	//gi.soundindex ("misc/ddamage1.wav");
	// rogue

	//gi.soundindex ("items/protect.wav");
	//gi.soundindex ("items/protect4.wav");
	gi.soundindex ("weapons/noammo.wav");

	// Knightmare- does this even get used in deathmatch?
	if (!deathmatch->value)
		gi.soundindex ("infantry/inflies1.wav");

	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex ("models/objects/gibs/arm/tris.md2");
	gi.modelindex ("models/objects/gibs/leg/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/gibs/skull/tris.md2");
	gi.modelindex ("models/objects/gibs/head2/tris.md2");

	// Fog clipping - if "fogclip" is non-zero, force gl_clear to a good
	// value for obscuring HOM with fog... "good" is driver-dependent
	if(ent->fogclip)
	{
		if(gl_driver && !Q_stricmp(gl_driver->string,"3dfxgl"))
			gi.cvar_forceset("gl_clear", "0");
		else
			gi.cvar_forceset("gl_clear", "1");
	}

	// FMOD 3D sound attenuation:
	if(ent->attenuation <= 0.)
		ent->attenuation = 1.0;

	// FMOD 3D sound Doppler shift:
	if(st.shift > 0)
		ent->moveinfo.distance = st.shift;
	else if(st.shift < 0)
		ent->moveinfo.distance = 0.0;
	else
		ent->moveinfo.distance = 1.0;

	// cvar overrides for effects flags:
	if (alert_sounds->value)
		world->effects |= FX_WORLDSPAWN_ALERTSOUNDS;
	if (corpse_fade->value)
		world->effects |= FX_WORLDSPAWN_CORPSEFADE;
	if (jump_kick->value)
		world->effects |= FX_WORLDSPAWN_JUMPKICK;
	if (footstep_sounds->value)
		world->effects |= FX_WORLDSPAWN_STEPSOUNDS;

//Knightmare- this is now handled client-side
#ifdef FMOD_FOOTSTEPS

	if (deathmatch->value || coop->value)
		qFMOD_Footsteps = false;
	else if (world->effects & FX_WORLDSPAWN_STEPSOUNDS)
	{
		qFMOD_Footsteps = true;
		FMOD_Init();
	}
	else
		qFMOD_Footsteps = false;
#endif
//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "m");
	
	// 1 FLICKER (first variety)
	gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	gi.configstring(CS_LIGHTS+4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");
}

//
//ROGUE
//

//
// Monster spawning code
//
// Used by the carrier, the medic_commander, and the black widow
//
// The sequence to create a flying monster is:
//
//  FindSpawnPoint - tries to find suitable spot to spawn the monster in
//  CreateFlyMonster  - this verifies the point as good and creates the monster

// To create a ground walking monster:
//
//  FindSpawnPoint - same thing
//  CreateGroundMonster - this checks the volume and makes sure the floor under the volume is suitable
//

// FIXME - for the black widow, if we want the stalkers coming in on the roof, we'll have to tweak some things

//
// CreateMonster
//
edict_t *CreateMonster(vec3_t origin, vec3_t angles, char *classname)
{
	edict_t		*newEnt;

	newEnt = G_Spawn();

	VectorCopy(origin, newEnt->s.origin);
	VectorCopy(angles, newEnt->s.angles);
	newEnt->classname = ED_NewString (classname);
	//newEnt->monsterinfo.aiflags |= AI_DO_NOT_COUNT;
	newEnt->monsterinfo.monsterflags |= MFL_DO_NOT_COUNT;

	VectorSet(newEnt->gravityVector, 0, 0, -1);
	ED_CallSpawn(newEnt);
	newEnt->s.renderfx |= RF_IR_VISIBLE;

	return newEnt;
}

edict_t *CreateFlyMonster (vec3_t origin, vec3_t angles, vec3_t mins, vec3_t maxs, char *classname)
{
	if (!mins || !maxs || VectorCompare (mins, vec3_origin) || VectorCompare (maxs, vec3_origin))
	{
		DetermineBBox (classname, mins, maxs);
	}

	if (!CheckSpawnPoint(origin, mins, maxs))
		return NULL;

	return (CreateMonster (origin, angles, classname));
}

// This is just a wrapper for CreateMonster that looks down height # of CMUs and sees if there
// are bad things down there or not
//
// this is from m_move.c
#define	STEPSIZE	18

edict_t *CreateGroundMonster (vec3_t origin, vec3_t angles, vec3_t entMins, vec3_t entMaxs, char *classname, int height)
{
//	trace_t		tr;
	edict_t		*newEnt;
//	vec3_t		start, stop;
//	int			failure = 0;
//	vec3_t		mins, maxs;
//	int			x, y;	
//	float		mid, bottom;
	vec3_t		mins, maxs;

	// if they don't provide us a bounding box, figure it out
	if (!entMins || !entMaxs || VectorCompare (entMins, vec3_origin) || VectorCompare (entMaxs, vec3_origin))
	{
		DetermineBBox (classname, mins, maxs);
	}
	else
	{
		VectorCopy (entMins, mins);
		VectorCopy (entMaxs, maxs);
	}

	// check the ground to make sure it's there, it's relatively flat, and it's not toxic
	if (!CheckGroundSpawnPoint(origin, mins, maxs, height, -1))
		return NULL;

	newEnt = CreateMonster (origin, angles, classname);
	if (!newEnt)
		return NULL;

	return newEnt;
}


// FindSpawnPoint
// PMM - this is used by the medic commander (possibly by the carrier) to find a good spawn point
// if the startpoint is bad, try above the startpoint for a bit

qboolean FindSpawnPoint (vec3_t startpoint, vec3_t mins, vec3_t maxs, vec3_t spawnpoint, float maxMoveUp)
{
	trace_t		tr;
	float		height;
	vec3_t		top;

	height = maxs[2] - mins[2];

	tr = gi.trace (startpoint, mins, maxs, startpoint, NULL, MASK_MONSTERSOLID|CONTENTS_PLAYERCLIP);
	if((tr.startsolid || tr.allsolid) || (tr.ent != world))
	{
//		if ( ((tr.ent->svflags & SVF_MONSTER) && (tr.ent->health <= 0)) ||
//			 (tr.ent->svflags & SVF_DAMAGEABLE) )
//		{
//			T_Damage (tr.ent, self, self, vec3_origin, self->enemy->s.origin,
//						pain_normal, hurt, 0, 0, MOD_UNKNOWN);

		VectorCopy (startpoint, top);
		top[2] += maxMoveUp;
/*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (top);
		gi.WritePosition (startpoint);
		gi.multicast (startpoint, MULTICAST_ALL);	
*/
		tr = gi.trace (top, mins, maxs, startpoint, NULL, MASK_MONSTERSOLID);
		if (tr.startsolid || tr.allsolid)
		{
//			if ((g_showlogic) && (g_showlogic->value))
//				if (tr.ent)
//					gi.dprintf("FindSpawnPoint: failed to find a point -- blocked by %s\n", tr.ent->classname);
//				else
//					gi.dprintf("FindSpawnPoint: failed to find a point\n");

			return false;
		} 
		else
		{
//			if ((g_showlogic) && (g_showlogic->value))
//				gi.dprintf ("FindSpawnPoint: %s -> %s\n", vtos (startpoint), vtos (tr.endpos));
			VectorCopy (tr.endpos, spawnpoint);
			return true;
		}
	}
	else
	{
		VectorCopy (startpoint, spawnpoint);
		return true;
	}
}

// FIXME - all of this needs to be tweaked to handle the new gravity rules
// if we ever want to spawn stuff on the roof

//
// CheckSpawnPoint
//
// PMM - checks volume to make sure we can spawn a monster there (is it solid?)
//
// This is all fliers should need

qboolean CheckSpawnPoint (vec3_t origin, vec3_t mins, vec3_t maxs)
{
	trace_t	tr;

	if (!mins || !maxs || VectorCompare(mins, vec3_origin) || VectorCompare (maxs, vec3_origin))
	{
		return false;
	}

	tr = gi.trace (origin, mins, maxs, origin, NULL, MASK_MONSTERSOLID);
	if(tr.startsolid || tr.allsolid)
	{
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf("createmonster in wall. removing\n");
		return false;
	}
	if (tr.ent != world)
	{
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf("createmonster in entity %s\n", tr.ent->classname);
		return false;
	}	
	return true;
}

//
// CheckGroundSpawnPoint
//
// PMM - used for walking monsters
//  checks:
//		1)	is there a ground within the specified height of the origin?
//		2)	is the ground non-water?
//		3)	is the ground flat enough to walk on?
//

qboolean CheckGroundSpawnPoint (vec3_t origin, vec3_t entMins, vec3_t entMaxs, float height, float gravity)
{
	trace_t		tr;
	vec3_t		start, stop;
	int			failure = 0;
	vec3_t		mins, maxs;
	int			x, y;	
	float		mid, bottom;

	if (!CheckSpawnPoint (origin, entMins, entMaxs))
		return false;

	// FIXME - this is too conservative about angled surfaces

	VectorCopy (origin, stop);
	// FIXME - gravity vector
	stop[2] = origin[2] + entMins[2] - height;

	/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (origin);
	gi.WritePosition (stop);
	gi.multicast (start, MULTICAST_ALL);
	*/

	tr = gi.trace (origin, entMins, entMaxs, stop, NULL, MASK_MONSTERSOLID | MASK_WATER);
	// it's not going to be all solid or start solid, since that's checked above

	if ((tr.fraction < 1) && (tr.contents & MASK_MONSTERSOLID))
	{
		// we found a non-water surface down there somewhere.  now we need to check to make sure it's not too sloped
		//
		// algorithm straight out of m_move.c:M_CheckBottom()
		//

		// first, do the midpoint trace

		VectorAdd (tr.endpos, entMins, mins);
		VectorAdd (tr.endpos, entMaxs, maxs);


		// first, do the easy flat check
		//
#ifdef ROGUE_GRAVITY
		// FIXME - this will only handle 0,0,1 and 0,0,-1 gravity vectors
		if(gravity > 0)
			start[2] = maxs[2] + 1;
		else
			start[2] = mins[2] - 1;
#else
		start[2] = mins[2] - 1;
#endif
		for	(x=0 ; x<=1 ; x++)
		{
			for	(y=0 ; y<=1 ; y++)
			{
				start[0] = x ? maxs[0] : mins[0];
				start[1] = y ? maxs[1] : mins[1];
				if (gi.pointcontents (start) != CONTENTS_SOLID)
					goto realcheck;
			}
		}

		// if it passed all four above checks, we're done
		return true;

realcheck:

		// check it for real

		start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
		start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
		start[2] = mins[2];

		tr = gi.trace (start, vec3_origin, vec3_origin, stop, NULL, MASK_MONSTERSOLID);

		if (tr.fraction == 1.0)
			return false;
		mid = bottom = tr.endpos[2];

#ifdef ROGUE_GRAVITY
		if(gravity < 0)
		{
			start[2] = mins[2];
			stop[2] = start[2] - STEPSIZE - STEPSIZE;
			mid = bottom = tr.endpos[2] + entMins[2];
		}
		else
		{
			start[2] = maxs[2];
			stop[2] = start[2] + STEPSIZE + STEPSIZE;
			mid = bottom = tr.endpos[2] - entMaxs[2];
		}
#else
		stop[2] = start[2] - 2*STEPSIZE;
		mid = bottom = tr.endpos[2] + entMins[2];
#endif

		for	(x=0 ; x<=1 ; x++)
			for	(y=0 ; y<=1 ; y++)
			{
				start[0] = stop[0] = x ? maxs[0] : mins[0];
				start[1] = stop[1] = y ? maxs[1] : mins[1];
				
				/*
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_DEBUGTRAIL);
				gi.WritePosition (start);
				gi.WritePosition (stop);
				gi.multicast (start, MULTICAST_ALL);	
				*/
				tr = gi.trace (start, vec3_origin, vec3_origin, stop, NULL, MASK_MONSTERSOLID);

//PGM
#ifdef ROGUE_GRAVITY
// FIXME - this will only handle 0,0,1 and 0,0,-1 gravity vectors
				if(gravity > 0)
				{
					if (tr.fraction != 1.0 && tr.endpos[2] < bottom)
						bottom = tr.endpos[2];
					if (tr.fraction == 1.0 || tr.endpos[2] - mid > STEPSIZE)
					{
//						if ((g_showlogic) && (g_showlogic->value))
//							gi.dprintf ("spawn - rejecting due to uneven ground\n");
						return false;
					}
				}
				else
				{
					if (tr.fraction != 1.0 && tr.endpos[2] > bottom)
						bottom = tr.endpos[2];
					if (tr.fraction == 1.0 || mid - tr.endpos[2] > STEPSIZE)
					{
//						if ((g_showlogic) && (g_showlogic->value))
//							gi.dprintf ("spawn - rejecting due to uneven ground\n");
						return false;
					}
				}
#else
				if (tr.fraction != 1.0 && tr.endpos[2] > bottom)
					bottom = tr.endpos[2];
				if (tr.fraction == 1.0 || mid - tr.endpos[2] > STEPSIZE)
					{
						return false;
					}
#endif
			}

		return true;		// we can land on it, it's ok
	}

	// otherwise, it's either water (bad) or not there (too far)
	// if we're here, it's bad below
//	if ((g_showlogic) && (g_showlogic->value))
//	{
//		if (tr.fraction < 1)
//			if ((g_showlogic) && (g_showlogic->value))
//				gi.dprintf("groundmonster would fall into water/slime/lava\n");
//		else
//			if ((g_showlogic) && (g_showlogic->value))
//				gi.dprintf("groundmonster would fall too far\n");
//	}

	return false;
}

void DetermineBBox (char *classname, vec3_t mins, vec3_t maxs)
{
	// FIXME - cache this stuff
	edict_t		*newEnt;

	newEnt = G_Spawn();

	VectorCopy(vec3_origin, newEnt->s.origin);
	VectorCopy(vec3_origin, newEnt->s.angles);
	newEnt->classname = ED_NewString (classname);
	//newEnt->monsterinfo.aiflags |= AI_DO_NOT_COUNT;
	newEnt->monsterinfo.monsterflags |= MFL_DO_NOT_COUNT;

	ED_CallSpawn(newEnt);
	
	VectorCopy (newEnt->mins, mins);
	VectorCopy (newEnt->maxs, maxs);

	G_FreeEdict (newEnt);
}

// ****************************
// SPAWNGROW stuff
// ****************************

#define SPAWNGROW_LIFESPAN		0.3

void spawngrow_think (edict_t *self)
{
	int i;

	for (i=0; i<2; i++)
	{
			self->s.angles[0] = rand()%360;
			self->s.angles[1] = rand()%360;
			self->s.angles[2] = rand()%360;
	}
	if ((level.time < self->wait) && (self->s.frame < 2))
		self->s.frame++;
	if (level.time >= self->wait)
	{
		if (self->s.effects & EF_SPHERETRANS)
		{
			G_FreeEdict (self);
			return;
		}
		else if (self->s.frame > 0)
			self->s.frame--;
		else
		{
			G_FreeEdict (self);
			return;
		}
	}
	self->nextthink += FRAMETIME;
}

void SpawnGrow_Spawn (vec3_t startpos, int size)
{
	edict_t *ent;
	int	i;
	float	lifespan;

	ent = G_Spawn();
	VectorCopy(startpos, ent->s.origin);
	for (i=0; i<2; i++)
	{
			ent->s.angles[0] = rand()%360;
			ent->s.angles[1] = rand()%360;
			ent->s.angles[2] = rand()%360;
	}
	ent->solid = SOLID_NOT;
//	ent->s.renderfx = RF_FULLBRIGHT | RF_IR_VISIBLE;
	ent->s.renderfx = RF_IR_VISIBLE;
	ent->movetype = MOVETYPE_NONE;
	ent->classname = "spawngro";

	if (size <= 1)
	{
		lifespan = SPAWNGROW_LIFESPAN;
		ent->s.modelindex = gi.modelindex("models/items/spawngro2/tris.md2");
	}
	else if (size == 2)
	{
		ent->s.modelindex = gi.modelindex("models/items/spawngro3/tris.md2");
		lifespan = 2;
	}
	else
	{
		ent->s.modelindex = gi.modelindex("models/items/spawngro/tris.md2");
		lifespan = SPAWNGROW_LIFESPAN;
	}

	ent->think = spawngrow_think;

	ent->wait = level.time + lifespan;
	ent->nextthink = level.time + FRAMETIME;
	if (size != 2)
		ent->s.effects |= EF_SPHERETRANS;
	gi.linkentity (ent);
}


// ****************************
// WidowLeg stuff
// ****************************

#define	MAX_LEGSFRAME	23
#define	LEG_WAIT_TIME	1

void ThrowMoreStuff (edict_t *self, vec3_t point);
void ThrowSmallStuff (edict_t *self, vec3_t point);
void ThrowWidowGibLoc (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, qboolean fade);
void ThrowWidowGibSized (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, int hitsound, qboolean fade);

void widowlegs_think (edict_t *self)
{
	vec3_t	offset;
	vec3_t	point;
	vec3_t	f,r,u;

	if (self->s.frame == 17)
	{
		VectorSet (offset, 11.77, -7.24, 23.31);
		AngleVectors (self->s.angles, f, r, u);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);
	}

	if (self->s.frame < MAX_LEGSFRAME)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		return;
	}
	else if (self->wait == 0)
	{
		self->wait = level.time + LEG_WAIT_TIME;
	}
	if (level.time > self->wait)
	{
		AngleVectors (self->s.angles, f, r, u);

		VectorSet (offset, -65.6, -8.44, 28.59);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);

		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib1/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib2/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);

		VectorSet (offset, -1.04, -51.18, 7.04);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);

		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib1/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib2/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib3/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);

		G_FreeEdict (self);
		return;
	}
	if ((level.time > (self->wait - 0.5)) && (self->count == 0))
	{
		self->count = 1;
		AngleVectors (self->s.angles, f, r, u);

		VectorSet (offset, 31, -88.7, 10.96);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
//		ThrowSmallStuff (self, point);

		VectorSet (offset, -12.67, -4.39, 15.68);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
//		ThrowSmallStuff (self, point);

		self->nextthink = level.time + FRAMETIME;
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void Widowlegs_Spawn (vec3_t startpos, vec3_t angles)
{
	edict_t *ent;

	ent = G_Spawn();
	VectorCopy(startpos, ent->s.origin);
	VectorCopy(angles, ent->s.angles);
	ent->solid = SOLID_NOT;
	ent->s.renderfx = RF_IR_VISIBLE;
	ent->movetype = MOVETYPE_NONE;
	ent->classname = "widowlegs";

	ent->s.modelindex = gi.modelindex("models/monsters/legs/tris.md2");
	ent->think = widowlegs_think;

	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}

// Hud toggle ripped from TPP source

int nohud = 0;

void Hud_On()
{
	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);
	nohud = 0;
}

void Hud_Off()
{
	gi.configstring (CS_STATUSBAR, NULL);
	nohud = 1;
}

void Cmd_ToggleHud ()
{
	if (deathmatch->value)
		return;
  if (nohud)
		Hud_On();
	else
		Hud_Off();
}