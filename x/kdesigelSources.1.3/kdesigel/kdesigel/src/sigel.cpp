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
#ifdef _WINDOWS
#include <qwindowsstyle.h>
#include <qdir.h>
#else
#include <qmotifplusstyle.h>
#endif

#include <pvm3.h>
#include <csignal>
#ifndef _WINDOWS
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "SIGEL_GP/SIG_GPManager.h"
#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_Tools/SIG_IO.h"


extern "C"
{

  // signal handler to be installed from main()
  void sigelStandardSignalHandler(int signal) {
    int result = 1;

    switch ( signal ) {
      case SIGABRT:
        SIGEL_Tools::SIG_IO::cerr << "Abort\n";
        break;
      case SIGFPE:
        SIGEL_Tools::SIG_IO::cerr << "Arithmetic error signal\n";
        break;
      case SIGILL:
        SIGEL_Tools::SIG_IO::cerr << "Invalid execution\n";
        break;
      case SIGINT:
        SIGEL_Tools::SIG_IO::cerr << "Asynchronous interactive attention\n";
        break;
      case SIGSEGV:
        SIGEL_Tools::SIG_IO::cerr << "Invalid storage access\n";
        break;
      case SIGTERM:
        // SIGEL_Tools::SIG_IO::cerr << "Asynchronous termination request\n";
        result = 0;
        break;
     }

    pvm_halt();
    exit( result );
  }


  // simple wrapper to call experiment.RegisterDynPVMClients()
  // this C function is launched as a thread
#ifdef _WINDOWS
  DWORD WINAPI MeJustCallingRegisterDynPVMClients(LPVOID inRawGPM) {
    SIGEL_GP::SIG_GPManager *gpm = (SIGEL_GP::SIG_GPManager *)inRawGPM;
    gpm->RegisterDynPVMClients();
    return 0;
  }
#else
  void MeJustCallingRegisterDynPVMClients(void *inRawGPM) {
    SIGEL_GP::SIG_GPManager *gpm = (SIGEL_GP::SIG_GPManager *)inRawGPM;
    gpm->RegisterDynPVMClients();
  }
#endif

}

bool guiEnabled = TRUE;


