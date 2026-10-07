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
#include <QStyleFactory>
#include <qstring.h>
#include <qtextstream.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <csignal>
#include <memory>

#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_SlaveGUI/SIG_SimulationWindow.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_GP/SIG_GPPVMData.h"
#include "SIGEL_GP/SIG_GPFitnessFunction.h"
#include "SIGEL_GP/SIG_GPExperiment.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"
#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"

#include <pvm3.h>

using namespace SIGEL_SlaveGUI;
using namespace SIGEL_GP;

extern "C"
{
  int masterTaskId = 0;

  void sendFitnessToMaster( double fitnessValue ) {
    pvm_initsend( PvmDataDefault );
    pvm_pkdouble( &fitnessValue, 1, 1 );
    pvm_send( masterTaskId, 5 );
  }

  void sigelStandardSignalHandler(int signal) {

    int result = 1;

    switch ( signal ) {
      case SIGABRT:
        SIGEL_Tools::SIG_IO::cerr << "Abort" << Qt::endl;
        break;
      case SIGFPE:
        SIGEL_Tools::SIG_IO::cerr << "Arithmetic error signal" << Qt::endl;
        break;
      case SIGILL:
        SIGEL_Tools::SIG_IO::cerr << "Invalid execution" << Qt::endl;
        break;
      case SIGINT:
        SIGEL_Tools::SIG_IO::cerr << "Asynchronous interactive attention" << Qt::endl;
        break;
      case SIGSEGV:
        SIGEL_Tools::SIG_IO::cerr << "Invalid storage access" << Qt::endl;
        break;
      case SIGTERM:
        result = 0;
        break;
    };

    sendFitnessToMaster( 0 );

    pvm_exit();

    exit( result );
  };
};

bool guiEnabled = false;

void installSigelStandardSignalHandler() {
  std::signal( SIGABRT, sigelStandardSignalHandler );
  std::signal( SIGFPE, sigelStandardSignalHandler );
  std::signal( SIGILL, sigelStandardSignalHandler );
  std::signal( SIGINT, sigelStandardSignalHandler );
  std::signal( SIGSEGV, sigelStandardSignalHandler );
  std::signal( SIGTERM, sigelStandardSignalHandler );
}

// The name of the fitness function; the ID itself for an unknown ID.
QString nameOfFitnessFunction( QString const &serializedId ) {
  const std::optional<int> fitnessIndex = SIGEL_GP::SIG_GPFitnessFunctionRegistry::indexOf( serializedId );
  if (!fitnessIndex)
    return serializedId;

  const QList<const SIGEL_GP::SIG_GPFitnessFunction *> &fitnessFunctions = SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions();
  const SIGEL_GP::SIG_GPFitnessFunction *fitnessFunction = fitnessFunctions[*fitnessIndex];

  return fitnessFunction->name();
}

int showSimulation( SIGEL_Robot::SIG_Robot const &robot,
                    SIGEL_Environment::SIG_Environment const &environment,
                    SIGEL_Simulation::SIG_SimulationParameters const &simulationParameters,
                    SIGEL_Program::SIG_Program const &program,
                    SIG_MovieStaticRunInfo const &staticRunInfo ) {
  SIG_SimulationWindow simWindow(nullptr, "simWindow");

  simWindow.setWindowTitle("Simulation Visualisation");
  simWindow.setStaticRunInfo( staticRunInfo );
  simWindow.show();

  try {
    simWindow.visualizeThis( robot, environment, simulationParameters, program );
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    return 1;
  }

  return QApplication::exec();
}

int showFirstIndividualOfExperimentFile( int argc, char *argv[], QString const &experimentFileName ) {
  QFile experimentFile( experimentFileName );
  if (!experimentFile.open( QIODevice::ReadOnly )) {
    SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentFileName << "!" << Qt::endl;
    pvm_halt();
    return 1;
  }

  SIGEL_GP::SIG_GPExperiment experiment;

  QTextStream experimentStream( &experimentFile );

  try {
    experiment.loadExperiment( experimentStream );
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    return 1;
  }

  experimentFile.close();

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Slave is used to visualize!" << Qt::endl;
#endif

  SIGEL_Robot::SIG_Robot modifiedRobot( experiment.robot );

  try {
    modifiedRobot.prepareDynaMechs();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    return 1;
  }

  QApplication a(argc, argv);
  QApplication::setStyle( QStyleFactory::create( "Fusion" ) );

  SIGEL_GP::SIG_GPIndividual &individual = experiment.population.getIndividual( 0 );
  SIGEL_Program::SIG_Program &program = individual.getProgramVar();

  SIG_MovieStaticRunInfo staticRunInfo;
  staticRunInfo.setExperimentFileName( experimentFileName );
  staticRunInfo.individualName = individual.getName();
  staticRunInfo.individualFitness = individual.getFitness();
  staticRunInfo.individualProgramLength = program.getProgramLength();
  staticRunInfo.fitnessFunctionName = nameOfFitnessFunction( experiment.gpParameter.getFitnessName() );

  return showSimulation( modifiedRobot, experiment.environment, experiment.simulationParameter, program, staticRunInfo );
}

