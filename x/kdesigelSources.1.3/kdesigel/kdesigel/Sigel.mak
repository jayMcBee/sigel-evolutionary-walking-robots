# Microsoft Developer Studio Generated NMAKE File, Based on Sigel.dsp
!IF "$(CFG)" == ""
CFG=Sigel - Win32 Release
!MESSAGE Keine Konfiguration angegeben. Sigel - Win32 Release wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "Sigel - Win32 Release" && "$(CFG)" != "Sigel - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Sigel.mak" CFG="Sigel - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Sigel - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Sigel - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_RobotBase.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"\
 ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h" "$(OUTDIR)\Sigel.exe"

!ELSE 

ALL : "MetaSIGEL - Win32 Release" "dm - Win32 Release" "pvm - Win32 Release" "SIGELCommon - Win32 Release" "SOLID20 - Win32 Release" "qhull - Win32 Release" "newmat09 - Win32 Release" "Dynamo - Win32 Release" "cv97 - Win32 Release" ".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h"\
 ".\include\SIGEL_MasterGUI\SIG_RobotBase.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h" "$(OUTDIR)\Sigel.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"cv97 - Win32 ReleaseCLEAN" "Dynamo - Win32 ReleaseCLEAN" "newmat09 - Win32 ReleaseCLEAN" "qhull - Win32 ReleaseCLEAN" "SOLID20 - Win32 ReleaseCLEAN" "SIGELCommon - Win32 ReleaseCLEAN" "pvm - Win32 ReleaseCLEAN" "dm - Win32 ReleaseCLEAN" "MetaSIGEL - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\moc_SIG_AddIndividualsDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_AllIndividualsView.obj"
	-@erase "$(INTDIR)\moc_SIG_EditCommandDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_EditHostDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_EditHostDialogBase.obj"
	-@erase "$(INTDIR)\moc_SIG_EnvironmentBase.obj"
	-@erase "$(INTDIR)\moc_SIG_EnvironmentView.obj"
	-@erase "$(INTDIR)\moc_SIG_Experiment.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentListView.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentView.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentViewBase.obj"
	-@erase "$(INTDIR)\moc_SIG_GPParameter.obj"
	-@erase "$(INTDIR)\moc_SIG_GPParameterBase.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualList.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualListBase.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualView.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualViewBase.obj"
	-@erase "$(INTDIR)\moc_SIG_LanguageParameters.obj"
	-@erase "$(INTDIR)\moc_SIG_LanguageParametersBase.obj"
	-@erase "$(INTDIR)\moc_SIG_MainWindow.obj"
	-@erase "$(INTDIR)\moc_SIG_RenameDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_RobotBase.obj"
	-@erase "$(INTDIR)\moc_SIG_RobotView.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationParameter.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationParameterBase.obj"
	-@erase "$(INTDIR)\moc_SIG_TextView.obj"
	-@erase "$(INTDIR)\SIG_AddIndividualsDialog.obj"
	-@erase "$(INTDIR)\SIG_AllIndividualsView.obj"
	-@erase "$(INTDIR)\SIG_EditCommandDialog.obj"
	-@erase "$(INTDIR)\SIG_EditHostDialog.obj"
	-@erase "$(INTDIR)\SIG_EditHostDialogBase.obj"
	-@erase "$(INTDIR)\SIG_EnvironmentBase.obj"
	-@erase "$(INTDIR)\SIG_EnvironmentView.obj"
	-@erase "$(INTDIR)\SIG_Experiment.obj"
	-@erase "$(INTDIR)\SIG_ExperimentItem.obj"
	-@erase "$(INTDIR)\SIG_ExperimentListView.obj"
	-@erase "$(INTDIR)\SIG_ExperimentView.obj"
	-@erase "$(INTDIR)\SIG_ExperimentViewBase.obj"
	-@erase "$(INTDIR)\SIG_GPParameterBase.obj"
	-@erase "$(INTDIR)\SIG_IndividualList.obj"
	-@erase "$(INTDIR)\SIG_IndividualListBase.obj"
	-@erase "$(INTDIR)\SIG_IndividualListItem.obj"
	-@erase "$(INTDIR)\SIG_IndividualView.obj"
	-@erase "$(INTDIR)\SIG_IndividualViewBase.obj"
	-@erase "$(INTDIR)\SIG_InfoBox.obj"
	-@erase "$(INTDIR)\SIG_LanguageParametersBase.obj"
	-@erase "$(INTDIR)\SIG_MainWindow.obj"
	-@erase "$(INTDIR)\SIG_RenameDialog.obj"
	-@erase "$(INTDIR)\SIG_RobotBase.obj"
	-@erase "$(INTDIR)\SIG_RobotView.obj"
	-@erase "$(INTDIR)\SIG_SimulationParameter.obj"
	-@erase "$(INTDIR)\SIG_SimulationParameterBase.obj"
	-@erase "$(INTDIR)\SIG_TextView.obj"
	-@erase "$(INTDIR)\sigel.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Sigel.exe"
	-@erase ".\Release2\SIG_GPParameter.obj"
	-@erase ".\Release2\SIG_LanguageParameters.obj"
	-@erase ".\Release2\vc60.idb"
	-@erase ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_RobotBase.h"
	-@erase ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp"
	-@erase ".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G6 /MD /W3 /GR /GX /O2 /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"$(INTDIR)\Sigel.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sigel.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Sigel.pdb" /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\Sigel.exe" /libpath:"$(PVM_ROOT)\lib\WIN32" 
