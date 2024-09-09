#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/rus/Makefile,v $
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

# Nick - fixed 'make build_release flags.

CFLAGS_RELEASE=$(BASE_CFLAGS) -O3 -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

# Nick - 'make build_debug' flags.
CFLAGS_DEBUG=$(BASE_CFLAGS) -g

# Always uncommented
LDFLAGS=-ldl -lm
SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
 
build_debug:
	$(MAKE) rus$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) rus$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = rus_item.o rus_spawn.o rus_guninfo.o rus_weapon.o rus_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

rus$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)
rus_item.o: rus_item.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
rus_spawn.o: rus_spawn.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
rus_guninfo.o: rus_guninfo.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
rus_weapon.o: rus_weapon.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
rus_main.o: rus_main.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
 ../game.h ../p_menu.h ../g_cmds.h  ../p_classes.h
