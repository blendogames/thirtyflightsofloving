# Makefile for Thirty Flights of Loving Game DLL
# Written by Ethan "flibitijibibo" Lee

ifeq ($(shell uname), Darwin)
	SUFFIX = dylib
else
	SUFFIX = so
	LDFLAGS := -Wl,--no-undefined
endif

TFOL_SRC = \
	g_abel_camshoot.c \
	g_abel_civspawner.c \
	g_abel_dancespawner.c \
	g_abel_floatspawner.c \
	g_abel_gunarm.c \
	g_abel_items.c \
	g_abel_look.c \
	g_abel_monster.c \
	g_abel_script.c \
	g_abel_target.c \
	g_ai.c \
	g_camera.c \
	g_chase.c \
	g_cmds.c \
	g_combat.c \
	g_crane.c \
	g_ctf.c \
	g_fog.c \
	g_func.c \
	g_items.c \
	g_jetpack.c \
	g_lights.c \
	g_lock.c \
	g_main.c \
	g_misc.c \
	g_model.c \
	g_monster.c \
	g_moreai.c \
	g_mtrain.c \
	g_patchplayermodels.c \
	g_pendulum.c \
	g_phys.c \
	g_reflect.c \
	g_save.c \
	g_spawn.c \
	g_svcmds.c \
	g_target.c \
	g_thing.c \
	g_tracktrain.c \
	g_trigger.c \
	g_turret.c \
	g_utils.c \
	g_vehicle.c \
	g_weapon.c \
	km_cvar.c \
	m_actor.c \
	m_actor_weap.c \
	m_berserk.c \
	m_boss2.c \
	m_boss3.c \
	m_boss31.c \
	m_boss32.c \
	m_brain.c \
	m_chick.c \
	m_flash.c \
	m_flipper.c \
	m_float.c \
	m_flyer.c \
	m_gladiator.c \
	m_gunner.c \
	m_hover.c \
	m_infantry.c \
	m_insane.c \
	m_medic.c \
	m_move.c \
	m_mutant.c \
	m_parasite.c \
	m_soldier.c \
	m_supertank.c \
	m_tank.c \
	p_chase.c \
	p_client.c \
	p_hud.c \
	p_menu.c \
	p_text.c \
	p_trail.c \
	p_view.c \
	p_weapon.c \
	q_shared.c \
	acesrc/acebot_ai.c \
	acesrc/acebot_cmds.c \
	acesrc/acebot_compress.c \
	acesrc/acebot_items.c \
	acesrc/acebot_movement.c \
	acesrc/acebot_nodes.c \
	acesrc/acebot_spawn.c

all:
	mkdir -p baseq2
	$(CC) -o baseq2/kmq2gamex64.$(SUFFIX) -g -w $(LDFLAGS) -fpic -fPIC -shared $(TFOL_SRC) -lSDL2 -lm
	cp ../quake2/kmquake2 .

clean:
	rm -f baseq2/kmq2gamex86_64.so
