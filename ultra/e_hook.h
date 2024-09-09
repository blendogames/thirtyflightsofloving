
/*---- ABANDON MOD 4/25/99 10:11:35 AM JMC ----*/

#define HOOK_SPEED_DEFAULT						"750"	// How fast the hook pulls
#define HOOK_FIRE_SPEED_DEFAULT					"800"	// How fast the hook fires
#define	HOOK_INIT_DAMAGE_DEFAULT				"1"		// Hook initial hit damage
#define HOOK_DAMAGE_DEFAULT						"10"	// Hook sustained damage
#define HOOK_DIAM_DEFAULT						"2"		// Hook diameter

/*---- ABANDON MOD 4/25/99 10:11:38 AM JMC ----*/


void abandon_hook_laser_think (edict_t *self);			// Hook laser think routine 
												// This move the two ends of the laser 
												// beam to the proper positions

edict_t *abandon_hook_laser_start (edict_t *ent);		// create a laser and return a pointer to it

void abandon_hook_reset (edict_t *rhook);				// reset the hook.  pull all entities out of
												// the world and reset the clients weapon state

qboolean hook_cond_reset(edict_t *self);		// resets the hook if it needs to be

void abandon_hook_service (edict_t *self);				// Do all the service hook crap (move client, release etc)

void abandon_hook_track (edict_t *self);				// keeps the invisible hook entity on 
												// hook->enemy (can be world or an entity)

void abandon_hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf); // the hook has hit something

void abandon_fire_hook (edict_t *owner, vec3_t start, vec3_t forward);	// creates the invisible hook entity 
																// and sends it on its way attaches a laser to it

void abandon_hook_fire (edict_t *ent, qboolean altfire);		// a call has been made to fire the hook

//void Weapon_Hook (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

void abandon_Weapon_Hook (edict_t *ent);				/* boring service routine*/

void stuffcmd(edict_t *e, char *s);				/* Stuffs a command to the client 
												 (just like typing in the console)*/

