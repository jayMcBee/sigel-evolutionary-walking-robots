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
#include <qvaluelist.h>
#include <qmessagebox.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qstatusbar.h>
#include <qfontdialog.h>

#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_MasterGUI/SIG_InfoBox.h"

#include <cstdlib>

namespace SIGEL_MasterGUI
{

SIG_MainWindow::SIG_MainWindow( QWidget * parent, const char * name, WFlags f ) : QMainWindow( parent, name, f )
{
#ifdef _WINDOWS
  QString sigelRoot( ::getenv( "SIGEL_ROOT" ) );
#else
  QString sigelRoot( std::getenv( "SIGEL_ROOT" ) );
#endif

  resize( 900, 750 );
  setCaption( "SIGEL" );

  splitter = new QSplitter( this, "Splitter" );
  splitter->setFrameStyle( QFrame::Box | QFrame::Sunken );

  setCentralWidget( splitter );

  widgetStack = new QWidgetStack( splitter, "WidgetStack" ); // this was splitter before...
  widgetStack->setMargin( 6 );
  widgetStack->setFrameStyle( QFrame::Box | QFrame::Sunken );

  experimentListView = new SIGEL_MasterGUI::SIG_ExperimentListView( splitter, "ExperimentListView", widgetStack );

  splitter->moveToFirst( experimentListView );
  splitter->setOpaqueResize();
  
  QValueList<int> valList;
  valList += 2;
  valList += 6;
  splitter->setSizes( valList );

  splitter->setMargin( 6 );

  // create the file menu and toolbar
  fileMenu = new QPopupMenu(this);
  fileToolBar = new QToolBar( this, "fileToolbar" );
  fileToolBar->setLabel( "File" );

  QToolBar *viewToolBar = new QToolBar( this, "viewToolBar" );
  viewToolBar->setLabel( "View" );

  QToolBar *individualsToolBar = new QToolBar( this, "individualsToolBar" );
  individualsToolBar->setLabel( "Individuals" );

  // create the new experiment action
  QAction *newExperimentAction = new QAction( "New", QIconSet( QPixmap( sigelRoot + "/pixmaps/newExperimentSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/newExperimentLarge.xpm" ) ), "&New Experiment", CTRL+Key_N, this, "newExperimentAction" );
  newExperimentAction->addTo( fileMenu );
  newExperimentAction->addTo( fileToolBar );
  newExperimentAction->addTo( experimentListView->experimentListViewMenu );
  QObject::connect( newExperimentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotNewExperiment() ) );
  newExperimentAction->setStatusTip( "Create a new experiment..." );

