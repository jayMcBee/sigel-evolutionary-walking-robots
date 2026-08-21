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
#include "compat/q2compat.h"
#include <pvm3.h>
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_GP::SIG_GPFitnessTrainer::SIG_GPFitnessTrainer(SIGEL_GP::SIG_GPExperiment& exp)
  :exp(exp),
   pvmTasks( exp.population.getSize() ),
   pvmHosts(),
   dynHosts(),
   freshDynHosts(),
   nextHostNumber(0),
   toSpawnList(),
   PVMData( modifiedRobot,
     exp.environment,
     exp.simulationParameter,
     exp.gpParameter.getFitnessName(),
     false),
   modifiedRobot( exp.robot ),
   nextFreeNumber(0)
{
  toSpawnList.setAutoDelete( true );
  pvmTasks.setAutoDelete( true );
  pvmHosts.setAutoDelete( true );

  switch (exp.simulationParameter.getSimulationLibrary()) {
    case SIGEL_Simulation::SIG_SimulationParameters::DynaMo:
      try {
       modifiedRobot.prepareDynaMo();
      }
      catch (SIGEL_Tools::SIG_Exception &e) {
        SIGEL_Tools::SIG_IO::cerr << e.getMessage();
        exit( 1 );
      };
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::DynaMechs:
      try {
        modifiedRobot.prepareDynaMechs();
      }
            catch (SIGEL_Tools::SIG_Exception &e) {
        SIGEL_Tools::SIG_IO::cerr << e.getMessage();
        exit( 1 );
      };
      break;
    };

  for (unsigned int i=0; i<pvmTasks.size(); i++)
    pvmTasks.insert( i, 0 );

  int noOfActiveHosts = 0;
  SIG_GPPVMHost *actHost = exp.gpParameter.getHostList().first();

  while (actHost) {

    if (actHost->enabled)
      noOfActiveHosts++;
      actHost = exp.gpParameter.getHostList().next();
    };

  pvmHosts.resize( noOfActiveHosts );

  int hostCounter = 0;
  actHost = exp.gpParameter.getHostList().first();

  while (actHost) {
    if (actHost->enabled) {
      pvmHosts.insert( hostCounter, new SIG_GPActivePVMHost( *actHost ) );

      Q2CString actHostNameQCString = actHost->name.toUtf8();

      char const *actHostNameCString = actHostNameQCString;

      int singleInfo = 0;

#ifdef _WINDOWS
	  int info = pvm_addhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );
#else
	  int info = pvm_addhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );
#endif

      hostCounter++;
    };

    actHost = exp.gpParameter.getHostList().next();
  };
};

SIGEL_GP::SIG_GPFitnessTrainer::~SIG_GPFitnessTrainer() {
  for (unsigned int i=0; i<pvmHosts.size(); i++) {
      SIG_GPActivePVMHost *actHost = pvmHosts[i];

      Q2CString actHostNameQCString = actHost->name.toUtf8();

      char const *actHostNameCString = actHostNameQCString;

      int singleInfo = 0;

#ifdef _WINDOWS
      int info = pvm_delhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );
#else
      int info = pvm_delhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );
#endif
    };

  // This class owns the entries of both dynamic host lists.
  dynHosts.deleteContents();
  freshDynHosts.deleteContents();
};


void SIGEL_GP::SIG_GPFitnessTrainer::addDynHost(QString newHost) {
  QString buffer;

  // make a new host from scratch using 'newHost' hostname;
  // 1 slave, enabled, in "/tmp/_SIGEL_EVOLUTION_TEMP"
#ifdef _WINDOWS
	buffer = newHost + " 1 1 \".\"\n";
#else
  buffer = newHost + " 1 1 \"/tmp/_SIGEL_EVOLUTION_TEMP\"\n";
#endif

  SIGEL_GP::SIG_GPPVMHost *newPVMHost = new SIGEL_GP::SIG_GPPVMHost( buffer );

  // need to remember that this host is dynamic to delete/free it after evaluation
  dynHosts.append( new QString(newHost) );

  // the freshDynHost list just contains hosts not yet added to the pvmHosts
  // list; this is the job of SIGEL_GP::SIG_GPFitnessTrainer::getNextHost()
  freshDynHosts.append(newPVMHost);
}