LINK32_OBJS= \
	"$(INTDIR)\moc_SIG_EditHostDialogBase.obj" \
	"$(INTDIR)\moc_SIG_EnvironmentBase.obj" \
	"$(INTDIR)\moc_SIG_ExperimentViewBase.obj" \
	"$(INTDIR)\moc_SIG_GPParameterBase.obj" \
	"$(INTDIR)\moc_SIG_IndividualListBase.obj" \
	"$(INTDIR)\moc_SIG_IndividualViewBase.obj" \
	"$(INTDIR)\moc_SIG_LanguageParametersBase.obj" \
	"$(INTDIR)\moc_SIG_RobotBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationParameterBase.obj" \
	"$(INTDIR)\SIG_AddIndividualsDialog.obj" \
	"$(INTDIR)\SIG_AllIndividualsView.obj" \
	"$(INTDIR)\SIG_EditCommandDialog.obj" \
	"$(INTDIR)\SIG_EditHostDialog.obj" \
	"$(INTDIR)\SIG_EditHostDialogBase.obj" \
	"$(INTDIR)\SIG_EnvironmentBase.obj" \
	"$(INTDIR)\SIG_EnvironmentView.obj" \
	"$(INTDIR)\SIG_Experiment.obj" \
	"$(INTDIR)\SIG_ExperimentItem.obj" \
	"$(INTDIR)\SIG_ExperimentListView.obj" \
	"$(INTDIR)\SIG_ExperimentView.obj" \
	"$(INTDIR)\SIG_ExperimentViewBase.obj" \
	".\Release2\SIG_GPParameter.obj" \
	"$(INTDIR)\SIG_GPParameterBase.obj" \
	"$(INTDIR)\SIG_IndividualList.obj" \
	"$(INTDIR)\SIG_IndividualListBase.obj" \
	"$(INTDIR)\SIG_IndividualListItem.obj" \
	"$(INTDIR)\SIG_IndividualView.obj" \
	"$(INTDIR)\SIG_IndividualViewBase.obj" \
	"$(INTDIR)\SIG_InfoBox.obj" \
	".\Release2\SIG_LanguageParameters.obj" \
	"$(INTDIR)\SIG_LanguageParametersBase.obj" \
	"$(INTDIR)\SIG_MainWindow.obj" \
	"$(INTDIR)\SIG_RenameDialog.obj" \
	"$(INTDIR)\SIG_RobotBase.obj" \
	"$(INTDIR)\SIG_RobotView.obj" \
	"$(INTDIR)\SIG_SimulationParameter.obj" \
	"$(INTDIR)\SIG_SimulationParameterBase.obj" \
	"$(INTDIR)\SIG_TextView.obj" \
	"$(INTDIR)\sigel.obj" \
	"$(INTDIR)\moc_SIG_AddIndividualsDialog.obj" \
	"$(INTDIR)\moc_SIG_AllIndividualsView.obj" \
	"$(INTDIR)\moc_SIG_EditCommandDialog.obj" \
	"$(INTDIR)\moc_SIG_EditHostDialog.obj" \
	"$(INTDIR)\moc_SIG_EnvironmentView.obj" \
	"$(INTDIR)\moc_SIG_Experiment.obj" \
	"$(INTDIR)\moc_SIG_ExperimentListView.obj" \
	"$(INTDIR)\moc_SIG_ExperimentView.obj" \
	"$(INTDIR)\moc_SIG_GPParameter.obj" \
	"$(INTDIR)\moc_SIG_IndividualList.obj" \
	"$(INTDIR)\moc_SIG_IndividualView.obj" \
	"$(INTDIR)\moc_SIG_LanguageParameters.obj" \
	"$(INTDIR)\moc_SIG_MainWindow.obj" \
	"$(INTDIR)\moc_SIG_RenameDialog.obj" \
	"$(INTDIR)\moc_SIG_RobotView.obj" \
	"$(INTDIR)\moc_SIG_SimulationParameter.obj" \
	"$(INTDIR)\moc_SIG_TextView.obj" \
	".\supportingLibs\cv97\Release\cv97.lib" \
	".\supportingLibs\Dynamo\Release\Dynamo.lib" \
	".\supportingLibs\newmat09\Release\newmat09.lib" \
	".\supportingLibs\qhull\Release\qhull.lib" \
	".\supportingLibs\SOLID20\Release\SOLID20.lib" \
	"$(OUTDIR)\SIGELCommon.lib" \
	".\SUPPORTINGLIBS\pvm\Release\pvm.lib" \
	".\supportingLibs\dynamechs\Release\dynamechs.lib" \
	".\MT_Release\MetaSIGEL.lib"

