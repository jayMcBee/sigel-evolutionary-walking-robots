# Microsoft Developer Studio Generated NMAKE File, Based on sigel_slave.dsp
!IF "$(CFG)" == ""
CFG=sigel_slave - Win32 Release
!MESSAGE Keine Konfiguration angegeben. SIGEL_SLAVE - WIN32 RELEASE wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "sigel_slave - Win32 Release" && "$(CFG)" != "sigel_slave - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "sigel_slave.mak" CFG="SIGEL_SLAVE - WIN32 DEBUG"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "sigel_slave - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "sigel_slave - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

OUTDIR=.\ReleaseSlave
INTDIR=.\ReleaseSlave
# Begin Custom Macros
OutDir=.\ReleaseSlave
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"

!ELSE 

ALL : "MetaSIGEL - Win32 Release" "dm - Win32 Release" "pvm - Win32 Release" "SOLID20 - Win32 Release" "qhull - Win32 Release" "newmat09 - Win32 Release" "Dynamo - Win32 Release" "cv97 - Win32 Release" "SIGELCommon - Win32 Release" ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SIGELCommon - Win32 ReleaseCLEAN" "cv97 - Win32 ReleaseCLEAN" "Dynamo - Win32 ReleaseCLEAN" "newmat09 - Win32 ReleaseCLEAN" "qhull - Win32 ReleaseCLEAN" "SOLID20 - Win32 ReleaseCLEAN" "pvm - Win32 ReleaseCLEAN" "dm - Win32 ReleaseCLEAN" "MetaSIGEL - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\moc_SIG_MovieSettingsDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_MovieSettingsDialogBase.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationControls.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationVisualisationWidget.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWidget.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWidgetBase.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWindow.obj"
	-@erase "$(INTDIR)\SIG_MovieSettingsDialog.obj"
	-@erase "$(INTDIR)\SIG_MovieSettingsDialogBase.obj"
	-@erase "$(INTDIR)\SIG_SimulationControls.obj"
	-@erase "$(INTDIR)\SIG_SimulationVisualisationWidget.obj"
	-@erase "$(INTDIR)\SIG_SimulationWidget.obj"
	-@erase "$(INTDIR)\SIG_SimulationWidgetBase.obj"
	-@erase "$(INTDIR)\SIG_SimulationWindow.obj"
	-@erase "$(INTDIR)\sigel_slave.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\sigel_slave.exe"
	-@erase ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h"
	-@erase ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h"
	-@erase ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G6 /MD /W3 /GR /GX /O2 /I "supportingLibs/Solid-2_0/include" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /I "include" /I "ui" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"$(INTDIR)\sigel_slave.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sigel_slave.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dynamechs.lib msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\sigel_slave.pdb" /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\sigel_slave.exe" /libpath:"supportingLibs\dynamechs\Release" 
LINK32_OBJS= \
	"$(INTDIR)\moc_SIG_MovieSettingsDialog.obj" \
	"$(INTDIR)\moc_SIG_MovieSettingsDialogBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationControls.obj" \
	"$(INTDIR)\moc_SIG_SimulationVisualisationWidget.obj" \
	"$(INTDIR)\moc_SIG_SimulationWidget.obj" \
	"$(INTDIR)\moc_SIG_SimulationWidgetBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationWindow.obj" \
	"$(INTDIR)\SIG_MovieSettingsDialog.obj" \
	"$(INTDIR)\SIG_MovieSettingsDialogBase.obj" \
	"$(INTDIR)\SIG_SimulationControls.obj" \
	"$(INTDIR)\SIG_SimulationVisualisationWidget.obj" \
	"$(INTDIR)\SIG_SimulationWidget.obj" \
	"$(INTDIR)\SIG_SimulationWidgetBase.obj" \
	"$(INTDIR)\SIG_SimulationWindow.obj" \
	"$(INTDIR)\sigel_slave.obj" \
	".\Release\SIGELCommon.lib" \
	".\supportingLibs\cv97\Release\cv97.lib" \
	".\supportingLibs\Dynamo\Release\Dynamo.lib" \
	".\supportingLibs\newmat09\Release\newmat09.lib" \
	".\supportingLibs\qhull\Release\qhull.lib" \
	".\supportingLibs\SOLID20\Release\SOLID20.lib" \
	".\SUPPORTINGLIBS\pvm\Release\pvm.lib" \
	".\supportingLibs\dynamechs\Release\dynamechs.lib" \
	".\MT_Release\MetaSIGEL.lib"

"$(OUTDIR)\sigel_slave.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\ReleaseSlave\sigel_slave.exe
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\ReleaseSlave
# End Custom Macros

$(DS_POSTBUILD_DEP) : "MetaSIGEL - Win32 Release" "dm - Win32 Release" "pvm - Win32 Release" "SOLID20 - Win32 Release" "qhull - Win32 Release" "newmat09 - Win32 Release" "Dynamo - Win32 Release" "cv97 - Win32 Release" "SIGELCommon - Win32 Release" ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"
   stripmessage ".\ReleaseSlave\sigel_slave.exe"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

