# Microsoft Developer Studio Project File - Name="missionpack" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=missionpack - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "missionpack.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "missionpack.mak" CFG="missionpack - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "missionpack - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "missionpack - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "missionpack - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\gamex86.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=.\extractfuncs\extractfuncs.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "missionpack - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:".\gamex86.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Building game function table...
PreLink_Cmds=.\extractfuncs\extractfuncs.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "missionpack - Win32 Release"
# Name "missionpack - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c"
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

SOURCE=.\g_camera.c
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

SOURCE=.\g_crane.c
# End Source File
# Begin Source File

SOURCE=.\g_fog.c
# End Source File
# Begin Source File

SOURCE=.\g_func.c
# End Source File
# Begin Source File

SOURCE=.\g_items.c
# End Source File
# Begin Source File

SOURCE=.\g_jetpack.c
# End Source File
# Begin Source File

SOURCE=.\g_lights.c
# End Source File
# Begin Source File

SOURCE=.\g_lock.c
# End Source File
# Begin Source File

SOURCE=.\g_main.c
# End Source File
# Begin Source File

SOURCE=.\g_mappack.c
# End Source File
# Begin Source File

SOURCE=.\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\g_monster.c
# End Source File
# Begin Source File

SOURCE=.\g_newai.c
# End Source File
# Begin Source File

SOURCE=.\g_newdm.c
# End Source File
# Begin Source File

SOURCE=.\g_newfnc.c
# End Source File
# Begin Source File

SOURCE=.\g_newtarg.c
# End Source File
# Begin Source File

SOURCE=.\g_newtrig.c
# End Source File
# Begin Source File

SOURCE=.\g_newutils.c
# End Source File
# Begin Source File

SOURCE=.\g_newweap.c
# End Source File
# Begin Source File

SOURCE=.\g_patchplayermodels.c
# End Source File
# Begin Source File

SOURCE=.\g_pendulum.c
# End Source File
# Begin Source File

SOURCE=.\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\g_reflect.c
# End Source File
# Begin Source File

SOURCE=.\g_save.c
# End Source File
# Begin Source File

SOURCE=.\g_sound.c
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

SOURCE=.\g_thing.c
# End Source File
# Begin Source File

SOURCE=.\g_tracktrain.c
# End Source File
# Begin Source File

SOURCE=.\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\g_turret.c
# End Source File
# Begin Source File

SOURCE=.\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\g_vehicle.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\grenade.c
# End Source File
# Begin Source File

SOURCE=.\km_cvar.c
# End Source File
# Begin Source File

SOURCE=.\m_actor.c
# End Source File
# Begin Source File

SOURCE=.\m_actor_weap.c
# End Source File
# Begin Source File

SOURCE=.\m_berserk.c
# End Source File
# Begin Source File

SOURCE=.\m_boss2.c
# End Source File
# Begin Source File

SOURCE=.\m_boss3.c
# End Source File
# Begin Source File

SOURCE=.\m_boss31.c
# End Source File
# Begin Source File

SOURCE=.\m_boss32.c
# End Source File
# Begin Source File

SOURCE=.\m_boss5.c
# End Source File
# Begin Source File

SOURCE=.\m_brain.c
# End Source File
# Begin Source File

SOURCE=.\m_brainbeta.c
# End Source File
# Begin Source File

SOURCE=.\m_carrier.c
# End Source File
# Begin Source File

SOURCE=.\m_chick.c
# End Source File
# Begin Source File

SOURCE=.\m_dog.c
# End Source File
# Begin Source File

SOURCE=.\m_fixbot.c
# End Source File
# Begin Source File

SOURCE=.\m_flash.c
# End Source File
# Begin Source File

SOURCE=.\m_flipper.c
# End Source File
# Begin Source File

SOURCE=.\m_float.c
# End Source File
# Begin Source File

SOURCE=.\m_flyer.c
# End Source File
# Begin Source File

SOURCE=.\m_gekk.c
# End Source File
# Begin Source File

SOURCE=.\m_gladb.c
# End Source File
# Begin Source File

SOURCE=.\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=.\m_gunner.c
# End Source File
# Begin Source File

SOURCE=.\m_hover.c
# End Source File
# Begin Source File