"$(OUTDIR)\Sigel.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Release\Sigel.exe
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "MetaSIGEL - Win32 Release" "dm - Win32 Release" "pvm - Win32 Release" "SIGELCommon - Win32 Release" "SOLID20 - Win32 Release" "qhull - Win32 Release" "newmat09 - Win32 Release" "Dynamo - Win32 Release" "cv97 - Win32 Release" ".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp" ".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp"\
 ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_RobotBase.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h" "$(OUTDIR)\Sigel.exe"
   stripmessage ".\Release\Sigel.exe"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Sigel.exe"

!ELSE 

ALL : "MetaSIGEL - Win32 Debug" "dm - Win32 Debug" "pvm - Win32 Debug" "SIGELCommon - Win32 Debug" "SOLID20 - Win32 Debug" "qhull - Win32 Debug" "newmat09 - Win32 Debug" "Dynamo - Win32 Debug" "cv97 - Win32 Debug" "$(OUTDIR)\Sigel.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"cv97 - Win32 DebugCLEAN" "Dynamo - Win32 DebugCLEAN" "newmat09 - Win32 DebugCLEAN" "qhull - Win32 DebugCLEAN" "SOLID20 - Win32 DebugCLEAN" "SIGELCommon - Win32 DebugCLEAN" "pvm - Win32 DebugCLEAN" "dm - Win32 DebugCLEAN" "MetaSIGEL - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\moc_SIG_AddIndividualsDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_AllIndividualsView.obj"
	-@erase "$(INTDIR)\moc_SIG_EditCommandDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_EditHostDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_EditHostDialogBase.obj"
	-@erase "$(INTDIR)\moc_SIG_EnvironmentBase.obj"
	-@erase "$(INTDIR)\moc_SIG_EnvironmentView.obj"
	-@erase "$(INTDIR)\moc_SIG_Experiment.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentListView.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentView.obj"
	-@erase "$(INTDIR)\moc_SIG_ExperimentViewBase.obj"
	-@erase "$(INTDIR)\moc_SIG_GPParameter.obj"
	-@erase "$(INTDIR)\moc_SIG_GPParameterBase.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualList.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualListBase.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualView.obj"
	-@erase "$(INTDIR)\moc_SIG_IndividualViewBase.obj"
	-@erase "$(INTDIR)\moc_SIG_LanguageParameters.obj"
	-@erase "$(INTDIR)\moc_SIG_LanguageParametersBase.obj"
	-@erase "$(INTDIR)\moc_SIG_MainWindow.obj"
	-@erase "$(INTDIR)\moc_SIG_RenameDialog.obj"
	-@erase "$(INTDIR)\moc_SIG_RobotBase.obj"
	-@erase "$(INTDIR)\moc_SIG_RobotView.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationParameter.obj"
	-@erase "$(INTDIR)\moc_SIG_SimulationParameterBase.obj"
	-@erase "$(INTDIR)\moc_SIG_TextView.obj"
	-@erase "$(INTDIR)\SIG_EditHostDialogBase.obj"
	-@erase "$(INTDIR)\SIG_EnvironmentBase.obj"
	-@erase "$(INTDIR)\SIG_ExperimentViewBase.obj"
	-@erase "$(INTDIR)\SIG_GPParameterBase.obj"
	-@erase "$(INTDIR)\SIG_IndividualListBase.obj"
	-@erase "$(INTDIR)\SIG_IndividualViewBase.obj"
	-@erase "$(INTDIR)\SIG_LanguageParametersBase.obj"
	-@erase "$(INTDIR)\SIG_RobotBase.obj"
	-@erase "$(INTDIR)\SIG_SimulationParameterBase.obj"
	-@erase "$(INTDIR)\sigel.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Sigel.exe"
	-@erase "$(OUTDIR)\Sigel.ilk"
	-@erase "$(OUTDIR)\Sigel.pdb"
	-@erase ".\Debug2\SIG_AddIndividualsDialog.obj"
	-@erase ".\Debug2\SIG_AllIndividualsView.obj"
	-@erase ".\Debug2\SIG_EditCommandDialog.obj"
	-@erase ".\Debug2\SIG_EditHostDialog.obj"
	-@erase ".\Debug2\SIG_EnvironmentView.obj"
	-@erase ".\Debug2\SIG_Experiment.obj"
	-@erase ".\Debug2\SIG_ExperimentItem.obj"
	-@erase ".\Debug2\SIG_ExperimentListView.obj"
	-@erase ".\Debug2\SIG_ExperimentView.obj"
	-@erase ".\Debug2\SIG_GPParameter.obj"
	-@erase ".\Debug2\SIG_IndividualList.obj"
	-@erase ".\Debug2\SIG_IndividualListItem.obj"
	-@erase ".\Debug2\SIG_IndividualView.obj"
	-@erase ".\Debug2\SIG_InfoBox.obj"
	-@erase ".\Debug2\SIG_LanguageParameters.obj"
	-@erase ".\Debug2\SIG_MainWindow.obj"
	-@erase ".\Debug2\SIG_RenameDialog.obj"
	-@erase ".\Debug2\SIG_RobotView.obj"
	-@erase ".\Debug2\SIG_SimulationParameter.obj"
	-@erase ".\Debug2\SIG_TextView.obj"
	-@erase ".\Debug2\vc60.idb"
	-@erase ".\Debug2\vc60.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"$(INTDIR)\Sigel.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sigel.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Sigel.pdb" /debug /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\Sigel.exe" /pdbtype:sept /libpath:"$(PVM_ROOT)\lib\WIN32" 
