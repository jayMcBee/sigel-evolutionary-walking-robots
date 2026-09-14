/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <qapplication.h>
#include <QCloseEvent>
#include <QList>
#include <qmessagebox.h>
#include <QIcon>
#include <qpixmap.h>
#include <qstatusbar.h>
#include <qfontdialog.h>

#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_MasterGUI/SIG_InfoBox.h"

#include <cstdlib>

namespace SIGEL_MasterGUI
{

SIG_MainWindow::SIG_MainWindow( QWidget * parent, const char * name, Qt::WindowFlags f ) : QMainWindow( parent, f )
{
  setObjectName( QString::fromUtf8( name ) );
  // Qt 2's QMainWindow defaulted to small pixmaps (qmainwindow.cpp:177, ubp
  // FALSE) and drew each QIconSet member at its OWN size. These pixmaps are not
  // a uniform size -- measured across the 30 referenced files, Small runs
  // 9x25..25x25 and Large 19x48..48x48 -- and Qt 6 has exactly one iconSize per
  // toolbar, choosing from the QIcon by pixel size rather than by the Small and
  // Large roles Qt 2 named explicitly. 25 is the largest Small, so nothing is
  // scaled UP past what 1.3 drew; icons whose two pixmaps straddle it are still
  // resampled where Qt 2 blitted natively. That residue is not removable
  // without splitting each QIcon, and is recorded in PORTING.md.
  setIconSize( QSize( 25, 25 ) );
#ifdef _WINDOWS
  QString sigelRoot( ::getenv( "SIGEL_ROOT" ) );
#else
  QString sigelRoot( std::getenv( "SIGEL_ROOT" ) );
#endif

  resize( 900, 750 );
  setWindowTitle( "SIGEL" );

  splitter = new QSplitter( this );
  splitter->setObjectName( "Splitter" );
  splitter->setFrameStyle( QFrame::Box | QFrame::Sunken );

  setCentralWidget( splitter );

  widgetStack = new QStackedWidget( splitter );
  widgetStack->setObjectName( "WidgetStack" ); // this was splitter before...
  widgetStack->setContentsMargins( 6, 6, 6, 6 );
  widgetStack->setFrameStyle( QFrame::Box | QFrame::Sunken );

  experimentListView = new SIGEL_MasterGUI::SIG_ExperimentListView( splitter, "ExperimentListView", widgetStack );

  splitter->insertWidget( 0, experimentListView );
  splitter->setOpaqueResize();
  
  QList<int> valList;
  valList += 2;
  valList += 6;
  splitter->setSizes( valList );

  splitter->setContentsMargins( 6, 6, 6, 6 );

  // create the file menu and toolbar
  fileMenu = new QMenu(this);
  fileToolBar = new QToolBar( this );
  fileToolBar->setObjectName( "fileToolbar" );
  addToolBar( Qt::TopToolBarArea, fileToolBar );
  fileToolBar->setWindowTitle( "File" );

  QToolBar *viewToolBar = new QToolBar( this );
  viewToolBar->setObjectName( "viewToolBar" );
  addToolBar( Qt::TopToolBarArea, viewToolBar );
  viewToolBar->setWindowTitle( "View" );

  QToolBar *individualsToolBar = new QToolBar( this );
  individualsToolBar->setObjectName( "individualsToolBar" );
  addToolBar( Qt::TopToolBarArea, individualsToolBar );
  individualsToolBar->setWindowTitle( "Individuals" );

  // create the new experiment action
  QIcon icon_newExperimentAction( QPixmap( sigelRoot + "/pixmaps/newExperimentSmall.xpm" ) );
  icon_newExperimentAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/newExperimentLarge.xpm" ) );
  QAction *newExperimentAction = new QAction( icon_newExperimentAction, "&New Experiment", this );
  newExperimentAction->setToolTip( "New" );
  newExperimentAction->setIconText( "New" );
  newExperimentAction->setShortcut( Qt::CTRL | Qt::Key_N );
  fileMenu->addAction( newExperimentAction );
  fileToolBar->addAction( newExperimentAction );
  experimentListView->experimentListViewMenu->addAction( newExperimentAction );
  QObject::connect( newExperimentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotNewExperiment() ) );
  newExperimentAction->setStatusTip( "Create a new experiment..." );