void SIGEL_GP::SIG_GPFitnessTrainer::flushAllDynHosts( void ) {
  QString        dHostQstr;
  bool           res;
  int            i,dynDelNum;
  SIG_GPActivePVMHost *pHost;
  char          *cStrName;

  // are there any dynHosts ?
  if (dynHosts.count() > 0) {
    cStrName = new char[256];
    i = pvmHosts.size()-1;
    dynDelNum = 0;

    while (!pvmHosts.isEmpty()) {
      pHost = pvmHosts[i];

      // check next host if it's dynamic !
      res = false;

      for (unsigned int d=0; d<dynHosts.count(); d++) {
        dHostQstr = *dynHosts.at(d);

        if (dHostQstr == pHost->name) {
          res = true;
          sprintf(cStrName, pHost->name.toLatin1().constData());
        }
      }

      // delete the dynamic host from our list and from PVM
      if (res) {
        // for now this variable is not used
        // int singleInfo = 0;
        // int info = pvm_delhosts( &cStrName, 1, &singleInfo );

        pvmHosts.resize( pvmHosts.size()-1 );
        dynDelNum++;
        i--;
      }

      // host wasn't deleted, dynamic hosts are always at the end, that's it
      else {
        break;
      }
    }
    // dynHosts has become obsolete
    dynHosts.deleteContents();
    delete[] cStrName;

    // unfortunately that's it also for our new hosts, else we have a conflict
    // with our server thread cutting _all_ connections, known or unknown to dynHosts
    freshDynHosts.deleteContents();

    SIGEL_Tools::SIG_IO::cerr << "\t(all " << dynDelNum << " dynamic hosts removed | " <<  pvmHosts.size() << " static hosts remaining)\n\n";
  }
}


int SIGEL_GP::SIG_GPFitnessTrainer::spawnTask(SIGEL_GP::SIG_GPIndividual const& ind) {
  int hostNumber = getNextHost();

  int actId = nextFreeNumber;

  bool success = false;

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Trying to spawn task for individual "
			    << ind.getPoolPos()
			    << "\n";
#endif

  int oldSize = pvmTasks.size();
  int oldMaxIndex = oldSize - 1;
  if ( oldMaxIndex < (nextFreeNumber + 1) ) {
      pvmTasks.resize( oldSize + exp.population.getSize() );
      for (unsigned int i=oldSize; i<pvmTasks.size(); i++)
        pvmTasks.insert( i, 0 );
  };

  if (hostNumber != -1) {
      SIG_GPActivePVMHost *usedHost = pvmHosts[ hostNumber ];

      Q2CString hostNameQCString = usedHost->name.toUtf8();
      char const *hostNameCString = hostNameQCString;

      QString executableName;
#ifdef _WINDOWS
		// PVM has problems in finding the executable on windows clients
		// so you have to specify the search path for windows clients in
		// the hostfile. If no path or just a dot is specified, sigel
		// assumes that the client is a windows host and doesn't use a
		// path in the pvm_task call.
		if(usedHost->executableDir.path() == "."){
			executableName = "sigel_slave";
		} else {
   		executableName = usedHost->executableDir.canonicalPath() + "sigel_slave";
  		}
#else
		executableName = usedHost->executableDir.path() + "/sigel_slave";
#endif		
      Q2CString executableNameQCString = executableName.toUtf8();

      char const *executableNameCString = executableNameQCString;

      int taskId = 0;
      int spawnInfo = pvm_spawn( const_cast< char* >( executableNameCString ),
				 0,
				 PvmTaskHost,
				 const_cast< char* >( hostNameCString ),
				 1,
				 &taskId );

      if (spawnInfo == 1) {
        success = true;

        SIG_GPPVMTask *newTask = new SIG_GPPVMTask( *usedHost,
						      actId,
						      taskId,
						      ind.getPoolPos(),
						      QDateTime::currentDateTime() );

        pvmTasks.insert( actId, newTask );

        usedHost->noOfSlaves++;

        QString senderStr;
        QTextStream qts( &senderStr, QIODeviceBase::ReadWrite );
        SIGEL_Program::SIG_Program const &program = ind.getProgram();
        PVMData.setActGeneration(exp.population.getPoolGeneration());
        PVMData.setResetEveryGeneration(exp.gpParameter.getResetEveryGeneration());
        PVMData.savePVMDataTransfer(qts, program);
        PVMData.sendQStringToPVM(senderStr, taskId, 23);
      }
      else  {
        // an error occurred: process cannot been spawned
        QString errorText;

        switch(taskId) {
          case PvmBadParam :
            errorText = QString::asprintf("Invalid parameter in call to pvm_spawn.");
             break;
          case PvmNoHost :
            errorText = QString::asprintf("Host %s is not in the virtual machine.", hostNameCString);
            break;
          case PvmNoFile :
            errorText = QString::asprintf("Executable %s is not found on host %s.",executableNameCString,hostNameCString);
            break;
          case PvmNoMem :
            errorText = QString::asprintf("Malloc failed. Not enough memory on host %s.",hostNameCString);
            break;
          case PvmSysErr :
            errorText = QString::asprintf("pvmd is not responding.");
            break;
          case PvmOutOfRes :
            errorText = QString::asprintf("Out of resources on host %s.",hostNameCString);
            break;
          default:
            errorText = QString::asprintf("Unknown error occurred.");
        };
        //errorText = QString::asprintf("pvm_spawn() failed on %s (%d/%d) %s.",hostNameQCString,spawnInfo,taskId,errorText);
        //QMessageBox warn("Error", errorText ,QMessageBox::Warning, QMessageBox::Retry, QMessageBox::NoButton,QMessageBox::NoButton);
        //warn.exec();
        SIGEL_Tools::SIG_IO::cerr << "pvm_spawn() failed on \"" << hostNameQCString << "\"   (" << spawnInfo << "/" << taskId << ") - " << errorText.toUtf8() << "\n";
      }

  };

  if (!success) {
      Q2Array<int> *toSpawn = new Q2Array<int>(2);
      (*toSpawn)[0] = actId;
      (*toSpawn)[1] = ind.getPoolPos();
      toSpawnList.append( toSpawn );
    };

  nextFreeNumber++;

  return actId;
};


