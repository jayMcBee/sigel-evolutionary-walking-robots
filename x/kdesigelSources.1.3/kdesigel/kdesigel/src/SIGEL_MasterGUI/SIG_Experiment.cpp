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

  SIG_Experiment::SIG_Experiment( QString name, QWidgetStack *theWidgetStack, SIG_ExperimentItem *theExperimentItem ) : gpExperiment(), gpManager(0), experimentName(name), widgetStack( theWidgetStack ), experimentItem(theExperimentItem)
{
  // build the gp parameter menu
  menuGPParameter = new QPopupMenu( this );
  menuGPParameter->insertItem( "GPParameter" );

  // build the simulation parameter menu
  menuSimulationParameter = new QPopupMenu( this );
  menuSimulationParameter->insertItem( "Import", this, SLOT( slotSimulationParameterImport() ) );
  menuSimulationParameter->insertItem( "Export", this, SLOT( slotSimulationParameterExport() ) );

  // build the environment view menu
  menuEnvironmentView = new QPopupMenu( this );
  menuEnvironmentView->insertItem( "Import", this, SLOT( slotEnvironmentImport() ) );
  menuEnvironmentView->insertItem( "Export", this, SLOT( slotEnvironmentExport() ) );

  // build the robot view menu
  menuRobotView = new QPopupMenu( this );
  menuRobotView->insertItem( "RobotView" );

  // build experiment view menu
  menuExperimentView = new QPopupMenu( this );
  menuExperimentView->insertItem( "Start", this, SLOT( slotStartEvolution() ) );
  menuExperimentView->insertItem( "Stop", this, SLOT( slotStopEvolution() ) );

  gpParameter = new SIG_GPParameter( this , "GPParameter", 0, gpExperiment );
  simulationParameter = new SIG_SimulationParameter( this, "SimulationParameter", 0, gpExperiment);
  environmentView = new SIG_EnvironmentView( this, "EnvironmentView", 0, gpExperiment );
  robotView = new SIG_RobotView( this, "RobotView",0 , gpExperiment );
  experimentView = new SIG_ExperimentView( this, "ExperimentView", 0, gpExperiment );
  allIndividualsView = new SIG_AllIndividualsView( this, "AllIndividualsView", gpExperiment );
  languageParameters = new SIG_LanguageParameters( this, "LanguageParametersView", 0, gpExperiment );
  
  // put all the widgets on the stack
  widgetStack->addWidget( gpParameter, 0 );
  widgetStack->addWidget( simulationParameter, 0 );
  widgetStack->addWidget( environmentView, 0);
  widgetStack->addWidget( robotView,0 );
  widgetStack->addWidget( experimentView, 0 );
  widgetStack->addWidget( allIndividualsView, 0 );
  widgetStack->addWidget( languageParameters, 0 );

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
  widgetDict.setAutoDelete( true );

  getAllOutOfExperiment();
};

SIG_Experiment::~SIG_Experiment()
{
  // Before destroying the experiment take all widgets in the widgetDict from the widgetStack
  QDictIterator<QWidget> it( widgetDict );
  while ( it.current() )
    {
      widgetStack->removeWidget( it.current() );
      ++it;
    }

   if( gpManager )
      {
        delete gpManager;
      }
};

QString SIG_Experiment::getName() const
{
  return experimentName;
};

void SIG_Experiment::setName( QString newName )
{
  widgetDict.setAutoDelete( false );
  QWidget *theExperimentView = widgetDict[ experimentName ];
  widgetDict.remove( experimentName );
  widgetDict.insert( newName, theExperimentView );
  widgetDict.setAutoDelete( true );
  
  experimentName = newName;
  experimentItem->setText( 0, newName );
};

void SIG_Experiment::putAllIntoExperiment()
{
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
  QPopupMenu *showMenu = menuDict[option];
  QWidget *showWidget = widgetDict[option];
  if( showMenu && showWidget )
    {
      showMenu->popup( thePoint );
      widgetStack->raiseWidget( showWidget );
    }
};

void SIG_Experiment::slotSelectionChanged( QString option )
{
  widgetStack->raiseWidget( widgetDict[option] );
};

