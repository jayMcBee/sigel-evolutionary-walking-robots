include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_SlaveGUI.a

INCLUDES = $(all_includes)

libSIGEL_SlaveGUI_a_SOURCES = SIG_SimulationWindow.cpp SIG_SimulationWidget.cpp SIG_SimulationVisualisationWidget.cpp SIG_SimulationControls.cpp SIG_MovieSettingsDialog.cpp 


EXTRA_DIST = SIG_MovieSettingsDialog.cpp SIG_SimulationControls.cpp SIG_SimulationVisualisationWidget.cpp SIG_SimulationWidget.cpp SIG_SimulationWindow.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*moc.cpp

nodist_libSIGEL_SlaveGUI_a_SOURCES = SIG_MovieSettingsDialog_moc.cpp \
SIG_SimulationControls_moc.cpp \
SIG_SimulationVisualisationWidget_moc.cpp \
SIG_SimulationWidget_moc.cpp \
SIG_SimulationWindow_moc.cpp

%_moc.cpp: ../../include/SIGEL_SlaveGUI/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
