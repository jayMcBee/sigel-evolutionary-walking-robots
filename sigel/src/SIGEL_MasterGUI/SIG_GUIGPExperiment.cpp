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
#include <qfiledialog.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <QDateTime>

#include "SIGEL_MasterGUI/SIG_GUIGPExperiment.h"
#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"

#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"
#include "SIGEL_GP/SIG_GUIGPManager.h"

#include "SIGEL_RobotIO/SIG_RobotBuilder.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

#include "SIGEL_Tools/SIG_IO.h"


namespace SIGEL_MasterGUI
{

  SIG_GUIGPExperiment::SIG_GUIGPExperiment( QString name, QStackedWidget *theWidgetStack, SIG_ExperimentItem *theExperimentItem ) : gpExperiment(), guiGPManager(0), experimentName(name), widgetStack( theWidgetStack ), evolutionRunning(false), generationAtStart(0), experimentItem(theExperimentItem), experimentListView( static_cast<SIG_ExperimentListView *>( theExperimentItem->treeWidget() ) )
{
  // build the gp parameter menu
  menuGPParameter = new QMenu( this );
  menuGPParameter->addAction( "GP Parameters" );

  // build the simulation parameter menu
  menuSimulationParameter = new QMenu( this );
  menuSimulationParameter->addAction( "Import...", this, SLOT( slotSimulationParameterImport() ) );
  menuSimulationParameter->addAction( "Export...", this, SLOT( slotSimulationParameterExport() ) );

  // build the environment view menu
  menuEnvironmentView = new QMenu( this );
  menuEnvironmentView->addAction( "Import...", this, SLOT( slotEnvironmentImport() ) );
  menuEnvironmentView->addAction( "Export...", this, SLOT( slotEnvironmentExport() ) );

  // build the robot view menu
  menuRobotView = new QMenu( this );
  menuRobotView->addAction( "RobotView" );

  // build experiment view menu
  menuExperimentView = new QMenu( this );
  startEvolutionAction = menuExperimentView->addAction( "Start", this, SLOT( slotStartEvolution() ) );
  stopEvolutionAction = menuExperimentView->addAction( "Stop", this, SLOT( slotStopEvolution() ) );

  gpParameter = new SIG_GPParameter( this , "GPParameter", Qt::WindowFlags(), gpExperiment );
  simulationParameter = new SIG_SimulationParameter( this, "SimulationParameter", Qt::WindowFlags(), gpExperiment);
  environmentView = new SIG_EnvironmentView( this, "EnvironmentView", Qt::WindowFlags(), gpExperiment );
  robotView = new SIG_RobotView( this, "RobotView", Qt::WindowFlags(), gpExperiment );
  experimentView = new SIG_ExperimentView( this, "ExperimentView", Qt::WindowFlags(), gpExperiment, *this );
  allIndividualsView = new SIG_AllIndividualsView( this, "AllIndividualsView", gpExperiment );
  languageParameters = new SIG_LanguageParameters( this, "LanguageParametersView", Qt::WindowFlags(), gpExperiment );
  
  // put all the widgets on the stack
  widgetStack->addWidget( gpParameter );
  widgetStack->addWidget( simulationParameter );
  widgetStack->addWidget( environmentView );
  widgetStack->addWidget( robotView );
  widgetStack->addWidget( experimentView );
  widgetStack->addWidget( allIndividualsView );
  widgetStack->addWidget( languageParameters );

  // insert the widgets into the widget dictionary
  widgetDict.insert( "GP Parameters" , gpParameter );
  widgetDict.insert( "Simulation Parameters", simulationParameter );
  widgetDict.insert( "Language Parameters", languageParameters );
  widgetDict.insert( "Environment", environmentView );
  widgetDict.insert( "Robot", robotView );
  widgetDict.insert( "Individuals", allIndividualsView );
  widgetDict.insert( experimentName, experimentView );

  // insert the widgets into the menu dictionary
  menuDict.insert( "GP Parameters" , menuGPParameter );
  menuDict.insert( "Simulation Parameters", menuSimulationParameter );
  menuDict.insert( "Environment" , menuEnvironmentView );
  menuDict.insert( "Robot" , menuRobotView );
  menuDict.insert( experimentName, menuExperimentView );
  
  // The bar counts the generation's finished tournaments. Before the first
  // tournaments exist both counts are 0, and the bar shows it is busy.
  QObject::connect( &progressTimer, &QTimer::timeout, this, [this]()
    {
      if( guiGPManager )
	{
	  const auto [doneCount, plannedCount] = guiGPManager->tournamentProgress();
	  experimentView->generationProgBar->setRange( 0, plannedCount );
	  experimentView->generationProgBar->setValue( doneCount );
	}

      // SIG_GPFitnessTrainer waits for results with no timeout on the wait as a
      // whole, so a run that has lost PVM sits there for ever and the window
      // goes on saying an evolution is in progress. Stopping it is this class's
      // decision, not the trainer's.
      if( guiGPManager && guiGPManager->pvmIsLost() && endedBecause.isEmpty() )
	{
	  endedBecause = "PVM can no longer be reached, so no further individual"
			 " can be evaluated. The evolution has been stopped.";
	  guiGPManager->userTerminated = true;
	}
    } );

  QObject::connect( experimentView->pushbuttonStart,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotStartEvolution() ) );

