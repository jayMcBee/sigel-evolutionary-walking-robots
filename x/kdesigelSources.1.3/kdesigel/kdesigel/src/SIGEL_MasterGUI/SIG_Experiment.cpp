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

#include "SIGEL_MasterGUI/SIG_Experiment.h"

#include "SIGEL_GP/SIG_GUIGPManager.h"

#include "SIGEL_RobotIO/SIG_RobotBuilder.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

#include "SIGEL_Tools/SIG_IO.h"


namespace SIGEL_MasterGUI
{

  SIG_Experiment::SIG_Experiment( QString name, QStackedWidget *theWidgetStack, SIG_ExperimentItem *theExperimentItem ) : gpExperiment(), guiGPManager(0), experimentName(name), widgetStack( theWidgetStack ), experimentItem(theExperimentItem)
{
  // build the gp parameter menu
  menuGPParameter = new QMenu( this );
  menuGPParameter->addAction( "GPParameter" );

  // build the simulation parameter menu
  menuSimulationParameter = new QMenu( this );
  menuSimulationParameter->addAction( "Import", this, SLOT( slotSimulationParameterImport() ) );
  menuSimulationParameter->addAction( "Export", this, SLOT( slotSimulationParameterExport() ) );

  // build the environment view menu
  menuEnvironmentView = new QMenu( this );
  menuEnvironmentView->addAction( "Import", this, SLOT( slotEnvironmentImport() ) );
  menuEnvironmentView->addAction( "Export", this, SLOT( slotEnvironmentExport() ) );

  // build the robot view menu
  menuRobotView = new QMenu( this );
  menuRobotView->addAction( "RobotView" );

  // build experiment view menu
  menuExperimentView = new QMenu( this );
  menuExperimentView->addAction( "Start", this, SLOT( slotStartEvolution() ) );
  menuExperimentView->addAction( "Stop", this, SLOT( slotStopEvolution() ) );

  gpParameter = new SIG_GPParameter( this , "GPParameter", Qt::WindowFlags(), gpExperiment );
  simulationParameter = new SIG_SimulationParameter( this, "SimulationParameter", Qt::WindowFlags(), gpExperiment);
  environmentView = new SIG_EnvironmentView( this, "EnvironmentView", Qt::WindowFlags(), gpExperiment );
  robotView = new SIG_RobotView( this, "RobotView", Qt::WindowFlags(), gpExperiment );
  experimentView = new SIG_ExperimentView( this, "ExperimentView", Qt::WindowFlags(), gpExperiment );
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
  widgetDict.insert( "GP-Parameters" , gpParameter );
  widgetDict.insert( "Simulation-Parameters", simulationParameter );
  widgetDict.insert( "Language-Parameters", languageParameters );
  widgetDict.insert( "Environment", environmentView );
  widgetDict.insert( "Robot", robotView );
  widgetDict.insert( "Individuals", allIndividualsView );
  widgetDict.insert( experimentName, experimentView );

  // insert the widgets into the menu dictionary
  menuDict.insert( "GP-Parameters" , menuGPParameter );
  menuDict.insert( "Simulation-Parameters", menuSimulationParameter );
  menuDict.insert( "Environment" , menuEnvironmentView );
  menuDict.insert( "Robot" , menuRobotView );
  menuDict.insert( experimentName, menuExperimentView );
  
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

  QObject::connect( this,
		    SIGNAL( signalEvolutionNotRunning( bool ) ),
		    allIndividualsView,
		    SLOT( slotEvolutionNotRunning( bool ) ) );
  getAllOutOfExperiment();
};

SIG_Experiment::~SIG_Experiment()
{
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

QString SIG_Experiment::getName() const
{
  return experimentName;
};

void SIG_Experiment::setName( QString newName )
{
  widgetDict.insert( newName, widgetDict.take( experimentName ) );
  
  experimentName = newName;
  experimentItem->setText( 0, newName );
};

// D29's run counter. File-static rather than a class member so that no
// experiment can be destroyed out from under it.
static int g_runningEvolutions = 0;

bool SIG_Experiment::anyEvolutionRunning()
{
  return g_runningEvolutions > 0;
}

SIG_Experiment::RunScope::RunScope()  { ++g_runningEvolutions; }
SIG_Experiment::RunScope::~RunScope() { --g_runningEvolutions; }

void SIG_Experiment::putAllIntoExperiment()
{
  // DELIBERATE DEVIATION FROM 1.3, decided 2026-09-04: parameters may not
  // change once a run has started. That is how GP is normally implemented --
  // the parameters define the run -- and 1.3's behaviour here is not the
  // specification.
  //
  // The guard belongs HERE rather than on the widgets, because this function
  // is reached by a path no widget guard covers:
  // SIG_ExperimentListView::slotSelectionChanged ends with an UNCONDITIONAL
  // putAllIntoExperiment(), two lines after it has already asked
  // anyEvolutionRunning() for a different purpose. So disabling the pages
  // leaves a page switch able to push widget state into a live run.
  //
  // 1.3 does disable the five pages while running (slotStartEvolution below),
  // so no user-typed value can currently reach here mid-run -- this makes the
  // property structural instead of incidental, and covers any future caller.
  //
  // slotStartEvolution calls this BEFORE guiGPManager->start(), so the settings a
  // user chose are still committed at start; only writes after that are
  // refused.
  if ( anyEvolutionRunning() )
    return;

  experimentView->putIntoExperiment();
  gpParameter->putIntoExperiment();
  simulationParameter->putIntoExperiment();
  languageParameters->putIntoExperiment();
  environmentView->putIntoExperiment();
};

void SIG_Experiment::getAllOutOfExperiment()
{
  experimentView->getOutOfExperiment();
  gpParameter->getOutOfExperiment();
  simulationParameter->getOutOfExperiment();
  robotView->getOutOfExperiment();
  languageParameters->getOutOfExperiment();
  environmentView->getOutOfExperiment();
  allIndividualsView->slotCompleteRefreshList();
};

QString SIG_Experiment::checkEnding( QString fileName, QString ending )
{
  QString endWithPoint = "." + ending;
  if( fileName.right( endWithPoint.length() ) == endWithPoint )
    return fileName;
  else
    return fileName.append( "." + ending);
};

void SIG_Experiment::slotRightClick( QString option, const QPoint & thePoint )
{
  QMenu *showMenu = menuDict.value( option );
  QWidget *showWidget = widgetDict.value( option );
  if( showMenu && showWidget )
    {
      showMenu->popup( thePoint );
      widgetStack->setCurrentWidget( showWidget );
    }
};

void SIG_Experiment::slotSelectionChanged( QString option )
{
  // Qt 2's raiseWidget() began "if ( !w || !isMyChild(w) ) return;", so a
  // missing key was a silent no-op; Qt 6 warns and does nothing instead.
  if ( QWidget *showWidget = widgetDict.value( option ) )
    widgetStack->setCurrentWidget( showWidget );
};

void SIG_Experiment::slotStartEvolution()
{
  if( (gpExperiment.robot.getBodies().size() != 0) && (gpExperiment.population.getSize() >= 4) && (gpExperiment.gpParameter.getFitnessName() != QString()) )
    {
      delete guiGPManager;
      
      guiGPManager = new SIGEL_GP::SIG_GUIGPManager( *this );
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cout << "Starting Evolution (Haha)\n";
#endif
      putAllIntoExperiment();
      
      emit signalEvolutionNotRunning( false );
      experimentView->pushbuttonStart->setEnabled( false );
      experimentView->pushbuttonStop->setEnabled( true );
      
      // disable all the widgets while the evolution is running...
      gpParameter->setEnabled( false );
      simulationParameter->setEnabled( false );
      robotView->setEnabled( false );
      languageParameters->setEnabled( false );
      environmentView->setEnabled( false );
      // allIndividualsView->setEnabled( false );
      
      // Entered AFTER putAllIntoExperiment() above, so the settings the user
      // chose are committed, and before start(), so nothing can change them
      // from here on. start() runs the evolution synchronously and services
      // the GUI through haveABreak()'s processEvents, so widgets and menus
      // really are reachable during it.
      //
      // A SCOPE GUARD, not two assignments: it survives an exception out of
      // start(), which would otherwise leave the experiment locked for good.
      // D30a. slotEvolutionStopped() is the only thing that re-enables Start
      // and the five pages disabled above, so it must run even if start()
      // throws. The RunScope already survives a throw; this is the other half.
      {
        RunScope runScope;
        try {
          guiGPManager->start();
        }
        catch (...) {
          slotEvolutionStopped();
          throw;
        }
      }

      slotEvolutionStopped();
    }
  else
    {
      QMessageBox::warning( this, "Can't start evolution...", "The evolution cannot be started. There may be several reasons:<ul><li>There is no robot loaded.</li><li>There are less than four individuals in the population</li><li>No fitness function name was specified.</li></ul>");
    }
};

void SIG_Experiment::slotStopEvolution()
{
  // D30a. Do NOT emit signalEvolutionNotRunning( true ) here. This is a
  // request to stop, not a stop: it only sets guiGPManager->userTerminated
  // below, start() has not returned, and anyEvolutionRunning() is still true,
  // so the locked actions must stay locked. slotEvolutionStopped() emits it
  // after start() returns.
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cout << "Stopping Evolution (Haha)\n";
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

void SIG_Experiment::slotSimulationParameterImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import Simulation Parameters...", QString(), "Simulation Parameter Files (*.sip);;All Files (*)" );
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

void SIG_Experiment::slotSimulationParameterExport()
{
  simulationParameter->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export Simulation Parameters...", QString(), "Simulation Parameter Files (*.sip);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "sip" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.simulationParameter.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.simulationParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotEnvironmentImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import Environment...", QString(), "Environment Files (*.env);;All Files (*)" );
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

void SIG_Experiment::slotEnvironmentExport()
{
  environmentView->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export Environment...", QString(), "Environment Files (*.env);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "env" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.environment.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.environment.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotGPParameterImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import GP Parameter...", QString(), "GP Parameter Files (*.gpp);;All Files (*)" );
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

void SIG_Experiment::slotGPParameterExport()
{
  gpParameter->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export GP Parameter...", QString(), "GP Parameter Files (*.gpp);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "gpp" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.gpParameter.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.gpParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotLanguageParameterImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import Language Parameter...", QString(), "Language Parameter Files (*.lap);;All Files (*)" );
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(QIODevice::ReadOnly) )
	{
	  QTextStream theStream( &file );
	  SIGEL_Robot::SIG_LanguageParameters *newLanguageParameters = new SIGEL_Robot::SIG_LanguageParameters( theStream, true );
	  gpExperiment.robot.setLangParam( newLanguageParameters );
	}
      file.close();
      languageParameters->getOutOfExperiment();
    }
};

void SIG_Experiment::slotLanguageParameterExport()
{
  languageParameters->putIntoExperiment();
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export Language Parameters...", QString(), "Language Parameter Files (*.lap);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "lap" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.robot.getLangParam()->writeToFileTransfer( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.robot.getLangParam()->writeToFileTransfer( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotPopulationImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import Population...", QString(), "Population Files (*.pop);;All Files (*)" );
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

void SIG_Experiment::slotPopulationExport()
{
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export Population...", QString(), "Population files (*.pop);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "pop" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.population.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.population.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotRobotImport()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Import Robot...", QString(), "Raw Robot Files (*.rrb);;All Files (*)" );
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
	  QMessageBox::warning( this, "Robot import error!", e.getMessage() );
	  gpExperiment.robot.clear();
	}
      // perhaps it is enough to update the language parameter screen and the robot view
      this->getAllOutOfExperiment();
    }
};

void SIG_Experiment::slotGNUPlotExport()
{
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Export to GNU plot...", QString(), "GNU plot data file (*.dat);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "dat" );
      gpExperiment.exportExperimentHistoryToGNUPlot( fileName );
    }
};

void SIG_Experiment::slotRobotLoad()
{
  // D29. These write gpExperiment.* DIRECTLY -- slotRobotLoad replaces the
  // whole robot -- so none of them passes through putAllIntoExperiment and the
  // guard there does not cover them. Their page buttons are disabled during a
  // run, but menuSimulationParameter and menuEnvironmentView are parented on
  // SIG_Experiment rather than on the pages, so a right-click on the tree item
  // pops them regardless, and slotRightClick does not test enablement.
  if ( anyEvolutionRunning() )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, "Load Robot...", QString(), "Compiled Robot Files (*.crb);;All Files (*)" );
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
	      QMessageBox::warning( this, "Robot import error!", e.getMessage() );
	      gpExperiment.robot.clear();
	    }
	}
      file.close();
      // perhaps it is enough to update the language parameter screen and the robot view
      this->getAllOutOfExperiment();
    }
};