SOURCE=.\m_infantry.c
# End Source File
# Begin Source File

SOURCE=.\m_insane.c
# End Source File
# Begin Source File

SOURCE=.\m_medic.c
# End Source File
# Begin Source File

SOURCE=.\m_move.c
# End Source File
# Begin Source File

SOURCE=.\m_mutant.c
# End Source File
# Begin Source File

SOURCE=.\m_parasite.c
# End Source File
# Begin Source File

SOURCE=.\m_soldier.c
# End Source File
# Begin Source File

SOURCE=.\m_stalker.c
# End Source File
# Begin Source File

SOURCE=.\m_supertank.c
# End Source File
# Begin Source File

SOURCE=.\m_tank.c
# End Source File
# Begin Source File

SOURCE=.\m_turret.c
# End Source File
# Begin Source File

SOURCE=.\m_widow.c
# End Source File
# Begin Source File

SOURCE=.\m_widow2.c
# End Source File
# Begin Source File

SOURCE=.\p_chasecam.c
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

SOURCE=.\p_text.c
# End Source File
# Begin Source File

SOURCE=.\p_trail.c
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
# Begin Source File

SOURCE=.\z_hound.c
# End Source File
# Begin Source File

SOURCE=.\z_sentien.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\fmod.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\g_main.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\km_cvar.h
# End Source File
# Begin Source File

SOURCE=.\laz_misc.h
# End Source File
# Begin Source File

SOURCE=.\m_actor.h
# End Source File
# Begin Source File

SOURCE=.\m_berserk.h
# End Source File
# Begin Source File

SOURCE=.\m_boss2.h
# End Source File
# Begin Source File

SOURCE=.\m_boss31.h
# End Source File
# Begin Source File

SOURCE=.\m_boss32.h
# End Source File
# Begin Source File

SOURCE=.\m_brain.h
# End Source File
# Begin Source File

SOURCE=.\m_chick.h
# End Source File
# Begin Source File

SOURCE=.\m_dog.h
# End Source File
# Begin Source File

SOURCE=.\m_fiend.h
# End Source File
# Begin Source File

SOURCE=.\m_fixbot.h
# End Source File
# Begin Source File

SOURCE=.\m_flipper.h
# End Source File
# Begin Source File

SOURCE=.\m_float.h
# End Source File
# Begin Source File

SOURCE=.\m_flyer.h
# End Source File
# Begin Source File

SOURCE=.\m_gekk.h
# End Source File
# Begin Source File

SOURCE=.\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=.\m_gunner.h
# End Source File
# Begin Source File

SOURCE=.\m_hover.h
# End Source File
# Begin Source File

SOURCE=.\m_infantry.h
# End Source File
# Begin Source File

SOURCE=.\m_insane.h
# End Source File
# Begin Source File

SOURCE=.\m_medic.h
# End Source File
# Begin Source File

SOURCE=.\m_mutant.h
# End Source File
# Begin Source File

SOURCE=.\m_parasite.h
# End Source File
# Begin Source File

SOURCE=.\m_player.h
# End Source File
# Begin Source File

SOURCE=.\m_rider.h
# End Source File
# Begin Source File

SOURCE=.\m_soldier.h
# End Source File
# Begin Source File

SOURCE=.\m_soldierh.h
# End Source File
# Begin Source File

SOURCE=.\m_supertank.h
# End Source File
# Begin Source File

SOURCE=.\m_tank.h
# End Source File
# Begin Source File

SOURCE=.\m_turret.h
# End Source File
# Begin Source File

SOURCE=.\m_zombie.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\p_menu.h
# End Source File
# Begin Source File

SOURCE=.\p_text.h
# End Source File
# Begin Source File

SOURCE=.\pak.h
# End Source File
# Begin Source File

SOURCE=.\q_shared.h
# End Source File
# Begin Source File

SOURCE=.\z_anim.h
# End Source File
# Begin Source File

SOURCE=.\z_hound.h
# End Source File
# Begin Source File

SOURCE=.\z_sentien.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "*.def,*.res"
# Begin Source File

SOURCE=.\game.def
# End Source File
# End Group
# Begin Source File

SOURCE=.\missionpack.rc
# End Source File
# End Target
# End Project