double SIGEL_GP::SIG_GPFitnessTrainer::checkTask(int taskId)
{
  double result = -1;

  SIG_GPPVMTask *pvmTask = pvmTasks[ taskId ];

  if (pvmTask)
  {
		int info = pvm_probe(pvmTask->pvmTaskId, 5);

    if (info != 0)
  	{
	  	pvm_recv(pvmTask->pvmTaskId, 5);
	  	pvm_upkdouble(&result,1,1);

	  	pvmTask->host.noOfSlaves--;
	  	pvmTasks.insert( taskId, 0 );
		}

    else
		{
		  if (exp.gpParameter.getTimeOutMinutes() > 0)
	    {
	      int const timeOutSeconds = exp.gpParameter.getTimeOutMinutes() * 60;

	      QDateTime timeOutTime = pvmTask->spawnTime.addSecs( timeOutSeconds );

	      if (QDateTime::currentDateTime() >= timeOutTime)
				{
		      SIGEL_Tools::SIG_IO::cerr << "sigel_slave timed out -- force quit via 'pvm_kill()'\n";

		  		pvm_kill( pvmTask->pvmTaskId );
		  		pvmTask->host.noOfSlaves--;
		  		Q2Array<int> *toSpawn = new Q2Array<int>(2);
		  		(*toSpawn)[0] = taskId;
		  		(*toSpawn)[1] = pvmTask->indPosition;

		  		toSpawnList.append( toSpawn );

		  		pvmTasks.insert( taskId, 0 );
				}
			}
		}
  }

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Checking task "
			    << taskId
			    << "\n";
#endif

  return result;
};

void SIGEL_GP::SIG_GPFitnessTrainer::stopTrainersSlaves()
{
  int const noOfTries = 100;

  for (int i=0; i < nextFreeNumber; i++) 
    {
      if ( pvmTasks[i] )
	for (int j=0; j<noOfTries; j++)
	  {
	    int info = pvm_kill( pvmTasks[i]->pvmTaskId );

	    if (info >= 0)
	      break;
	  };
    };
};