void SIG_Experiment::slotStartEvolution()
{
  if( (gpExperiment.robot.getBodyIter().count() != 0) && (gpExperiment.population.getSize() >= 4) && (gpExperiment.gpParameter.getFitnessName() != QString::null) )
    {
      delete gpManager;
      
      gpManager = new SIGEL_GP::SIG_GUIGPManager( *this );
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
      
      gpManager->start();
      
      slotEvolutionStopped();
    }
  else
    {
      QMessageBox::warning( this, "Can't start evolution...", "The evolution cannot be started. There may be several reasons:<ul><li>There is no robot loaded.</li><li>There are less than four individuals in the population</li><li>No fitness function name was specified.</li></ul>");
    }
};

void SIG_Experiment::slotStopEvolution()
{
  emit signalEvolutionNotRunning( true );
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

  gpManager->userTerminated = true;
};

void SIG_Experiment::slotSimulationParameterImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Simulation Parameter Files (*.sip);;All Files (*)", 0, "FileOpenSimulationParameter", "Import Simulation Parameters...");
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "Simulation Parameter Files (*.sip);;All Files (*)", 0, "FileOpenSimulationParameter", "Export Simulation Parameters...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "sip" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.simulationParameter.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.simulationParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotEnvironmentImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Environment Files (*.env);;All Files (*)", 0, "FileImportEnvironment", "Import Environment...");
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "Environment Files (*.env);;All Files (*)", 0, "EnvironmentExportDialog", "Export Environment...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "env" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.environment.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.environment.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotGPParameterImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "GP Parameter Files (*.gpp);;All Files (*)", 0, "FileImportGPParameter", "Import GP Parameter...");
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "GP Parameter Files (*.gpp);;All Files (*)", 0, "GPParameterExportDialog", "Export GP Parameter...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "gpp" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.gpParameter.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.gpParameter.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotLanguageParameterImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Language Parameter Files (*.lap);;All Files (*)", 0, "FileImportLanguageParameter", "Import Language Parameter...");
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "Language Parameter Files (*.lap);;All Files (*)", 0, "FileExportLanguageParameter", "Export Language Parameters...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "lap" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.robot.getLangParam()->writeToFileTransfer( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.robot.getLangParam()->writeToFileTransfer( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotPopulationImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Population Files (*.pop);;All Files (*)", 0, "FileImportPopulation", "Import Population...");
  if ( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "Population files (*.pop);;All Files (*)", 0, "FileExportPopulation", "Export Population...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "pop" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.population.writeToFile( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
	{
	  QTextStream theStream( &file );
	  gpExperiment.population.writeToFile( theStream );
	}
      file.close();
    }
};

void SIG_Experiment::slotRobotImport()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Raw Robot Files (*.rrb);;All Files (*)", 0, "FileImportRobot", "Import Robot...");
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "GNU plot data file (*.dat);;All Files (*)", 0, "FileExportGNUPlot", "Export to GNU plot...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "dat" );
      gpExperiment.exportExperimentHistoryToGNUPlot( fileName );
    }
};

void SIG_Experiment::slotRobotLoad()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Compiled Robot Files (*.crb);;All Files (*)", 0, "FileLoadRobot", "Load Robot...");
  if( !fileName.isEmpty() )
    {
      QFile file( fileName );
      if( file.open(IO_ReadOnly) )
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
  QString fileName = QFileDialog::getSaveFileName( QString::null, "Cooked Robot Files (*.crb);;All Files (*)", 0, "FileSaveRobot", "Save Robot...");
  if( !fileName.isEmpty() )
    {
      fileName = checkEnding( fileName, "crb" );
      QFile file( fileName );
      if( file.exists() )
	switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	  {
	  case QMessageBox::Yes:
	    if( file.open(IO_WriteOnly) )
	      {
		QTextStream theStream( &file );
		gpExperiment.robot.writeToFileTransfer( theStream );
	      }
	    file.close();
	    break;
	  }
      if( file.open(IO_WriteOnly) )
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
  if( (gpExperiment.robot.getBodyIter().count() != 0))
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
  if( gpManager )
    {
      delete gpManager;
      gpManager = 0;
    }
  */
};

}