  QObject::connect( experimentView->pushbuttonStop,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotStopEvolution() ) );

  QObject::connect( gpParameter->pushbuttonImport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotGPParameterImport() ) );

  QObject::connect( gpParameter->pushbuttonExport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotGPParameterExport() ) );

  QObject::connect( languageParameters->pushbuttonImport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotLanguageParameterImport() ) );

  QObject::connect( languageParameters->pushbuttonExport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotLanguageParameterExport() ) );

  QObject::connect( simulationParameter->pushbuttonImport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotSimulationParameterImport() ) );

  QObject::connect( simulationParameter->pushbuttonExport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotSimulationParameterExport() ) );

  QObject::connect( environmentView->pushbuttonImport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotEnvironmentImport() ) );

  QObject::connect( environmentView->pushbuttonExport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotEnvironmentExport() ) );

  QObject::connect( robotView->pushbuttonImport,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotRobotImport() ) );

  QObject::connect( robotView->pushbuttonLoad,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotRobotLoad() ) );

  QObject::connect( robotView->pushbuttonSave,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotRobotSave() ) );

  QObject::connect( robotView->pushbuttonRobInfo,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotRobotInfo() ) );

  QObject::connect( allIndividualsView,
		    SIGNAL( signalDataRefreshNeeded() ),
		    this,
		    SLOT( putAllIntoExperiment() ) );

  // Any experiment's run locks every experiment, so both of these ask the list
  // view rather than this experiment.
  QObject::connect( experimentListView,
		    SIGNAL( evolutionNotRunning( bool ) ),
		    allIndividualsView,
		    SLOT( slotEvolutionNotRunning( bool ) ) );

  QObject::connect( experimentListView,
		    SIGNAL( evolutionNotRunning( bool ) ),
		    this,
		    SLOT( slotEvolutionNotRunning( bool ) ) );
  getAllOutOfExperiment();
};

SIG_GUIGPExperiment::~SIG_GUIGPExperiment()
{
  progressTimer.stop();
  // Before destroying the experiment take all widgets in the widgetDict from the widgetStack
  for ( QWidget *w : widgetDict )
    widgetStack->removeWidget( w );
  // Qt 2's widgetDict had setAutoDelete(true), so ~QDict deleted every widget
  // it still held (qgdict.cpp deleteItem). QHash owns nothing, and the widgets
  // were just orphaned by removeWidget(), so the delete has to be explicit.
  qDeleteAll( widgetDict );

   if( guiGPManager )
      {
        delete guiGPManager;
      }
};

QString SIG_GUIGPExperiment::getName() const
{
  return experimentName;
};

void SIG_GUIGPExperiment::setName( QString newName )
{
  widgetDict.insert( newName, widgetDict.take( experimentName ) );
  menuDict.insert( newName, menuDict.take( experimentName ) );

  experimentName = newName;
  experimentItem->setText( 0, newName );
};

bool SIG_GUIGPExperiment::isRunning()
{
  return evolutionRunning;
};

void SIG_GUIGPExperiment::putAllIntoExperiment()
{
  // Parameters may not change once a run has started: the parameters define
  // the run. The check belongs here rather than on the widgets, because
  // SIG_ExperimentListView::slotSelectionChanged calls this unconditionally, so
  // a page switch could otherwise push widget state into a live run.
  if ( experimentListView->isRunning() )
    return;

  experimentView->putIntoExperiment();
  gpParameter->putIntoExperiment();
  simulationParameter->putIntoExperiment();
  languageParameters->putIntoExperiment();
  environmentView->putIntoExperiment();
};

void SIG_GUIGPExperiment::getAllOutOfExperiment()
{
  experimentView->getOutOfExperiment();
  gpParameter->getOutOfExperiment();
  simulationParameter->getOutOfExperiment();
  robotView->getOutOfExperiment();
  languageParameters->getOutOfExperiment();
  environmentView->getOutOfExperiment();
  allIndividualsView->slotCompleteRefreshList();
};

