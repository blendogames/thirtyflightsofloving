#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/ita/Makefile,v $
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
	$(MAKE) ita$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) ita$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = ita_item.o ita_spawn.o ita_guninfo.o ita_weapon.o ita_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

ita$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)
ita_item.o: ita_item.c ita_main.h ita_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
ita_spawn.o: ita_spawn.c ita_main.h ita_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
ita_guninfo.o: ita_guninfo.c ita_main.h ita_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
ita_weapon.o: ita_weapon.c ita_main.h ita_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
ita_main.o: ita_main.c ita_main.h ita_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
 ../game.h ../p_menu.h ../g_cmds.h  ../p_classes.h