LINK32_OBJS= \
	"$(INTDIR)\moc_SIG_EditHostDialogBase.obj" \
	"$(INTDIR)\moc_SIG_EnvironmentBase.obj" \
	"$(INTDIR)\moc_SIG_ExperimentViewBase.obj" \
	"$(INTDIR)\moc_SIG_GPParameterBase.obj" \
	"$(INTDIR)\moc_SIG_IndividualListBase.obj" \
	"$(INTDIR)\moc_SIG_IndividualViewBase.obj" \
	"$(INTDIR)\moc_SIG_LanguageParametersBase.obj" \
	"$(INTDIR)\moc_SIG_RobotBase.obj" \
	"$(INTDIR)\moc_SIG_SimulationParameterBase.obj" \
	".\Debug2\SIG_AddIndividualsDialog.obj" \
	".\Debug2\SIG_AllIndividualsView.obj" \
	".\Debug2\SIG_EditCommandDialog.obj" \
	".\Debug2\SIG_EditHostDialog.obj" \
	"$(INTDIR)\SIG_EditHostDialogBase.obj" \
	"$(INTDIR)\SIG_EnvironmentBase.obj" \
	".\Debug2\SIG_EnvironmentView.obj" \
	".\Debug2\SIG_Experiment.obj" \
	".\Debug2\SIG_ExperimentItem.obj" \
	".\Debug2\SIG_ExperimentListView.obj" \
	".\Debug2\SIG_ExperimentView.obj" \
	"$(INTDIR)\SIG_ExperimentViewBase.obj" \
	".\Debug2\SIG_GPParameter.obj" \
	"$(INTDIR)\SIG_GPParameterBase.obj" \
	".\Debug2\SIG_IndividualList.obj" \
	"$(INTDIR)\SIG_IndividualListBase.obj" \
	".\Debug2\SIG_IndividualListItem.obj" \
	".\Debug2\SIG_IndividualView.obj" \
	"$(INTDIR)\SIG_IndividualViewBase.obj" \
	".\Debug2\SIG_InfoBox.obj" \
	".\Debug2\SIG_LanguageParameters.obj" \
	"$(INTDIR)\SIG_LanguageParametersBase.obj" \
	".\Debug2\SIG_MainWindow.obj" \
	".\Debug2\SIG_RenameDialog.obj" \
	"$(INTDIR)\SIG_RobotBase.obj" \
	".\Debug2\SIG_RobotView.obj" \
	".\Debug2\SIG_SimulationParameter.obj" \
	"$(INTDIR)\SIG_SimulationParameterBase.obj" \
	".\Debug2\SIG_TextView.obj" \
	"$(INTDIR)\sigel.obj" \
	"$(INTDIR)\moc_SIG_AddIndividualsDialog.obj" \
	"$(INTDIR)\moc_SIG_AllIndividualsView.obj" \
	"$(INTDIR)\moc_SIG_EditCommandDialog.obj" \
	"$(INTDIR)\moc_SIG_EditHostDialog.obj" \
	"$(INTDIR)\moc_SIG_EnvironmentView.obj" \
	"$(INTDIR)\moc_SIG_Experiment.obj" \
	"$(INTDIR)\moc_SIG_ExperimentListView.obj" \
	"$(INTDIR)\moc_SIG_ExperimentView.obj" \
	"$(INTDIR)\moc_SIG_GPParameter.obj" \
	"$(INTDIR)\moc_SIG_IndividualList.obj" \
	"$(INTDIR)\moc_SIG_IndividualView.obj" \
	"$(INTDIR)\moc_SIG_LanguageParameters.obj" \
	"$(INTDIR)\moc_SIG_MainWindow.obj" \
	"$(INTDIR)\moc_SIG_RenameDialog.obj" \
	"$(INTDIR)\moc_SIG_RobotView.obj" \
	"$(INTDIR)\moc_SIG_SimulationParameter.obj" \
	"$(INTDIR)\moc_SIG_TextView.obj" \
	".\supportingLibs\cv97\Debug\cv97.lib" \
	".\supportingLibs\Dynamo\Debug\Dynamo.lib" \
	".\supportingLibs\newmat09\Debug\newmat09.lib" \
	".\supportingLibs\qhull\Debug\qhull.lib" \
	".\supportingLibs\SOLID20\Debug\SOLID20.lib" \
	"$(OUTDIR)\SIGELCommon.lib" \
	".\SUPPORTINGLIBS\pvm\Debug\pvm.lib" \
	".\supportingLibs\dynamechs\Debug\dynamechs.lib" \
	".\MT_Debug\MetaSIGEL.lib"

