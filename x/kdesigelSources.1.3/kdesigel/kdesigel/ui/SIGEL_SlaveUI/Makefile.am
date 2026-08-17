include $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_SlaveUI.a

INCLUDES = $(all_includes)

libSIGEL_SlaveUI_a_SOURCES = SIG_SimulationWidgetBase.ui SIG_MovieSettingsDialogBase.ui 


EXTRA_DIST = SIG_MovieSettingsDialogBase.ui SIG_SimulationWidgetBase.ui 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*.cpp

nodist_libSIGEL_SlaveUI_a_SOURCES = SIG_MovieSettingsDialogBase.cpp \
SIG_SimulationWidgetBase.cpp \
SIG_MovieSettingsDialogBase_moc.cpp \
SIG_SimulationWidgetBase_moc.cpp

%_moc.cpp: ../../include/SIGEL_SlaveGUI/%.h %.ui
	$(QTDIR)/bin/moc -nw -o $@ $<

%.cpp: %.ui %.h
	$(QTDIR)/bin/uic -o $@ -impl ../../include/SIGEL_SlaveGUI/$*.h $<

%.h: %.ui
	$(QTDIR)/bin/uic -o ../../include/SIGEL_SlaveGUI/$@ $<
