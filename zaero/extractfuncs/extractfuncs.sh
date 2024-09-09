#!/bin/bash
f=`dirname $0`
cd $f
cd ..
./extractfuncs/extractfuncs_i386 *.c -o g_func_list.h g_func_decs.h
./extractfuncs/extractfuncs_i386 *.c -t mmove_t -o g_mmove_list.h g_mmove_decs.h

read -p "Press Enter to close" -t 3600

