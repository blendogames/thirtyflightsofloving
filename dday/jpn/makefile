#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/jpn/Makefile,v $
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

CFLAGS_RELEASE=$(BASE_CFLAGS) -g -mpentiumpro -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

CFLAGS_DEBUG=$(BASE_CFLAGS) -g 

# Always uncommented
LDFLAGS=-ldl -lm
SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

build_debug:
	$(MAKE) jpn$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) jpn$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = jpn_item.o jpn_spawn.o jpn_guninfo.o jpn_weapon.o jpn_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

jpn$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)
jpn_item.o: jpn_item.c jpn_main.h jpn_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
jpn_spawn.o: jpn_spawn.c jpn_main.h jpn_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
jpn_guninfo.o: jpn_guninfo.c jpn_main.h jpn_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
jpn_weapon.o: jpn_weapon.c jpn_main.h jpn_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h 
jpn_main.o: jpn_main.c jpn_main.h jpn_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
 ../game.h ../p_menu.h ../g_cmds.h  ../p_classes.h
