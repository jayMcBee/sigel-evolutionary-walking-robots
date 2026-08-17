include  $(top_srcdir)/makefile-rules

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libMT_GUI.a

INCLUDES = $(all_includes)

libMT_GUI_a_SOURCES = MT_PopListViewItem.cpp MT_ExperimentItem.cpp DoubleSpinBox.cpp MT_WidgetBase.cpp MT_StatisticsWidget.cpp MT_SelectionWidget.cpp MT_SearchWidget.cpp MT_PopulationWidget.cpp MT_MainWindow.cpp MT_IndividualWidget.cpp MT_ExperimentWidget.cpp MT_EstimationWidget.cpp MT_Editor.cpp MT_AddConstantsWidget.cpp 

EXTRA_DIST = MT_AddConstantsWidget.cpp MT_Editor.cpp MT_EstimationWidget.cpp MT_ExperimentWidget.cpp MT_IndividualWidget.cpp MT_MainWindow.cpp MT_PopulationWidget.cpp MT_SearchWidget.cpp MT_SelectionWidget.cpp MT_StatisticsWidget.cpp MT_WidgetBase.cpp DoubleSpinBox.cpp MT_ExperimentItem.cpp MT_PopListViewItem.cpp 

####### kdevelop will overwrite this part!!! (end)############

CLEANFILES=MT_*moc.cpp

nodist_libMT_GUI_a_SOURCES = MT_AddConstantsWidget_moc.cpp \
MT_Editor_moc.cpp \
MT_EstimationWidget_moc.cpp \
MT_ExperimentWidget_moc.cpp \
MT_IndividualWidget_moc.cpp \
MT_MainWindow_moc.cpp \
MT_PopulationWidget_moc.cpp \
MT_SearchWidget_moc.cpp \
MT_SelectionWidget_moc.cpp \
MT_StatisticsWidget_moc.cpp

%_moc.cpp: ../../include/MT_GUI/%.h %.cpp
	$(QTDIR)bin/moc -nw -o $@ $<