  QAction *renameExperimentAction = new QAction( "Rename", "&Rename Experiment", CTRL+Key_R, this, "renameExperimentAction" );
  renameExperimentAction->addTo( fileMenu );
  QObject::connect( renameExperimentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotRenameExperiment() ) );
  renameExperimentAction->setStatusTip( "Rename the currently selected experiment..." );

  QAction *deleteExperimentAction = new QAction( "Delete", QIconSet( QPixmap( sigelRoot + "/pixmaps/deleteExperimentSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/deleteExperimentLarge.xpm" ) ), "&Delete Experiment", CTRL+Key_D, this, "deleteExperimentAction" );
  deleteExperimentAction->addTo( fileMenu );
  deleteExperimentAction->addTo( fileToolBar );
  deleteExperimentAction->addTo( experimentListView->experimentListViewMenu );
  QObject::connect( deleteExperimentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotDeleteExperiment() ) );
  deleteExperimentAction->setStatusTip( "Delete the selected experiment..." );

  fileMenu->insertSeparator();
  fileToolBar->addSeparator();
  experimentListView->experimentListViewMenu->insertSeparator();

  QAction *openExperimentAction = new QAction( "Open", QIconSet( QPixmap( sigelRoot + "/pixmaps/openExperimentSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/openExperimentLarge.xpm" ) ), "&Open Experiment", CTRL+Key_O, this, "openExperimentAction");
  openExperimentAction->addTo( fileMenu );
  openExperimentAction->addTo( fileToolBar );
  openExperimentAction->addTo( experimentListView->experimentListViewMenu );
  QObject::connect( openExperimentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotLoadExperiment() ) );
  openExperimentAction->setStatusTip( "Open an experiment..." );

  QAction *saveExperimentAction = new QAction( "Save", QIconSet( QPixmap( sigelRoot + "/pixmaps/saveExperimentSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/saveExperimentLarge.xpm" ) ),"&Save Experiment", CTRL+Key_S, this, "saveExperimentAction");
  saveExperimentAction->addTo( fileMenu );
  saveExperimentAction->addTo( fileToolBar );
  saveExperimentAction->addTo( experimentListView->experimentListViewMenu );
  QObject::connect( saveExperimentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotSaveExperiment() ) );
  saveExperimentAction->setStatusTip( "Save the selected experiment..." );

  fileMenu->insertSeparator();

  // create the import menu
  QPopupMenu *importMenu = new QPopupMenu( this, "importMenu" );

  // create the actions and insert them into the menu
  QAction *importGPParametersAction = new QAction( "Import GP-Parameters", "GP-Parameters", CTRL+SHIFT+Key_G, this, "importGPParametersAction");
  importGPParametersAction->addTo( importMenu );
  QObject::connect( importGPParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotGPParametersImport() ) );
  QAction *importSimulationParametersAction = new QAction( "Import Simulation-Parameters", "Simulation-Parameters", CTRL+SHIFT+Key_S, this, "importSimulationParametersAction");
  importSimulationParametersAction->addTo( importMenu );
  QObject::connect( importSimulationParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotSimulationParametersImport() ) );
  QAction *importLanguageParametersAction = new QAction( "Import Language-Parameters", "Language-Parameters", CTRL+SHIFT+Key_L, this, "importLanguageParametersAction");
  importLanguageParametersAction->addTo( importMenu );
  QObject::connect( importLanguageParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotLanguageParametersImport() ) );
  QAction *importEnvironmentAction = new QAction( "Import Environment", "Environment", CTRL+SHIFT+Key_E, this, "importEnvironmentAction");
  importEnvironmentAction->addTo( importMenu );
  QObject::connect( importEnvironmentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotEnvironmentImport() ) );
  QAction *importPopulationAction = new QAction( "Import Population", "Population", CTRL+SHIFT+Key_P, this, "importPopulationAction");
  importPopulationAction->addTo( importMenu );
  QObject::connect( importPopulationAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotPopulationImport() ) );
  QAction *importRobotAction = new QAction( "Import Robot", "Robot", CTRL+SHIFT+Key_R, this, "importRobotAction");
  importRobotAction->addTo( importMenu );
  QObject::connect( importRobotAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotRobotImport() ) );

  importMenu->insertSeparator();

  QAction *importProgramAction = new QAction( "Import Program", "Program", CTRL+SHIFT+Key_K, this, "importProgramAction");
  importProgramAction->addTo( importMenu );
  QObject::connect( importProgramAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotProgramImport() ) );

  QAction *importIndividualAction = new QAction( "Import Individual", "Individual", CTRL+SHIFT+Key_L, this, "importIndividualAction");
  importIndividualAction->addTo( importMenu );
  QObject::connect( importIndividualAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotIndividualImport() ) );
  
  fileMenu->insertItem( "Import", importMenu );

  // create the export menu
  QPopupMenu *exportMenu = new QPopupMenu( this, "exportMenu" );

  QAction *exportGPParametersAction = new QAction( "Export GP-Parameters", "GP-Parameters", CTRL+ALT+Key_G, this, "exportGPParametersAction");
  exportGPParametersAction->addTo( exportMenu );
  QObject::connect( exportGPParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotGPParametersExport() ) );
  QAction *exportSimulationParametersAction = new QAction( "Export Simulation-Parameters", "Simulation-Parameters", CTRL+ALT+Key_S, this, "exportSimulationParametersAction");
  exportSimulationParametersAction->addTo( exportMenu );
  QObject::connect( exportSimulationParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotSimulationParametersExport() ) );
  QAction *exportLanguageParametersAction = new QAction( "Export Language-Parameters", "Language-Parameters", CTRL+ALT+Key_L, this, "exportLanguageParametersAction");
  exportLanguageParametersAction->addTo( exportMenu );
  QObject::connect( exportLanguageParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotLanguageParametersExport() ) );
  QAction *exportEnvironmentAction = new QAction( "Export Environment", "Environment", CTRL+ALT+Key_E, this, "exportEnvironmentAction");
  exportEnvironmentAction->addTo( exportMenu );
  QObject::connect( exportEnvironmentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotEnvironmentExport() ) );
  QAction *exportPopulationAction = new QAction( "Export Population", "Population", CTRL+ALT+Key_P, this, "exportPopulationAction");
  exportPopulationAction->addTo( exportMenu );
  QObject::connect( exportPopulationAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotPopulationExport() ) );

  exportMenu->insertSeparator();

  QAction *exportProgramAction = new QAction( "Export Program", "Program", CTRL+ALT+Key_K, this, "exportProgramAction");
  exportProgramAction->addTo( exportMenu );
  QObject::connect( exportProgramAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotProgramExport() ) );

  QAction *exportIndividualAction = new QAction( "Export Individual", "Individual", CTRL+ALT+Key_L, this, "exportIndividualAction");
  exportIndividualAction->addTo( exportMenu );
  QObject::connect( exportIndividualAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotIndividualExport() ) );

  exportMenu->insertSeparator();

  QAction *exportToGNUPlotAction = new QAction( "Export to GNU plot", "...to GNU plot", ALT+Key_6, this, "exportToGNUPlot");
  exportToGNUPlotAction->addTo( exportMenu );
  QObject::connect( exportToGNUPlotAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotGNUPlotExport() ) );

  fileMenu->insertItem( "Export", exportMenu );

  fileMenu->insertSeparator();

  fileToolBar->addSeparator();

  QAction *quitProgramAction = new QAction( "Quit", QIconSet( QPixmap( sigelRoot + "/pixmaps/quitApplicationSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/quitApplicationLarge.xpm" ) ), "&Quit", CTRL+Key_Q, this, "quitProgramAction" );
  quitProgramAction->addTo( fileMenu );
  quitProgramAction->addTo( fileToolBar );
  QObject::connect( quitProgramAction,
		    SIGNAL( activated() ),
		    this,
		    SLOT( slotAboutToQuit() ) );
  quitProgramAction->setStatusTip( "Quit application..." );

  // create the view menu
  viewMenu = new QPopupMenu( this, "viewMenu" );
 
  // insert a lot of actions into the view menu
  
  QAction *viewPopulationAction = new QAction( "Population", QIconSet( QPixmap( sigelRoot + "/pixmaps/individualSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/individualLarge.xpm" ) ), "&Population", ALT+Key_P, this, "viewPopulationAction" );
  viewPopulationAction->addTo( viewMenu );
  viewPopulationAction->addTo( viewToolBar );
  QObject::connect( viewPopulationAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowIndividuals() ) );
  viewPopulationAction->setStatusTip( "View population." );

  QAction *viewRobotAction = new QAction( "Robot", QIconSet( QPixmap( sigelRoot + "/pixmaps/robotSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/robotLarge.xpm" ) ), "&Robot", ALT+Key_R, this, "viewRobotAction" );
  viewRobotAction->addTo( viewMenu );
  viewRobotAction->addTo( viewToolBar );
  QObject::connect( viewRobotAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowRobot() ) );
  viewRobotAction->setStatusTip( "View robot." );

  QAction *viewLanguageParametersAction = new QAction( "Language", QIconSet( QPixmap( sigelRoot + "/pixmaps/balloonSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/balloonLarge.xpm" ) ), "&Language Parameters", ALT+Key_L, this, "viewLanguageParametersAction" );
  viewLanguageParametersAction->addTo( viewMenu );
  viewLanguageParametersAction->addTo( viewToolBar );
  QObject::connect( viewLanguageParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowLanguageParameters() ) );
  viewLanguageParametersAction->setStatusTip( "View language parameters." );

  QAction *viewGPParametersAction = new QAction( "Genetic", QIconSet( QPixmap( sigelRoot + "/pixmaps/dnaSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/dnaLarge.xpm" ) ), "&GP Parameters", ALT+Key_G, this, "viewGPParametersAction" );
  viewGPParametersAction->addTo( viewMenu );
  viewGPParametersAction->addTo( viewToolBar );
  QObject::connect( viewGPParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowGPParameters() ) );
  viewGPParametersAction->setStatusTip( "View genetic programming parameters." );

  QAction *viewSimulationParametersAction = new QAction( "Simulation", QIconSet( QPixmap( sigelRoot + "/pixmaps/simulationParameterSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/simulationParameterLarge.xpm" ) ), "&Simulation Parameters", ALT+Key_S, this, "viewSimulaitonParametersAction" );
  viewSimulationParametersAction->addTo( viewMenu );
  viewSimulationParametersAction->addTo( viewToolBar );
  QObject::connect( viewSimulationParametersAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowSimulationParameters() ) );
  viewSimulationParametersAction->setStatusTip( "View simulation parameters." );

  QAction *viewEnvironmentAction = new QAction( "Environment", QIconSet( QPixmap( sigelRoot + "/pixmaps/environSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/environLarge.xpm" ) ),"&Environment", ALT+Key_E, this, "viewEnvironmentAction" );
  viewEnvironmentAction->addTo( viewMenu );
  viewEnvironmentAction->addTo( viewToolBar );
  QObject::connect( viewEnvironmentAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotShowEnvironment() ) );
  viewEnvironmentAction->setStatusTip( "View environment." );

  // viewMenu->insertSeparator();
  optionsMenu = new QPopupMenu( this, "optionsMenu" );
  optionsMenu->setCheckable( true );
  bigPixmapID = optionsMenu->insertItem( "Use big pixmaps", this, SLOT( slotUseBigPixmaps() ) );
  textLabelsID = optionsMenu->insertItem( "Use textlabels", this, SLOT( slotUseTextLabels() ) );
  optionsMenu->insertSeparator();
  optionsMenu->insertItem( "Change font", this, SLOT( slotChangeFont() ) );

  noExperimentActionGroup = new QActionGroup( this, "noExperimentActionGroup", false );
  noExperimentActionGroup->insert( renameExperimentAction );
  noExperimentActionGroup->insert( deleteExperimentAction );
  noExperimentActionGroup->insert( saveExperimentAction );
  noExperimentActionGroup->insert( viewGPParametersAction );
  noExperimentActionGroup->insert( viewSimulationParametersAction );
  noExperimentActionGroup->insert( viewLanguageParametersAction );
  noExperimentActionGroup->insert( viewRobotAction );
  noExperimentActionGroup->insert( viewEnvironmentAction );
  noExperimentActionGroup->insert( viewPopulationAction );

  noExperimentActionGroup->insert( importGPParametersAction );
  noExperimentActionGroup->insert( importSimulationParametersAction );
  noExperimentActionGroup->insert( importRobotAction );
  noExperimentActionGroup->insert( importLanguageParametersAction );
  noExperimentActionGroup->insert( importPopulationAction );
  noExperimentActionGroup->insert( importEnvironmentAction );
  noExperimentActionGroup->insert( importProgramAction );
  noExperimentActionGroup->insert( importIndividualAction );
  noExperimentActionGroup->insert( exportGPParametersAction );
  noExperimentActionGroup->insert( exportSimulationParametersAction );
  noExperimentActionGroup->insert( exportLanguageParametersAction );
  noExperimentActionGroup->insert( exportEnvironmentAction );
  noExperimentActionGroup->insert( exportPopulationAction );
  noExperimentActionGroup->insert( exportToGNUPlotAction );
  noExperimentActionGroup->insert( exportProgramAction );
  noExperimentActionGroup->insert( exportIndividualAction );

  // create the help menu
  helpMenu = new QPopupMenu( this, "helpMenu" );
  helpMenu->insertItem( "About", this, SLOT( slotAbout() ) );

  // create the action menu
  individualsMenu = new QPopupMenu( this, "actionsMenu" );

  QAction *addIndividualsAction = new QAction( "Add", QIconSet( QPixmap( sigelRoot + "/pixmaps/addIndividualsSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/addIndividualsLarge.xpm" ) ), "&Add", ALT+Key_A, this, "addIndividualsAction" );
  addIndividualsAction->addTo( individualsMenu );
  addIndividualsAction->addTo( individualsToolBar );
  QObject::connect( addIndividualsAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotAddIndividuals() ) );
  addIndividualsAction->setStatusTip( "Add individuals to the current experiment." );

  QAction *deleteIndividualsAction = new QAction( "Delete", QIconSet( QPixmap( sigelRoot + "/pixmaps/deleteIndividualsSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/deleteIndividualsLarge.xpm" ) ), "&Delete", ALT+Key_D, this, "deleteIndividualsAction" );
  deleteIndividualsAction->addTo( individualsMenu );
  deleteIndividualsAction->addTo( individualsToolBar );
  QObject::connect( deleteIndividualsAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotDeleteIndividuals() ) );
  deleteIndividualsAction->setStatusTip( "Delete the selected individuals from the currently selected experiment." );

  QAction *resetIndividualsAction = new QAction( "Reset", "Reset", ALT+Key_O, this, "resetIndividualsAction" );
  resetIndividualsAction->addTo( individualsMenu );
  QObject::connect( resetIndividualsAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotResetIndividuals() ) );
  resetIndividualsAction->setStatusTip( "Reset the selected individuals." );

  individualsMenu->insertSeparator();
  individualsToolBar->addSeparator();

  QAction *visualizeIndividualsAction = new QAction( "Visualize", QIconSet( QPixmap( sigelRoot + "/pixmaps/visualizeSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/visualizeLarge.xpm" ) ), "&Visualize", ALT+Key_V, this, "visualizeIndividualsAction" );
  visualizeIndividualsAction->addTo( individualsMenu );
  visualizeIndividualsAction->addTo( individualsToolBar );
  QObject::connect( visualizeIndividualsAction,
		    SIGNAL( activated() ),
		    experimentListView,
		    SLOT( slotVisualizeIndividuals() ) );
  visualizeIndividualsAction->setStatusTip( "Visualize the selected individuals." );

  noExperimentActionGroup->insert( addIndividualsAction );
  noExperimentActionGroup->insert( deleteIndividualsAction );
  noExperimentActionGroup->insert( resetIndividualsAction );
  noExperimentActionGroup->insert( visualizeIndividualsAction );

  noExperimentActionGroup->setEnabled( false );

  evolutionRunningActionGroup = new QActionGroup( this, "evolutionRunningActionGroup", false);
  evolutionRunningActionGroup->insert( renameExperimentAction );
  evolutionRunningActionGroup->insert( deleteExperimentAction );
  evolutionRunningActionGroup->insert( saveExperimentAction );
  evolutionRunningActionGroup->insert( importGPParametersAction );
  evolutionRunningActionGroup->insert( importSimulationParametersAction );
  evolutionRunningActionGroup->insert( importRobotAction );
  evolutionRunningActionGroup->insert( importLanguageParametersAction );
  evolutionRunningActionGroup->insert( importPopulationAction );
  evolutionRunningActionGroup->insert( importEnvironmentAction );
  evolutionRunningActionGroup->insert( importProgramAction );
  evolutionRunningActionGroup->insert( importIndividualAction );
  evolutionRunningActionGroup->insert( exportGPParametersAction );
  evolutionRunningActionGroup->insert( exportSimulationParametersAction );
  evolutionRunningActionGroup->insert( exportLanguageParametersAction );
  evolutionRunningActionGroup->insert( exportEnvironmentAction );
  evolutionRunningActionGroup->insert( exportPopulationAction );
  evolutionRunningActionGroup->insert( exportToGNUPlotAction );
  evolutionRunningActionGroup->insert( exportProgramAction );
  evolutionRunningActionGroup->insert( exportIndividualAction );
  evolutionRunningActionGroup->insert( addIndividualsAction );
  evolutionRunningActionGroup->insert( deleteIndividualsAction );
  evolutionRunningActionGroup->insert( resetIndividualsAction );
  evolutionRunningActionGroup->insert( visualizeIndividualsAction );

  /**
   * Meta-GP menu and toolbar definition
   * begin
   **/

  // create the menu and toolbar
  mtMenu	= new QPopupMenu( this, "helpMenu" );
  mtToolBar = new QToolBar( this, "mtToolbar" );

  // use MetaGP button
  mtUseAction = new QAction("Meta",
						QIconSet( QPixmap( sigelRoot + "/pixmaps/mt_UseSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/mt_UseSmall.xpm" )),
						"&Use MetaGP",
						0, this, 0, true);
  mtUseAction->setStatusTip( "Activates the use of the Meta-System to speed up the evolution." );
  mtUseAction->addTo( mtMenu );
  mtUseAction->addTo( mtToolBar );
  mtUseAction->setEnabled(false);
  QObject::connect(mtUseAction, SIGNAL( toggled(bool) ), SLOT( slotMTUseMT(bool) ));

  // configure MetaGP-System button
  mtConfigureAction = new QAction("Configure Meta",
							QIconSet( QPixmap( sigelRoot + "/pixmaps/mt_ConfSmall.xpm" ), QPixmap( sigelRoot + "/pixmaps/mt_ConfSmall.xpm" )),
							"&Configure System",
							0, this);
  mtConfigureAction->setStatusTip( "Pops up the Meta Systems configuration dialog." );
  mtConfigureAction->addTo( mtMenu );
  mtConfigureAction->addTo( mtToolBar );
  mtConfigureAction->setEnabled(false);
  QObject::connect(mtConfigureAction, SIGNAL( activated() ), SLOT( slotMTConfigureSystem() ));

  // MetaGP system selection (evaluator or classifier)
  mtMenu->insertSeparator();
  mtChoiceTypeActionGroup = new QActionGroup(this, "mtChoiceTypeActionGroup", true);
  mtChoiceEvaluatorAction = new QAction("choose evaluator system",
									"&Evaluator System",
									0, mtChoiceTypeActionGroup,
									"mtEvaluator", true);
  mtChoiceEvaluatorAction->setOn(true);
  mtChoiceEvaluatorAction->setStatusTip( "Chooses the Evaluator System. Replaces the simulation based fitness calculation by a MetaGP System which needs less simulation." );
  mtChoiceClassifierAction = new QAction("choose classifier system",
									"Cl&assifier System",
									0, mtChoiceTypeActionGroup,
									"mtClassifier", true);
  mtChoiceClassifierAction->setStatusTip( "Chooses the Classifer System. Replaces the fitness based tournament decision by a MetaGP System which needs less fitness calculation." );
  mtChoiceTypeActionGroup->addTo( mtMenu );
  mtChoiceTypeActionGroup->setEnabled(false);
  QObject::connect(mtChoiceTypeActionGroup, SIGNAL( selected(QAction *) ), SLOT( slotMTSwitchSystem(QAction*) ));

  // about-box button
  mtMenu->insertSeparator();
  mtMenu->insertItem( "A&bout", this, SLOT( slotAbout() ) );

  noExperimentActionGroup->insert( mtUseAction );
  QObject::connect(experimentListView,
	  SIGNAL( actExpChanged() ),
	  this,
	  SLOT( slotActExpChanged() ));
