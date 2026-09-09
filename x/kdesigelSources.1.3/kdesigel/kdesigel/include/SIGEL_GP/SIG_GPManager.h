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
#ifndef SIGEL_GP_SIG_GPMANAGER_H
#define SIGEL_GP_SIG_GPMANAGER_H

#include <QList>
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_GP/SIG_GPTournament.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include "MT_Control/MT_Classifier.h"
#include "MT_Control/MT_Evaluator.h"

#include <qdatetime.h>

#include <sys/types.h>
#ifndef _WINDOWS
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#endif

namespace SIGEL_GP
{

/**
 * This class manages the genetic-programming algorithm.
 *
 * It uses the class SIG_Experiment to organize the data of the
 * different experiments, like the GPParameter, which are used to 
 * store the current genetic-programming-parameters. A nessecary part 
 * of the evolution, the fitnesstrainer, are included, which manages 
 * the use of the fitnessfunctions. The class SIG_GPTournament is included
 * becauses it is needed for the organization of the tournaments of a
 * evolution run. The SIG_Randomizer supplies the system with random seeds.
 */

  class SIG_GPManager
    {

  /**
   * This flag is set true if the environment that contains this
   * SIG_GPManager has requested the termination of the evolution.
   *
   * The evolution will stop at the next possible moment, independent
   * from the saveExit GP-Parameter or the termination type.
   */
 public:
  bool userTerminated;

  /**
   * Two flags to synchronize the main thread and server thread when disconnecting
   * dynamically registered clients; if 'disconnectClients' is set, the server thread
   * will disconnect all clients causing them to cleanup temp. files since it'll be
   * no longer used for computations.
   * The 'allDisconnected' flag is set to true when all clients have been disconnected,
   * at which point only atically declared clients -- i.e. declared in the *.exp file --
   * are known to the SIGEL master application, all dynamic hosts must register again
   * for the next fitness computation phase.
   * Has to be volatile of course since the compiler needs to know that these variables
   * can get changed elsewhere, not just in our local code (local thread).
   */
   volatile bool allDisconnected;
   volatile bool disconnectClients;

   /**
    * Simple flag indicating if the threaded server is up and running, i.e. if we
    * have to expect dynamic clients participate on the fitness evaluations.
    */
    volatile bool serverIsUp;

    /**
    * condition variable required to synchronize the threads.
    */
#ifdef _WINDOWS
	HANDLE cond;
#else	
    pthread_cond_t cond;
#endif

  void start();

 private:
  /**
   * This list contains the indices of all tournaments in tours
   * that are free to start or that are waiting for the calculation
   * of fitness values for individuals that were created by them.
   */
  QList<int> taskCanDoList;

  /**
   * The experiment object, which contains all the data related to the
   * current experiment.
   */
 protected:
  SIG_GPExperiment &actExperiment;

 protected:
  bool schlussJetzt;

 private:
  QDateTime startTime;

  /**
   * The fitnesstrainer object, which manages the computation of the
   * fitnessvalues with PVM.
   */
 private:
  SIG_GPFitnessTrainer *trainer;
  
  /**
   * This QList is used to store the randomly created tournaments for one
   * generation.
   */
 private:
  QList<SIG_GPTournament *> tours;

  /**
   * The randomizer object, used to create randomseeds for the evolution.
   */
 private:
  SIGEL_Tools::SIG_Randomizer randomizer;

 private:
  bool fitnessCalculated;

  /**
   * This is the current generation and it starts with 0 every time you restart the evolution.
   */
 private:
  int currentGenerationNo;

  /**
   * The constructor of a SIG_GPManager object.
   * @post
   * A gp-manager was created and the data from the experiment is set to
   * the values of the GPManager.
   * @param exp
   * A reference of the experiment to work with.
   */
 public:
  SIG_GPManager(SIG_GPExperiment &experiment);

   /**
    * The destructor of the SIG_GPManager class.
   * @pre
   * One of the terminationcondidtions is fulfilled
   * @post
   * The gp-manager is destructed.
   */
 public:
  ~SIG_GPManager();

