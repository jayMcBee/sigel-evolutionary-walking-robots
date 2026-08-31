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
  // Qt 2 defaulted to the Small pixmap of each QIconSet (22x22 here);
  // Qt 6 would otherwise use a style-dependent size and rescale them.
  setIconSize( QSize( 22, 22 ) );
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
  importGPParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_G );
  importMenu->addAction( importGPParametersAction );
  QObject::connect( importGPParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotGPParametersImport() ) );
  QAction *importSimulationParametersAction = new QAction( "Simulation-Parameters", this );
  importSimulationParametersAction->setToolTip( "Import Simulation-Parameters" );
  importSimulationParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_S );
  importMenu->addAction( importSimulationParametersAction );
  QObject::connect( importSimulationParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotSimulationParametersImport() ) );
  QAction *importLanguageParametersAction = new QAction( "Language-Parameters", this );
  importLanguageParametersAction->setToolTip( "Import Language-Parameters" );
  importLanguageParametersAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_L );
  importMenu->addAction( importLanguageParametersAction );
  QObject::connect( importLanguageParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotLanguageParametersImport() ) );
  QAction *importEnvironmentAction = new QAction( "Environment", this );
  importEnvironmentAction->setToolTip( "Import Environment" );
  importEnvironmentAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_E );
  importMenu->addAction( importEnvironmentAction );
  QObject::connect( importEnvironmentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotEnvironmentImport() ) );
  QAction *importPopulationAction = new QAction( "Population", this );
  importPopulationAction->setToolTip( "Import Population" );
  importPopulationAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_P );
  importMenu->addAction( importPopulationAction );
  QObject::connect( importPopulationAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotPopulationImport() ) );
  QAction *importRobotAction = new QAction( "Robot", this );
  importRobotAction->setToolTip( "Import Robot" );
  importRobotAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_R );
  importMenu->addAction( importRobotAction );
  QObject::connect( importRobotAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotRobotImport() ) );

  importMenu->addSeparator();

  QAction *importProgramAction = new QAction( "Program", this );
  importProgramAction->setToolTip( "Import Program" );
  importProgramAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_K );
  importMenu->addAction( importProgramAction );
  QObject::connect( importProgramAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotProgramImport() ) );

  QAction *importIndividualAction = new QAction( "Individual", this );
  importIndividualAction->setToolTip( "Import Individual" );
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
  exportGPParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_G );
  exportMenu->addAction( exportGPParametersAction );
  QObject::connect( exportGPParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotGPParametersExport() ) );
  QAction *exportSimulationParametersAction = new QAction( "Simulation-Parameters", this );
  exportSimulationParametersAction->setToolTip( "Export Simulation-Parameters" );
  exportSimulationParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_S );
  exportMenu->addAction( exportSimulationParametersAction );
  QObject::connect( exportSimulationParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotSimulationParametersExport() ) );
  QAction *exportLanguageParametersAction = new QAction( "Language-Parameters", this );
  exportLanguageParametersAction->setToolTip( "Export Language-Parameters" );
  exportLanguageParametersAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_L );
  exportMenu->addAction( exportLanguageParametersAction );
  QObject::connect( exportLanguageParametersAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotLanguageParametersExport() ) );
  QAction *exportEnvironmentAction = new QAction( "Environment", this );
  exportEnvironmentAction->setToolTip( "Export Environment" );
  exportEnvironmentAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_E );
  exportMenu->addAction( exportEnvironmentAction );
  QObject::connect( exportEnvironmentAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotEnvironmentExport() ) );
  QAction *exportPopulationAction = new QAction( "Population", this );
  exportPopulationAction->setToolTip( "Export Population" );
  exportPopulationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_P );
  exportMenu->addAction( exportPopulationAction );
  QObject::connect( exportPopulationAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotPopulationExport() ) );

  exportMenu->addSeparator();

  QAction *exportProgramAction = new QAction( "Program", this );
  exportProgramAction->setToolTip( "Export Program" );
  exportProgramAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_K );
  exportMenu->addAction( exportProgramAction );
  QObject::connect( exportProgramAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotProgramExport() ) );

  QAction *exportIndividualAction = new QAction( "Individual", this );
  exportIndividualAction->setToolTip( "Export Individual" );
  exportIndividualAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_L );
  exportMenu->addAction( exportIndividualAction );
  QObject::connect( exportIndividualAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotIndividualExport() ) );

  exportMenu->addSeparator();

  QAction *exportToGNUPlotAction = new QAction( "...to GNU plot", this );
  exportToGNUPlotAction->setToolTip( "Export to GNU plot" );
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

  noExperimentActionGroup = new QActionGroup( this );
  noExperimentActionGroup->setObjectName( "noExperimentActionGroup" );
  noExperimentActionGroup->setExclusive( false );
  noExperimentActionGroup->addAction( renameExperimentAction );
  noExperimentActionGroup->addAction( deleteExperimentAction );
  noExperimentActionGroup->addAction( saveExperimentAction );
  noExperimentActionGroup->addAction( viewGPParametersAction );
  noExperimentActionGroup->addAction( viewSimulationParametersAction );
  noExperimentActionGroup->addAction( viewLanguageParametersAction );
  noExperimentActionGroup->addAction( viewRobotAction );
  noExperimentActionGroup->addAction( viewEnvironmentAction );
  noExperimentActionGroup->addAction( viewPopulationAction );

  noExperimentActionGroup->addAction( importGPParametersAction );
  noExperimentActionGroup->addAction( importSimulationParametersAction );
  noExperimentActionGroup->addAction( importRobotAction );
  noExperimentActionGroup->addAction( importLanguageParametersAction );
  noExperimentActionGroup->addAction( importPopulationAction );
  noExperimentActionGroup->addAction( importEnvironmentAction );
  noExperimentActionGroup->addAction( importProgramAction );
  noExperimentActionGroup->addAction( importIndividualAction );
  noExperimentActionGroup->addAction( exportGPParametersAction );
  noExperimentActionGroup->addAction( exportSimulationParametersAction );
  noExperimentActionGroup->addAction( exportLanguageParametersAction );
  noExperimentActionGroup->addAction( exportEnvironmentAction );
  noExperimentActionGroup->addAction( exportPopulationAction );
  noExperimentActionGroup->addAction( exportToGNUPlotAction );
  noExperimentActionGroup->addAction( exportProgramAction );
  noExperimentActionGroup->addAction( exportIndividualAction );

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
  visualizeIndividualsAction->setShortcut( Qt::ALT | Qt::Key_V );
  individualsMenu->addAction( visualizeIndividualsAction );
  individualsToolBar->addAction( visualizeIndividualsAction );
  QObject::connect( visualizeIndividualsAction,
		    SIGNAL( triggered() ),
		    experimentListView,
		    SLOT( slotVisualizeIndividuals() ) );
  visualizeIndividualsAction->setStatusTip( "Visualize the selected individuals." );

  noExperimentActionGroup->addAction( addIndividualsAction );
  noExperimentActionGroup->addAction( deleteIndividualsAction );
  noExperimentActionGroup->addAction( resetIndividualsAction );
  noExperimentActionGroup->addAction( visualizeIndividualsAction );

  noExperimentActionGroup->setEnabled( false );

  evolutionRunningActionGroup = new QActionGroup( this );
  evolutionRunningActionGroup->setObjectName( "evolutionRunningActionGroup" );
  evolutionRunningActionGroup->setExclusive( false );
  evolutionRunningActionGroup->addAction( renameExperimentAction );
  evolutionRunningActionGroup->addAction( deleteExperimentAction );
  evolutionRunningActionGroup->addAction( saveExperimentAction );
  evolutionRunningActionGroup->addAction( importGPParametersAction );
  evolutionRunningActionGroup->addAction( importSimulationParametersAction );
  evolutionRunningActionGroup->addAction( importRobotAction );
  evolutionRunningActionGroup->addAction( importLanguageParametersAction );
  evolutionRunningActionGroup->addAction( importPopulationAction );
  evolutionRunningActionGroup->addAction( importEnvironmentAction );
  evolutionRunningActionGroup->addAction( importProgramAction );
  evolutionRunningActionGroup->addAction( importIndividualAction );
  evolutionRunningActionGroup->addAction( exportGPParametersAction );
  evolutionRunningActionGroup->addAction( exportSimulationParametersAction );
  evolutionRunningActionGroup->addAction( exportLanguageParametersAction );
  evolutionRunningActionGroup->addAction( exportEnvironmentAction );
  evolutionRunningActionGroup->addAction( exportPopulationAction );
  evolutionRunningActionGroup->addAction( exportToGNUPlotAction );
  evolutionRunningActionGroup->addAction( exportProgramAction );
  evolutionRunningActionGroup->addAction( exportIndividualAction );
  evolutionRunningActionGroup->addAction( addIndividualsAction );
  evolutionRunningActionGroup->addAction( deleteIndividualsAction );
  evolutionRunningActionGroup->addAction( resetIndividualsAction );
  evolutionRunningActionGroup->addAction( visualizeIndividualsAction );

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
  mtChoiceEvaluatorAction->setCheckable( true );
  mtChoiceEvaluatorAction->setChecked(true);
  mtChoiceEvaluatorAction->setStatusTip( "Chooses the Evaluator System. Replaces the simulation based fitness calculation by a MetaGP System which needs less simulation." );
  mtChoiceClassifierAction = new QAction( "Cl&assifier System", mtChoiceTypeActionGroup );
  mtChoiceClassifierAction->setToolTip( "choose classifier system" );
  mtChoiceClassifierAction->setCheckable( true );
  mtChoiceClassifierAction->setStatusTip( "Chooses the Classifer System. Replaces the fitness based tournament decision by a MetaGP System which needs less fitness calculation." );
  mtMenu->addActions( mtChoiceTypeActionGroup->actions() );
  mtChoiceTypeActionGroup->setEnabled(false);
  QObject::connect(mtChoiceTypeActionGroup, SIGNAL( triggered( QAction * ) ), SLOT( slotMTSwitchSystem(QAction*) ));

  // about-box button
  mtMenu->addSeparator();
  mtMenu->addAction( "A&bout", this, SLOT( slotAbout() ) );

  noExperimentActionGroup->addAction( mtUseAction );
  QObject::connect(experimentListView,
	  SIGNAL( actExpChanged() ),
	  this,
	  SLOT( slotActExpChanged() ));
