include $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libMT_UI.a

INCLUDES = $(all_includes)

libMT_UI_a_SOURCES = MT_StatisticsWidgetBase.ui MT_SelectionWidgetBase.ui MT_SearchWidgetBase.ui MT_PopulationWidgetBase.ui MT_IndividualWidgetBase.ui MT_ExperimentWidgetBase.ui MT_EstimationWidgetBase.ui MT_AddIndividualsWidget.ui MT_AddConstantsWidgetBase.ui 


EXTRA_DIST = MT_AddConstantsWidgetBase.ui MT_AddIndividualsWidget.ui MT_EstimationWidgetBase.ui MT_ExperimentWidgetBase.ui MT_IndividualWidgetBase.ui MT_PopulationWidgetBase.ui MT_SearchWidgetBase.ui MT_SelectionWidgetBase.ui MT_StatisticsWidgetBase.ui 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=SIG_*.cpp

nodist_libMT_UI_a_SOURCES = MT_StatisticsWidgetBase.cpp \
MT_SelectionWidgetBase.cpp \
MT_SearchWidgetBase.cpp \
MT_PopulationWidgetBase.cpp \
MT_IndividualWidgetBase.cpp \
MT_ExperimentWidgetBase.cpp \
MT_EstimationWidgetBase.cpp \
MT_AddIndividualsWidget.cpp \
MT_AddConstantsWidgetBase.cpp \
MT_StatisticsWidgetBase_moc.cpp \
MT_SelectionWidgetBase_moc.cpp \
MT_SearchWidgetBase_moc.cpp \
MT_PopulationWidgetBase_moc.cpp \
MT_IndividualWidgetBase_moc.cpp \
MT_ExperimentWidgetBase_moc.cpp \
MT_EstimationWidgetBase_moc.cpp \
MT_AddConstantsWidgetBase_moc.cpp \
MT_AddIndividualsWidget_moc.cpp

%_moc.cpp: ../../include/MT_GUI/%.h %.ui
	$(QTDIR)bin/moc -nw -o $@ $<

%.cpp: %.ui %.h
	$(QTDIR)bin/uic -o $@ -impl ../../include/MT_GUI/$*.h $<

%.h: %.ui
	$(QTDIR)bin/uic -o ../../include/MT_GUI/$@ $<
