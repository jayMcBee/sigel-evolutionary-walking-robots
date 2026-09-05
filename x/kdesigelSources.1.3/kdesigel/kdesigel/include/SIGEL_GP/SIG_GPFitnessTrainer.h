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
#ifndef SIGEL_GP_SIG_GPFITNESSTRAINER_H
#define SIGEL_GP_SIG_GPFITNESSTRAINER_H

#include <QList>
#include <QString>
#include "SIGEL_GP/SIG_GPExperiment.h"
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_GP/SIG_GPPVMData.h"
#include "SIGEL_GP/SIG_GPActivePVMHost.h"
#include "SIGEL_GP/SIG_GPPVMTask.h"



namespace SIGEL_GP
{

/**
* This class controls the fitness computation tasks lauchned by PVM. 
*
*/

class SIG_GPFitnessTrainer
{
  /**
   * A refernce to the actual experiment datas.
   */
 private:
 SIG_GPExperiment& exp;

  /**
   * The array of TIDs from PVM, to identify the PVMtasks.
   */
 private:
  QList< SIG_GPPVMTask * > pvmTasks;

 /**
  * our hosts participating in fitness calculations
  */
 private:
  QList< SIG_GPActivePVMHost * > pvmHosts;

 /**
  * Names of all dynamic hosts, which are valid for a single evaluation period
  */
  QList< QString * > dynHosts;

 /**
  * New dynamic hosts that have registered but were not yet added to 'pvmHosts'.
  */
  QList< SIG_GPPVMHost * > freshDynHosts;

 private:
  int nextHostNumber;

  /**
   * The array of indices, representing the position of the individual in the pool, which is to simulate.
   */
 private:
QList< QList<int> * > toSpawnList;


/**
 * This is an object of the class, which contains the data encode and decode 
 * routines, related to pvmdatatransfer.
 */
 private:
SIG_GPPVMData PVMData;

 private:
SIGEL_Robot::SIG_Robot modifiedRobot; 

  /**
   * This attribute contains the next free number, which is no indice in the array of the slaves.
   */
 private:
int nextFreeNumber ;

  /**
   * The constructor of the object of a GPFitnesstrainer.
   * @pre
   * The GPManager is started and demands an object of the GPFItnessTrainer. A reference to an
   * experiment object have to be given as a parameter.
   * @post
   * The object of a GPFitnessTrainer is created, the reference to the experiment object given as 
   * a parameter is set to the attributes. The QArrays are initialized with ZERO. The nextFreeNumber 
   * is set to ZERO.
   * @param exp
   * The actual experiment data of the GPManager.
   */
 public:
SIG_GPFitnessTrainer(SIG_GPExperiment& exp);

  /**
   * The destructor of the an SIG_GPFitnessTrainer-object
   * @pre
   * The GPManager have to be destructed and the related classes have to be destructed first. 
   * @post
   * The GPFitnessTrainer-object is destructed, no PVM-task is running anymore.
   */
 public:
~SIG_GPFitnessTrainer(); 

  /**
   * This operation spawns a PVM-task. The individual, given by the parameter, contains the
   * robot control program, which is to evaluate.
   * @pre
   * The experiment attribute have to be set, because if not, there is no structure data, which
   * is needed for the simulation run.
   * @post
   * The PVM_task is spawned computing the fitnessvalue of the robot controll program, contained
   * in the individual. 
   * @param ind
   * The individual, which containes the robot controll program.
   * @return
   * An integer, which represents the TID of the PVM-task. If an error occurred at the spawning
   * process, the returned integer has the value -1.
   */
 public:
virtual int spawnTask(SIG_GPIndividual const& ind);

  /** This operation checks a PVM-task, if the computation continues or not. This happens by
   * checking out the message buffer, which the checked PVM-task have to use. If the computation
   * is completed, the operation will return the fitnessvalue of the evaluated robot controll
   * program. If it not, the negative value -1 is returned.
   * @pre
   * There is an PVM-task to check.
   * @post
   * If the desired data was placed in the message buffer, it is removed and forwarded to the
   * individual. If not, nothing happened.
   * @param task
   * The TID of a PVM-task.
   * @return
   * The fitnessvalue of the evaluated robot controll program or -1 as a sign that an error 
   * occurred.
   */
 public:
virtual double checkTask(int task);

  /**
   * This operation will stop all PVM-tasks.
   * @pre
   * There is a signal of an userbreak and the GPFitnessTrainer have to stop all computations 
   * of fitnessvalues.
   * @post
   * No PVM-task is running anymore.
   */
 public:
void stopTrainersSlaves();

  /**
   * This operation sweeps over the toSpawnArray, to evaluate which task is to start next.  
   * @pre
   * The toSpawnArray is not empty.
   * @post
   * The toSpawnArray is updated.
   */
 public:
void sweepToSpawn();

 public:
 int getNextHost();

 /** This methods adds a dynamic PVM host, which will used
  * for this evaluation only and not saved to the experiment
  * file like all other (static) PVM hosts !
  */
  void addDynHost(QString newHost);

 /**
  * This method is called to delete all dynamic hosts from our
  * internal list of PVM hosts (pvmHosts). After flushing the dynamic hosts
  * they'll be no longer used for fitness evaluations.
  */
  void flushAllDynHosts( void );

};

}
#endif //  SIGEL_GP_SIG_GPFITNESSTRAINER_H