QString SIG_GUIGPExperiment::checkEnding( QString fileName, QString ending )
{
  QString endWithPoint = "." + ending;
  if( fileName.right( endWithPoint.length() ) == endWithPoint )
    return fileName;

  // The file dialog asked only about the name it returned. An existing file
  // under the name with the ending added is never overwritten: a date stamp
  // goes between name and ending, one second later while that name is taken
  // as well.
  if( !QFile::exists( fileName + endWithPoint ) )
    return fileName + endWithPoint;

  QDateTime stampTime = QDateTime::currentDateTime();
  QString stampedName;
  do
    {
      stampedName = fileName + "-" + stampTime.toString( "yyyy-MM-dd-hh-mm-ss" ) + endWithPoint;
      stampTime = stampTime.addSecs( 1 );
    }
  while( QFile::exists( stampedName ) );
  return stampedName;
};

// Dialogs here take experimentListView as their parent: this widget is never
// shown, so a dialog parented to it is not tied to the main window.
void SIG_GUIGPExperiment::slotRightClick( QString option, const QPoint & thePoint )
{
  QMenu *showMenu = menuDict.value( option );
  QWidget *showWidget = widgetDict.value( option );
  if( showMenu && showWidget )
    {
      startEvolutionAction->setEnabled( !experimentListView->isRunning() );
      stopEvolutionAction->setEnabled( isRunning() );
      showMenu->popup( thePoint );
      widgetStack->setCurrentWidget( showWidget );
    }
};

void SIG_GUIGPExperiment::slotSelectionChanged( QString option )
{
  // Qt 2's raiseWidget() began "if ( !w || !isMyChild(w) ) return;", so a
  // missing key was a silent no-op; Qt 6 warns and does nothing instead.
  if ( QWidget *showWidget = widgetDict.value( option ) )
    widgetStack->setCurrentWidget( showWidget );
};

void SIG_GUIGPExperiment::slotEvolutionNotRunning( bool isNotRunning )
{
  experimentView->pushbuttonStart->setEnabled( isNotRunning );
  // Start and Stop are on this page too, and a disabled parent takes its
  // children with it, so the page is locked widget by widget.
  experimentView->checkboxHistory->setEnabled( isNotRunning );
  experimentView->sliderIntervall->setEnabled( isNotRunning );
  experimentView->multilineeditComment->setEnabled( isNotRunning );
  experimentView->pushbuttonPostscript->setEnabled( isNotRunning );
  experimentView->pushbuttonShowFitnessCurve->setEnabled( isNotRunning );
  gpParameter->setEnabled( isNotRunning );
  simulationParameter->setEnabled( isNotRunning );
  robotView->setEnabled( isNotRunning );
  languageParameters->setEnabled( isNotRunning );
  environmentView->setEnabled( isNotRunning );
};

void SIG_GUIGPExperiment::slotStartEvolution()
{
  // One run at a time.
  if( experimentListView->isRunning() )
    return;

  // First: the test and SIG_GUIGPManager's constructor read these values.
  putAllIntoExperiment();

  if( (gpExperiment.robot.getBodies().size() != 0) && (gpExperiment.population.getSize() >= 4) && SIGEL_GP::SIG_GPFitnessFunctionRegistry::indexOf( gpExperiment.gpParameter.getFitnessName() ) )
    {
      delete guiGPManager;
      
      guiGPManager = new SIGEL_GP::SIG_GUIGPManager( *this );
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cout << "Starting Evolution" << Qt::endl;
#endif
      
      // The emit locks every experiment through slotEvolutionNotRunning. Stop is
      // not one of the widgets it touches, so it is set here.
      emit signalEvolutionNotRunning( false );
      experimentView->pushbuttonStop->setEnabled( true );
      
      // Set after putAllIntoExperiment above, so the user's settings commit,
      // and before start(). slotEvolutionStopped() must run even if start()
      // throws.
      evolutionRunning = true;
      endedBecause = QString();
      generationAtStart = gpExperiment.population.getPoolGeneration();
      runStartedAt = QDateTime::currentDateTime();
      progressTimer.start( 200 );
      try {
        guiGPManager->start();
      }
      catch (...) {
        endedBecause = "The evolution stopped because of an error.";
        slotEvolutionStopped();
        throw;
      }

      slotEvolutionStopped();
    }
  else
    {
      QMessageBox::warning( experimentListView, "Can't Start Evolution", "The evolution cannot be started. There may be several reasons:<ul><li>There is no robot loaded.</li><li>There are fewer than four individuals in the population.</li><li>No fitness function is selected.</li></ul>");
    }
};

