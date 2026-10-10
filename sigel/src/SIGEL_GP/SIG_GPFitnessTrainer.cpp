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
#include <QByteArray>
#include <QList>
#include <pvm3.h>
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_GP::SIG_GPFitnessTrainer::SIG_GPFitnessTrainer(SIGEL_GP::SIG_GPExperiment& exp)
  :pvmLost(false),
   exp(exp),
   pvmTasks( exp.population.getSize() ),
   pvmHosts(),
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
  pthread_mutex_init(&dynHostsMutex, nullptr);

  try {
    modifiedRobot.prepareDynaMechs();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
    exit( 1 );
  };

  pvmTasks.fill( nullptr );

  int noOfActiveHosts = 0;
  for (SIG_GPPVMHost *actHost : exp.gpParameter.getHostList())
    if (actHost->enabled)
      noOfActiveHosts++;

  pvmHosts.resize( noOfActiveHosts );

  int hostCounter = 0;

  for (SIG_GPPVMHost *actHost : exp.gpParameter.getHostList()) {
    if (actHost->enabled) {
      delete pvmHosts[ hostCounter ];
      pvmHosts[ hostCounter ] = new SIG_GPActivePVMHost( *actHost );

      const QByteArray actHostNameQCString = actHost->name.toUtf8();

      char const *actHostNameCString = actHostNameQCString.constData();

      int singleInfo = 0;

	  int info = pvm_addhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );

      hostCounter++;
    };
  };
};

SIGEL_GP::SIG_GPFitnessTrainer::~SIG_GPFitnessTrainer() {
  // This class owns its dynamic host list.
  qDeleteAll( freshDynHosts );
  freshDynHosts.clear();

  // setAutoDelete was the only free for both of these.
  qDeleteAll( pvmTasks );
  pvmTasks.clear();

  for (unsigned int i=0; i<pvmHosts.size(); i++) {
      SIG_GPActivePVMHost *actHost = pvmHosts[i];

      const QByteArray actHostNameQCString = actHost->name.toUtf8();

      char const *actHostNameCString = actHostNameQCString.constData();

      int singleInfo = 0;

      int info = pvm_delhosts( const_cast< char** >(&actHostNameCString), 1, &singleInfo );
    };

  // The loop above only tells PVM to drop each host. The objects are freed here.
  qDeleteAll( pvmHosts );
  pvmHosts.clear();
};

void SIGEL_GP::SIG_GPFitnessTrainer::addDynHost(QString newHost) {
  QString buffer;

  // make a new host from scratch using 'newHost' hostname;
  // 1 slave, enabled, in "/tmp/_SIGEL_EVOLUTION_TEMP"
  buffer = newHost + " 1 1 \"/tmp/_SIGEL_EVOLUTION_TEMP\"\n";

  SIGEL_GP::SIG_GPPVMHost *newPVMHost = new SIGEL_GP::SIG_GPPVMHost( buffer );

  pthread_mutex_lock( &dynHostsMutex );

  // the freshDynHost list just contains hosts not yet added to the pvmHosts
  // list; this is the job of SIGEL_GP::SIG_GPFitnessTrainer::getNextHost()
  freshDynHosts.append(newPVMHost);

  pthread_mutex_unlock( &dynHostsMutex );
}

