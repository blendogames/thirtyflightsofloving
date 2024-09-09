#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/ai/Makefile,v $
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, ai.
#

ARCH=x86_64
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp -I..

#CFLAGS_RELEASE=$(BASE_CFLAGS) -O1

# Nick - fixed 'make build_release' flags.

CFLAGS_RELEASE=$(BASE_CFLAGS) -O3 -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
# Nick - fixed 'make build_debug' flags.
CFLAGS_DEBUG=$(BASE_CFLAGS) -g

# Always uncommented
LDFLAGS=-ldl -lm
SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
 
build_debug:
	$(MAKE) ai$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) ai$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = bot_common.o bot_spawn.o ai_weapons.o ai_tools.o ai_nodes.o \
 ai_navigation.o ai_movement.o ai_links.o ai_items.o ai_dropnodes.o \
 ai_class_dmbot.o ai_class_monster_default.o AStar.o ai_main.o
 

#ai_item.o ai_spawn.o ai_guninfo.o ai_weapon.o ai_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

ai$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)


bot_common.o: bot_common.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_class_dmbot.o: ai_class_dmbot.c ../g_local.h ../g_defines.h \
  ../q_shared.h ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h \
  ../g_maps.h ../p_observer.h ai_local.h ai_nodes_local.h \
  ai_nodes_shared.h ai_weapons.h
AStar.o: AStar.h AStar.c ../g_local.h ../g_defines.h ../q_shared.h ../game.h \
  ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h ../p_observer.h \
  ai_local.h ai_nodes_local.h ai_nodes_shared.h ai_weapons.h
ai_class_monster_default.o: ai_class_monster_default.c ../g_local.h \
  ../g_defines.h ../q_shared.h ../game.h ../ai/ai.h ../ai/AStar.h \
  ../p_menu.h ../g_maps.h ../p_observer.h ai_local.h ai_nodes_local.h \
  ai_nodes_shared.h ai_weapons.h
ai_dropnodes.o: ai_dropnodes.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_items.o: ai_items.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_links.o: ai_links.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_main.o: ai_main.c ../g_local.h ../g_defines.h ../q_shared.h ../game.h \
  ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h ../p_observer.h \
  ai_local.h ai_nodes_local.h ai_nodes_shared.h ai_weapons.h
ai_movement.o: ai_movement.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_navigation.o: ai_navigation.c ../g_local.h ../g_defines.h \
  ../q_shared.h ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h \
  ../g_maps.h ../p_observer.h ai_local.h ai_nodes_local.h \
  ai_nodes_shared.h ai_weapons.h
ai_nodes.o: ai_nodes.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_tools.o: ai_tools.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
ai_weapons.o: ai_weapons.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
bot_common.o: bot_common.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
bot_spawn.o: bot_spawn.c ../g_local.h ../g_defines.h ../q_shared.h \
  ../game.h ../ai/ai.h ../ai/AStar.h ../p_menu.h ../g_maps.h \
  ../p_observer.h ai_local.h ai_nodes_local.h ai_nodes_shared.h \
  ai_weapons.h
