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
#include <QCoreApplication>
#include <QStyleFactory>
#include <qdir.h>
#include <qtextstream.h>

#include <pvm3.h>
#include <csignal>
#include <cstdio>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "SIGEL_GP/SIG_GPManager.h"
#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_Tools/SIG_Logging.h"

// The headless run, while it evolves. The first SIGINT or SIGTERM stops it
// the way the Stop button does, so it saves; a second one ends it at once.
static SIGEL_GP::SIG_GPManager *headlessManager = nullptr;

extern "C"
{
  // signal handler to be installed from main()
  void sigelStandardSignalHandler(int signal) {
    // The first signal stops the run so that it saves; a second one, while
    // the flag is set, ends a run that no longer reads it.
    if ( (signal == SIGINT || signal == SIGTERM)
         && headlessManager && !headlessManager->userTerminated ) {
      headlessManager->userTerminated = true;
      return;
    }

    int result = 1;

    switch ( signal ) {
      case SIGABRT:
        SIG_LOG_WARNING( "sigel" ) << "Abort";
        break;
      case SIGFPE:
        SIG_LOG_WARNING( "sigel" ) << "Arithmetic error signal";
        break;
      case SIGILL:
        SIG_LOG_WARNING( "sigel" ) << "Invalid execution";
        break;
      case SIGINT:
        SIG_LOG_WARNING( "sigel" ) << "Asynchronous interactive attention";
        break;
      case SIGSEGV:
        SIG_LOG_WARNING( "sigel" ) << "Invalid storage access";
        break;
      case SIGTERM:
        result = 0;
        break;
     }

    pvm_halt();
    exit( result );
  }

  // simple wrapper to call experiment.RegisterDynPVMClients()
  // this C function is launched as a thread
  void MeJustCallingRegisterDynPVMClients(void *inRawGPM) {
    SIGEL_GP::SIG_GPManager *gpm = static_cast<SIGEL_GP::SIG_GPManager *>(inRawGPM);
    gpm->RegisterDynPVMClients();
  }
}

bool guiEnabled = true;

void installSigelStandardSignalHandler() {
  std::signal( SIGABRT, sigelStandardSignalHandler );
  std::signal( SIGFPE, sigelStandardSignalHandler );
  std::signal( SIGILL, sigelStandardSignalHandler );
  std::signal( SIGINT, sigelStandardSignalHandler );
  std::signal( SIGSEGV, sigelStandardSignalHandler );
  std::signal( SIGTERM, sigelStandardSignalHandler );
}

int main( int argc, char *argv[] ) {
  int arg;

  SIGEL_Tools::SIG_Logging::initializeLogging();
  installSigelStandardSignalHandler();

  bool mtEvolve=false;

  // slaves have a priority of 19 when computing to make them behave nice when run in
  // the background using our batchsystem. Since SIGEL is most of the time waiting for
  // the slaves, set priority even lower.
  // (Don't change this ! Prio.19 was chosen for a very special reason..)
  setpriority(PRIO_PROCESS, 0, 20);

  // Start PVM
  int info = pvm_start_pvmd( 0, nullptr, 0 );

  // Register to PVM
  int myTaskId=pvm_mytid();

  bool dynClients = false;

  // Parse the argument line
  if(argc >= 2) {
    QString option( argv[1] );

    // start evolution w/o GUI
    if ( (option == "-evolve") || (option == "-e") ) {
      guiEnabled = false;
    }
    // start evolution w. dynamic number of clients
    else if( (option == "-devolve") || (option == "-de") ) {
      guiEnabled = false;
      dynClients = true;
    }
	// just start the meta evolution wo SIGEL and wo GUI
	else if( (option == "-mtevolve") || (option == "-me") ) {
		guiEnabled = false;
		mtEvolve = true;
	}
    // start SIGEL with GUI
    else {
      guiEnabled = true;
      printf("Options:\n\n\t-devolve, -de\t.....\tEvolve with dynamic clients\n\t-evolve, -e\t.....\tEvolve without GUI\n");
    }
  }

  // Start SIGEL with GUI
  if ( guiEnabled ) {
    QApplication app( argc, argv );
    app.setStyle( QStyleFactory::create( "Fusion" ) );

    SIGEL_MasterGUI::SIG_MainWindow *mainWindow = new SIGEL_MasterGUI::SIG_MainWindow( nullptr );
    mainWindow->show();

    int result = app.exec();

    pvm_halt();

    return result;
  }
  // Start SIGEL w/o GUI to evolve the given experiment
  else {
    SIG_LOG_INFO( "sigel" ) << "Master is used to evolve.";

    if (argc < 3) {
      SIG_LOG_WARNING( "sigel" ) << "No experiment name supplied! Cannot start evolution loop!";
      pvm_halt();
      return 1;
    }

    QString experimentName( argv[2] );
    QFile experimentFile( experimentName );
    if (!experimentFile.open( QIODevice::ReadOnly )) {
      SIG_LOG_WARNING( "sigel" ) << "Error opening " << experimentName << "!";
      pvm_halt();
      return 1;
    }

    // The trainer's destructor calls pvm_delhosts, so the experiment and the manager end before pvm_halt().
    try {
      SIGEL_GP::SIG_GPMetaGPExperiment experiment;

      experiment.experimentName = experimentName;
      // sets the autosave path
      experiment.setPath(experimentName);
      QTextStream experimentLoadStream( &experimentFile );
      experiment.loadExperiment( experimentLoadStream );
      experimentFile.close();

      SIGEL_GP::SIG_GPManager gpManager( experiment );

      // launch the server thread, thus enabling the
      // clients to register all the time while we're running
      if (dynClients) {
        pthread_t serv_thread;
        // The function returns void, not void *; nothing reads the thread's result.
        pthread_create(&serv_thread, nullptr, reinterpret_cast<void *(*)(void *)>(&MeJustCallingRegisterDynPVMClients), &gpManager);
      }

	if(mtEvolve){
		// This path has no GUI. It uses QCoreApplication, so no QWidget may be
		// created here.
		QCoreApplication app( argc, argv );

		// start just the meta evolution (w/o sigel)
		if(argc < 4)
		{
			SIG_LOG_WARNING( "sigel" ) << "No time limit supplied. Can't start meta evolution. You have to supply the time limit in minutes after the experiment filename.";
			pvm_halt();
			return 1;
		}
		int minutes = QString(argv[3]).toInt();
		if(minutes < 0 || minutes > 34560)
		{
			SIG_LOG_WARNING( "sigel" ) << "Minutes must be in the interval [0-34560].";
			pvm_halt();
			return 1;
		}

		experiment.mtController->startTimedEvolution(minutes);

		app.exec();	// enter event-loop
	} else {
		// start() runs the evolution on this thread and returns when it is done
		headlessManager = &gpManager;
		gpManager.start();
	}

      // A signal during the save would lose the run.
      std::signal( SIGINT, SIG_IGN );
      std::signal( SIGTERM, SIG_IGN );

      if (!experiment.saveToFile( experimentName )) {
        SIG_LOG_WARNING( "sigel" ) << "Error saving " << experimentName << "!";
        pvm_halt();
        return 1;
      }
    }
    catch (SIGEL_Tools::SIG_Exception &e) {
      // The manager no longer exists; the signal handler must not use it.
      headlessManager = nullptr;
      SIG_LOG_WARNING( "sigel" ) << "Error in " << experimentName << ": " << e.getMessage().trimmed();
      pvm_halt();
      return 1;
    }

    pvm_halt();

    return 0;
  }
}
