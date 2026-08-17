include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_Simulation.a

INCLUDES = $(all_includes)

libSIGEL_Simulation_a_SOURCES = SIG_EarlyRunTermSimulation.cpp SIG_TranslationalController.cpp SIG_SimulationQueries.cpp SIG_SimulationParameters.cpp SIG_SimulationData.cpp SIG_SimulationCannotSolveException.cpp SIG_Simulation.cpp SIG_RotationalController.cpp SIG_RegisterWrongSizeException.cpp SIG_Register.cpp SIG_RecorderNoQueriesSetException.cpp SIG_RecorderBadRecordingOrderException.cpp SIG_Recorder.cpp SIG_Interpreter.cpp SIG_DynaSystemWrongNumberException.cpp SIG_DynaSystem.cpp SIG_DynaSensor.cpp SIG_DynaMoSimulationQueries.cpp SIG_DynaMoSimulationData.cpp SIG_DynaMoCommandInterface.cpp SIG_DynaMechsSimulationQueries.cpp SIG_DynaMechsSimulationData.cpp SIG_DynaMechsLink.cpp SIG_DynaMechsCommandInterface.cpp SIG_DynaLink.cpp SIG_DynaJoint.cpp SIG_DynaDrive.cpp SIG_DynaCallbacks.cpp SIG_Dyna.cpp SIG_CommandInterface.cpp 


EXTRA_DIST = SIG_CommandInterface.cpp SIG_Dyna.cpp SIG_DynaCallbacks.cpp SIG_DynaDrive.cpp SIG_DynaJoint.cpp SIG_DynaLink.cpp SIG_DynaMechsCommandInterface.cpp SIG_DynaMechsLink.cpp SIG_DynaMechsSimulationData.cpp SIG_DynaMechsSimulationQueries.cpp SIG_DynaMoCommandInterface.cpp SIG_DynaMoSimulationData.cpp SIG_DynaMoSimulationQueries.cpp SIG_DynaSensor.cpp SIG_DynaSystem.cpp SIG_DynaSystemWrongNumberException.cpp SIG_Interpreter.cpp SIG_Recorder.cpp SIG_RecorderBadRecordingOrderException.cpp SIG_RecorderNoQueriesSetException.cpp SIG_Register.cpp SIG_RegisterWrongSizeException.cpp SIG_RotationalController.cpp SIG_Simulation.cpp SIG_SimulationCannotSolveException.cpp SIG_SimulationData.cpp SIG_SimulationParameters.cpp SIG_SimulationQueries.cpp SIG_TranslationalController.cpp SIG_EarlyRunTermSimulation.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*moc.cpp

nodist_libSIGEL_Simulation_a_SOURCES = SIG_DynaSystem_moc.cpp \
SIG_Simulation_moc.cpp

%_moc.cpp: ../../include/SIGEL_Simulation/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