double computeFitness( int argc, char *argv[],
                       QString const &serializedId,
                       SIGEL_Program::SIG_Program &program,
                       SIGEL_Robot::SIG_Robot &robot,
                       SIGEL_Environment::SIG_Environment &environment,
                       SIGEL_Simulation::SIG_SimulationParameters &simulationParameters ) {
  const std::optional<int> fitnessIndex = SIGEL_GP::SIG_GPFitnessFunctionRegistry::indexOf( serializedId );
  if (!fitnessIndex) {
    SIGEL_Tools::SIG_IO::cerr << "Unknown fitness function \"" << serializedId << "\"; its fitness is 0." << Qt::endl;
    return 0;
  }

  // Remote ZORC needs a QApplication for its dialog
  const SIGEL_GP::SIG_GPRemoteZORCFitnessFunction remoteZORC;
  std::unique_ptr< QApplication > app;
  if (serializedId == remoteZORC.serializedId()) {
    app = std::make_unique< QApplication >(argc, argv);
    app->setStyle( QStyleFactory::create( "Fusion" ) );
  }

  const QList<const SIGEL_GP::SIG_GPFitnessFunction *> &fitnessFunctions = SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions();
  const SIGEL_GP::SIG_GPFitnessFunction *fitnessFunction = fitnessFunctions[*fitnessIndex];

  try {
    return fitnessFunction->evalFitness( program, robot, environment, simulationParameters );
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    return 0;
  }
}

int showIndividualFromPVM( int argc, char *argv[],
                           SIGEL_GP::SIG_GPPVMData &pvmData,
                           SIGEL_Program::SIG_Program &program,
                           SIGEL_Robot::SIG_Robot &robot,
                           SIGEL_Environment::SIG_Environment &environment,
                           SIGEL_Simulation::SIG_SimulationParameters &simulationParameters ) {
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Slave is used to visualize!" << Qt::endl;
#endif
  QApplication a(argc, argv);
  QApplication::setStyle( QStyleFactory::create( "Fusion" ) );

  SIG_MovieStaticRunInfo staticRunInfo;
  staticRunInfo.setExperimentFileName( pvmData.getExperimentName() );
  staticRunInfo.individualName = pvmData.getIndividualName();
  staticRunInfo.individualFitness = pvmData.getIndividualFitness();
  staticRunInfo.individualProgramLength = program.getProgramLength();
  staticRunInfo.fitnessFunctionName = nameOfFitnessFunction( pvmData.getFitnessFunctionName() );

  // if we use the RemoteZORC-Fitnessfunction: run evaluation to transmit the program !
  const SIGEL_GP::SIG_GPRemoteZORCFitnessFunction remoteZORC;
  if (pvmData.getFitnessFunctionName() == remoteZORC.serializedId())
    remoteZORC.evalFitness( program, robot, environment, simulationParameters );

  return showSimulation( robot, environment, simulationParameters, program, staticRunInfo );
}

int runPVMJob( int argc, char *argv[] ) {
  // evolvers must be nice to other concurrently running programs
  constexpr int lowestPriority = 19;
  setpriority(PRIO_PROCESS, 0, lowestPriority);

  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;
  SIGEL_Program::SIG_Program program;

  //Register to PVM
  int myTaskID = pvm_mytid();
  masterTaskId = pvm_parent();

  if ((masterTaskId==PvmSysErr) || (masterTaskId==PvmNoParent)) {
    SIGEL_Tools::SIG_IO::cerr << "Program hasn't been started as a PVM slave!" << Qt::endl;
    exit(1);
  }

  SIGEL_GP::SIG_GPPVMData pvmData( robot,
         environment,
         simulationParameters,
         "",
         false );

  QString pvmDataString = pvmData.getQStringFromPVM( masterTaskId, 23 );

  QTextStream pvmDataStream( &pvmDataString, QIODeviceBase::ReadWrite );

  try {
    pvmData.loadPVMDataTransfer( pvmDataStream, program );
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    return 1;
  }

  if ( pvmData.getVisualize() ) {
    int returnValue = showIndividualFromPVM( argc, argv, pvmData, program, robot, environment, simulationParameters );

    pvm_exit();

    return returnValue;
  }

  // launched to compute !! Just evaluate fitness, no window-stuff.
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Slave is used to calculate a fitness!" << Qt::endl;
#endif

  double fitnessValue = computeFitness( argc, argv, pvmData.getFitnessFunctionName(), program, robot, environment, simulationParameters );

  sendFitnessToMaster( fitnessValue );

  pvm_exit();

  return 0;
}

int main( int argc, char *argv[] ) {
  installSigelStandardSignalHandler();

  bool standAlone = false;

  // parse command line arguments
  if (argc >= 2) {
  	QString option( argv[1] );
		if ((option == "-visualize") || (option == "-v"))
			standAlone = true;
    	else {
			SIGEL_Tools::SIG_IO::cerr << "Options:\n\t-visualize, -v\t...\tstart in visualize mode\n" << Qt::endl;
			standAlone = false;
	  	}
 	}

  // MODE:  StandAlone, just run the slave to visualize robot+program
  if (standAlone) {
    SIGEL_Tools::SIG_IO::cerr << "Slave started manually to visualize!" << Qt::endl;

    if (argc < 3) {
      SIGEL_Tools::SIG_IO::cerr << "No experiment name supplied! Cannot visualize!" << Qt::endl;
      return 1;
    }

    return showFirstIndividualOfExperimentFile( argc, argv, QString( argv[2] ) );
  } // MODE:  StandAlone (if end)

  return runPVMJob( argc, argv );
}
