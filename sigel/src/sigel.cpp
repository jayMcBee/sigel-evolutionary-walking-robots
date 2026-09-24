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
#include <QHashSeed>
// Qt 2 forced a style on both platforms. Qt 6 ships only "Windows" and
// "Fusion" -- QMotifPlusStyle does not exist and cannot be reproduced, so the
// X11 branch takes Fusion, the closest cross-platform equivalent.
#include <QStyleFactory>
#include <qdir.h>

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

bool guiEnabled = true;


int main( int argc, char *argv[] ) {
  // Qt 6 randomises QHash iteration order per process unless the seed is
  // pinned, and `sigel' links THREE QHashes -- SIG_GUIGPExperiment's widgetDict and
  // menuDict, and SIG_ExperimentListView's experimentDict. ~SIG_GUIGPExperiment
  // iterates widgetDict and calls widgetStack->removeWidget() on each, and that
  // stack is SHARED -- SIG_ExperimentListView hands its own widgetStack to every
  // experiment it constructs. So with a second experiment still loaded, the
  // order in which one experiment's widgets leave the stack can decide which
  // page is current afterwards. Pinning the seed makes that the same on every
  // run. No hashed container's order reaches a file.
  //
  // Not needed in sigel_slave.cpp: it links GUI_SLAVE, not SIGEL_MasterGUI, and
  // has no QHash at all.
  QHashSeed::setDeterministicGlobalSeed();

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

  bool mtEvolve=false;

  // slaves have a priority of 19 when computing to make them behave nice when run in
  // the background using our batchsystem. Since SIGEL is most of the time waiting for
  // the slaves, set priority even lower.
  // (Don't change this ! Prio.19 was chosen for a very special reason..)
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
  bool useHostFile = dir.exists("hostfile");

  if (useHostFile) { // hostfile found
    QString fPath = dir.absoluteFilePath("hostfile");
    char *cfPath  = new char[fPath.length()+1];
    strcpy(cfPath, fPath.toLatin1().constData());
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

//  bool guiEnabled = true;
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
#ifdef _WINDOWS
    app.setStyle( QStyleFactory::create( "Windows" ) );
#else		
    app.setStyle( QStyleFactory::create( "Fusion" ) );
#endif

    SIGEL_MasterGUI::SIG_MainWindow *mainWindow = new SIGEL_MasterGUI::SIG_MainWindow( 0 , "MainWindow" );
    // Qt 2's setMainWidget() marked the widget whose closing ends the app and
    // applied the X11 -geometry / -title command-line options to it
    // (qapplication_x11.cpp:1846). Qt 6 has no such call: quitOnLastWindowClosed
    // defaults to true, which covers the first half; -geometry and -title are
    // gone from Qt entirely and cannot be reproduced.
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
    SIGEL_Tools::SIG_IO::cerr << "Master is used to evolve." << Qt::endl;

    if (argc < 3) {
      SIGEL_Tools::SIG_IO::cerr << "No experiment name supplied! Cannot start evolution loop!" << Qt::endl;
      pvm_halt();
      return 1;
    }

    QString experimentName( argv[2] );
    QFile experimentFile( experimentName );
    if (!experimentFile.open( QIODevice::ReadOnly )) {
      SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentName << "!" << Qt::endl;
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
      // 1.3 cast the thread function to (void *), which pthread_create takes as
      // void *(*)(void *). Older compilers let that through; C++17 does not, so
      // the cast is spelled out. The function still returns nothing and the
      // return value is still never read, exactly as before.
      pthread_create(&serv_thread, NULL,(void *(*)(void *)) &MeJustCallingRegisterDynPVMClients,(void *) &gpManager);


#endif
    }

	if(mtEvolve){
	
		// Qt 2's third argument was GUIenabled: this branch ran with NO GUI
		// connection at all. QCoreApplication is that in Qt 6, and it keeps the
		// same restriction -- no QWidget may be created on this path.
		QCoreApplication app( argc, argv );

		// start just the meta evolution (w/o sigel)
		if(argc < 4)
		{
			SIGEL_Tools::SIG_IO::cerr << "No time limit supplied. Can't start meta evolution.\nYou have to supply the time limit in minutes after the experiment filename." << Qt::endl;
			pvm_halt();
			return 1;
		}
		int minutes = QString(argv[3]).toInt();
		if(minutes < 0 || minutes > 34560)
		{
			SIGEL_Tools::SIG_IO::cerr << "Minutes must be in the interval [0-34560]." << Qt::endl;
			pvm_halt();
			return 1;
		}

		experiment.mtController->startTimedEvolution(minutes);

		app.exec();	// enter event-loop

	} else {

		// start() runs the evolution on this thread and returns when it is done
		gpManager.start();
	}

    if (!experimentFile.open( QIODevice::WriteOnly )) {
      SIGEL_Tools::SIG_IO::cerr << "Error opening " << experimentName << "!" << Qt::endl;
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