//  noExperimentActionGroup->insert( mtChoiceTypeActionGroup );
//  noExperimentActionGroup->insert( mtConfigureAction );

  /**
   * Meta-GP menu and toolbar definition
   * end
   **/

  menuBar()->insertItem( "&File", fileMenu );
  menuBar()->insertItem( "&View", viewMenu );
  menuBar()->insertItem( "&Individuals", individualsMenu );
  menuBar()->insertItem( "&Options", optionsMenu );

  menuBar()->insertItem( "&MetaGP", mtMenu );	// MetaGP-Menü einfügen

  menuBar()->insertItem( "&Help", helpMenu );

  QObject::connect( experimentListView,
		    SIGNAL( isNotEmpty( bool ) ),
		    noExperimentActionGroup,
		    SLOT( setEnabled( bool ) ) );

  widgetBase = new QLabel( this, "baseWidget" ); // new QLabel( "I am the void", this, "baseWidget");
  widgetBase->setPixmap( QPixmap( sigelRoot + "/pixmaps/noExperiment.png" ) );
  widgetBase->setScaledContents( true );
  widgetStack->addWidget( widgetBase, 0 );
  widgetStack->raiseWidget( widgetBase );

  QObject::connect( experimentListView,
		    SIGNAL( isNotEmpty( bool ) ),
		    this,
		    SLOT(slotShowEmpty( bool ) ) );

  QObject::connect( experimentListView,
		    SIGNAL( evolutionNotRunning( bool ) ),
		    evolutionRunningActionGroup,
		    SLOT( setEnabled( bool ) ) );
  statusBar()->message( "Ready.", 5000 );
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
      widgetStack->raiseWidget( widgetBase );
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
  bool usesBigPixmaps = !this->usesBigPixmaps();
  optionsMenu->setItemChecked( bigPixmapID, usesBigPixmaps );
  this->setUsesBigPixmaps( usesBigPixmaps );
};

