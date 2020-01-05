IF EXIST extractfuncs.exe GOTO local

.\extractfuncs\extractfuncs *.c -o g_func_list.h g_func_decs.h
.\extractfuncs\extractfuncs *.c -t mmove_t -o g_mmove_list.h g_mmove_decs.h
GOTO end

:local
cd ..
.\extractfuncs\extractfuncs *.c -o g_func_list.h g_func_decs.h
.\extractfuncs\extractfuncs *.c -t mmove_t -o g_mmove_list.h g_mmove_decs.h
:end

pause

