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
// Qt 2 forced a style on both platforms. Qt 6 ships only "Windows" and
// "Fusion" -- QMotifPlusStyle does not exist and cannot be reproduced, so the
// X11 branch takes Fusion, the closest cross-platform equivalent.
#include <QStyleFactory>
#include <qstring.h>
#include <qtextstream.h>

#ifndef _WINDOWS
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <csignal>

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

#include "SIGEL_GP/SIG_GPSimpleFitnessFunction.h"
#include "SIGEL_GP/SIG_GPRealSpeedFitnessFunction.h"
#include "SIGEL_GP/SIG_GPNiceWalkingFitnessFunction.h"
#include "SIGEL_GP/SIG_GPAdaptiveWalkingFitnessFunction.h"
#include "SIGEL_GP/SIG_GPForceFitnessFunction.h"
#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"

#include <pvm3.h>

using namespace SIGEL_SlaveGUI;
using namespace SIGEL_GP;

extern "C"
{
  int masterTaskId = 0;

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
#include <qapplication.h>
      case SIGTERM:
        result = 0;
        break;
    };

    double fitnessValue = 0;

    pvm_initsend( PvmDataDefault );
    pvm_pkdouble( &fitnessValue, 1, 1 );
    pvm_send( masterTaskId, 5 );

    pvm_exit();

    exit( result );
  };
};

bool guiEnabled = false;

