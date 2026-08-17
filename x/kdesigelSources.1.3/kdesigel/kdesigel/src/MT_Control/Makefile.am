include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libMT_Control.a

INCLUDES = $(all_includes)

libMT_Control_a_SOURCES = MT_Substitute.cpp MT_Evaluator.cpp MT_Classifier.cpp MT_Controller.cpp 

EXTRA_DIST = MT_Controller.cpp MT_Classifier.cpp MT_Evaluator.cpp MT_Substitute.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=MT_*moc.cpp

nodist_libMT_Control_a_SOURCES = MT_Controller_moc.cpp

%_moc.cpp: ../../include/MT_Control/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