  QAction *renameExperimentAction = new QAction( "&Rename Experiment", this );
  renameExperimentAction->setToolTip( "Rename" );
  renameExperimentAction->setIconText( "Rename" );
  renameExperimentAction->setShortcut( Qt::CTRL | Qt::Key_R );
  fileMenu->addAction( renameExperimentAction );
  QObject::connect( renameExperimentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotRenameExperiment() ) );
  renameExperimentAction->setStatusTip( "Rename the currently selected experiment..." );

  QIcon icon_deleteExperimentAction( QPixmap( sigelRoot + "/pixmaps/deleteExperimentSmall.xpm" ) );
  icon_deleteExperimentAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/deleteExperimentLarge.xpm" ) );
  QAction *deleteExperimentAction = new QAction( icon_deleteExperimentAction, "&Delete Experiment", this );
  deleteExperimentAction->setToolTip( "Delete" );
  deleteExperimentAction->setIconText( "Delete" );
  deleteExperimentAction->setShortcut( Qt::CTRL | Qt::Key_D );
  fileMenu->addAction( deleteExperimentAction );
  fileToolBar->addAction( deleteExperimentAction );
  experimentListView->experimentListViewMenu->addAction( deleteExperimentAction );
  QObject::connect( deleteExperimentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotDeleteExperiment() ) );
  deleteExperimentAction->setStatusTip( "Delete the selected experiment..." );

  fileMenu->addSeparator();
  fileToolBar->addSeparator();
  experimentListView->experimentListViewMenu->addSeparator();

  QIcon icon_openExperimentAction( QPixmap( sigelRoot + "/pixmaps/openExperimentSmall.xpm" ) );
  icon_openExperimentAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/openExperimentLarge.xpm" ) );
  QAction *openExperimentAction = new QAction( icon_openExperimentAction, "&Open Experiment", this );
  openExperimentAction->setToolTip( "Open" );
  openExperimentAction->setIconText( "Open" );
  openExperimentAction->setShortcut( Qt::CTRL | Qt::Key_O );
  fileMenu->addAction( openExperimentAction );
  fileToolBar->addAction( openExperimentAction );
  experimentListView->experimentListViewMenu->addAction( openExperimentAction );
  QObject::connect( openExperimentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotLoadExperiment() ) );
  openExperimentAction->setStatusTip( "Open an experiment..." );

  QIcon icon_saveExperimentAction( QPixmap( sigelRoot + "/pixmaps/saveExperimentSmall.xpm" ) );
  icon_saveExperimentAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/saveExperimentLarge.xpm" ) );
  QAction *saveExperimentAction = new QAction( icon_saveExperimentAction, "&Save Experiment", this );
  saveExperimentAction->setToolTip( "Save" );
  saveExperimentAction->setIconText( "Save" );
  saveExperimentAction->setShortcut( Qt::CTRL | Qt::Key_S );
  fileMenu->addAction( saveExperimentAction );
  fileToolBar->addAction( saveExperimentAction );
  experimentListView->experimentListViewMenu->addAction( saveExperimentAction );
  QObject::connect( saveExperimentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotSaveExperiment() ) );
  saveExperimentAction->setStatusTip( "Save the selected experiment..." );

  fileMenu->addSeparator();

  // create the import menu
  QMenu *importMenu = new QMenu( this );
  importMenu->setObjectName( "importMenu" );

  // create the actions and insert them into the menu
  QAction *importGPParametersAction = new QAction( "GP-Parameters", this );
  importGPParametersAction->setToolTip( "Import GP-Parameters" );
  importGPParametersAction->setIconText( "Import GP-Parameters" );
  importGPParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_G );
  importMenu->addAction( importGPParametersAction );
  QObject::connect( importGPParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotGPParametersImport() ) );
  QAction *importSimulationParametersAction = new QAction( "Simulation-Parameters", this );
  importSimulationParametersAction->setToolTip( "Import Simulation-Parameters" );
  importSimulationParametersAction->setIconText( "Import Simulation-Parameters" );
  importSimulationParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_S );
  importMenu->addAction( importSimulationParametersAction );
  QObject::connect( importSimulationParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotSimulationParametersImport() ) );
  QAction *importLanguageParametersAction = new QAction( "Language-Parameters", this );
  importLanguageParametersAction->setToolTip( "Import Language-Parameters" );
  importLanguageParametersAction->setIconText( "Import Language-Parameters" );
  importLanguageParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_L );
  importMenu->addAction( importLanguageParametersAction );
  QObject::connect( importLanguageParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotLanguageParametersImport() ) );
  QAction *importEnvironmentAction = new QAction( "Environment", this );
  importEnvironmentAction->setToolTip( "Import Environment" );
  importEnvironmentAction->setIconText( "Import Environment" );
  importEnvironmentAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_E );
  importMenu->addAction( importEnvironmentAction );
  QObject::connect( importEnvironmentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotEnvironmentImport() ) );
  QAction *importPopulationAction = new QAction( "Population", this );
  importPopulationAction->setToolTip( "Import Population" );
  importPopulationAction->setIconText( "Import Population" );
  importPopulationAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_P );
  importMenu->addAction( importPopulationAction );
  QObject::connect( importPopulationAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotPopulationImport() ) );
  QAction *importRobotAction = new QAction( "Robot", this );
  importRobotAction->setToolTip( "Import Robot" );
  importRobotAction->setIconText( "Import Robot" );
  importRobotAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_R );
  importMenu->addAction( importRobotAction );
  QObject::connect( importRobotAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotRobotImport() ) );

  importMenu->addSeparator();

  QAction *importProgramAction = new QAction( "Program", this );
  importProgramAction->setToolTip( "Import Program" );
  importProgramAction->setIconText( "Import Program" );
  importProgramAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_K );
  importMenu->addAction( importProgramAction );
  QObject::connect( importProgramAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotProgramImport() ) );

  QAction *importIndividualAction = new QAction( "Individual", this );
  importIndividualAction->setToolTip( "Import Individual" );
  importIndividualAction->setIconText( "Import Individual" );
  importIndividualAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_L );
  importMenu->addAction( importIndividualAction );
  QObject::connect( importIndividualAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotIndividualImport() ) );
  
  importMenu->setTitle( "Import" );
  fileMenu->addMenu( importMenu );

  // create the export menu
  QMenu *exportMenu = new QMenu( this );
  exportMenu->setObjectName( "exportMenu" );

  QAction *exportGPParametersAction = new QAction( "GP-Parameters", this );
  exportGPParametersAction->setToolTip( "Export GP-Parameters" );
  exportGPParametersAction->setIconText( "Export GP-Parameters" );
  exportGPParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_G );
  exportMenu->addAction( exportGPParametersAction );
  QObject::connect( exportGPParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotGPParametersExport() ) );
  QAction *exportSimulationParametersAction = new QAction( "Simulation-Parameters", this );
  exportSimulationParametersAction->setToolTip( "Export Simulation-Parameters" );
  exportSimulationParametersAction->setIconText( "Export Simulation-Parameters" );
  exportSimulationParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_S );
  exportMenu->addAction( exportSimulationParametersAction );
  QObject::connect( exportSimulationParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotSimulationParametersExport() ) );
  QAction *exportLanguageParametersAction = new QAction( "Language-Parameters", this );
  exportLanguageParametersAction->setToolTip( "Export Language-Parameters" );
  exportLanguageParametersAction->setIconText( "Export Language-Parameters" );
  exportLanguageParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_L );
  exportMenu->addAction( exportLanguageParametersAction );
  QObject::connect( exportLanguageParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotLanguageParametersExport() ) );
  QAction *exportEnvironmentAction = new QAction( "Environment", this );
  exportEnvironmentAction->setToolTip( "Export Environment" );
  exportEnvironmentAction->setIconText( "Export Environment" );
  exportEnvironmentAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_E );
  exportMenu->addAction( exportEnvironmentAction );
  QObject::connect( exportEnvironmentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotEnvironmentExport() ) );
  QAction *exportPopulationAction = new QAction( "Population", this );
  exportPopulationAction->setToolTip( "Export Population" );
  exportPopulationAction->setIconText( "Export Population" );
  exportPopulationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_P );
  exportMenu->addAction( exportPopulationAction );
  QObject::connect( exportPopulationAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotPopulationExport() ) );

  exportMenu->addSeparator();

  QAction *exportProgramAction = new QAction( "Program", this );
  exportProgramAction->setToolTip( "Export Program" );
  exportProgramAction->setIconText( "Export Program" );
  exportProgramAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_K );
  exportMenu->addAction( exportProgramAction );
  QObject::connect( exportProgramAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotProgramExport() ) );

  QAction *exportIndividualAction = new QAction( "Individual", this );
  exportIndividualAction->setToolTip( "Export Individual" );
  exportIndividualAction->setIconText( "Export Individual" );
  exportIndividualAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_L );
  exportMenu->addAction( exportIndividualAction );
  QObject::connect( exportIndividualAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotIndividualExport() ) );

  exportMenu->addSeparator();

  QAction *exportToGNUPlotAction = new QAction( "...to GNU plot", this );
  exportToGNUPlotAction->setToolTip( "Export to GNU plot" );
  exportToGNUPlotAction->setIconText( "Export to GNU plot" );
  exportToGNUPlotAction->setShortcut( Qt::ALT | Qt::Key_6 );
  exportMenu->addAction( exportToGNUPlotAction );
  QObject::connect( exportToGNUPlotAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotGNUPlotExport() ) );

  exportMenu->setTitle( "Export" );
  fileMenu->addMenu( exportMenu );

  fileMenu->addSeparator();

  fileToolBar->addSeparator();

  QIcon icon_quitProgramAction( QPixmap( sigelRoot + "/pixmaps/quitApplicationSmall.xpm" ) );
  icon_quitProgramAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/quitApplicationLarge.xpm" ) );
  QAction *quitProgramAction = new QAction( icon_quitProgramAction, "&Quit", this );
  quitProgramAction->setToolTip( "Quit" );
  quitProgramAction->setIconText( "Quit" );
  quitProgramAction->setShortcut( Qt::CTRL | Qt::Key_Q );
  fileMenu->addAction( quitProgramAction );
  fileToolBar->addAction( quitProgramAction );
  QObject::connect( quitProgramAction,
		    SIGNAL( triggered() ),
		    this,
		    SLOT( slotAboutToQuit() ) );
  quitProgramAction->setStatusTip( "Quit application..." );

  // create the view menu
  viewMenu = new QMenu( this );
  viewMenu->setObjectName( "viewMenu" );
 
  // insert a lot of actions into the view menu
  
  QIcon icon_viewPopulationAction( QPixmap( sigelRoot + "/pixmaps/individualSmall.xpm" ) );
  icon_viewPopulationAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/individualLarge.xpm" ) );
  QAction *viewPopulationAction = new QAction( icon_viewPopulationAction, "&Population", this );
  viewPopulationAction->setToolTip( "Population" );
  viewPopulationAction->setIconText( "Population" );
  viewPopulationAction->setShortcut( Qt::ALT | Qt::Key_P );
  viewMenu->addAction( viewPopulationAction );
  viewToolBar->addAction( viewPopulationAction );
  QObject::connect( viewPopulationAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowIndividuals() ) );
  viewPopulationAction->setStatusTip( "View population." );

  QIcon icon_viewRobotAction( QPixmap( sigelRoot + "/pixmaps/robotSmall.xpm" ) );
  icon_viewRobotAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/robotLarge.xpm" ) );
  QAction *viewRobotAction = new QAction( icon_viewRobotAction, "&Robot", this );
  viewRobotAction->setToolTip( "Robot" );
  viewRobotAction->setIconText( "Robot" );
  viewRobotAction->setShortcut( Qt::ALT | Qt::Key_R );
  viewMenu->addAction( viewRobotAction );
  viewToolBar->addAction( viewRobotAction );
  QObject::connect( viewRobotAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowRobot() ) );
  viewRobotAction->setStatusTip( "View robot." );

  QIcon icon_viewLanguageParametersAction( QPixmap( sigelRoot + "/pixmaps/balloonSmall.xpm" ) );
  icon_viewLanguageParametersAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/balloonLarge.xpm" ) );
  QAction *viewLanguageParametersAction = new QAction( icon_viewLanguageParametersAction, "&Language Parameters", this );
  viewLanguageParametersAction->setToolTip( "Language" );
  viewLanguageParametersAction->setIconText( "Language" );
  viewLanguageParametersAction->setShortcut( Qt::ALT | Qt::Key_L );
  viewMenu->addAction( viewLanguageParametersAction );
  viewToolBar->addAction( viewLanguageParametersAction );
  QObject::connect( viewLanguageParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowLanguageParameters() ) );
  viewLanguageParametersAction->setStatusTip( "View language parameters." );

  QIcon icon_viewGPParametersAction( QPixmap( sigelRoot + "/pixmaps/dnaSmall.xpm" ) );
  icon_viewGPParametersAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/dnaLarge.xpm" ) );
  QAction *viewGPParametersAction = new QAction( icon_viewGPParametersAction, "&GP Parameters", this );
  viewGPParametersAction->setToolTip( "Genetic" );
  viewGPParametersAction->setIconText( "Genetic" );
  viewGPParametersAction->setShortcut( Qt::ALT | Qt::Key_G );
  viewMenu->addAction( viewGPParametersAction );
  viewToolBar->addAction( viewGPParametersAction );
  QObject::connect( viewGPParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowGPParameters() ) );
  viewGPParametersAction->setStatusTip( "View genetic programming parameters." );

  QIcon icon_viewSimulationParametersAction( QPixmap( sigelRoot + "/pixmaps/simulationParameterSmall.xpm" ) );
  icon_viewSimulationParametersAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/simulationParameterLarge.xpm" ) );
  QAction *viewSimulationParametersAction = new QAction( icon_viewSimulationParametersAction, "&Simulation Parameters", this );
  viewSimulationParametersAction->setToolTip( "Simulation" );
  viewSimulationParametersAction->setIconText( "Simulation" );
  viewSimulationParametersAction->setShortcut( Qt::ALT | Qt::Key_S );
  viewMenu->addAction( viewSimulationParametersAction );
  viewToolBar->addAction( viewSimulationParametersAction );
  QObject::connect( viewSimulationParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowSimulationParameters() ) );
  viewSimulationParametersAction->setStatusTip( "View simulation parameters." );

  QIcon icon_viewEnvironmentAction( QPixmap( sigelRoot + "/pixmaps/environSmall.xpm" ) );
  icon_viewEnvironmentAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/environLarge.xpm" ) );
  QAction *viewEnvironmentAction = new QAction( icon_viewEnvironmentAction, "&Environment", this );
  viewEnvironmentAction->setToolTip( "Environment" );
  viewEnvironmentAction->setIconText( "Environment" );
  viewEnvironmentAction->setShortcut( Qt::ALT | Qt::Key_E );
  viewMenu->addAction( viewEnvironmentAction );
  viewToolBar->addAction( viewEnvironmentAction );
  QObject::connect( viewEnvironmentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotShowEnvironment() ) );
  viewEnvironmentAction->setStatusTip( "View environment." );

  // viewMenu->addSeparator();
  optionsMenu = new QMenu( this );
  optionsMenu->setObjectName( "optionsMenu" );
  bigPixmapAction = optionsMenu->addAction( "Use big pixmaps", this, SLOT( slotUseBigPixmaps() ) );
  // Qt 2 drew a tick for setItemChecked() on any popup item; a Qt 6 QAction
  // shows one only once it is checkable.
  bigPixmapAction->setCheckable( true );
  textLabelsAction = optionsMenu->addAction( "Use textlabels", this, SLOT( slotUseTextLabels() ) );
  textLabelsAction->setCheckable( true );
  optionsMenu->addSeparator();
  optionsMenu->addAction( "Change font", this, SLOT( slotChangeFont() ) );

  noExperimentActions.append( renameExperimentAction );
  noExperimentActions.append( deleteExperimentAction );
  noExperimentActions.append( saveExperimentAction );
  noExperimentActions.append( viewGPParametersAction );
  noExperimentActions.append( viewSimulationParametersAction );
  noExperimentActions.append( viewLanguageParametersAction );
  noExperimentActions.append( viewRobotAction );
  noExperimentActions.append( viewEnvironmentAction );
  noExperimentActions.append( viewPopulationAction );

  noExperimentActions.append( importGPParametersAction );
  noExperimentActions.append( importSimulationParametersAction );
  noExperimentActions.append( importRobotAction );
  noExperimentActions.append( importLanguageParametersAction );
  noExperimentActions.append( importPopulationAction );
  noExperimentActions.append( importEnvironmentAction );
  noExperimentActions.append( importProgramAction );
  noExperimentActions.append( importIndividualAction );
  noExperimentActions.append( exportGPParametersAction );
  noExperimentActions.append( exportSimulationParametersAction );
  noExperimentActions.append( exportLanguageParametersAction );
  noExperimentActions.append( exportEnvironmentAction );
  noExperimentActions.append( exportPopulationAction );
  noExperimentActions.append( exportToGNUPlotAction );
  noExperimentActions.append( exportProgramAction );
  noExperimentActions.append( exportIndividualAction );

  // create the help menu
  helpMenu = new QMenu( this );
  helpMenu->setObjectName( "helpMenu" );
  helpMenu->addAction( "About", this, SLOT( slotAbout() ) );

  // create the action menu
  individualsMenu = new QMenu( this );
  individualsMenu->setObjectName( "actionsMenu" );

  QIcon icon_addIndividualsAction( QPixmap( sigelRoot + "/pixmaps/addIndividualsSmall.xpm" ) );
  icon_addIndividualsAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/addIndividualsLarge.xpm" ) );
  QAction *addIndividualsAction = new QAction( icon_addIndividualsAction, "&Add", this );
  addIndividualsAction->setToolTip( "Add" );
  addIndividualsAction->setIconText( "Add" );
  addIndividualsAction->setShortcut( Qt::ALT | Qt::Key_A );
  individualsMenu->addAction( addIndividualsAction );
  individualsToolBar->addAction( addIndividualsAction );
  QObject::connect( addIndividualsAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotAddIndividuals() ) );
  addIndividualsAction->setStatusTip( "Add individuals to the current experiment." );

  QIcon icon_deleteIndividualsAction( QPixmap( sigelRoot + "/pixmaps/deleteIndividualsSmall.xpm" ) );
  icon_deleteIndividualsAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/deleteIndividualsLarge.xpm" ) );
  QAction *deleteIndividualsAction = new QAction( icon_deleteIndividualsAction, "&Delete", this );
  deleteIndividualsAction->setToolTip( "Delete" );
  deleteIndividualsAction->setIconText( "Delete" );
  deleteIndividualsAction->setShortcut( Qt::ALT | Qt::Key_D );
  individualsMenu->addAction( deleteIndividualsAction );
  individualsToolBar->addAction( deleteIndividualsAction );
  QObject::connect( deleteIndividualsAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotDeleteIndividuals() ) );
  deleteIndividualsAction->setStatusTip( "Delete the selected individuals from the currently selected experiment." );

  QAction *resetIndividualsAction = new QAction( "Reset", this );
  resetIndividualsAction->setToolTip( "Reset" );
  resetIndividualsAction->setIconText( "Reset" );
  resetIndividualsAction->setShortcut( Qt::ALT | Qt::Key_O );
  individualsMenu->addAction( resetIndividualsAction );
  QObject::connect( resetIndividualsAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotResetIndividuals() ) );
  resetIndividualsAction->setStatusTip( "Reset the selected individuals." );

  individualsMenu->addSeparator();
  individualsToolBar->addSeparator();

  QIcon icon_visualizeIndividualsAction( QPixmap( sigelRoot + "/pixmaps/visualizeSmall.xpm" ) );
  icon_visualizeIndividualsAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/visualizeLarge.xpm" ) );
  QAction *visualizeIndividualsAction = new QAction( icon_visualizeIndividualsAction, "&Visualize", this );
  visualizeIndividualsAction->setToolTip( "Visualize" );
  visualizeIndividualsAction->setIconText( "Visualize" );
  visualizeIndividualsAction->setShortcut( Qt::ALT | Qt::Key_V );
  individualsMenu->addAction( visualizeIndividualsAction );
  individualsToolBar->addAction( visualizeIndividualsAction );
  QObject::connect( visualizeIndividualsAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotVisualizeIndividuals() ) );
  visualizeIndividualsAction->setStatusTip( "Visualize the selected individuals." );

  noExperimentActions.append( addIndividualsAction );
  noExperimentActions.append( deleteIndividualsAction );
  noExperimentActions.append( resetIndividualsAction );
  noExperimentActions.append( visualizeIndividualsAction );

  slotEnableNoExperimentActions( false );

  // D30. Neither of these was in ANY lock list, so both stayed live through a
  // run -- and both end in a slot that emits isNotEmpty(true), which is what
  // re-enabled everything else. Locking them shuts the route as well as the
  // symptom.
  evolutionRunningActions.append( newExperimentAction );
  evolutionRunningActions.append( openExperimentAction );
  evolutionRunningActions.append( renameExperimentAction );
  evolutionRunningActions.append( deleteExperimentAction );
  evolutionRunningActions.append( saveExperimentAction );
  evolutionRunningActions.append( importGPParametersAction );
  evolutionRunningActions.append( importSimulationParametersAction );
  evolutionRunningActions.append( importRobotAction );
  evolutionRunningActions.append( importLanguageParametersAction );
  evolutionRunningActions.append( importPopulationAction );
  evolutionRunningActions.append( importEnvironmentAction );
  evolutionRunningActions.append( importProgramAction );
  evolutionRunningActions.append( importIndividualAction );
  evolutionRunningActions.append( exportGPParametersAction );
  evolutionRunningActions.append( exportSimulationParametersAction );
  evolutionRunningActions.append( exportLanguageParametersAction );
  evolutionRunningActions.append( exportEnvironmentAction );
  evolutionRunningActions.append( exportPopulationAction );
  evolutionRunningActions.append( exportToGNUPlotAction );
  evolutionRunningActions.append( exportProgramAction );
  evolutionRunningActions.append( exportIndividualAction );
  evolutionRunningActions.append( addIndividualsAction );
  evolutionRunningActions.append( deleteIndividualsAction );
  evolutionRunningActions.append( resetIndividualsAction );
  evolutionRunningActions.append( visualizeIndividualsAction );

  /**
   * Meta-GP menu and toolbar definition
   * begin
   **/

  // create the menu and toolbar
  mtMenu	= new QMenu( this );
  mtMenu->setObjectName( "helpMenu" );
  mtToolBar = new QToolBar( this );
  mtToolBar->setObjectName( "mtToolbar" );
  addToolBar( Qt::TopToolBarArea, mtToolBar );

  // use MetaGP button
  QIcon icon_mtUseAction( QPixmap( sigelRoot + "/pixmaps/mt_UseSmall.xpm" ) );
  icon_mtUseAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/mt_UseSmall.xpm" ) );
  mtUseAction = new QAction( icon_mtUseAction, "&Use MetaGP", this );
  mtUseAction->setToolTip( "Meta" );
  mtUseAction->setIconText( "Meta" );
  mtUseAction->setCheckable( true );
  mtUseAction->setStatusTip( "Activates the use of the Meta-System to speed up the evolution." );
  mtMenu->addAction( mtUseAction );
  mtToolBar->addAction( mtUseAction );
  mtUseAction->setEnabled(false);
  QObject::connect(mtUseAction, SIGNAL( toggled(bool) ), SLOT( slotMTUseMT(bool) ));

  // configure MetaGP-System button
  QIcon icon_mtConfigureAction( QPixmap( sigelRoot + "/pixmaps/mt_ConfSmall.xpm" ) );
  icon_mtConfigureAction.addPixmap( QPixmap( sigelRoot + "/pixmaps/mt_ConfSmall.xpm" ) );
  mtConfigureAction = new QAction( icon_mtConfigureAction, "&Configure System", this );
  mtConfigureAction->setToolTip( "Configure Meta" );
  mtConfigureAction->setIconText( "Configure Meta" );
  mtConfigureAction->setStatusTip( "Pops up the Meta Systems configuration dialog." );
  mtMenu->addAction( mtConfigureAction );
  mtToolBar->addAction( mtConfigureAction );
  mtConfigureAction->setEnabled(false);
  QObject::connect(mtConfigureAction, SIGNAL( triggered() ), SLOT( slotMTConfigureSystem() ));

  // MetaGP system selection (evaluator or classifier)
  mtMenu->addSeparator();
  mtChoiceTypeActionGroup = new QActionGroup( this );
  mtChoiceTypeActionGroup->setObjectName( "mtChoiceTypeActionGroup" );
  mtChoiceTypeActionGroup->setExclusive( true );
  mtChoiceEvaluatorAction = new QAction( "&Evaluator System", mtChoiceTypeActionGroup );
  mtChoiceEvaluatorAction->setToolTip( "choose evaluator system" );
  mtChoiceEvaluatorAction->setIconText( "choose evaluator system" );
  mtChoiceEvaluatorAction->setCheckable( true );
  mtChoiceEvaluatorAction->setChecked(true);
  mtSelectedSystem = mtChoiceEvaluatorAction;   // Qt 2: d->selected
  mtChoiceEvaluatorAction->setStatusTip( "Chooses the Evaluator System. Replaces the simulation based fitness calculation by a MetaGP System which needs less simulation." );
  mtChoiceClassifierAction = new QAction( "Cl&assifier System", mtChoiceTypeActionGroup );
  mtChoiceClassifierAction->setToolTip( "choose classifier system" );
  mtChoiceClassifierAction->setIconText( "choose classifier system" );
  mtChoiceClassifierAction->setCheckable( true );
  mtChoiceClassifierAction->setStatusTip( "Chooses the Classifer System. Replaces the fitness based tournament decision by a MetaGP System which needs less fitness calculation." );
  mtMenu->addActions( mtChoiceTypeActionGroup->actions() );
  mtChoiceTypeActionGroup->setEnabled(false);
  QObject::connect(mtChoiceTypeActionGroup, SIGNAL( triggered( QAction * ) ), SLOT( slotMTSwitchSystem(QAction*) ));

  // DELIBERATE DIVERGENCE FROM 1.3, decided 2026-09-02. 1.3 put a second
  // "A&bout" here, on the MetaGP menu, wired to the SAME slotAbout() as
  // Help > About and therefore opening the identical SIG_InfoBox --
  // "mtMenu->addSeparator(); mtMenu->addAction( \"A&bout\", this,
  // SLOT( slotAbout() ) );". Preserved through C1-C10 and verified
  // against the running 1.3 binary before being removed, so this is a
  // CHOICE and not a porting error. Its trailing separator goes with it,
  // or the menu would end on one. Help > About is untouched.

  noExperimentActions.append( mtUseAction );

  // D29. None of the four MetaGP actions was in evolutionRunningActions, so
  // all four stayed live throughout a run -- mtUseAction was in
  // noExperimentActions only, i.e. disabled with no experiment and enabled
  // the moment one is loaded, run or no run. They change MetaGP state, which
  // is a run parameter, so D29 covers them.
  //
  // Two of the four are measured hazards on the 2003 binary, not theory:
  //   Configure System, opening its MTMainWindow during a run, CRASHES 1.3.
  //     Four observations; the cleanest had MetaGP set BEFORE Start, four
  //     untouched generations, then one injected click -- dead in 10 s with
  //     `QGVector::operator[]: Index 359 out of range'. Against it, ~25 other
  //     injected mid-run events across two runs did nothing, so the trigger
  //     is this path and not mid-run interaction in general.
  //   Use MetaGP toggled mid-run WEDGES the run: an error dialog, then the
  //     evolution never advances again while Stop stays enabled and the GUI
  //     keeps repainting -- 4.5 minutes with zero slave spawns against a 62 s
  //     per generation baseline. Worse than the crash, because it looks like
  //     a healthy run.
  // The other two are locked because they are the same class, not because
  // anyone has crashed them -- fixing only the observed instance is how three
  // earlier defects in this area each survived their first fix.
  evolutionRunningActions.append( mtUseAction );
  evolutionRunningActions.append( mtConfigureAction );
  evolutionRunningActions.append( mtChoiceEvaluatorAction );
  evolutionRunningActions.append( mtChoiceClassifierAction );
  QObject::connect(experimentListView,
	  SIGNAL( actExpChanged() ),
	  this,
	  SLOT( slotActExpChanged() ));
