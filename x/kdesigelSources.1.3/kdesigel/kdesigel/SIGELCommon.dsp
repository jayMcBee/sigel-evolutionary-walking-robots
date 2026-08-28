# Microsoft Developer Studio Project File - Name="SIGELCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SIGELCOMMON - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SIGELCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SIGELCommon.mak" CFG="SIGELCOMMON - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SIGELCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SIGELCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SIGELCommon", MDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SIGELCommon - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "ui" /I "supportingLibs\fparser" /I ".\include" /I "$(PVM_ROOT)\include" /I "$(PVM_ROOT)\src" /I "$(QTDIR)\include" /I ".\supportingLibs\cv97" /I ".\supportingLibs\DynaMechs\dm" /I ".\supportingLibs\Dynamo\src\inc" /I ".\supportingLibs\newmat09" /I ".\supportingLibs\qhull" /I ".\supportingLibs\SOLID20\src" /I ".\supportingLibs\SOLID20\include" /D "_WINDOWS" /D "QT_ALTERNATE_QTSMANIP" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "SIGELCommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "ui" /I "supportingLibs\fparser" /I ".\include" /I "$(PVM_ROOT)\include" /I "$(PVM_ROOT)\src" /I "$(QTDIR)\include" /I ".\supportingLibs\cv97" /I ".\supportingLibs\DynaMechs\dm" /I ".\supportingLibs\Dynamo\src\inc" /I ".\supportingLibs\newmat09" /I ".\supportingLibs\qhull" /I ".\supportingLibs\SOLID20\src" /I ".\supportingLibs\SOLID20\include" /D "QT_ALTERNATE_QTSMANIP" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
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

# Name "SIGELCommon - Win32 Release"
# Name "SIGELCommon - Win32 Debug"
# Begin Group "Environment"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Environment\SIG_Environment.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Environment\SIG_Environment.h
# End Source File
# End Group
# Begin Group "GP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPActivePVMHost.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPActivePVMHost.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPAdaptiveWalkingFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPAdaptiveWalkingFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPCrossOverTournament.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPCrossOverTournament.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPException.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPExperiment.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPExperiment.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPExperimentHistoryEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPExperimentHistoryEntry.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPFitnessTrainer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPFitnessTrainer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPForceFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPForceFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPFullDataRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPFullDataRecorder.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPIndividual.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPIndividual.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPManager.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPManager.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPMutationTournament.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPMutationTournament.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPNiceWalkingFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPNiceWalkingFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPOperations.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPOperations.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPParameter.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPPopulation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPPopulation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPPVMData.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPPVMData.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPPVMHost.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPPVMHost.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPPVMTask.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPPVMTask.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPRealSpeedFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPRealSpeedFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPSimpleFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPSimpleFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPSimpleRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPSimpleRecorder.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPSimpleTournament.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPSimpleTournament.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPStepperFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPStepperFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPTournament.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPTournament.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPTournamentIndividual.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPTournamentIndividual.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GPZorcWalkingFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GPZorcWalkingFitnessFunction.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\SIG_GUIGPManager.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\SIG_GUIGPManager.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_GP\WIN_SIG_GPRemoteZORCFitnessFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_GP\WIN_SIG_GPRemoteZORCFitnessFunction.h
# End Source File
# End Group
# Begin Group "Program"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Program\SIG_Program.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Program\SIG_Program.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Program\SIG_ProgramLine.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Program\SIG_ProgramLine.h
# End Source File
# End Group
# Begin Group "RealInterface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_RealInterface\SIG_RealInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RealInterface\SIG_RealInterface.h
# End Source File
# End Group
# Begin Group "Robot"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Robot\IFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\IFunctions.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Body.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Body.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_CommandParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_CommandParameters.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_ContactSensor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_ContactSensor.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_CylindricalJoint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_CylindricalJoint.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Drive.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Drive.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Geometry.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Geometry.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_GeometryIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_GeometryIterator.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_GlueJoint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_GlueJoint.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Joint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Joint.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_JointSensor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_JointSensor.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_LanguageParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_LanguageParameters.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Link.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Link.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Material.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Material.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Mirtich.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Mirtich.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_PitchRollSensor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_PitchRollSensor.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Polygon.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Polygon.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Robot.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Robot.h
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_RobotExceptions.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_RotationalJoint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_RotationalJoint.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_Sensor.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_Sensor.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_TranslationalJoint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_TranslationalJoint.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Robot\SIG_WrongKinematicsException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Robot\SIG_WrongKinematicsException.h
# End Source File
# End Group
# Begin Group "RobotIO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotBuilder.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotCompiler.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotCompilerObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotCompilerObjects.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotCompilerStructure.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotCompilerStructure.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotFactory.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotIOExceptions.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotIOExceptions.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotScanner.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotScanner.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_RobotUnstreamer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_RobotUnstreamer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_Scanner.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_Scanner.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_RobotIO\SIG_UnstreamerScanner.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_RobotIO\SIG_UnstreamerScanner.h
# End Source File
# End Group
# Begin Group "Simulation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\moc_SIG_Simulation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_CommandInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_CommandInterface.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_DynaMechsCommandInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_DynaMechsCommandInterface.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_DynaMechsLink.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_DynaMechsLink.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_DynaMechsSimulationData.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_DynaMechsSimulationData.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_DynaMechsSimulationQueries.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_DynaMechsSimulationQueries.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_EarlyRunTermSimulation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_EarlyRunTermSimulation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_Interpreter.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_Interpreter.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_Recorder.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_Recorder.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_RecorderBadRecordingOrderException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_RecorderBadRecordingOrderException.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_RecorderNoQueriesSetException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_RecorderNoQueriesSetException.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_Register.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_Register.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_RegisterWrongSizeException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_RegisterWrongSizeException.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_Simulation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_Simulation.h