OUTDIR=.\SlaveDebug
INTDIR=.\SlaveDebug
# Begin Custom Macros
OutDir=.\SlaveDebug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"

!ELSE 

ALL : "MetaSIGEL - Win32 Debug" "dm - Win32 Debug" "pvm - Win32 Debug" "SOLID20 - Win32 Debug" "qhull - Win32 Debug" "newmat09 - Win32 Debug" "Dynamo - Win32 Debug" "cv97 - Win32 Debug" "SIGELCommon - Win32 Debug" ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SIGELCommon - Win32 DebugCLEAN" "cv97 - Win32 DebugCLEAN" "Dynamo - Win32 DebugCLEAN" "newmat09 - Win32 DebugCLEAN" "qhull - Win32 DebugCLEAN" "SOLID20 - Win32 DebugCLEAN" "pvm - Win32 DebugCLEAN" "dm - Win32 DebugCLEAN" "MetaSIGEL - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\moc_SIG_MovieSettingsDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_MovieSettingsDialogBase.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationControls.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationVisualisationWidget.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWidget.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWidgetBase.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationWindow.obj"
	-@erase "$(INTDIR)\SIG_MovieSettingsDialog.obj"
	-@erase "$(INTDIR)\SIG_MovieSettingsDialogBase.obj"
	-@erase "$(INTDIR)\SIG_SimulationControls.obj"
	-@erase "$(INTDIR)\SIG_SimulationVisualisationWidget.obj"
	-@erase "$(INTDIR)\SIG_SimulationWidget.obj"
	-@erase "$(INTDIR)\SIG_SimulationWidgetBase.obj"
	-@erase "$(INTDIR)\SIG_SimulationWindow.obj"
	-@erase "$(INTDIR)\sigel_slave.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\sigel_slave.exe"
	-@erase "$(OUTDIR)\sigel_slave.ilk"
	-@erase "$(OUTDIR)\sigel_slave.pdb"
	-@erase ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h"
	-@erase ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h"
	-@erase ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp"
	-@erase ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "supportingLibs/Solid20/include" /I "ui" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /I "include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"$(INTDIR)\sigel_slave.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sigel_slave.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dynamechs.lib msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\sigel_slave.pdb" /debug /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\sigel_slave.exe" /pdbtype:sept /libpath:"supportingLibs\dynamechs\Debug" 
LINK32_OBJS= \
	"$(INTDIR)\moc_SIG_MovieSettingsDialog.obj" \
	"$(INTDIR)\moc_SIG_MovieSettingsDialogBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationControls.obj" \
	"$(INTDIR)\moc_SIG_SimulationVisualisationWidget.obj" \
	"$(INTDIR)\moc_SIG_SimulationWidget.obj" \
	"$(INTDIR)\moc_SIG_SimulationWidgetBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationWindow.obj" \
	"$(INTDIR)\SIG_MovieSettingsDialog.obj" \
	"$(INTDIR)\SIG_MovieSettingsDialogBase.obj" \
	"$(INTDIR)\SIG_SimulationControls.obj" \
	"$(INTDIR)\SIG_SimulationVisualisationWidget.obj" \
	"$(INTDIR)\SIG_SimulationWidget.obj" \
	"$(INTDIR)\SIG_SimulationWidgetBase.obj" \
	"$(INTDIR)\SIG_SimulationWindow.obj" \
	"$(INTDIR)\sigel_slave.obj" \
	".\Debug\SIGELCommon.lib" \
	".\supportingLibs\cv97\Debug\cv97.lib" \
	".\supportingLibs\Dynamo\Debug\Dynamo.lib" \
	".\supportingLibs\newmat09\Debug\newmat09.lib" \
	".\supportingLibs\qhull\Debug\qhull.lib" \
	".\supportingLibs\SOLID20\Debug\SOLID20.lib" \
	".\SUPPORTINGLIBS\pvm\Debug\pvm.lib" \
	".\supportingLibs\dynamechs\Debug\dynamechs.lib" \
	".\MT_Debug\MetaSIGEL.lib"

"$(OUTDIR)\sigel_slave.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\SlaveDebug\sigel_slave.exe
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\SlaveDebug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "MetaSIGEL - Win32 Debug" "dm - Win32 Debug" "pvm - Win32 Debug" "SOLID20 - Win32 Debug" "qhull - Win32 Debug" "newmat09 - Win32 Debug" "Dynamo - Win32 Debug" "cv97 - Win32 Debug" "SIGELCommon - Win32 Debug" ".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" ".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" ".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h" ".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h" "$(OUTDIR)\sigel_slave.exe"
   stripmessage ".\SlaveDebug\sigel_slave.exe"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("sigel_slave.dep")
!INCLUDE "sigel_slave.dep"
!ELSE 
!MESSAGE Warning: cannot find "sigel_slave.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "sigel_slave - Win32 Release" || "$(CFG)" == "sigel_slave - Win32 Debug"
SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialog.cpp