void SIGEL_GP::SIG_GPFitnessTrainer::sweepToSpawn()
{
  Q2Array< int > *actJob = toSpawnList.first();
  Q2Array< int > *prevJob = 0;

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Sweeping to spawn!\n";
#endif

  while (actJob)
    {
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Entering spawn loop!\n";
#endif

      int hostNumber = getNextHost();

      bool success = false;

      if (hostNumber != -1)
	{
	  SIG_GPActivePVMHost *usedHost = pvmHosts[ hostNumber ];

	  Q2CString usedHostNameQCString = usedHost->name.toUtf8();
	  char const *usedHostNameCString = usedHostNameQCString;

	  QString executableName;
#ifdef _WINDOWS	
		if(usedHost->executableDir.path() == "."){
			executableName = "sigel_slave";
		} else {
			executableName = usedHost->executableDir.canonicalPath() + "/sigel_slave";
		}
#else
	  executableName = usedHost->executableDir.path() + "/sigel_slave";
#endif	
	  Q2CString executableNameQCString = executableName.toUtf8();

	  char const *executableNameCString = executableNameQCString;

	  int taskId = 0;
	  int spawnInfo = pvm_spawn( const_cast< char* >( executableNameCString ),
				     static_cast< char** >(0),
				     PvmTaskHost,
				     const_cast< char* >( usedHostNameCString ),
				     1,
				     &taskId );

	  if (spawnInfo == 1)
	    {
	      success = true;

	      int internalId = (*actJob)[0];
	      int individualNumber = (*actJob)[1];

	      SIG_GPIndividual &ind = exp.population.getIndividual( individualNumber );

	      SIG_GPPVMTask *newTask = new SIG_GPPVMTask( *usedHost,
							  internalId,
							  taskId,
							  individualNumber,
							  QDateTime::currentDateTime() );

	      pvmTasks.insert( internalId, newTask );

	      usedHost->noOfSlaves++;

	      QString senderStr;
	      QTextStream qts( &senderStr, QIODeviceBase::ReadWrite );
	      SIGEL_Program::SIG_Program const &program = ind.getProgram();
         PVMData.setActGeneration(exp.population.getPoolGeneration());
         PVMData.setResetEveryGeneration(exp.gpParameter.getResetEveryGeneration());
	      PVMData.savePVMDataTransfer(qts, program);
	      PVMData.sendQStringToPVM(senderStr, taskId, 23);  
	    };
	};

      if (success)
	{
	  toSpawnList.remove();
	  actJob = toSpawnList.current();
	  if (actJob == prevJob)
	    break;
	}
      else
	{
	  prevJob = actJob;
	  actJob = toSpawnList.next();
	};
    };
};

int SIGEL_GP::SIG_GPFitnessTrainer::getNextHost() {
  int result = -1;
  SIG_GPPVMHost   *freshHost;
  char            *cStrName;
#ifdef _WINDOWS
	HANDLE 			mutex;
#else	
  pthread_mutex_t  mutex;
#endif

  // now we make ourself running exclusively to add all new hosts from the freshDynHosts list
#ifdef _WINDOWS
	mutex = CreateMutex(NULL, false, NULL);	// create a lock if not already created
	WaitForSingleObject(mutex, INFINITE);
#else	
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_lock( &mutex );
#endif

  cStrName = new char[256];

  // add all new dynamic hosts to pvmHosts and declare them to PVM
  for (unsigned int i=0; i<freshDynHosts.count(); i++) {
    // get the next new host to be added to our pvmHost list
    freshHost = freshDynHosts.at(i);

    pvmHosts.resize( pvmHosts.size() + 1 );
    pvmHosts.insert( pvmHosts.size()-1, new SIG_GPActivePVMHost(*freshHost) );

    sprintf(cStrName, "%s", freshHost->name.toLatin1().constData());

    int singleInfo = 0;
    int info = pvm_addhosts(&cStrName , 1, &singleInfo );

    fprintf(stderr, "\to new host added to pvmHosts: \"%s\"\n", cStrName);
  }

  freshDynHosts.deleteContents();
  delete[] cStrName;

#ifdef _WINDOWS
	ReleaseMutex(mutex);		// release the lock
#else
  pthread_mutex_unlock( &mutex );
#endif

  // nextHostNumber might refer to a host that's no longer available !
  if(pvmHosts.size() != 0)
    nextHostNumber = nextHostNumber%pvmHosts.size();

  // this thing seems to check all hosts in our active-host-list whether they
  // have the resources to start another sigel_slave
  for (unsigned int j=0; j<pvmHosts.size(); j++) {
      SIG_GPActivePVMHost *nextHost = pvmHosts[ nextHostNumber ];
      if (nextHost->noOfSlaves < nextHost->maxSlaves)
	   result = nextHostNumber;

      nextHostNumber = ++nextHostNumber % pvmHosts.size();

      if (result != -1)
        break;
  }

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Next Host: "
			    << result
			    << "\n";
#endif

  return result;
};
