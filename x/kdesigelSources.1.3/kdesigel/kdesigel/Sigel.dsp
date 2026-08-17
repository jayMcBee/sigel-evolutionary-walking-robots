# Microsoft Developer Studio Project File - Name="Sigel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Sigel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sigel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sigel.mak" CFG="Sigel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sigel - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sigel - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winspool.lib odbc32.lib odbccp32.lib msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib $(PVM_ROOT)\lib\WIN32\libpvm3.lib $(PVM_ROOT)\lib\WIN32\libgpvm3.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /libpath:"$(PVM_ROOT)\lib\WIN32"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "ui" /I "supportingLibs/fparser" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "include" /I "supportingLibs/Solid20/include" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib pvm.lib MT_Debug\MetaSIGEL.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept /libpath:"supportingLibs\pvm\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Sigel - Win32 Release"
# Name "Sigel - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\include\MT_GPSystem\moc_MT_GPManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_EditHostDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_EnvironmentBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_ExperimentViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_GPParameterBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_IndividualListBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_IndividualViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_LanguageParametersBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_RobotBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\moc_SIG_SimulationParameterBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_AddIndividualsDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_AllIndividualsView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_EditCommandDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_EditHostDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_EnvironmentView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_Experiment.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentItem.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentListView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_ExperimentView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_GPParameter.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Intermediate_Dir "Release2"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualList.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualListItem.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_IndividualView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_InfoBox.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_LanguageParameters.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Intermediate_Dir "Release2"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_MainWindow.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_RenameDialog.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_RobotBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_RobotView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_SimulationParameter.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\SIG_TextView.cpp

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sigel.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h
InputName=SIG_AddIndividualsDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AddIndividualsDialog.h
InputName=SIG_AddIndividualsDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h
InputName=SIG_AllIndividualsView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_AllIndividualsView.h
InputName=SIG_AllIndividualsView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h
InputName=SIG_EditCommandDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditCommandDialog.h
InputName=SIG_EditCommandDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h
InputName=SIG_EditHostDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EditHostDialog.h
InputName=SIG_EditHostDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h
InputName=SIG_EnvironmentView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_EnvironmentView.h
InputName=SIG_EnvironmentView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_Experiment.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_Experiment.h
InputName=SIG_Experiment

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_Experiment.h
InputName=SIG_Experiment

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_ExperimentItem.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_ExperimentListView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentListView.h
InputName=SIG_ExperimentListView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentListView.h
InputName=SIG_ExperimentListView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h
InputName=SIG_ExperimentView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_ExperimentView.h
InputName=SIG_ExperimentView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_GPParameter.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

USERDEP__SIG_G=".\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui"	
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_GPParameter.h
InputName=SIG_GPParameter

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_GPParameter.h
InputName=SIG_GPParameter

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_IndividualList.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualList.h
InputName=SIG_IndividualList

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualList.h
InputName=SIG_IndividualList

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_IndividualListItem.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_IndividualView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualView.h
InputName=SIG_IndividualView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_IndividualView.h
InputName=SIG_IndividualView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_InfoBox.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h
InputName=SIG_LanguageParameters

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_LanguageParameters.h
InputName=SIG_LanguageParameters

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_MainWindow.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_MainWindow.h
InputName=SIG_MainWindow

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_MainWindow.h
InputName=SIG_MainWindow

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h
InputName=SIG_RenameDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RenameDialog.h
InputName=SIG_RenameDialog

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_RobotView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RobotView.h
InputName=SIG_RobotView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_RobotView.h
InputName=SIG_RobotView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h
InputName=SIG_SimulationParameter

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_SimulationParameter.h
InputName=SIG_SimulationParameter

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_MasterGUI\SIG_TextView.h

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_TextView.h
InputName=SIG_TextView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Intermediate_Dir "Debug2"
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_MasterGUI\SIG_TextView.h
InputName=SIG_TextView

"$(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_MasterGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "mocs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_AddIndividualsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_AllIndividualsView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EditCommandDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EditHostDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_EnvironmentView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_Experiment.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_ExperimentListView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_ExperimentView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_GPParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_IndividualList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_IndividualView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_LanguageParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_RenameDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_RobotView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_SimulationParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_MasterGUI\moc_SIG_TextView.cpp
# End Source File
# End Group
# Begin Group "ui"

# PROP Default_Filter "*.ui"
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui
InputName=SIG_EditHostDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EditHostDialogBase.ui
InputName=SIG_EditHostDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui
InputName=SIG_EnvironmentBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_EnvironmentBase.ui
InputName=SIG_EnvironmentBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui
InputName=SIG_ExperimentViewBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_ExperimentViewBase.ui
InputName=SIG_ExperimentViewBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui
InputName=SIG_GPParameterBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_GPParameterBase.ui
InputName=SIG_GPParameterBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui
InputName=SIG_IndividualListBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualListBase.ui
InputName=SIG_IndividualListBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui
InputName=SIG_IndividualViewBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_IndividualViewBase.ui
InputName=SIG_IndividualViewBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui
InputName=SIG_LanguageParametersBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_LanguageParametersBase.ui
InputName=SIG_LanguageParametersBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui
InputName=SIG_RobotBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_RobotBase.ui
InputName=SIG_RobotBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui

!IF  "$(CFG)" == "Sigel - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui
InputName=SIG_SimulationParameterBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Sigel - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_MasterUI
ProjDir=.
InputPath=.\ui\SIGEL_MasterUI\SIG_SimulationParameterBase.ui
InputName=SIG_SimulationParameterBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_MasterGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