"$(OUTDIR)\Sigel.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Debug\Sigel.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Lizenzhinweis entfernen
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "MetaSIGEL - Win32 Debug" "dm - Win32 Debug" "pvm - Win32 Debug" "SIGELCommon - Win32 Debug" "SOLID20 - Win32 Debug" "qhull - Win32 Debug" "newmat09 - Win32 Debug" "Dynamo - Win32 Debug" "cv97 - Win32 Debug" "$(OUTDIR)\Sigel.exe"
   stripmessage ".\Debug\Sigel.exe"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Sigel.dep")
!INCLUDE "Sigel.dep"
!ELSE 
!MESSAGE Warning: cannot find "Sigel.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sigel - Win32 Release" || "$(CFG)" == "Sigel - Win32 Debug"
SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp

"$(INTDIR)\moc_SIG_EditHostDialogBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp

"$(INTDIR)\moc_SIG_EnvironmentBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp

"$(INTDIR)\moc_SIG_ExperimentViewBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp

"$(INTDIR)\moc_SIG_GPParameterBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp

"$(INTDIR)\moc_SIG_IndividualListBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp

"$(INTDIR)\moc_SIG_IndividualViewBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp

"$(INTDIR)\moc_SIG_LanguageParametersBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp

"$(INTDIR)\moc_SIG_RobotBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp

"$(INTDIR)\moc_SIG_SimulationParameterBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_AddIndividualsDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_AddIndividualsDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_AddIndividualsDialog.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_AllIndividualsView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_AllIndividualsView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_AllIndividualsView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h" ".\include\SIGEL_MasterGUI\SIG_IndividualView.h" ".\include\SIGEL_MasterGUI\SIG_IndividualList.h" ".\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_EditCommandDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_EditCommandDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_EditCommandDialog.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_EditHostDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_EditHostDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_EditHostDialog.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_EditHostDialog.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp

"$(INTDIR)\SIG_EditHostDialogBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp

"$(INTDIR)\SIG_EnvironmentBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_EnvironmentView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_EnvironmentView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_EnvironmentView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_EnvironmentView.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_Experiment.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_Experiment.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_Experiment.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_Experiment.h" ".\include\SIGEL_MasterGUI\SIG_GPParameter.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameter.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentView.h" ".\include\SIGEL_MasterGUI\SIG_RobotView.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentView.h" ".\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParameters.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualList.h" ".\include\SIGEL_MasterGUI\SIG_IndividualView.h" ".\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentItem.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_ExperimentItem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_ExperimentItem.obj" : $(SOURCE)
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentListView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_ExperimentListView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_ExperimentListView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_RenameDialog.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentListView.h" ".\include\SIGEL_MasterGUI\SIG_Experiment.h" ".\include\SIGEL_MasterGUI\SIG_GPParameter.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameter.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentView.h" ".\include\SIGEL_MasterGUI\SIG_RobotView.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentView.h" ".\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParameters.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualList.h" ".\include\SIGEL_MasterGUI\SIG_IndividualView.h" ".\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_ExperimentView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_ExperimentView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_ExperimentView.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp

