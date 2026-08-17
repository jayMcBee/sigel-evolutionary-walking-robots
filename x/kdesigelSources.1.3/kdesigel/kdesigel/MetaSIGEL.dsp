# Microsoft Developer Studio Project File - Name="MetaSIGEL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MetaSIGEL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MetaSIGEL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MetaSIGEL.mak" CFG="MetaSIGEL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MetaSIGEL - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MetaSIGEL - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MetaSIGEL___Win32_Release"
# PROP BASE Intermediate_Dir "MetaSIGEL___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MT_Release"
# PROP Intermediate_Dir "MT_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "include" /I "$(QTDIR)/include" /I "ui" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/Dynamechs/dm" /I "supportingLibs/fparser" /I "supportingLibs/qhull" /I "supportingLibs/pvm" /I "supportingLibs/pvm/inc" /I "$(PVM_ROOT)/include" /D "_WINDOWS" /D "NDEBUG" /D "QT_ALTERNATE_QTSMANIP" /D "WIN32" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MetaSIGEL___Win32_Debug"
# PROP BASE Intermediate_Dir "MetaSIGEL___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MT_Debug"
# PROP Intermediate_Dir "MT_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "include" /I "$(QTDIR)/include" /I "ui" /I "supportingLibs/cv97" /I "supportingLibs/newmat09" /I "supportingLibs/Dynamo/src/inc" /I "supportingLibs/Dynamechs/dm" /I "supportingLibs/fparser" /I "supportingLibs/qhull" /I "supportingLibs/pvm" /I "supportingLibs/pvm/inc" /I "$(PVM_ROOT)/include" /D "QT_ALTERNATE_QTSMANIP" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "MetaSIGEL - Win32 Release"
# Name "MetaSIGEL - Win32 Debug"
# Begin Group "MT_GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\MT_GUI\DoubleSpinBox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\DoubleSpinBox.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_AddConstantsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_AddConstantsWidget.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_AddConstantsWidget.h
InputName=MT_AddConstantsWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_AddConstantsWidget.h
InputName=MT_AddConstantsWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_Editor.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_Editor.h
InputName=MT_Editor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_Editor.h
InputName=MT_Editor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_EstimationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_EstimationWidget.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_ExperimentItem.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_ExperimentItem.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_ExperimentWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_ExperimentWidget.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_ExperimentWidget.h
InputName=MT_ExperimentWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_ExperimentWidget.h
InputName=MT_ExperimentWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_IndividualWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_IndividualWidget.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_IndividualWidget.h
InputName=MT_IndividualWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_IndividualWidget.h
InputName=MT_IndividualWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_MainWindow.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_MainWindow.h
InputName=MT_MainWindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_MainWindow.h
InputName=MT_MainWindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_PopListViewItem.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_PopListViewItem.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_PopulationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_PopulationWidget.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_PopulationWidget.h
InputName=MT_PopulationWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_GUI
InputPath=.\include\MT_GUI\MT_PopulationWidget.h
InputName=MT_PopulationWidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_SearchWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_SearchWidget.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_SelectionWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_SelectionWidget.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_StatisticsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\MT_StatisticsWidget.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GUI\MT_WidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_GUI\MT_WidgetBase.h
# End Source File
# End Group
# Begin Group "MT_Control"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\MT_Control\MT_Classifier.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_Control\MT_Classifier.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_Control\MT_Controller.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_Control\MT_Controller.h

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_Control
InputPath=.\include\MT_Control\MT_Controller.h
InputName=MT_Controller

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\MT_Control
InputPath=.\include\MT_Control\MT_Controller.h
InputName=MT_Controller

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\MT_Control\MT_Evaluator.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_Control\MT_Evaluator.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_Control\MT_Substitute.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_Control\MT_Substitute.h
# End Source File
# End Group
# Begin Group "MT_GPSystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_FitnessTranier.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_FitnessTranier.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_GPManager.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_GPManager.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Individual.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Individual.h
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Instruction.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Interpreter.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Interpreter.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Operand.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Operand.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Population.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Population.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Program.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Program.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Programline.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Programline.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Randomizer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Randomizer.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Search.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Search.h
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_SelectionMethod.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Statistics.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_StatisticsElement.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_StatisticsElement.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Tournament.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Tournament.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_TournamentManager.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_TournamentManager.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_TrainingCase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_TrainingCase.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_Trainingset.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_Trainingset.h
# End Source File
# Begin Source File

SOURCE=.\src\MT_GPSystem\MT_TranslatedIndividual.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\MT_TranslatedIndividual.h
# End Source File
# End Group
# Begin Group "mocs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_AddConstantsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_AddConstantsWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_AddIndividualsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_Control\moc_MT_Controller.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_EstimationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_EstimationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_ExperimentWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_ExperimentWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GPSystem\moc_MT_GPManager.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_IndividualWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_IndividualWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_PopulationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_PopulationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_SearchWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_SearchWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_SelectionWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_SelectionWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MT_GUI\moc_MT_StatisticsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\moc_MT_StatisticsWidgetBase.cpp
# End Source File
# End Group
# Begin Group "GUI"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=.\ui\MT_UI\MT_AddConstantsWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_AddConstantsWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_AddConstantsWidgetBase.ui
InputName=MT_AddConstantsWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_AddConstantsWidgetBase.ui
InputName=MT_AddConstantsWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_AddIndividualsWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_AddIndividualsWidget.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_AddIndividualsWidget.ui
InputName=MT_AddIndividualsWidget

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_AddIndividualsWidget.ui
InputName=MT_AddIndividualsWidget

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_EstimationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_EstimationWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_EstimationWidgetBase.ui
InputName=MT_EstimationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_EstimationWidgetBase.ui
InputName=MT_EstimationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_ExperimentWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_ExperimentWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_ExperimentWidgetBase.ui
InputName=MT_ExperimentWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_ExperimentWidgetBase.ui
InputName=MT_ExperimentWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_IndividualWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_IndividualWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_IndividualWidgetBase.ui
InputName=MT_IndividualWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_IndividualWidgetBase.ui
InputName=MT_IndividualWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_PopulationWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_PopulationWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_PopulationWidgetBase.ui
InputName=MT_PopulationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_PopulationWidgetBase.ui
InputName=MT_PopulationWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_SearchWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_SearchWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_SearchWidgetBase.ui
InputName=MT_SearchWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_SearchWidgetBase.ui
InputName=MT_SearchWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_SelectionWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_SelectionWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_SelectionWidgetBase.ui
InputName=MT_SelectionWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_SelectionWidgetBase.ui
InputName=MT_SelectionWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_StatisticsWidgetBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ui\MT_UI\MT_StatisticsWidgetBase.ui

!IF  "$(CFG)" == "MetaSIGEL - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_StatisticsWidgetBase.ui
InputName=MT_StatisticsWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MetaSIGEL - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui
InputDir=.\ui\MT_UI
ProjDir=.
InputPath=.\ui\MT_UI\MT_StatisticsWidgetBase.ui
InputName=MT_StatisticsWidgetBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(ProjDir)\include\MT_GUI\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i ..\..\include\MT_GUI\$(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(ProjDir)\include\MT_GUI\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(ProjDir)\include\MT_GUI\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