!IF  "$(CFG)" == "SIGELCommon - Win32 Release"

# Begin Custom Build - moc'ing $(InputName)
ProjDir=.
InputPath=.\include\SIGEL_Simulation\SIG_Simulation.h
InputName=SIG_Simulation

"$(ProjDir)\src\SIGEL_Simulation\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_Simulation\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "SIGELCommon - Win32 Debug"

# Begin Custom Build - moc'ing $(InputName)
ProjDir=.
InputPath=.\include\SIGEL_Simulation\SIG_Simulation.h
InputName=SIG_Simulation

"$(ProjDir)\src\SIGEL_Simulation\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc $(InputPath) -o $(ProjDir)\src\SIGEL_Simulation\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_SimulationCannotSolveException.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_SimulationCannotSolveException.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_SimulationData.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_SimulationData.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_SimulationParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_SimulationParameters.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Simulation\SIG_SimulationQueries.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Simulation\SIG_SimulationQueries.h
# End Source File
# End Group
# Begin Group "Tools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Tools\SIG_Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Tools\SIG_Exception.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Tools\SIG_IO.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Tools\SIG_IO.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Tools\SIG_Randomizer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Tools\SIG_Randomizer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Tools\SIG_TypeConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Tools\SIG_TypeConverter.h
# End Source File
# End Group
# Begin Group "Visualisation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_EnvironmentRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_EnvironmentRenderer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_EnvironmentVisualisation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_EnvironmentVisualisation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_FloatingText.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_FloatingText.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_Renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_Renderer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_RenderRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_RenderRecorder.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_RobotRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_RobotRenderer.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_RobotVisualisation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_RobotVisualisation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_SceneObject.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_SceneObject.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_SimulationVisualisation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_SimulationVisualisation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_ViewSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_ViewSettings.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_Visualisation.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_Visualisation.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_Visualisation\SIG_VisualSceneObject.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_Visualisation\SIG_VisualSceneObject.h
# End Source File
# End Group
# Begin Group "CommonGUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\SIGEL_CommonGUI\moc_SIG_VisualisationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_CommonGUI\SIG_FloatingTextLabel.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_CommonGUI\SIG_FloatingTextLabel.h
# End Source File
# Begin Source File

SOURCE=.\src\SIGEL_CommonGUI\SIG_VisualisationWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SIGEL_CommonGUI\SIG_VisualisationWidget.h

!IF  "$(CFG)" == "SIGELCommon - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\SIGEL_CommonGUI
ProjDir=.
InputPath=.\include\SIGEL_CommonGUI\SIG_VisualisationWidget.h
InputName=SIG_VisualisationWidget

"$(ProjDir)\src\SIGEL_CommonGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(ProjDir)\src\SIGEL_CommonGUI\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "SIGELCommon - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\include\SIGEL_CommonGUI
ProjDir=.
InputPath=.\include\SIGEL_CommonGUI\SIG_VisualisationWidget.h
InputName=SIG_VisualisationWidget

"$(ProjDir)\src\SIGEL_CommonGUI\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(ProjDir)\src\SIGEL_CommonGUI\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
