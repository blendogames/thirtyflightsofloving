# Microsoft Developer Studio Generated NMAKE File, Based on game.dsp
!IF "$(CFG)" == ""
CFG=game - Win32 Debug Alpha
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの game - Win32 Debug Alpha を設定します。
!ENDIF 

!IF "$(CFG)" != "game - Win32 Release" && "$(CFG)" != "game - Win32 Debug" &&\
 "$(CFG)" != "game - Win32 Debug Alpha" && "$(CFG)" !=\
 "game - Win32 Release Alpha"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug Alpha"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "game - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "game - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "game - Win32 Debug Alpha" ("Win32 (ALPHA) Dynamic-Link Library" 用)
!MESSAGE "game - Win32 Release Alpha" ("Win32 (ALPHA) Dynamic-Link Library" 用)
!MESSAGE 
!ERROR 無効な構成が指定されています。
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

OUTDIR=.\..\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\..\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gamex86.dll"

!ELSE 

ALL : "$(OUTDIR)\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /ML /W3 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winmm.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\gamex86.pdb" /machine:I386 /def:".\game.def"\
 /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

OUTDIR=.\..\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gamex86.dll"

!ELSE 

ALL : "$(OUTDIR)\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\gamex86.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "BUILDING_REF_GL" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winmm.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\gamex86.pdb" /debug /machine:I386 /def:".\game.def"\
 /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

OUTDIR=.\..\DebugAxp
INTDIR=.\DebugAxp
# Begin Custom Macros
OutDir=.\..\DebugAxp
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gameaxp.dll"

!ELSE 

ALL : "$(OUTDIR)\gameaxp.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.lib"
	-@erase "$(OUTDIR)\gameaxp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
CPP=cl.exe
CPP_PROJ=/nologo /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /QA21164 /MTd\
 /c 
CPP_OBJS=.\DebugAxp/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /pdb:"$(OUTDIR)\gameaxp.pdb" /debug\
 /machine:ALPHA /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll"\
 /implib:"$(OUTDIR)\gameaxp.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

OUTDIR=.\..\ReleaseAXP
INTDIR=.\ReleaseAXP
# Begin Custom Macros
OutDir=.\..\ReleaseAXP
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gameaxp.dll"

!ELSE 

ALL : "$(OUTDIR)\gameaxp.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
CPP=cl.exe
CPP_PROJ=/nologo /MT /Gt0 /W3 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /QA21164 /c 
CPP_OBJS=.\ReleaseAXP/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib /nologo /base:"0x20000000"\
 /subsystem:windows /dll /pdb:"$(OUTDIR)\gameaxp.pdb" /machine:ALPHA\
 /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll" /implib:"$(OUTDIR)\gameaxp.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "game - Win32 Release" || "$(CFG)" == "game - Win32 Debug" ||\
 "$(CFG)" == "game - Win32 Debug Alpha" || "$(CFG)" ==\
 "game - Win32 Release Alpha"
SOURCE=.\g_ai.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_turret.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_utils.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_actor.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_berserk.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss2.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss3.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss31.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss32.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_brain.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_chick.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flash.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flipper.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_float.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flyer.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gladiator.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gunner.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_hover.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_infantry.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_insane.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_medic.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_mutant.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_parasite.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_soldier.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_supertank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_tank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_trail.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\q_shared.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ENDIF 


!ENDIF 