"$(INTDIR)\SIG_ExperimentViewBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_GPParameter.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MD /W3 /GR /GX /O2 /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"Release/Sigel.pch" /YX /Fo"Release2/" /Fd"Release2/" /FD /c 

".\Release2\SIG_GPParameter.obj" : $(SOURCE)
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

CPP_SWITCHES=/nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"Debug/Sigel.pch" /YX /Fo"Debug2/" /Fd"Debug2/" /FD /GZ /c 

".\Debug2\SIG_GPParameter.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_GPParameter.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialog.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp

"$(INTDIR)\SIG_GPParameterBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualList.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_IndividualList.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_IndividualList.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_IndividualList.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp

"$(INTDIR)\SIG_IndividualListBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualListItem.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_IndividualListItem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_IndividualListItem.obj" : $(SOURCE)
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_IndividualView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_IndividualView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_IndividualView.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp

"$(INTDIR)\SIG_IndividualViewBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_InfoBox.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_InfoBox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_InfoBox.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_TextView.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_LanguageParameters.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MD /W3 /GR /GX /O2 /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"Release/Sigel.pch" /YX /Fo"Release2/" /Fd"Release2/" /FD /c 

".\Release2\SIG_LanguageParameters.obj" : $(SOURCE)
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

CPP_SWITCHES=/nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /Fp"Debug/Sigel.pch" /YX /Fo"Debug2/" /Fd"Debug2/" /FD /GZ /c 

".\Debug2\SIG_LanguageParameters.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_LanguageParameters.h" ".\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp

"$(INTDIR)\SIG_LanguageParametersBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_MainWindow.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_MainWindow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_MainWindow.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_MainWindow.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentListView.h" ".\include\SIGEL_MasterGUI\SIG_Experiment.h" ".\include\SIGEL_MasterGUI\SIG_GPParameter.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameter.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentView.h" ".\include\SIGEL_MasterGUI\SIG_RobotView.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentView.h" ".\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParameters.h" ".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h" ".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h" ".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualList.h" ".\include\SIGEL_MasterGUI\SIG_IndividualView.h" ".\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h" ".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h" ".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_RenameDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_RenameDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_RenameDialog.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_RenameDialog.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_RobotBase.cpp

"$(INTDIR)\SIG_RobotBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_RobotView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_RobotView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_RobotView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_RobotView.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\SIG_SimulationParameter.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_SimulationParameter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_SimulationParameter.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_SimulationParameter.h" ".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp

"$(INTDIR)\SIG_SimulationParameterBase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\SIG_TextView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"


"$(INTDIR)\SIG_TextView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"


".\Debug2\SIG_TextView.obj" : $(SOURCE) ".\include\SIGEL_MasterGUI\SIG_TextView.h"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\sigel.cpp

"$(INTDIR)\sigel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h
InputName=SIG_AddIndividualsDialog

".\src\SIGEL_MasterGUI\moc_SIG_AddIndividualsDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h
InputName=SIG_AddIndividualsDialog

".\src\SIGEL_MasterGUI\moc_SIG_AddIndividualsDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h
InputName=SIG_AllIndividualsView

".\src\SIGEL_MasterGUI\moc_SIG_AllIndividualsView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h
InputName=SIG_AllIndividualsView

".\src\SIGEL_MasterGUI\moc_SIG_AllIndividualsView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h
InputName=SIG_EditCommandDialog

".\src\SIGEL_MasterGUI\moc_SIG_EditCommandDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h
InputName=SIG_EditCommandDialog

".\src\SIGEL_MasterGUI\moc_SIG_EditCommandDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h
InputName=SIG_EditHostDialog

".\src\SIGEL_MasterGUI\moc_SIG_EditHostDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h
InputName=SIG_EditHostDialog

".\src\SIGEL_MasterGUI\moc_SIG_EditHostDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h
InputName=SIG_EnvironmentView

".\src\SIGEL_MasterGUI\moc_SIG_EnvironmentView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h
InputName=SIG_EnvironmentView

".\src\SIGEL_MasterGUI\moc_SIG_EnvironmentView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_Experiment.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_Experiment.h
InputName=SIG_Experiment

".\src\SIGEL_MasterGUI\moc_SIG_Experiment.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_Experiment.h
InputName=SIG_Experiment