int SIGEL_GP::SIG_GPFitnessTrainer::spawnTask(SIGEL_GP::SIG_GPIndividual const& ind) {
  int hostNumber = getNextHost();

  int actId = nextFreeNumber;

  bool success = false;

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Trying to spawn task for individual \""
			    << ind.getName()
			    << "\"" << Qt::endl;
#endif

  int oldSize = pvmTasks.size();
  int oldMaxIndex = oldSize - 1;
  if ( oldMaxIndex < (nextFreeNumber + 1) ) {
      pvmTasks.resize( oldSize + exp.population.getSize() );
      for (qsizetype i=oldSize; i<pvmTasks.size(); i++)
        pvmTasks[ i ] = nullptr;
  };

  if (hostNumber != -1) {
      SIG_GPActivePVMHost *usedHost = pvmHosts[ hostNumber ];

      const QByteArray hostNameQCString = usedHost->name.toUtf8();
      char const *hostNameCString = hostNameQCString.constData();

      QString executableName;
		// "." asks PVM to find sigel_slave on its own search path.
		if (usedHost->executableDir.path() == ".")
			executableName = "sigel_slave";
		else
			executableName = usedHost->executableDir.path() + "/sigel_slave";
      const QByteArray executableNameQCString = executableName.toUtf8();

      char const *executableNameCString = executableNameQCString.constData();

      int taskId = 0;
      int spawnInfo = pvm_spawn( const_cast< char* >( executableNameCString ),
				 nullptr,
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

        delete pvmTasks[ actId ];
        pvmTasks[ actId ] = newTask;

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

        // pvm_spawn reports a per-task error through taskId and a whole-call
        // error through its return, and an unreachable daemon is the second
        // kind: taskId is left at 0 and only the return says why. Spawning is
        // the first thing that touches the daemon, so this is where a run that
        // can never start finds out.
        if (spawnInfo == PvmSysErr || taskId == PvmSysErr)
          pvmLost = true;

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
        SIGEL_Tools::SIG_IO::cerr << "pvm_spawn() failed for individual \"" << ind.getName() << "\" on \"" << hostNameQCString << "\"   (" << spawnInfo << "/" << taskId << ") - " << errorText.toUtf8() << Qt::endl;
      }
  };

  if (!success) {
      toSpawnList.append( SIG_GPTaskToSpawn{ actId, ind.getPoolPos() } );
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
		QString const indName = exp.population.getIndividual( pvmTask->indPosition ).getName();

		int info = pvm_probe(pvmTask->pvmTaskId, 5);

    // pvm_probe answers three ways: a buffer id above zero, zero for nothing
    // waiting, and a negative error code. A negative must not reach the receive
    // below, which would block for a message that cannot come.
    if (info > 0)
  	{
	  	pvm_recv(pvmTask->pvmTaskId, 5);
	  	pvm_upkdouble(&result,1,1);

	  	// No fitness is negative. A slave's -1 would also read as this
	  	// method's "no result yet" and leave the caller polling a task
	  	// deleted below.
	  	if (result < 0)
	  	{
	  		SIGEL_Tools::SIG_IO::cerr << "Individual \"" << indName
	  					  << "\" returned " << result << "; it is recorded as fitness 0." << Qt::endl;
	  		result = 0;
	  	}

	  	SIGEL_Tools::SIG_IO::cerr << "\tFitness " << result << " for individual \"" << indName << "\" from host \"" << pvmTask->host.name << "\"." << Qt::endl;
	  	pvmTask->host.noOfSlaves--;
	  	delete pvmTasks[ taskId ];   // insert() freed the finished task
	  	pvmTasks[ taskId ] = nullptr;
		}

    else if (info < 0)
		{
		  // PvmSysErr is the daemon itself; anything else leaves a slave running
		  // that nothing will collect from, so it is killed here exactly as the
		  // timeout below kills one that has to be given up.
		  if (info == PvmSysErr)
		    pvmLost = true;

		  SIGEL_Tools::SIG_IO::cerr << "pvm_probe() failed with "
					    << ( info == PvmSysErr ? "pvmd is not responding"
								   : "an error" )
					    << " (" << info << ") -- giving up on the task for individual \"" << indName << "\""
					    << Qt::endl;

		  pvm_kill( pvmTask->pvmTaskId );
		  pvmTask->host.noOfSlaves--;
		  toSpawnList.append( SIG_GPTaskToSpawn{ taskId, pvmTask->indPosition } );

		  delete pvmTasks[ taskId ];
		  pvmTasks[ taskId ] = nullptr;
		}

    else
		{
		  if (exp.gpParameter.getTimeOutMinutes() > 0)
	    {
	      int const timeOutSeconds = exp.gpParameter.getTimeOutMinutes() * 60;

	      QDateTime timeOutTime = pvmTask->spawnTime.addSecs( timeOutSeconds );

	      if (QDateTime::currentDateTime() >= timeOutTime)
				{
		      SIGEL_Tools::SIG_IO::cerr << "sigel_slave timed out for individual \"" << indName << "\" -- force quit via 'pvm_kill()'" << Qt::endl;

		  		pvm_kill( pvmTask->pvmTaskId );
		  		pvmTask->host.noOfSlaves--;
		  		toSpawnList.append( SIG_GPTaskToSpawn{ taskId, pvmTask->indPosition } );

		  		delete pvmTasks[ taskId ];   // insert() freed the finished task
		  		pvmTasks[ taskId ] = nullptr;
				}
			}
		}
  }

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Checking task "
			    << taskId
			    << Qt::endl;
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
  // One pass over toSpawnList: each task gets one spawn try, and a spawned task
  // is removed.
  qsizetype toSpawnIndex = 0;

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Sweeping to spawn!" << Qt::endl;
#endif

  while (toSpawnIndex < toSpawnList.size())
    {
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Entering spawn loop!" << Qt::endl;
#endif

      int hostNumber = getNextHost();

      bool success = false;

      if (hostNumber != -1)
	{
	  SIG_GPActivePVMHost *usedHost = pvmHosts[ hostNumber ];

	  const QByteArray usedHostNameQCString = usedHost->name.toUtf8();
	  char const *usedHostNameCString = usedHostNameQCString.constData();

	  QString executableName;
	  // "." asks PVM to find sigel_slave on its own search path.
	  if (usedHost->executableDir.path() == ".")
		executableName = "sigel_slave";
	  else
		executableName = usedHost->executableDir.path() + "/sigel_slave";
	  const QByteArray executableNameQCString = executableName.toUtf8();

	  char const *executableNameCString = executableNameQCString.constData();

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

	      int internalId = toSpawnList.at( toSpawnIndex ).internalId;
	      int individualNumber = toSpawnList.at( toSpawnIndex ).individualPosition;

	      SIG_GPIndividual &ind = exp.population.getIndividual( individualNumber );

	      SIG_GPPVMTask *newTask = new SIG_GPPVMTask( *usedHost,
							  internalId,
							  taskId,
							  individualNumber,
							  QDateTime::currentDateTime() );

	      delete pvmTasks[ internalId ];
	      pvmTasks[ internalId ] = newTask;

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
	toSpawnList.removeAt( toSpawnIndex );
      else
	toSpawnIndex++;
    };
};

int SIGEL_GP::SIG_GPFitnessTrainer::getNextHost() {
  int result = -1;
  SIG_GPPVMHost   *freshHost;

  // now we make ourself running exclusively to add all new hosts from the freshDynHosts list
  pthread_mutex_lock( &dynHostsMutex );

  // add all new dynamic hosts to pvmHosts and declare them to PVM
  for (unsigned int i=0; i<freshDynHosts.count(); i++) {
    // get the next new host to be added to our pvmHost list
    freshHost = freshDynHosts.at(i);

    const QByteArray freshHostNameQCString = freshHost->name.toLatin1();
    char const *freshHostNameCString = freshHostNameQCString.constData();

    int singleInfo = 0;
    int info = pvm_addhosts( const_cast< char** >(&freshHostNameCString), 1, &singleInfo );

    // PvmDupHost: the host is still in PVM from an earlier join.
    if (info < 1 && singleInfo != PvmDupHost) {
      SIGEL_Tools::SIG_IO::cerr << "\tPVM cannot add the dynamic host \"" << freshHost->name << "\" (error " << singleInfo << "); it is not used." << Qt::endl;
      continue;
    }

    pvmHosts.resize( pvmHosts.size() + 1 );
    delete pvmHosts[ pvmHosts.size()-1 ];
    pvmHosts[ pvmHosts.size()-1 ] = new SIG_GPActivePVMHost(*freshHost);

    fprintf(stderr, "\tAdded dynamic host \"%s\" to the host list.\n", freshHostNameCString);
  }

  qDeleteAll( freshDynHosts );
  freshDynHosts.clear();

  pthread_mutex_unlock( &dynHostsMutex );

  // nextHostNumber might refer to a host that's no longer available !
  if(pvmHosts.size() != 0)
    nextHostNumber = nextHostNumber % static_cast< uint >(pvmHosts.size());

  // this thing seems to check all hosts in our active-host-list whether they
  // have the resources to start another sigel_slave
  for (unsigned int j=0; j<pvmHosts.size(); j++) {
      SIG_GPActivePVMHost *nextHost = pvmHosts[ nextHostNumber ];
      if (nextHost->noOfSlaves < nextHost->maxSlaves)
	   result = nextHostNumber;

      nextHostNumber = (nextHostNumber + 1) % static_cast< uint >(pvmHosts.size());

      if (result != -1)
        break;
  }

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Next Host: "
			    << result
			    << Qt::endl;
#endif

  return result;
};
