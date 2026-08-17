include $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_MasterUI.a

INCLUDES = $(all_includes)

libSIGEL_MasterUI_a_SOURCES = SIG_SimulationParameterBase.ui SIG_RobotBase.ui SIG_LanguageParametersBase.ui SIG_IndividualViewBase.ui SIG_IndividualListBase.ui SIG_GPParameterBase.ui SIG_ExperimentViewBase.ui SIG_EnvironmentBase.ui SIG_EditHostDialogBase.ui 


EXTRA_DIST = SIG_EditHostDialogBase.ui SIG_EnvironmentBase.ui SIG_ExperimentViewBase.ui SIG_GPParameterBase.ui SIG_IndividualListBase.ui SIG_IndividualViewBase.ui SIG_LanguageParametersBase.ui SIG_RobotBase.ui SIG_SimulationParameterBase.ui 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*.cpp

nodist_libSIGEL_MasterUI_a_SOURCES = SIG_EnvironmentBase.cpp \
SIG_EditHostDialogBase.cpp \
SIG_EnvironmentBase.cpp \
SIG_ExperimentViewBase.cpp \
SIG_GPParameterBase.cpp \
SIG_IndividualListBase.cpp \
SIG_IndividualViewBase.cpp \
SIG_LanguageParametersBase.cpp \
SIG_RobotBase.cpp \
SIG_SimulationParameterBase.cpp \
SIG_EnvironmentBase_moc.cpp \
SIG_EditHostDialogBase_moc.cpp \
SIG_EnvironmentBase_moc.cpp \
SIG_ExperimentViewBase_moc.cpp \
SIG_GPParameterBase_moc.cpp \
SIG_IndividualListBase_moc.cpp \
SIG_IndividualViewBase_moc.cpp \
SIG_LanguageParametersBase_moc.cpp \
SIG_RobotBase_moc.cpp \
SIG_SimulationParameterBase_moc.cpp

%_moc.cpp: ../../include/SIGEL_MasterGUI/%.h %.ui
	$(QTDIR)/bin/moc -nw -o $@ $<

%.cpp: %.ui %.h
	$(QTDIR)/bin/uic -o $@ -impl ../../include/SIGEL_MasterGUI/$*.h $<

%.h: %.ui
	$(QTDIR)/bin/uic -o ../../include/SIGEL_MasterGUI/$@ $<