".\src\SIGEL_MasterGUI\moc_SIG_Experiment.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_ExperimentListView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentListView.h
InputName=SIG_ExperimentListView

".\src\SIGEL_MasterGUI\moc_SIG_ExperimentListView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h
InputName=SIG_ExperimentView

".\src\SIGEL_MasterGUI\moc_SIG_ExperimentView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h
InputName=SIG_ExperimentView

".\src\SIGEL_MasterGUI\moc_SIG_ExperimentView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_GPParameter.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_GPParameter.h
InputName=SIG_GPParameter

".\src\SIGEL_MasterGUI\moc_SIG_GPParameter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_GPParameter.h
InputName=SIG_GPParameter

".\src\SIGEL_MasterGUI\moc_SIG_GPParameter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_IndividualList.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualList.h
InputName=SIG_IndividualList

".\src\SIGEL_MasterGUI\moc_SIG_IndividualList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualList.h
InputName=SIG_IndividualList

".\src\SIGEL_MasterGUI\moc_SIG_IndividualList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_IndividualView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualView.h
InputName=SIG_IndividualView

".\src\SIGEL_MasterGUI\moc_SIG_IndividualView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualView.h
InputName=SIG_IndividualView

".\src\SIGEL_MasterGUI\moc_SIG_IndividualView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h
InputName=SIG_LanguageParameters

".\src\SIGEL_MasterGUI\moc_SIG_LanguageParameters.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h
InputName=SIG_LanguageParameters

".\src\SIGEL_MasterGUI\moc_SIG_LanguageParameters.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_MainWindow.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_MainWindow.h
InputName=SIG_MainWindow

".\src\SIGEL_MasterGUI\moc_SIG_MainWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_MainWindow.h
InputName=SIG_MainWindow

".\src\SIGEL_MasterGUI\moc_SIG_MainWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h
InputName=SIG_RenameDialog

".\src\SIGEL_MasterGUI\moc_SIG_RenameDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h
InputName=SIG_RenameDialog

".\src\SIGEL_MasterGUI\moc_SIG_RenameDialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_RobotView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RobotView.h
InputName=SIG_RobotView

".\src\SIGEL_MasterGUI\moc_SIG_RobotView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RobotView.h
InputName=SIG_RobotView

".\src\SIGEL_MasterGUI\moc_SIG_RobotView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h
InputName=SIG_SimulationParameter

".\src\SIGEL_MasterGUI\moc_SIG_SimulationParameter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h
InputName=SIG_SimulationParameter

".\src\SIGEL_MasterGUI\moc_SIG_SimulationParameter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\include\SIGEL_MasterGUI\SIG_TextView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_TextView.h
InputName=SIG_TextView

".\src\SIGEL_MasterGUI\moc_SIG_TextView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_TextView.h
InputName=SIG_TextView

".\src\SIGEL_MasterGUI\moc_SIG_TextView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_AddIndividualsDialog.cpp

"$(INTDIR)\moc_SIG_AddIndividualsDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_AllIndividualsView.cpp

"$(INTDIR)\moc_SIG_AllIndividualsView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EditCommandDialog.cpp

"$(INTDIR)\moc_SIG_EditCommandDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EditHostDialog.cpp

"$(INTDIR)\moc_SIG_EditHostDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EnvironmentView.cpp

"$(INTDIR)\moc_SIG_EnvironmentView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_Experiment.cpp

"$(INTDIR)\moc_SIG_Experiment.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_ExperimentListView.cpp

"$(INTDIR)\moc_SIG_ExperimentListView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_ExperimentView.cpp

"$(INTDIR)\moc_SIG_ExperimentView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_GPParameter.cpp

"$(INTDIR)\moc_SIG_GPParameter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_IndividualList.cpp

"$(INTDIR)\moc_SIG_IndividualList.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_IndividualView.cpp

"$(INTDIR)\moc_SIG_IndividualView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_LanguageParameters.cpp

"$(INTDIR)\moc_SIG_LanguageParameters.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_MainWindow.cpp

"$(INTDIR)\moc_SIG_MainWindow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_RenameDialog.cpp

"$(INTDIR)\moc_SIG_RenameDialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_RobotView.cpp

"$(INTDIR)\moc_SIG_RobotView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_SimulationParameter.cpp

"$(INTDIR)\moc_SIG_SimulationParameter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_TextView.cpp

"$(INTDIR)\moc_SIG_TextView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui
InputName=SIG_EditHostDialogBase

".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h"	".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui
InputName=SIG_EditHostDialogBase

".\include\SIGEL_MasterGUI\SIG_EditHostDialogBase.h"	".\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui
InputName=SIG_EnvironmentBase