void SIG_MainWindow::slotUseTextLabels()
{
  bool usesTextLabels = !this->usesTextLabel();
  optionsMenu->setItemChecked( textLabelsID, usesTextLabels);
  this->setUsesTextLabel( usesTextLabels );
};

void SIG_MainWindow::slotChangeFont()
{
  bool ok;
  QFont newFont = QFontDialog::getFont( &ok, QApplication::font() );
  if( ok )
    qApp->setFont( newFont, true );
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
		if(mtChoiceClassifierAction->isOn()){
			if(!actExperiment->gpExperiment.mtController->switchSystem(CLASSIFIER_SUBST))
				mtChoiceEvaluatorAction->setOn(true);
		} else {
			if(!actExperiment->gpExperiment.mtController->switchSystem(EVALUATOR_SUBST))
				mtChoiceClassifierAction->setOn(true);
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
			mtUseAction->setOn(true);
			if(actExperiment->gpExperiment.mtController->UsedSystem() == EVALUATOR_SUBST){
				mtChoiceEvaluatorAction->setOn(true);
			} else {
				mtChoiceClassifierAction->setOn(true);
			}

		} else {

			// no meta gp-system is used
			mtChoiceTypeActionGroup->setEnabled(false);
			mtConfigureAction->setEnabled(false);
			mtUseAction->setOn(false);

		}
	}
};

}