//  noExperimentActions.append( mtChoiceTypeActionGroup );
//  noExperimentActions.append( mtConfigureAction );

  /**
   * Meta-GP menu and toolbar definition
   * end
   **/

  fileMenu->setTitle( "&File" );
  menuBar()->addMenu( fileMenu );
  viewMenu->setTitle( "&View" );
  menuBar()->addMenu( viewMenu );
  individualsMenu->setTitle( "&Individuals" );
  menuBar()->addMenu( individualsMenu );
  optionsMenu->setTitle( "&Options" );
  menuBar()->addMenu( optionsMenu );

  mtMenu->setTitle( "&MetaGP" );
  menuBar()->addMenu( mtMenu );	// insert the MetaGP menu

  helpMenu->setTitle( "&Help" );
  menuBar()->addMenu( helpMenu );

  QObject::connect( experimentListView,
		    SIGNAL( isNotEmpty( bool ) ),
		    this,
		    SLOT( slotEnableNoExperimentActions( bool ) ) );

  widgetBase = new QLabel( this );
  widgetBase->setObjectName( "baseWidget" ); // new QLabel( "I am the void", this );
  widgetBase->setPixmap( QPixmap( sigelRoot + "/pixmaps/noExperiment.png" ) );
  widgetBase->setScaledContents( true );
  widgetStack->addWidget( widgetBase );
  widgetStack->setCurrentWidget( widgetBase );

  QObject::connect( experimentListView,
		    SIGNAL( isNotEmpty( bool ) ),
		    this,
		    SLOT(slotShowEmpty( bool ) ) );

  QObject::connect( experimentListView,
		    SIGNAL( evolutionNotRunning( bool ) ),
		    this,
		    SLOT( slotEnableEvolutionRunningActions( bool ) ) );
  statusBar()->showMessage( "Ready.", 5000 );
};