int main( int argc, char *argv[] ) {
  // Install the sigel standard signal handler
#ifdef _WINDOWS
  ::signal( SIGABRT, sigelStandardSignalHandler );
  ::signal( SIGFPE, sigelStandardSignalHandler );
  ::signal( SIGILL, sigelStandardSignalHandler );
  ::signal( SIGINT, sigelStandardSignalHandler );
  ::signal( SIGSEGV, sigelStandardSignalHandler );
  ::signal( SIGTERM, sigelStandardSignalHandler );
#else
  std::signal( SIGABRT, sigelStandardSignalHandler );
  std::signal( SIGFPE, sigelStandardSignalHandler );
  std::signal( SIGILL, sigelStandardSignalHandler );
  std::signal( SIGINT, sigelStandardSignalHandler );
  std::signal( SIGSEGV, sigelStandardSignalHandler );
  std::signal( SIGTERM, sigelStandardSignalHandler );
#endif

  SIGEL_Robot::SIG_Robot *robot = 0;
  SIGEL_Environment::SIG_Environment *environment = 0;
  SIGEL_Simulation::SIG_SimulationParameters *simulationParameters = 0;
  SIGEL_Program::SIG_Program *program = 0;
  SIGEL_GP::SIG_GPExperiment *experiment = 0;
  bool visualize = false;
  QString fitnessFunctionName;
  int actGeneration = 0;
  int resetEveryGeneration = 0;

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

    QString experimentName( argv[2] );
    QFile experimentFile( experimentName );
    if (!experimentFile.open( QIODevice::ReadOnly )) {
	  	SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentName << "!" << Qt::endl;
	  	pvm_halt();
	  	return 1;
    }

    experiment = new SIGEL_GP::SIG_GPExperiment();

    QTextStream experimentStream( &experimentFile );

    try {
      experiment->loadExperiment( experimentStream );
    }
    catch (SIGEL_Tools::SIG_Exception &e) {
      SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
      return 1;
    }

    experimentFile.close();

    robot = &experiment->robot;
    environment = &experiment->environment;
    simulationParameters = &experiment->simulationParameter;
    program = &experiment->population.getIndividual( 0 ).getProgramVar();
    visualize = true;

  } // MODE:  StandAlone (if end)

  // MODE:  Evolve, slave was started via PVM to calc. fitness
  else {
    // evolvers must be nice to other concurrently running programs;
    // thus use setpriority OSCall to decrease the priority
#ifdef _WINDOWS
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#else
    setpriority(PRIO_PROCESS, 0, 19);
#endif

    robot = new SIGEL_Robot::SIG_Robot();
    environment = new SIGEL_Environment::SIG_Environment();
    simulationParameters = new SIGEL_Simulation::SIG_SimulationParameters();
    program = new SIGEL_Program::SIG_Program();

      //Register to PVM
    int myTaskID = pvm_mytid();
    masterTaskId = pvm_parent();

    if ((masterTaskId==PvmSysErr) || (masterTaskId==PvmNoParent)) {
      SIGEL_Tools::SIG_IO::cerr << "Program hasn't been started as a PVM slave!" << Qt::endl;
      exit(1);
    }

    SIGEL_GP::SIG_GPPVMData pvmData( *robot,
           *environment,
           *simulationParameters,
           "",
           false );

    QString pvmDataString = pvmData.getQStringFromPVM( masterTaskId, 23 );

    QTextStream pvmDataStream( &pvmDataString, QIODeviceBase::ReadWrite );

    try {
      pvmData.loadPVMDataTransfer( pvmDataStream, *program );
    }
    catch (SIGEL_Tools::SIG_Exception &e) {
      SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
      return 1;
    }

    visualize = pvmData.getVisualize();
    fitnessFunctionName = pvmData.getFitnessFunctionName();
    actGeneration = pvmData.getActGeneration();
    resetEveryGeneration = pvmData.getResetEveryGeneration();
  }

  int returnValue = 0;

  if ( visualize ) {
#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "Slave is used to visualize!" << Qt::endl;
#endif
    // now get the robot data, either via pvm or load from exp file
    SIGEL_Robot::SIG_Robot *modifiedRobot;

    if (standAlone) {
      modifiedRobot = new SIGEL_Robot::SIG_Robot( *robot );

      switch (simulationParameters->getSimulationLibrary()) {
        case SIGEL_Simulation::SIG_SimulationParameters::DynaMo:
          try {
            modifiedRobot->prepareDynaMo();
          }
          catch (SIGEL_Tools::SIG_Exception &e) {
            SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
            return 1;
          };
          break;

        case SIGEL_Simulation::SIG_SimulationParameters::DynaMechs:
          try {
            modifiedRobot->prepareDynaMechs();
          }
          catch (SIGEL_Tools::SIG_Exception &e) {
            SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
            return 1;
          };
          break;
      }
    } // if(standAlone) - condition
    else modifiedRobot = robot;

    QApplication a(argc, argv);

#ifdef _WINDOWS
      QApplication::setStyle( QStyleFactory::create( "Windows" ) );
#else
      QApplication::setStyle( QStyleFactory::create( "Fusion" ) );
#endif		
      SIG_SimulationWindow *simWindow = new SIG_SimulationWindow(0, "simWindow");

      simWindow->setWindowTitle("Simulation Visualisation");
      simWindow->show();

      // if we use the RemoteZORC-Fitnessfunction: run evaluation to transmit the program !
      if (fitnessFunctionName == "RemoteZORCFitnessFunction") {
        SIGEL_GP::SIG_GPFitnessFunction *fitFunc = new SIGEL_GP::SIG_GPRemoteZORCFitnessFunction();
        fitFunc->evalFitness( *program, *robot, *environment, *simulationParameters );
      }

     try {
       simWindow->visualizeThis( *modifiedRobot, *environment, *simulationParameters, *program );
     }
     catch (SIGEL_Tools::SIG_Exception &e) {
       SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
       return 1;
     }

     returnValue = a.exec();

       delete simWindow;

     if ( !standAlone )
       pvm_exit();
     }

    // launched to compute !! Just evaluate fitness, no window-stuff.
    else {
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Slave is used to calculate a fitness!" << Qt::endl;
#endif

      SIGEL_GP::SIG_GPFitnessFunction *fitnessFunction = 0;

		// prepare using the SimpleFitlessFunction
      if (fitnessFunctionName == "SimpleFitnessFunction")
				fitnessFunction = new SIGEL_GP::SIG_GPSimpleFitnessFunction();

		// prepare using the RealSpeedFitnessFunction
      else if (fitnessFunctionName == "RealSpeedFitnessFunction")
				fitnessFunction = new SIGEL_GP::SIG_GPRealSpeedFitnessFunction();

		// prepare using the NiceWalkingFitnessFunction
      else if (fitnessFunctionName == "NiceWalkingFitnessFunction")
				fitnessFunction = new SIGEL_GP::SIG_GPNiceWalkingFitnessFunction();

		// prepare using the AdaptiveWalkingFitnessFunction
      else if (fitnessFunctionName == "ZorcWalkingFitnessFunction")
				fitnessFunction = new SIGEL_GP::SIG_GPAdaptiveWalkingFitnessFunction();

      // prepare for using the ForceFitnessFunction
     else if (fitnessFunctionName == "ForceFitnessFunction")
  			fitnessFunction = new SIGEL_GP::SIG_GPForceFitnessFunction();

		// prepare to use RemoteZORC-Fitnessfunction, need GUI for requesters
		else if (fitnessFunctionName == "RemoteZORCFitnessFunction") {
   				QApplication *app = new QApplication(argc, argv);
#ifdef _WINDOWS
				app->setStyle( QStyleFactory::create( "Windows" ) );
#else					
				app->setStyle( QStyleFactory::create( "Fusion" ) );
#endif			

				fitnessFunction = new SIGEL_GP::SIG_GPRemoteZORCFitnessFunction();
	  	}

		// whoopsie !
      else SIGEL_Tools::SIG_IO::cerr << "Error: Unknown fitness function!" << Qt::endl;

      double fitnessValue = 0;
      fitnessFunction->setActGeneration(actGeneration);
      fitnessFunction->setResetEveryGeneration(resetEveryGeneration);
      if (fitnessFunction) {
        try {
          fitnessValue = fitnessFunction->evalFitness( *program, *robot, *environment, *simulationParameters );
          delete fitnessFunction;
        }
        catch (SIGEL_Tools::SIG_Exception &e) {
          fitnessValue = 0;
        };
      };

      pvm_initsend( PvmDataDefault );
      pvm_pkdouble( &fitnessValue, 1, 1 );
      pvm_send( masterTaskId, 5 );

      pvm_exit();

    };

  return returnValue;

};
