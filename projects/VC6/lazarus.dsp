# Microsoft Developer Studio Project File - Name="lazarus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=lazarus - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lazarus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lazarus.mak" CFG="lazarus - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lazarus - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "lazarus - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lazarus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "c:\3dfx\sdk\glide2x\src\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /machine:I386 /out:"./kmq2gamex86.dll"
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "lazarus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /Zi /Od /I "c:\3dfx\sdk\glide2x\src\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"./kmq2gamex86.dll"

!ENDIF 

# Begin Target

# Name "lazarus - Win32 Release"
# Name "lazarus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "acesrc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\game\acebot_ai.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_cmds.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_compress.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_items.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_movement.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_nodes.c
# End Source File
# Begin Source File

SOURCE=..\..\game\acebot_spawn.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\game\g_ai.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_camera.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_chase.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_cmds.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_combat.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_crane.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_ctf.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_fog.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_func.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_items.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_jetpack.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_lights.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_lock.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_main.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_model.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_monster.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_moreai.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_mtrain.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_patchplayermodels.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_pendulum.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_phys.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_reflect.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_save.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_spawn.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_target.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_thing.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_tracktrain.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_trigger.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_turret.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_utils.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_vehicle.c
# End Source File
# Begin Source File

SOURCE=..\..\game\g_weapon.c
# End Source File
# Begin Source File

SOURCE=..\..\game\km_cvar.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_actor.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_actor_weap.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_berserk.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss2.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss3.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss31.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss32.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_brain.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_chick.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flipper.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_float.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flyer.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_gunner.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_hover.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_infantry.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_insane.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_medic.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_move.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_mutant.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_parasite.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_soldier.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_supertank.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_tank.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_chase.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_client.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_hud.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_text.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_trail.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_view.c
# End Source File
# Begin Source File

SOURCE=..\..\game\p_weapon.c
# End Source File
# Begin Source File

SOURCE=..\..\game\q_shared.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "acebot"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\game\acebot.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\game\g_ctf.h
# End Source File
# Begin Source File

SOURCE=..\..\game\g_local.h
# End Source File
# Begin Source File

SOURCE=..\..\game\game.h
# End Source File
# Begin Source File

SOURCE=..\..\game\km_cvar.h
# End Source File
# Begin Source File

SOURCE=..\..\game\laz_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_actor.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_berserk.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss2.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss31.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_boss32.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_brain.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_chick.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flipper.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_float.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flyer.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_gunner.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_hover.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_infantry.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_insane.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_medic.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_mutant.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_parasite.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_player.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_soldier.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_supertank.h
# End Source File
# Begin Source File

SOURCE=..\..\game\m_tank.h
# End Source File
# Begin Source File

SOURCE=..\..\game\p_menu.h
# End Source File
# Begin Source File

SOURCE=..\..\game\p_text.h
# End Source File
# Begin Source File

SOURCE=..\..\game\pak.h
# End Source File
# Begin Source File

SOURCE=..\..\game\q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\game\lazarus.def
# End Source File
# Begin Source File

SOURCE=.\lazarus.rc
# End Source File
# End Group
# End Target
# End Project