SIG_MainWindow::~SIG_MainWindow(){};

void SIG_MainWindow::slotAbout()
{
  //QMessageBox::about( this, "Sigel", "Sigel Version 1.0\nDeveloped by:\n\tChristian 'Krasstexta' Aue\n\tAbdeladim 'Silent Ad' Benkacem\n\tMichael 'CJ QT' Gregorius\n\tAndree 'MC Overload' Ross\n\tAbdallah 'The Raiyan' Salah Raiyan\n\tDaniel 'Tabmaster Ispell' Sawitzki\n\tVolker 'SEXmaschine' Strunk\n\tHolger 'DJ NOOP' Tuerk\n\tChris 'MC Royal' Varcol" );

  SIG_InfoBox theInfoBox( 0, "InfoBox", true );
  theInfoBox.resize( 600, -1 );
  theInfoBox.exec();
};

void SIG_MainWindow::slotShowEmpty( bool isNotEmpty )
{
  if( !isNotEmpty )
    {
      widgetStack->setCurrentWidget( widgetBase );
    }
};

void SIG_MainWindow::slotAboutToQuit()
{
  switch( QMessageBox::warning( this, "Do you really...", "Do you really want to quit?\n"
				"There may be unsaved experiments!", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
    {
    case QMessageBox::Yes:
      qApp->quit();
      break;
    }
};

void SIG_MainWindow::slotUseBigPixmaps()
{
  // 25 is the largest Small pixmap, 48 the largest Large one -- both measured
  // from the .xpm files, not assumed uniform.
  bool usesBigPixmaps = iconSize().width() < 48;
  bigPixmapAction->setChecked( usesBigPixmaps );
  setIconSize( usesBigPixmaps ? QSize( 48, 48 ) : QSize( 25, 25 ) );
};

void SIG_MainWindow::slotUseTextLabels()
{
  bool usesTextLabels = toolButtonStyle() != Qt::ToolButtonTextUnderIcon;
  textLabelsAction->setChecked( usesTextLabels );
  setToolButtonStyle( usesTextLabels ? Qt::ToolButtonTextUnderIcon
                                     : Qt::ToolButtonIconOnly );
};

void SIG_MainWindow::slotChangeFont()
{
  bool ok;
  QFont newFont = QFontDialog::getFont( &ok, QApplication::font() );
  if( ok )
    qApp->setFont( newFont );
};

/********************************
 * MetaGP slots
 ********************************/
// actual experiment should use the metaGP system
// set the corresponding flag in actual experiment
// make the experiment create the metaGP-System
void SIG_MainWindow::slotMTUseMT(bool state)
{
	// D30 SECOND LAYER. The menu item is greyed during a run; this refuses
	// anyway, because a greyed menu is one layer and a slot that checks for
	// itself is another. Toggling MetaGP mid-run is what wedges 1.3.
	if (SIG_Experiment::anyEvolutionRunning())
		return;
	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		if(actExperiment->gpExperiment.mtController->useMeta(state)){
			mtConfigureAction->setEnabled(state);
			mtChoiceTypeActionGroup->setEnabled(state);
		}
	}
};

// show the configuration window for the metaGP-System
// used by the actual experiment
void SIG_MainWindow::slotMTConfigureSystem()
{
	// D30 SECOND LAYER, and this is the one that crashes. configureSystem()
	// deletes the trainer the running evolution is holding
	// (MT_Controller.cpp:402-404), so reaching it mid-run by ANY route aborts
	// the process. Refuse here as well as greying the menu.
	if (SIG_Experiment::anyEvolutionRunning())
		return;
	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		actExperiment->gpExperiment.mtController->configureSystem();
	};
};

