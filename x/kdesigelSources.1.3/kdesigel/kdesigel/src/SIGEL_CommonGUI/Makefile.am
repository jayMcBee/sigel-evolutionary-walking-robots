include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libSIGEL_CommonGUI.a

INCLUDES = $(all_includes)

libSIGEL_CommonGUI_a_SOURCES = SIG_VisualisationWidget.cpp SIG_FloatingTextLabel.cpp 


EXTRA_DIST = SIG_FloatingTextLabel.cpp SIG_VisualisationWidget.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*moc.cpp

nodist_libSIGEL_CommonGUI_a_SOURCES = SIG_VisualisationWidget_moc.cpp

%_moc.cpp: ../../include/SIGEL_CommonGUI/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<