void SIG_Experiment::slotRobotSave()
{
  QString fileName = QFileDialog::getSaveFileName( nullptr, "Save Robot...", QString(), "Cooked Robot Files (*.crb);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "crb" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.fileName() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(QIODevice::WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.robot.writeToFileTransfer( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(QIODevice::WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.robot.writeToFileTransfer( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotRobotInfo()
{ char robInf[4096];


  // need DynaMechs for that..
  if (gpExperiment.simulationParameter.getSimulationLibrary() != SIGEL_Simulation::SIG_SimulationParameters::DynaMechs)
  { QMessageBox::information( 0, "Can't display Robot Information..", "<B>DynaMechs required for this operation to function properly.</B>");
    return;
  }

  // get info and display
  if( (gpExperiment.robot.getBodies().size() != 0))
  { gpExperiment.robot.getRobotInformation(robInf, 4096);
    QMessageBox::information( 0, "Robot Information", robInf );
  }
  else
  { QMessageBox::information( 0, "Can't display Robot Information..", "<B>Erm..<BR><BR></B>maybe you should *load* a robot first ?");
  }
}

void SIG_Experiment::slotEvolutionStopped()
{
  emit signalEvolutionNotRunning( true );
  experimentView->pushbuttonStart->setEnabled( true );
  experimentView->pushbuttonStop->setEnabled( false );

  // enable the widgets
  gpParameter->setEnabled( true );
  simulationParameter->setEnabled( true );
  robotView->setEnabled( true );
  languageParameters->setEnabled( true );
  environmentView->setEnabled( true );
  // allIndividualsView->setEnabled( true );

  /*
  if( guiGPManager )
    {
      delete guiGPManager;
      guiGPManager = 0;
    }
  */
};

}