// switch the actual experiment to the other metaGP system
void SIG_MainWindow::slotMTSwitchSystem(QAction *selSystem)
{
	// D30 SECOND LAYER. Switching Evaluator/Classifier mid-run changes which
	// substitute the run is using. Refuse, as well as greying the group.
	if (SIG_Experiment::anyEvolutionRunning())
		return;
	// Qt 2 reached this slot only when the selection actually CHANGED:
	// QActionGroup::childToggled gated "emit selected(s)" on "s != d->selected",
	// and QAction::setOn returned early when the state was already correct.
	// Qt 6's QActionGroup::triggered(QAction*) fires on every click, so
	// re-clicking the already-active system would re-enter switchSystem()
	// where 1.3 did nothing at all.
	if (selSystem == mtSelectedSystem)
		return;

	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		if(mtChoiceClassifierAction->isChecked()){
			if(!actExperiment->gpExperiment.mtController->switchSystem(CLASSIFIER_SUBST))
				mtChoiceEvaluatorAction->setChecked(true);
		} else {
			if(!actExperiment->gpExperiment.mtController->switchSystem(EVALUATOR_SUBST))
				mtChoiceClassifierAction->setChecked(true);
		}
	}
	// switchSystem() may have refused and rolled the choice back above, so
	// track what is actually checked -- Qt 2's d->selected was likewise
	// re-updated by the second childToggled the rollback's setOn(TRUE) made.
	mtSelectedSystem = mtChoiceClassifierAction->isChecked()
	                   ? mtChoiceClassifierAction : mtChoiceEvaluatorAction;
};

