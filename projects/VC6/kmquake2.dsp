# Microsoft Developer Studio Project File - Name="quake2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=QUAKE2 - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "kmquake2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "kmquake2.mak" CFG="QUAKE2 - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "quake2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "quake2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "quake2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlib.lib libjpeg.lib ogg_static.lib vorbisfile_static.lib libpng.lib libcurl.lib glu32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC" /out:"./kmquake2.exe" /libpath:"..\..\win32\lib\VC6"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 wsock32.lib gdi32.lib kernel32.lib user32.lib winmm.lib zlib.lib libjpeg.lib ogg_static.lib vorbisfile_static.lib libpng.lib libcurl.lib glu32.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /out:".\kmquake2.exe" /libpath:"..\..\win32\lib\VC6"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "quake2 - Win32 Release"
# Name "quake2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\win32\cd_win.c
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_dma.c
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_mem.c
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_mix.c
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_stream.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\snd_win.c
# End Source File
# End Group
# Begin Group "client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\client\cl_cin.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_cinematic.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_console.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_download.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_effects.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_ents.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_event.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_http.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_input.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_inv.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_keys.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_lights.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_loc.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_main.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_parse.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_particle.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_pred.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_screen.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_string.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_tempent.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_utils.c
# End Source File
# Begin Source File

SOURCE=..\..\client\cl_view.c
# End Source File
# Begin Source File

SOURCE=..\..\game\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\..\client\x86.c
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qcommon\cmd.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\cmodel.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\common.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\crc.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\cvar.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\files.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\md4.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\net_chan.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\pmove.c
# End Source File
# Begin Source File

SOURCE=..\..\game\q_shared.c
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\wildcard.c
# End Source File
# End Group
# Begin Group "server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\server\sv_ccmds.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_ents.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_game.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_init.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_main.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_send.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_user.c
# End Source File
# Begin Source File

SOURCE=..\..\server\sv_world.c
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\win32\conproc.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\in_win.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\net_wins.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\q_shwin.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\sys_console.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\sys_win.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\vid_dll.c
# End Source File
# End Group
# Begin Group "renderer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\win32\glw_imp.c
# End Source File
# Begin Source File

SOURCE=..\..\win32\qgl_win.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_alias.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_alias_md2.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_alias_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_arb_program.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_backend.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_beam.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_bloom.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_draw.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_entity.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_fog.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_fragment.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_glstate.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_image.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_light.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_main.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_model.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_particle.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_sky.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_sprite.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_surface.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_vlights.c
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_warp.c
# End Source File
# End Group
# Begin Group "ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\ui\ui_backend.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_game.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_game_credits.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_game_saveload.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_main.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_addressbook.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_dmoptions.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_download.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_joinserver.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_playersetup.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_mp_startserver.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_multiplayer.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_controls.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_effects.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_interface.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_keys.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_screen.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_options_sound.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_quit.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_subsystem.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_video.c
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_video_advanced.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\client\anorms.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\anorms.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\anormtab.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\bspfile.h
# End Source File
# Begin Source File

SOURCE=..\..\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=..\..\client\cinematic.h
# End Source File
# Begin Source File

SOURCE=..\..\client\client.h
# End Source File
# Begin Source File

SOURCE=..\..\win32\conproc.h
# End Source File
# Begin Source File

SOURCE=..\..\client\console.h
# End Source File
# Begin Source File

SOURCE=..\..\game\game.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\glext.h
# End Source File
# Begin Source File

SOURCE=..\..\client\input.h
# End Source File
# Begin Source File

SOURCE=..\..\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\..\client\particles.h
# End Source File
# Begin Source File

SOURCE=..\..\client\q2palette.h
# End Source File
# Begin Source File

SOURCE=..\..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\qgl.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_alias.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_local.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\r_model.h
# End Source File
# Begin Source File

SOURCE=..\..\client\ref.h
# End Source File
# Begin Source File

SOURCE=..\..\client\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\server\server.h
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_loc.h
# End Source File
# Begin Source File

SOURCE=..\..\client\snd_ogg.h
# End Source File
# Begin Source File

SOURCE=..\..\client\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\ui_local.h
# End Source File
# Begin Source File

SOURCE=..\..\client\vid.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\vid_modes.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\vid_resolutions.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\vlights.h
# End Source File
# Begin Source File

SOURCE=..\..\renderer\warpsin.h
# End Source File
# Begin Source File

SOURCE=..\..\qcommon\wildcard.h
# End Source File
# Begin Source File

SOURCE=..\..\win32\winnewerror.h
# End Source File
# Begin Source File

SOURCE=..\..\win32\winquake.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\kmq2.rc
# End Source File
# Begin Source File

SOURCE=..\..\win32\q2.ico
# End Source File
# Begin Source File

SOURCE=..\..\win32\q2mp1.ico
# End Source File
# Begin Source File

SOURCE=..\..\win32\q2mp2.ico
# End Source File
# Begin Source File

SOURCE=..\..\win32\startup.bmp
# End Source File
# Begin Source File

SOURCE=..\..\win32\startup2.bmp
# End Source File
# Begin Source File

SOURCE=..\..\win32\startup2beta.bmp
# End Source File
# End Group
# End Target
# End Project
