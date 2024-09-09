/* Runes prototypes + defines */
extern	cvar_t	*use_runes;          // enables runes

extern	cvar_t	*use_lithiumrunes;   // enaable lithium style runes, otherwise CTF Techs

extern	cvar_t	*rune_flags;         // determines which runes will show in the game, add these:
//                                   1 = resist, 2 = strength, 4 = haste, 8 = regen, 16 = vampire
extern	cvar_t	*rune_spawn;         // chance a rune will spawn from another item respawning
extern	cvar_t	*rune_perplayer;     // sets runes per player that will appear in map
extern	cvar_t	*rune_life;          // seconds a rune will stay around before disappearing
extern	cvar_t	*rune_min;           // sets minimum number of runes to be in the game
extern	cvar_t	*rune_max;           // sets maximum number of runes to be in the game

// customisable names for runes
extern	cvar_t	*rune_resist_name;
extern	cvar_t	*rune_strength_name;
extern	cvar_t	*rune_regen_name;
extern	cvar_t	*rune_haste_name;
extern	cvar_t	*rune_vampire_name;
extern	cvar_t	*rune_ammogen_name;

extern	cvar_t	*rune_resist_shortname;
extern	cvar_t	*rune_strength_shortname;
extern	cvar_t	*rune_regen_shortname;
extern	cvar_t	*rune_haste_shortname;
extern	cvar_t	*rune_vampire_shortname;
extern	cvar_t	*rune_ammogen_shortname;

// customisable md2's for runes
extern	cvar_t	*rune_resist_mdl;
extern	cvar_t	*rune_strength_mdl;
extern	cvar_t	*rune_regen_mdl;
extern	cvar_t	*rune_haste_mdl;
extern	cvar_t	*rune_vampire_mdl;
extern	cvar_t	*rune_ammogen_mdl;

//customisable icons for runes
extern	cvar_t	*rune_resist_icon;
extern	cvar_t	*rune_strength_icon;
extern	cvar_t	*rune_regen_icon;
extern	cvar_t	*rune_haste_icon;
extern	cvar_t	*rune_vampire_icon;
extern	cvar_t	*rune_ammogen_icon;

// customisable sounds for runes
extern	cvar_t	*rune_resist_sound;
extern	cvar_t	*rune_strength_sound;
extern	cvar_t	*rune_regen_sound;
extern	cvar_t	*rune_haste_sound;
extern	cvar_t	*rune_vampire_sound;
extern	cvar_t	*rune_ammogen_sound;

extern	cvar_t	*rune_haste; // what should I use this for?
extern	cvar_t	*rune_resist;        // sets how much damage is divided by with resist rune
extern	cvar_t	*rune_strength;      // sets how much damage is multiplied by with strength rune
extern	cvar_t	*rune_regen;         // sets how fast health is gained back
extern	cvar_t	*rune_regen_armor;
extern	cvar_t	*rune_regen_health_max;      // sets maximum health that can be gained from regen rune
extern	cvar_t	*rune_regen_armor_max;      // sets maximum armor that can be gained from regen rune
extern	cvar_t	*rune_regen_armor_always;      // sets whether armor should be regened regardless of if currently held

extern	cvar_t	*rune_vampire;       // sets percentage of health gained from damage inflicted
extern	cvar_t	*rune_vampiremax;    // sets maximum health that can be gained from vampire rune


void lithium_setuprunes (void);
void lithium_runecolour (gitem_t *item, edict_t *ent);
