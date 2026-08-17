# Microsoft Developer Studio Project File - Name="sigel_slave" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SIGEL_SLAVE - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sigel_slave.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sigel_slave.mak" CFG="SIGEL_SLAVE - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sigel_slave - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sigel_slave - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseSlave"
# PROP Intermediate_Dir "ReleaseSlave"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "supportingLibs/Solid-2_0/include" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /I "include" /I "ui" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winspool.lib odbc32.lib odbccp32.lib dynamechs.lib msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib pvm.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /libpath:"supportingLibs\dynamechs\Release" /libpath:"supportingLibs\pvm\Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sigel_slave___Win32_Debug"
# PROP BASE Intermediate_Dir "sigel_slave___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SlaveDebug"
# PROP Intermediate_Dir "SlaveDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GR /GX /ZI /Od /I "supportingLibs/Solid20/include" /I "ui" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/DynaMechs/dm" /I "$(QTDIR)/include" /I "$(PVM_ROOT)/include" /I "$(PVM_ROOT)/src" /I "include" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dynamechs.lib msvcirt.lib wsock32.lib imm32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $(QTDIR)\lib\qutil.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib opengl32.lib glu32.lib pvm.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept /libpath:"supportingLibs\dynamechs\Debug" /libpath:"supportingLibs\pvm\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sigel_slave - Win32 Release"
# Name "sigel_slave - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "mocs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_MovieSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\moc_SIG_MovieSettingsDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationControls.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationVisualisationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\moc_SIG_SimulationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\moc_SIG_SimulationWindow.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationControls.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_SlaveGUI\SIG_SimulationWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sigel_slave.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h
InputName=SIG_MovieSettingsDialog

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialog.h
InputName=SIG_MovieSettingsDialog

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

USERDEP__SIG_M="$(ProjDir)\ui\SIGEL_SlaveUI\$(InputName).ui"	
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_MovieSettingsDialogBase.h
InputName=SIG_MovieSettingsDialogBase

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\uic.exe -i SIGEL_SlaveGUI\$(InputName).h -o $(ProjDir)\src\SIGEL_SlaveGUI\$(InputName).cpp $(ProjDir)\ui\SIGEL_SlaveGUI\$(InputName).ui 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h
InputName=SIG_SimulationControls

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationControls.h
InputName=SIG_SimulationControls

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.h
InputName=SIG_SimulationVisualisationWidget

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationVisualisationWidget.h
InputName=SIG_SimulationVisualisationWidget

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h
InputName=SIG_SimulationWidget

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidget.h
InputName=SIG_SimulationWidget

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

USERDEP__SIG_S="$(ProjDir)\ui\SIGEL_SlaveUI\$(InputName).ui"	
# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWidgetBase.h
InputName=SIG_SimulationWidgetBase

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\uic.exe -i SIGEL_SlaveGUI\$(InputName).h -o $(ProjDir)\src\SIGEL_SlaveGUI\$(InputName).cpp $(ProjDir)\ui\SIGEL_SlaveGUI\$(InputName).ui 
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h
InputName=SIG_SimulationWindow

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
ProjDir=.
InputPath=.\include\SIGEL_SlaveGUI\SIG_SimulationWindow.h
InputName=SIG_SimulationWindow

"$(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o $(ProjDir)\src\SIGEL_SlaveGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "ui"

# PROP Default_Filter "*.ui"
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui
InputName=SIG_MovieSettingsDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_MovieSettingsDialogBase.ui
InputName=SIG_MovieSettingsDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui

!IF  "$(CFG)" == "sigel_slave - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui
InputName=SIG_SimulationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sigel_slave - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\SIGEL_SlaveUI
ProjDir=.
InputPath=.\ui\SIGEL_SlaveUI\SIG_SimulationWidgetBase.ui
InputName=SIG_SimulationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\SIGEL_SlaveGUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