int main( int argc, char *argv[] ) {
  int arg;

  // Install the sigel standard signal handler
#ifdef _WINDOWS
  ::signal( SIGABRT, sigelStandardSignalHandler );
  ::signal( SIGFPE, sigelStandardSignalHandler );
  ::signal( SIGILL, sigelStandardSignalHandler );
  ::signal( SIGINT, sigelStandardSignalHandler );
  ::signal( SIGSEGV, sigelStandardSignalHandler );
  ::signal( SIGTERM, sigelStandardSignalHandler );
  HANDLE serv_thread;
#else
  std::signal( SIGABRT, sigelStandardSignalHandler );
  std::signal( SIGFPE, sigelStandardSignalHandler );
  std::signal( SIGILL, sigelStandardSignalHandler );
  std::signal( SIGINT, sigelStandardSignalHandler );
  std::signal( SIGSEGV, sigelStandardSignalHandler );
  std::signal( SIGTERM, sigelStandardSignalHandler );
#endif

  bool mtEvolve=FALSE;

  // slaves have a priority of 19 when computing to make them behave nice when run in
  // the background using our batchsystem. Since SIGEL is most of the time waiting for
  // the slaves, set priority even lower.
  // (Don't change this ! Prio.19 was choosen for a very special reason..)
#ifdef _WINDOWS
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#else
  setpriority(PRIO_PROCESS, 0, 20);
#endif

  // Start PVM
#ifdef _WINDOWS
  // The Windows version of PVM has trouble with paths
  // on the client hosts. So you have to specify the
  // search paths of windows clients in a hostfile.

  // The following code searches for the hostfile in
  // the SIGEL_ROOT dir and starts PVM-daemon with
  // this hostfile as argument.
  int info=-1;
  QDir dir(::getenv("SIGEL_ROOT"));
  bool useHostFile = dir.exists("hostfile", false);

  if (useHostFile) { // hostfile found
    QString fPath = dir.absFilePath("hostfile");
    char *cfPath  = new char[fPath.length()+1];
    strcpy(cfPath, fPath);
    info = pvm_start_pvmd( 1, &cfPath, 0 );	// start PVM daemon with the hostfile as argument
    delete[] cfPath;
  }
  else { // hostfile not found
    info = pvm_start_pvmd( 0, 0, 0 );  // start PVM daemon normally
  }
#else
  int info = pvm_start_pvmd( 0, 0, 0 );
#endif

  // Register to PVM
  int myTaskId=pvm_mytid();

//  bool guiEnabled = TRUE;
  bool dynClients = FALSE;

  // Parse the argument line
  if(argc >= 2) {
    QString option( argv[1] );

    // start evolution w/o GUI
    if ( (option == "-evolve") || (option == "-e") ) {
      guiEnabled = FALSE;
    }
    // start evolution w. dynamic number of clients
    else if( (option == "-devolve") || (option == "-de") ) {
      guiEnabled = FALSE;
      dynClients = TRUE;
    }
	// just start the meta evolution wo SIGEL and wo GUI
	else if( (option == "-mtevolve") || (option == "-me") ) {
		guiEnabled = FALSE;
		mtEvolve = TRUE;
	}
    // start SIGEL with GUI
    else {
      guiEnabled = TRUE;
      printf("Options:\n\n\t-devolve, -de\t.....\tEvolve with dynamic clients\n\t-evolve, -e\t.....\tEvolve without GUI\n");
    }
  }

  // Start SIGEL with GUI
  if ( guiEnabled ) {
    QApplication app( argc, argv, true );
#ifdef _WINDOWS
    app.setStyle( new QWindowsStyle() );
#else		
    app.setStyle( new QMotifPlusStyle() );
#endif

    SIGEL_MasterGUI::SIG_MainWindow *mainWindow = new SIGEL_MasterGUI::SIG_MainWindow( 0 , "MainWindow" );
    app.setMainWidget( mainWindow );
    mainWindow->show();

    int result = app.exec();

    pvm_halt();

    return result;
  }
  // Start SIGEL w/o GUI to evolve the given experiment
  else {
#ifdef _WINDOWS
    SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif
    SIGEL_Tools::SIG_IO::cerr << "Master is used to evolve.\n";

    if (argc < 3) {
      SIGEL_Tools::SIG_IO::cerr << "No experiment name supplied! Cannot start evolution loop!\n";
      pvm_halt();
      return 1;
    }

    QString experimentName( argv[2] );
    QFile experimentFile( experimentName );
    if (!experimentFile.open( IO_ReadOnly )) {
      SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentName << "!\n";
      pvm_halt();
      return 1;
    }

    SIGEL_GP::SIG_GPExperiment experiment;

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
#ifdef _WINDOWS
      serv_thread = CreateThread( NULL, 0, &MeJustCallingRegisterDynPVMClients, &gpManager, 0, 0 );
#else
      pthread_t serv_thread;
      pthread_create(&serv_thread, NULL,(void *) &MeJustCallingRegisterDynPVMClients,(void *) &gpManager);


#endif
    }

	if(mtEvolve){
	
		QApplication app( argc, argv, false );

		// start just the meta evolution (w/o sigel)
		if(argc < 4)
		{
			SIGEL_Tools::SIG_IO::cerr << "No time limit supplied. Can't start meta evolution.\nYou have to supply the time limit in minutes after the experiment filename.\n";
			pvm_halt();
			return 1;
		}
		int minutes = QString(argv[3]).toInt();
		if(minutes < 0 || minutes > 34560)
		{
			SIGEL_Tools::SIG_IO::cerr << "Minutes must be in the interval [0-34560].\n";
			pvm_halt();
			return 1;
		}

		experiment.mtController->startTimedEvolution(minutes);

		app.exec();	// enter event-loop

	} else {

		// start sigel evolution
		gpManager.start();
		gpManager.wait();
	}

    if (!experimentFile.open( IO_WriteOnly )) {
      SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentName << "!\n";
      pvm_halt();
      return 1;
    }

    QTextStream experimentSaveStream( &experimentFile );
    experiment.saveExperiment( experimentSaveStream );
    experimentFile.close();

#ifdef _WINDOWS
    if(dynClients){
      CloseHandle( serv_thread );
    }
#endif
    pvm_halt();

    return 0;
  }
}