void SIG_MainWindow::slotActExpChanged()
{
	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		if(actExperiment->gpExperiment.mtController->IsEnabled()){

			// currently selected experiment use meta gp-system
			//
			// D30. NOT while a run is going. This slot has no run check of
			// its own and it fires one line after the tree-click emit that
			// APPLIES the run lock (SIG_ExperimentListView::slotSelectionChanged), so
			// it handed these two straight back mid-run. Measured: with
			// MetaGP on, one click in the experiment tree during a run made
			// Configure System live again, and clicking it aborted the
			// process -- MT_Controller::configureSystem deletes the trainer
			// the running evolution is holding. The greying is one layer;
			// future_refactorings.md carries the second, inside MT_Controller.
			if (!SIG_Experiment::anyEvolutionRunning()) {
				mtChoiceTypeActionGroup->setEnabled(true);
				mtConfigureAction->setEnabled(true);
			}
			mtUseAction->setChecked(true);
			if(actExperiment->gpExperiment.mtController->UsedSystem() == EVALUATOR_SUBST){
				mtChoiceEvaluatorAction->setChecked(true);
			} else {
				mtChoiceClassifierAction->setChecked(true);
			}

		} else {

			// no meta gp-system is used
			mtChoiceTypeActionGroup->setEnabled(false);
			mtConfigureAction->setEnabled(false);
			mtUseAction->setChecked(false);

		}
	}
};


