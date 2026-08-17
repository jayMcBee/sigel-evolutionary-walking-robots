include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_MasterGUI.a

INCLUDES = $(all_includes)

libSIGEL_MasterGUI_a_SOURCES = SIG_TextView.cpp SIG_SimulationParameter.cpp SIG_RobotView.cpp SIG_RenameDialog.cpp SIG_MainWindow.cpp SIG_LanguageParameters.cpp SIG_InfoBox.cpp SIG_IndividualView.cpp SIG_IndividualListItem.cpp SIG_IndividualList.cpp SIG_GPParameter.cpp SIG_ExperimentView.cpp SIG_ExperimentListView.cpp SIG_ExperimentItem.cpp SIG_Experiment.cpp SIG_EnvironmentView.cpp SIG_EditHostDialog.cpp SIG_EditCommandDialog.cpp SIG_AllIndividualsView.cpp SIG_AddIndividualsDialog.cpp 


EXTRA_DIST = SIG_AddIndividualsDialog.cpp SIG_AllIndividualsView.cpp SIG_EditCommandDialog.cpp SIG_EditHostDialog.cpp SIG_EnvironmentView.cpp SIG_Experiment.cpp SIG_ExperimentItem.cpp SIG_ExperimentListView.cpp SIG_ExperimentView.cpp SIG_GPParameter.cpp SIG_IndividualList.cpp SIG_IndividualListItem.cpp SIG_IndividualView.cpp SIG_InfoBox.cpp SIG_LanguageParameters.cpp SIG_MainWindow.cpp SIG_RenameDialog.cpp SIG_RobotView.cpp SIG_SimulationParameter.cpp SIG_TextView.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*moc.cpp

nodist_libSIGEL_MasterGUI_a_SOURCES = SIG_AddIndividualsDialog_moc.cpp \
SIG_AllIndividualsView_moc.cpp \
SIG_EditCommandDialog_moc.cpp \
SIG_EditHostDialog_moc.cpp \
SIG_EnvironmentView_moc.cpp \
SIG_Experiment_moc.cpp \
SIG_ExperimentListView_moc.cpp \
SIG_ExperimentView_moc.cpp \
SIG_GPParameter_moc.cpp \
SIG_IndividualList_moc.cpp \
SIG_IndividualView_moc.cpp \
SIG_LanguageParameters_moc.cpp \
SIG_MainWindow_moc.cpp \
SIG_RenameDialog_moc.cpp \
SIG_RobotView_moc.cpp \
SIG_SimulationParameter_moc.cpp \
SIG_TextView_moc.cpp

%_moc.cpp: ../../include/SIGEL_MasterGUI/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