  // D25c gave the destructor a qDeleteAll( tours ). Q2PtrVector's copy
  // constructor cleared autoDelete on the copy (the deleted shim, matching
  // qcollection.h:64), so a copied manager freed nothing; a QList copy shares
  // the raw pointers and BOTH destructors would free them. Same hazard D7 and
  // D15 closed. Nothing copies a manager today -- SIG_GUIGPManager derives from
  // this class, and Phase C is exactly the case that would find it.
  SIG_GPManager( SIG_GPManager const & ) = delete;
  SIG_GPManager &operator=( SIG_GPManager const & ) = delete;

   /**
    * The evolutionLoop is the heart of the genetic programming algorithm.
    * It determine how the evolution works.
    * @pre
    * The GPManager is configured correctly, the population is initialized
    * and the fitnesstrainer is ready to run. 
    * @post
    * The evolution process, which is called genetic programming, has occurred.
    * It runed until a terminationcondition was fulfilled. 
    */
 private:
  void evolutionLoop();

	/*
	* see above evolutionLoop();
	* difference: the tournaments are executed by a Classifier   
	* @pre: there is a Meta Classifier System
	*/
  private:
  void evolutionLoop(MT_Classifier *MetaClassifier);

  /**
    * This function creates a set of tournaments.
    * @pre
    * The evolutionloop is started and a set of tournaments is needed.
    * @post
    * A set of tournaments is created and placed in the QList tours of
    * the GPManager.
    * @param quantity
    * This integer determines how many tournaments will be created.
    */
 private:
  void createTours(int quantity);

   /**
    * This function checks the actuality of the fitness values of every 
    * individual in the pool.
    * @pre
    * A experiment is loaded and the evulotionloop wants to prefrom the
    * first loop of the day.
    * @post
    * There is no individual with an fitnessvalue, which is not up to date,
    * in the pool.
    */
 private:
  void evalNewIndis();

  /*
   * see above evalNewIndis();
   * difference: only for this Individual i, which is a participant
   * on a tournament in this generation 
   */
 private:
  void evalNeededIndis();

   /**
    * This functions calculates the order in which the tournaments have to be
    * played. It is a topological sorting algorithm, used to avoid trouble with
    * tournament dependencies, caused by the steady state paradigm.
    * @pre
    * There is a set of tournaments created and placed in the QList tours.
    * @return
    * The returned QList of integer contains the indices of the tournaments
    * in the QList tours. This tournaments can be played without blocking
    * dependicies to other tournaments.
    */
 private:
  void calcInitTourSet();

 private:
  virtual void haveABreak();

 private:
  virtual void messageEvolutionStop();

 private:
  virtual void updateIndividualView( int );

  /**
   * This operation stops the gp-system.
   * @pre 
   * The evolutionloop is running.
   * @post
   * The gp-system is stopped.
   * @param generationBreak
   * A flag, which indicates, if there are still tournaments to play or not.
   */
 private:
  void stopIfNecessary(bool generationBreak);

  /**
   * This operation checks the termination status of the gp-system.
   * @param generationBreak
   * A flag, which indicates, if there are still tournaments to play or not.
   */
 private:
  bool checkTerminationConditions(bool generationBreak);

   /**
    * Runs the evolution on the calling thread and returns when it is
    * finished. Not a thread entry point, despite the name.
    * @pre
    * The GPManager object is created.
    */
 public:
  void run();
  
   /*
	* see above run();
	* difference: the tournaments are executed by a Classifier   
	* @pre: there is a Meta Classifier System
	*/
 public:
  void run (MT_Classifier *MetaClassifier);

  /**
   * This operation returns a refernces of the actual experiment.
   * @return
   * The actual experiment.
   */
private:
  SIG_GPExperiment& getActExperiment();

  /**
   * This operation returns a refernces of the actual GPFitnessTrainer.
   * @return
   * The actual GPFitnessTrainer.
   */  
private:
  SIG_GPFitnessTrainer& getActTrainer();


  /** This method is invoked from the SIGEL master application
   * when started with the '-de' argument.
   * It waits for 180 seconds to allow all clients that dynamically
   * participate on the evolutionary process to register, i.e.
   * submit their hostname.
   */
 public:
  void RegisterDynPVMClients( void );

};

}
#endif // SIGEL_GP_SIG_GPMANAGER_H
