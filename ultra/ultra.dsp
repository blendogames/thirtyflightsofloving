# Microsoft Developer Studio Project File - Name="ultra" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ultra - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ultra.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ultra.mak" CFG="ultra - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ultra - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ultra - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ultra - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /D "_WINDLL" /D "ERASER" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /profile /map /debug /machine:I386 /out:".\gamex86.dll"

!ELSEIF  "$(CFG)" == "ultra - Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "game___W"
# PROP BASE Intermediate_Dir "game___W"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /D "_WINDLL" /D "ERASER" /YX /FD /c
# ADD CPP /nologo /G5 /ML /W3 /GX /O2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /D "_WINDLL" /D "ERASER" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /profile /map:"../gamex86.map" /debug /machine:I386 /out:"../gamex86.dll"
# ADD LINK32 winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /pdb:none /machine:I386 /out:"./gamex86.dll"
# SUBTRACT LINK32 /profile /map /debug

!ENDIF 

# Begin Target

# Name "ultra - Win32 Debug"
# Name "ultra - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "EraserULTRA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aj_banning.c
# End Source File
# Begin Source File

SOURCE=.\aj_confcycle.c
# End Source File
# Begin Source File

SOURCE=.\aj_cvar.c
# End Source File
# Begin Source File

SOURCE=.\aj_lmctf.c
# End Source File
# Begin Source File

SOURCE=.\aj_main.c
# End Source File
# Begin Source File

SOURCE=.\aj_menu.c
# End Source File
# Begin Source File

SOURCE=.\aj_replacelist.c
# End Source File
# Begin Source File

SOURCE=.\aj_runes.c
# End Source File
# Begin Source File

SOURCE=.\aj_scoreboard.c
# End Source File
# Begin Source File

SOURCE=.\aj_statusbars.c
# End Source File
# Begin Source File

SOURCE=.\e_grapple.c
# End Source File
# Begin Source File

SOURCE=.\e_hook.c
# End Source File
# Begin Source File

SOURCE=.\funhook.c
# End Source File
# Begin Source File

SOURCE=.\sf_ammosort.c
# End Source File
# End Group
# Begin Group "Eraser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bot_ai.c
# End Source File
# Begin Source File

SOURCE=.\bot_die.c
# End Source File
# Begin Source File

SOURCE=.\bot_items.c
# End Source File
# Begin Source File

SOURCE=.\bot_misc.c
# End Source File
# Begin Source File

SOURCE=.\bot_nav.c
# End Source File
# Begin Source File

SOURCE=.\bot_spawn.c
# End Source File
# Begin Source File

SOURCE=.\bot_wpns.c
# End Source File
# Begin Source File

SOURCE=.\camclient.c
# End Source File
# Begin Source File

SOURCE=.\g_compress.c
# End Source File
# Begin Source File

SOURCE=.\g_map_mod.c
# End Source File
# Begin Source File

SOURCE=.\p_trail.c
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dm_ball.c
# End Source File
# Begin Source File

SOURCE=.\dm_tag.c
# End Source File
# Begin Source File

SOURCE=.\g_ai.c
# End Source File
# Begin Source File

SOURCE=.\g_chase.c
# End Source File
# Begin Source File

SOURCE=.\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\g_ctf.c
# End Source File
# Begin Source File

SOURCE=.\g_dm_rogue.c
# End Source File
# Begin Source File

SOURCE=.\g_func.c
# End Source File
# Begin Source File

SOURCE=.\g_func_rogue.c
# End Source File
# Begin Source File

SOURCE=.\g_items.c
# End Source File
# Begin Source File

SOURCE=.\g_main.c
# End Source File
# Begin Source File

SOURCE=.\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\g_misc_kex.c
# End Source File
# Begin Source File

SOURCE=.\g_model.c
# End Source File
# Begin Source File

SOURCE=.\g_monster.c
# End Source File
# Begin Source File

SOURCE=.\g_mtrain.c
# End Source File
# Begin Source File

SOURCE=.\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\g_save.c
# End Source File
# Begin Source File

SOURCE=.\g_spawn.c
# End Source File
# Begin Source File

SOURCE=.\g_sphere.c
# End Source File
# Begin Source File

SOURCE=.\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=.\g_target.c
# End Source File
# Begin Source File

SOURCE=.\g_target_rogue.c
# End Source File
# Begin Source File

SOURCE=.\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\g_trigger_rogue.c
# End Source File
# Begin Source File

SOURCE=.\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon_lmctf.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon_rogue.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon_xatrix.c
# End Source File
# Begin Source File

SOURCE=.\m_move.c
# End Source File
# Begin Source File

SOURCE=.\p_client.c
# End Source File
# Begin Source File

SOURCE=.\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\p_menu.c
# End Source File
# Begin Source File

SOURCE=.\p_view.c
# End Source File
# Begin Source File

SOURCE=.\p_weapon.c
# End Source File
# Begin Source File

SOURCE=.\q_shared.c
# End Source File
# End Group
# Begin Group "Zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\gzclose.c
# End Source File
# Begin Source File

SOURCE=.\zlib\gzlib.c
# End Source File
# Begin Source File

SOURCE=.\zlib\gzread.c
# End Source File
# Begin Source File

SOURCE=.\zlib\gzwrite.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\ioapi.c
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\zlib\unzip.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zip.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "EraserULTRA (h)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aj_banning.h
# End Source File
# Begin Source File

SOURCE=.\aj_confcycle.h
# End Source File
# Begin Source File

SOURCE=.\aj_lmctf.h
# End Source File
# Begin Source File

SOURCE=.\aj_main.h
# End Source File
# Begin Source File

SOURCE=.\aj_menu.h
# End Source File
# Begin Source File

SOURCE=.\aj_replacelist.h
# End Source File
# Begin Source File

SOURCE=.\aj_runes.h
# End Source File
# Begin Source File

SOURCE=.\aj_scoreboard.h
# End Source File
# Begin Source File

SOURCE=.\aj_startmax.h
# End Source File
# Begin Source File

SOURCE=.\aj_statusbars.h
# End Source File
# Begin Source File

SOURCE=.\aj_weaponbalancing.h
# End Source File
# Begin Source File

SOURCE=.\e_grapple.h
# End Source File
# Begin Source File

SOURCE=.\e_hook.h
# End Source File
# End Group
# Begin Group "Eraser (h)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bot_procs.h
# End Source File
# Begin Source File

SOURCE=.\camclient.h
# End Source File
# Begin Source File

SOURCE=.\g_map_mod.h
# End Source File
# End Group
# Begin Group "Misc (h)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\extra.h
# End Source File
# Begin Source File

SOURCE=.\g_ctf.h
# End Source File
# Begin Source File

SOURCE=.\g_items.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\m_player.h
# End Source File
# Begin Source File

SOURCE=.\p_menu.h
# End Source File
# Begin Source File

SOURCE=.\p_trail.h
# End Source File
# Begin Source File

SOURCE=.\pak.h
# End Source File
# Begin Source File

SOURCE=.\q_shared.h
# End Source File
# End Group
# Begin Group "Zlib (h)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\unzip.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zip.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\game.def
# End Source File
# Begin Source File

SOURCE=.\ultra.rc
# End Source File
# End Group
# Begin Source File

SOURCE=..\bots.cfg
# End Source File
# Begin Source File

SOURCE=..\chat.txt
# End Source File
# Begin Source File

SOURCE=..\default.cfg
# End Source File
# Begin Source File

SOURCE=.\Makefile
# End Source File
# End Target
# End Project