// The termination condition is tested the way SIG_GPManager tests it, against
// the settings this run was started with. A duration counts from runStartedAt,
// which is why the interface has to keep its own copy of the start time.
QString SIG_GUIGPExperiment::terminationAlreadyMet() const
{
  const SIGEL_GP::SIG_GPParameter &parameter = gpExperiment.gpParameter;
  const QDateTime now = QDateTime::currentDateTime();

  QString byTime;
  if( parameter.getTerminationUsesDate() )
    {
      const QDateTime end = parameter.getTerminationTime();
      if( end <= now )
	byTime = "it terminates by time, on " + end.toString( "d MMMM yyyy, hh:mm" )
		 + ", which has passed. Change the date under \"By time\"";
    }
  else
    {
      const int seconds =   ( ( ( parameter.getTerminationDurationDays() * 24
				  + parameter.getTerminationDurationHours() ) * 60
				+ parameter.getTerminationDurationMinutes() ) * 60 )
			  + parameter.getTerminationDurationSeconds();
      if( runStartedAt.addSecs( seconds ) <= now )
	byTime = "its running time of " + QString::number( seconds )
		 + " seconds is up. Change the duration under \"By time\"";
    }

  QString byGeneration;
  if( parameter.getTerminationGenerationNo() <= 0 )
    byGeneration = "it terminates after "
		   + QString::number( parameter.getTerminationGenerationNo() )
		   + " generations. Change the number under \"By generation\"";

  QString reason;
  switch( parameter.getTerminationModel() )
    {
    case SIGEL_GP::SIG_GPParameter::byTime:           reason = byTime; break;
    case SIGEL_GP::SIG_GPParameter::byGeneration:     reason = byGeneration; break;
    case SIGEL_GP::SIG_GPParameter::byTimeGeneration:
      reason = byTime.isEmpty() ? byGeneration : byTime; break;
    case SIGEL_GP::SIG_GPParameter::byUser:           break;
    }

  if( reason.isEmpty() )
    return QString();

  return "The evolution ended at once, because " + reason
	 + " on the GP Parameters page, tab Evolution control.";
};

void SIG_GUIGPExperiment::slotStopEvolution()
{
  // Do NOT emit signalEvolutionNotRunning( true ) here. This is a
  // request to stop, not a stop: it only sets guiGPManager->userTerminated
  // below, start() has not returned, and isRunning() is still true,
  // so the locked actions must stay locked. slotEvolutionStopped() emits it
  // after start() returns.
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cout << "Stopping Evolution" << Qt::endl;
#endif
  // experimentView->pushbuttonStart->setEnabled( true ); // will be done
  experimentView->pushbuttonStop->setEnabled( false );

  // enable the widgets
  // gpParameter->setEnabled( true );
  // simulationParameter->setEnabled( true );
  // robotView->setEnabled( true );
  // languageParameters->setEnabled( true );
  // environmentView->setEnabled( true );
  // allIndividualsView->setEnabled( true );

  guiGPManager->userTerminated = true;
};

void SIG_GUIGPExperiment::slotSimulationParameterImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import Simulation Parameters", QString(), "Simulation Parameter Files (*.sip);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.simulationParameter.readFromFile( theStream );
	}
      file.close();
      simulationParameter->getOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotSimulationParameterExport()
{
  // The tree menu that reaches this is never greyed, so the check is here.
  if ( experimentListView->isRunning() )
    return;

  simulationParameter->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export Simulation Parameters", QString(), "Simulation Parameter Files (*.sip);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "sip" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.simulationParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotEnvironmentImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import Environment", QString(), "Environment Files (*.env);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.environment.readFromFile( theStream );
	}
      file.close();
      environmentView->getOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotEnvironmentExport()
{
  // The tree menu that reaches this is never greyed, so the check is here.
  if ( experimentListView->isRunning() )
    return;

  environmentView->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export Environment", QString(), "Environment Files (*.env);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "env" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.environment.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotGPParameterImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import GP Parameters", QString(), "GP Parameter Files (*.gpp);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.gpParameter.readFromFile( theStream );
	}
      file.close();
      gpParameter->getOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotGPParameterExport()
{
  gpParameter->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export GP Parameters", QString(), "GP Parameter Files (*.gpp);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "gpp" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.gpParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotLanguageParameterImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import Language Parameters", QString(), "Language Parameter Files (*.lap);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  try
	    {
	      SIGEL_Robot::SIG_LanguageParameters *newLanguageParameters = new SIGEL_Robot::SIG_LanguageParameters( theStream, true );
	      gpExperiment.robot.setLangParam( newLanguageParameters );
	    }
	  catch ( const SIGEL_Tools::SIG_Exception &e )
	    {
	      QMessageBox::warning( experimentListView, "Import Language Parameters", e.getMessage() );
	    }
	}
      file.close();
      languageParameters->getOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotLanguageParameterExport()
{
  languageParameters->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export Language Parameters", QString(), "Language Parameter Files (*.lap);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "lap" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.robot.getLangParam()->writeToFileTransfer( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotPopulationImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import Population", QString(), "Population Files (*.pop);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.population.readFromFile( theStream );
	}
      file.close();
      allIndividualsView->slotCompleteRefreshList();
    }
};

