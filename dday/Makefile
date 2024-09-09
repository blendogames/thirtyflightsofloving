#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/Makefile,v $
# *   $Revision: 1.6 $
# *   $Date: 2002/07/25 09:07:45 $
# *
# ***********************************
#
# Copyright (C) 2002 Vipersoft
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# 
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

#ARCH=i386
ARCH=x86_64
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp

#CFLAGS_RELEASE=$(BASE_CFLAGS) -S

# Nick - this next line now builds correctly for Linux
# using 'make build_release'.
# This top level Makefile will build all Teams if you need (see below).

CFLAGS_RELEASE=$(BASE_CFLAGS) -O3 -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

# Nick - Use 'make build_debug' to use this.
CFLAGS_DEBUG=$(BASE_CFLAGS) -g

# Always uncommented
LDFLAGS=-ldl -lm
SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

build_debug:
	$(MAKE) game$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) game$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS =	\
	g_ai.o \
	g_arty.o \
	g_cmds.o \
	g_combat.o \
	g_ents.o \
	g_func.o \
	g_items.o \
	g_main.o \
	g_maps.o \
	g_misc.o \
	g_monster.o \
	g_objectives.o \
	g_phys.o \
	g_save.o \
	g_spawn.o \
	g_svcmds.o \
	g_target.o \
	g_trigger.o \
	g_turret.o \
	g_utils.o \
	g_weapon.o \
	m_actor.o \
	m_civilian.o \
	m_flash.o \
	m_insane.o \
	m_move.o \
	p_classes.o \
	p_client.o \
	p_generic_wep.o \
	p_hud.o \
	p_menu.o \
	p_observer.o \
	p_trail.o \
	p_view.o \
	p_weapon.o \
	q_shared.o \
	u_entmgr.o \
	x_fbomb.o \
	x_fire.o \
	usa/usa_item.o  usa/usa_spawn.o usa/usa_guninfo.o usa/usa_weapon.o usa/usa_main.o \
        grm/grm_item.o  grm/grm_spawn.o grm/grm_guninfo.o grm/grm_weapon.o grm/grm_main.o \
        rus/rus_item.o  rus/rus_spawn.o rus/rus_guninfo.o rus/rus_weapon.o rus/rus_main.o \
        gbr/gbr_item.o  gbr/gbr_spawn.o gbr/gbr_guninfo.o gbr/gbr_weapon.o gbr/gbr_main.o \
        jpn/jpn_item.o  jpn/jpn_spawn.o jpn/jpn_guninfo.o jpn/jpn_weapon.o jpn/jpn_main.o \
        usm/usm_item.o  usm/usm_spawn.o usm/usm_guninfo.o usm/usm_weapon.o usm/usm_main.o \
        ita/ita_item.o  ita/ita_spawn.o ita/ita_guninfo.o ita/ita_weapon.o ita/ita_main.o \
        pol/pol_item.o  pol/pol_spawn.o pol/pol_guninfo.o pol/pol_weapon.o pol/pol_main.o \
 	ai/bot_common.o ai/bot_spawn.o ai/ai_weapons.o ai/ai_tools.o \
	ai/ai_nodes.o ai/ai_navigation.o ai/ai_movement.o ai/ai_links.o  \
	ai/ai_items.o ai/ai_dropnodes.o ai/ai_class_dmbot.o \
	ai/ai_class_monster_default.o ai/AStar.o ai/ai_main.o



#	g_so.o # this file is a shared library thingie needed for Solaris
game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(MAKE) usa$(ARCH).$(SHLIBEXT) -C usa
	$(MAKE) grm$(ARCH).$(SHLIBEXT) -C grm
# Nick - Uncomment out the next two lines if you wish to build GBR & RUS source
	$(MAKE) rus$(ARCH).$(SHLIBEXT) -C rus	
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) -C gbr	

	$(MAKE) jpn$(ARCH).$(SHLIBEXT) -C jpn	
	$(MAKE) usm$(ARCH).$(SHLIBEXT) -C usm

	$(MAKE) pol$(ARCH).$(SHLIBEXT) -C pol
	$(MAKE) ita$(ARCH).$(SHLIBEXT) -C ita

#ai
	$(MAKE) ai$(ARCH).$(SHLIBEXT) -C ai

	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)
	$(MAKE) clean -C usa
	$(MAKE) clean -C grm
	$(MAKE) clean -C rus
	$(MAKE) clean -C gbr
	$(MAKE) clean -C jpn
	$(MAKE) clean -C usm
	$(MAKE) clean -C pol
	$(MAKE) clean -C ita

	$(MAKE) clean -C ai


depend:
	gcc -MM $(GAME_OBJS:.o=.c)
g_ai.o: g_ai.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_arty.o: g_arty.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_cmds.o: g_cmds.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_player.h g_cmds.h
g_combat.o: g_combat.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_dll.linux.o: g_dll.linux.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h u_findfunc.h 
g_ents.o: g_ents.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_feeder.o: g_feeder.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_func.o: g_func.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_items.o: g_items.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_main.o: g_main.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h 
g_maps.o: g_maps.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_misc.o: g_misc.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fire.h
g_monster.o: g_monster.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h x_fire.h
g_objectives.o: g_objectives.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_phys.o: g_phys.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_save.o: g_save.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_spawn.o: g_spawn.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_svcmds.o: g_svcmds.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_target.o: g_target.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_trigger.o: g_trigger.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_turret.o: g_turret.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h x_fire.h
g_utils.o: g_utils.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_weapon.o: g_weapon.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
m_actor.o: m_actor.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_actor.h
m_flash.o: m_flash.c q_shared.h
m_insane.o: m_insane.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_insane.h
m_move.o: m_move.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_classes.o: p_classes.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h p_classes.h
p_client.o: p_client.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_player.h g_cmds.h x_fire.h
p_generic_wep.o: p_generic_wep.c g_local.h g_defines.h q_shared.h \
 game.h p_menu.h g_maps.h m_player.h
p_hud.o: p_hud.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_menu.o: p_menu.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_observer.o: p_observer.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
p_trail.o: p_trail.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_view.o: p_view.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_player.h
p_weapon.o: p_weapon.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_player.h x_fbomb.h x_fire.h
q_shared.o: q_shared.c q_shared.h
u_entmgr.o: u_entmgr.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h 
x_fbomb.o: x_fbomb.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fbomb.h x_fire.h
x_fire.o: x_fire.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fire.h

###adding bots to makefile
#AStar.o: AStar.c                     
#ai_items.c     
#ai_items.c     
#ai_navigation.c  
#bot_common.c
#ai_class_dmbot.c            
#ai_links.c     
#ai_nodes.c       
#bot_spawn.c
#ai_class_monster_default.c  
#ai_main.c      
#ai_tools.c
#ai_dropnodes.c              
#ai_movement.c  
#ai_weapons.c