"$(INTDIR)\moc_SIG_MovieSettingsDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialogBase.cpp

"$(INTDIR)\moc_SIG_MovieSettingsDialogBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationControls.cpp

"$(INTDIR)\moc_SIG_SimulationControls.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationVisualisationWidget.cpp

"$(INTDIR)\moc_SIG_SimulationVisualisationWidget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidget.cpp

"$(INTDIR)\moc_SIG_SimulationWidget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidgetBase.cpp

"$(INTDIR)\moc_SIG_SimulationWidgetBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationWindow.cpp

"$(INTDIR)\moc_SIG_SimulationWindow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.cpp

"$(INTDIR)\SIG_MovieSettingsDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp

"$(INTDIR)\SIG_MovieSettingsDialogBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationControls.cpp

"$(INTDIR)\SIG_SimulationControls.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.cpp

"$(INTDIR)\SIG_SimulationVisualisationWidget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationWidget.cpp

"$(INTDIR)\SIG_SimulationWidget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp

"$(INTDIR)\SIG_SimulationWidgetBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationWindow.cpp

"$(INTDIR)\SIG_SimulationWindow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\sigel_slave.cpp

"$(INTDIR)\sigel_slave.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h
InputName=SIG_MovieSettingsDialog

".\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h
InputName=SIG_MovieSettingsDialog

".\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h
InputName=SIG_MovieSettingsDialogBase
USERDEP__SIG_M="$(ProjDir)\ui\SIGEL_SlaveUI\$(InputName).ui"	

".\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialogBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__SIG_M)
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\uic.exe -i SIGEL_SlaveGUI\$(InputName).h -o $(ProjDir)\src\SIGEL_SlaveGUI\$(InputName).cpp $(ProjDir)\ui\SIGEL_SlaveGUI\$(InputName).ui 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp 
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h
InputName=SIG_SimulationControls

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationControls.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h
InputName=SIG_SimulationControls

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationControls.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.h
InputName=SIG_SimulationVisualisationWidget

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationVisualisationWidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h
InputName=SIG_SimulationWidget

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h
InputName=SIG_SimulationWidget

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidget.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h
InputName=SIG_SimulationWidgetBase
USERDEP__SIG_S="$(ProjDir)\ui\SIGEL_SlaveUI\$(InputName).ui"	

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidgetBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__SIG_S)
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\uic.exe -i SIGEL_SlaveGUI\$(InputName).h -o $(ProjDir)\src\SIGEL_SlaveGUI\$(InputName).cpp $(ProjDir)\ui\SIGEL_SlaveGUI\$(InputName).ui 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp 
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h
InputName=SIG_SimulationWindow

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h
InputName=SIG_SimulationWindow

".\src\SIGEL_SlaveGUI\moc_SIG_SimulationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui
InputName=SIG_MovieSettingsDialogBase

".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h"	".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp"	".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui
InputName=SIG_MovieSettingsDialogBase

".\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h"	".\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp"	".\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui
InputName=SIG_SimulationWidgetBase

".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h"	".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp"	".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui
InputName=SIG_SimulationWidgetBase

".\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h"	".\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp"	".\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"SIGELCommon - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Release" 
   cd "."

"SIGELCommon - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"SIGELCommon - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Debug" 
   cd "."

"SIGELCommon - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"cv97 - Win32 Release" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Release" 
   cd "..\.."

"cv97 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"cv97 - Win32 Debug" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Debug" 
   cd "..\.."

"cv97 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"Dynamo - Win32 Release" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Release" 
   cd "..\.."

"Dynamo - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"Dynamo - Win32 Debug" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Debug" 
   cd "..\.."

"Dynamo - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"newmat09 - Win32 Release" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Release" 
   cd "..\.."

"newmat09 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"newmat09 - Win32 Debug" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Debug" 
   cd "..\.."

"newmat09 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"qhull - Win32 Release" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Release" 
   cd "..\.."

"qhull - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"qhull - Win32 Debug" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Debug" 
   cd "..\.."

"qhull - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"SOLID20 - Win32 Release" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Release" 
   cd "..\.."

"SOLID20 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"SOLID20 - Win32 Debug" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Debug" 
   cd "..\.."

"SOLID20 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"pvm - Win32 Release" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Release" 
   cd "..\.."

"pvm - Win32 ReleaseCLEAN" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"pvm - Win32 Debug" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Debug" 
   cd "..\.."

"pvm - Win32 DebugCLEAN" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"dm - Win32 Release" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Release" 
   cd "..\.."

"dm - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"dm - Win32 Debug" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Debug" 
   cd "..\.."

"dm - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

"MetaSIGEL - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Release" 
   cd "."

"MetaSIGEL - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

"MetaSIGEL - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Debug" 
   cd "."

"MetaSIGEL - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 


!ENDIF 

