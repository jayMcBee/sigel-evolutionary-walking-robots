include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)

noinst_LIBRARIES = libMT_GPSystem.a

libMT_GPSystem_a_SOURCES = MT_TranslatedIndividual.cpp MT_Trainingset.cpp MT_TrainingCase.cpp MT_TournamentManager.cpp MT_Tournament.cpp MT_StatisticsElement.cpp MT_Statistics.cpp MT_Search.cpp MT_Randomizer.cpp MT_Programline.cpp MT_Program.cpp MT_Population.cpp MT_Operand.cpp MT_Interpreter.cpp MT_Individual.cpp MT_GPManager.cpp MT_FitnessTrainer.cpp 


EXTRA_DIST = MT_FitnessTrainer.cpp MT_GPManager.cpp MT_Individual.cpp MT_Interpreter.cpp MT_Operand.cpp MT_Population.cpp MT_Program.cpp MT_Programline.cpp MT_Randomizer.cpp MT_Search.cpp MT_Statistics.cpp MT_StatisticsElement.cpp MT_Tournament.cpp MT_TournamentManager.cpp MT_TrainingCase.cpp MT_Trainingset.cpp MT_TranslatedIndividual.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=MT_*moc.cpp

nodist_libMT_GPSystem_a_SOURCES = MT_GPManager_moc.cpp

%_moc.cpp: ../../include/MT_GPSystem/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
	