void SIG_MainWindow::slotEnableNoExperimentActions( bool enable )
{
  // Qt 2's QActionGroup::setEnabled walked its own member list
  // (qaction.cpp:902-908). This is that loop.
  for ( QAction *a : noExperimentActions )
    a->setEnabled( enable );

  // D30. 24 of these 30 actions are ALSO in evolutionRunningActions, so this
  // loop used to hand back what a run had locked -- Use MetaGP among them.
  // (30, not 32: two appends are commented out at :699-700, which is also why
  // Configure System and the Evaluator/Classifier group are NOT in this list
  // and why an earlier version of this comment naming them was wrong. An
  // earlier version also disabled the group here, which nothing ever undid --
  // it stayed grey after the run ended. Both found by review.)
  // File > New Experiment and File > Open Experiment
  // reach it during a run (SIG_ExperimentListView.cpp, slotNewExperiment and :207 emit
  // isNotEmpty(true)), so one menu click undid the whole lock. Re-apply the
  // lock rather than filtering the list, so the two lists cannot drift apart.
  if ( enable && SIG_Experiment::anyEvolutionRunning() )
    slotEnableEvolutionRunningActions( false );
};

void SIG_MainWindow::slotEnableEvolutionRunningActions( bool enable )
{
  for ( QAction *a : evolutionRunningActions )
    a->setEnabled( enable );
};


void SIG_MainWindow::closeEvent( QCloseEvent *event )
{
  QMainWindow::closeEvent( event );
  if ( event->isAccepted() )
    qApp->quit();
};

}