//  noExperimentActionGroup->addAction( mtChoiceTypeActionGroup );
//  noExperimentActionGroup->addAction( mtConfigureAction );

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
  menuBar()->addMenu( mtMenu );	// MetaGP-Menü einfügen

  helpMenu->setTitle( "&Help" );
  menuBar()->addMenu( helpMenu );

  QObject::connect( experimentListView,
		    SIGNAL( isNotEmpty( bool ) ),
		    noExperimentActionGroup,
		    SLOT( setEnabled( bool ) ) );

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
		    evolutionRunningActionGroup,
		    SLOT( setEnabled( bool ) ) );
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
  bool usesBigPixmaps = iconSize().width() < 32;
  bigPixmapAction->setChecked( usesBigPixmaps );
  setIconSize( usesBigPixmaps ? QSize( 32, 32 ) : QSize( 22, 22 ) );
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
	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		actExperiment->gpExperiment.mtController->configureSystem();
	};
};

// switch the actual experiment to the other metaGP system
void SIG_MainWindow::slotMTSwitchSystem(QAction *selSystem)
{
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
};

void SIG_MainWindow::slotActExpChanged()
{
	SIG_Experiment *actExperiment = experimentListView->currentlySelectedExperiment();
	if(actExperiment){
		if(actExperiment->gpExperiment.mtController->IsEnabled()){

			// currently selected experiment use meta gp-system
			mtChoiceTypeActionGroup->setEnabled(true);
			mtConfigureAction->setEnabled(true);
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

}
