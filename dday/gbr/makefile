#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/gbr/Makefile,v $
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
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = gbr_item.o gbr_spawn.o gbr_guninfo.o gbr_weapon.o gbr_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

gbr$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)
gbr_item.o: gbr_item.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
gbr_spawn.o: gbr_spawn.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
gbr_guninfo.o: gbr_guninfo.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
gbr_weapon.o: gbr_weapon.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
gbr_main.o: gbr_main.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
 ../game.h ../p_menu.h ../g_cmds.h  ../p_classes.h
