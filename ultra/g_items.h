qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean	Pickup_Health (edict_t *ent, edict_t *other);
qboolean	Pickup_Ammo (edict_t *ent, edict_t *other);
qboolean	Pickup_Armor (edict_t *ent, edict_t *other);

edict_t	*AddToItemList(edict_t *ent, edict_t	*head);

extern int	jacket_armor_index;
extern int	combat_armor_index;
extern int	body_armor_index;
extern int	power_screen_index;
extern int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2