".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h"	".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui
InputName=SIG_EnvironmentBase

".\include\SIGEL_MasterGUI\SIG_EnvironmentBase.h"	".\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui
InputName=SIG_ExperimentViewBase

".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h"	".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui
InputName=SIG_ExperimentViewBase

".\include\SIGEL_MasterGUI\SIG_ExperimentViewBase.h"	".\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui
InputName=SIG_GPParameterBase

".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h"	".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui
InputName=SIG_GPParameterBase

".\include\SIGEL_MasterGUI\SIG_GPParameterBase.h"	".\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui
InputName=SIG_IndividualListBase

".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"	".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui
InputName=SIG_IndividualListBase

".\include\SIGEL_MasterGUI\SIG_IndividualListBase.h"	".\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui
InputName=SIG_IndividualViewBase

".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"	".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui
InputName=SIG_IndividualViewBase

".\include\SIGEL_MasterGUI\SIG_IndividualViewBase.h"	".\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui
InputName=SIG_LanguageParametersBase

".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h"	".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui
InputName=SIG_LanguageParametersBase

".\include\SIGEL_MasterGUI\SIG_LanguageParametersBase.h"	".\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui
InputName=SIG_RobotBase

".\include\SIGEL_MasterGUI\SIG_RobotBase.h"	".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui
InputName=SIG_RobotBase

".\include\SIGEL_MasterGUI\SIG_RobotBase.h"	".\ui\SIGEL_MasterUI\SIG_RobotBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

SOURCE=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui
InputName=SIG_SimulationParameterBase

".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h"	".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui
InputName=SIG_SimulationParameterBase

".\include\SIGEL_MasterGUI\SIG_SimulationParameterBase.h"	".\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp"	".\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h 
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp 
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp
<< 
	

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"cv97 - Win32 Release" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Release" 
   cd "..\.."

"cv97 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"cv97 - Win32 Debug" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Debug" 
   cd "..\.."

"cv97 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\cv97"
   $(MAKE) /$(MAKEFLAGS) /F ".\cv97.mak" CFG="cv97 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"Dynamo - Win32 Release" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Release" 
   cd "..\.."

"Dynamo - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"Dynamo - Win32 Debug" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Debug" 
   cd "..\.."

"Dynamo - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\Dynamo"
   $(MAKE) /$(MAKEFLAGS) /F ".\Dynamo.mak" CFG="Dynamo - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"newmat09 - Win32 Release" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Release" 
   cd "..\.."

"newmat09 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"newmat09 - Win32 Debug" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Debug" 
   cd "..\.."

"newmat09 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\newmat09"
   $(MAKE) /$(MAKEFLAGS) /F ".\newmat09.mak" CFG="newmat09 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"qhull - Win32 Release" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Release" 
   cd "..\.."

"qhull - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"qhull - Win32 Debug" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Debug" 
   cd "..\.."

"qhull - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\qhull"
   $(MAKE) /$(MAKEFLAGS) /F ".\qhull.mak" CFG="qhull - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"SOLID20 - Win32 Release" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Release" 
   cd "..\.."

"SOLID20 - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"SOLID20 - Win32 Debug" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Debug" 
   cd "..\.."

"SOLID20 - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\SOLID20"
   $(MAKE) /$(MAKEFLAGS) /F ".\SOLID20.mak" CFG="SOLID20 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"SIGELCommon - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Release" 
   cd "."

"SIGELCommon - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"SIGELCommon - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Debug" 
   cd "."

"SIGELCommon - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\SIGELCommon.mak" CFG="SIGELCommon - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"pvm - Win32 Release" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Release" 
   cd "..\.."

"pvm - Win32 ReleaseCLEAN" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"pvm - Win32 Debug" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Debug" 
   cd "..\.."

"pvm - Win32 DebugCLEAN" : 
   cd ".\SUPPORTINGLIBS\pvm"
   $(MAKE) /$(MAKEFLAGS) /F ".\pvm.mak" CFG="pvm - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"dm - Win32 Release" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Release" 
   cd "..\.."

"dm - Win32 ReleaseCLEAN" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Release" RECURSE=1 CLEAN 
   cd "..\.."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

"dm - Win32 Debug" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Debug" 
   cd "..\.."

"dm - Win32 DebugCLEAN" : 
   cd ".\supportingLibs\dynamechs"
   $(MAKE) /$(MAKEFLAGS) /F ".\dynamechs.mak" CFG="dm - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\.."

!ENDIF 

!IF  "$(CFG)" == "Sigel - Win32 Release"

"MetaSIGEL - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Release" 
   cd "."

"MetaSIGEL - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

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