void SIG_GUIGPExperiment::slotPopulationExport()
{
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export Population", QString(), "Population Files (*.pop);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "pop" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.population.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotRobotImport()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Import Robot", QString(), "Raw Robot Files (*.rrb);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      try
	{
	  SIGEL_RobotIO::SIG_RobotBuilder builder( fileName );
	  builder.buildInto( gpExperiment.robot );
	  robotView->textlabelUsedRobot->setText( "Used robot: " + fileName );
	}
      catch( SIGEL_Tools::SIG_Exception e )
	{
	  QMessageBox::warning( experimentListView, "Robot Import Error", e.getMessage() );
	  gpExperiment.robot.clear();
	}
      // perhaps it is enough to update the language parameter screen and the robot view
      this->getAllOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotGNUPlotExport()
{
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Export to gnuplot", QString(), "gnuplot Data Files (*.dat);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "dat" );
      gpExperiment.exportExperimentHistoryToGNUPlot( fileName );
    }
};

void SIG_GUIGPExperiment::slotRobotLoad()
{
  // These write gpExperiment.* directly, so putAllIntoExperiment's check does
  // not cover them.
  if ( experimentListView->isRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( experimentListView, "Load Robot", QString(), "Compiled Robot Files (*.crb);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  try
	    {
	      gpExperiment.robot.readFromFileTransfer( theStream );
	      robotView->textlabelUsedRobot->setText( "Used robot: " + fileName );
	    }
	  catch( SIGEL_Tools::SIG_Exception e )
	    {
	      QMessageBox::warning( experimentListView, "Robot Import Error", e.getMessage() );
	      gpExperiment.robot.clear();
	    }
	}
      file.close();
      // perhaps it is enough to update the language parameter screen and the robot view
      this->getAllOutOfExperiment();
    }
};

void SIG_GUIGPExperiment::slotRobotSave()
{
  QString fileName = QFileDialog::getSaveFileName( experimentListView, "Save Robot", QString(), "Compiled Robot Files (*.crb);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "crb" );
      QFile file( fileName );
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.robot.writeToFileTransfer( theStream );
	}
      file.close();
    }
};

void SIG_GUIGPExperiment::slotRobotInfo()
{ char robInf[4096];


  // need DynaMechs for that..
  if (gpExperiment.simulationParameter.getSimulationLibrary() != SIGEL_Simulation::SIG_SimulationParameters::DynaMechs)
  { QMessageBox::information( experimentListView, "Can't Display Robot Information", "<B>DynaMechs is required for this operation to work properly.</B>");
    return;
  }

  // get info and display
  if( (gpExperiment.robot.getBodies().size() != 0))
  { gpExperiment.robot.getRobotInformation(robInf, 4096);
    QMessageBox::information( experimentListView, "Robot Information", robInf );
  }
  else
  { QMessageBox::information( experimentListView, "Robot Information", "No robot is loaded. Load a robot first.");
  }
}

void SIG_GUIGPExperiment::slotEvolutionStopped()
{
  evolutionRunning = false;
  progressTimer.stop();
  experimentView->generationProgBar->reset();
  emit signalEvolutionNotRunning( true );
  experimentView->pushbuttonStop->setEnabled( false );

  // Show the generation the run reached.
  experimentView->lcdnumberGenerations->display( gpExperiment.population.getPoolGeneration() );

  // A run that completed no generation, and that nobody stopped, leaves the
  // window looking exactly like one that worked: the Start button simply comes
  // back. Say why when the termination condition is what ended it.
  if(    endedBecause.isEmpty()
      && gpExperiment.population.getPoolGeneration() == generationAtStart
      && guiGPManager && !guiGPManager->userTerminated )
    endedBecause = terminationAlreadyMet();

  if( !endedBecause.isEmpty() )
    {
      QMessageBox::warning( experimentListView, "Evolution Stopped", endedBecause );
      endedBecause = QString();
    }


  /*
  if( guiGPManager )
    {
      delete guiGPManager;
      guiGPManager = 0;
    }
  */
};

}

