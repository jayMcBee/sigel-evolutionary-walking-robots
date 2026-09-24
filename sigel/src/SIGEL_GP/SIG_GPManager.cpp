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
#include <QList>
#include <QString>
#include <QTextStream>
#include <algorithm>   // std::sort, std::count_if, numeric; Qt 2's QArray::sort compared raw bytes (memcmp)
#include "SIGEL_GP/SIG_GPManager.h"

#ifndef _WINDOWS
#include <unistd.h>
#else
#include <pvm3.h>
#endif
#include <qfile.h>
#include <qdir.h>
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_GP/SIG_GPMutationTournament.h"
#include "SIGEL_GP/SIG_GPSimpleTournament.h"
#include "SIGEL_GP/SIG_GPCrossOverTournament.h"
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"

#include "MT_Control/MT_Controller.h"
#include "MT_Control/MT_Classifier.h"

SIGEL_GP::SIG_GPManager::SIG_GPManager(SIGEL_GP::SIG_GPExperiment &experiment)
  : userTerminated( false ),
    allDisconnected( false ),
    disconnectClients( false ),
    serverIsUp( false ),
    taskCanDoList(),
    currentExperiment( experiment ),
    stopEvolutionNow( false ),
    tours(),
    randomizer( currentExperiment.gpParameter.getRandomSeed() ),
    fitnessCalculated( false ),
    currentGenerationNo(0)
{
	trainer = 0;
	if(currentExperiment.mtController->IsEnabled() && currentExperiment.mtController->UsedSystem() == EVALUATOR_SUBST){
		trainer = dynamic_cast<SIG_GPFitnessTrainer*>(currentExperiment.mtController->getFitnessTrainer());
		if(!trainer)
			trainer = new SIG_GPFitnessTrainer(currentExperiment);
	} else {
		trainer = new SIG_GPFitnessTrainer(currentExperiment);
	}
	// tours.setAutoDelete(true) was the free for every tournament: it ran in
	// Qt 2's ~QVector, in clear(), in insert() on the old occupant and in a
	// shrinking resize(). Each of those is written out at its site.
};


//Returns a reference to the current experiment's trainer
SIGEL_GP::SIG_GPFitnessTrainer &SIGEL_GP::SIG_GPManager::getActTrainer()
{
  return *trainer;
};

 //Returns a reference to the experiment
SIGEL_GP::SIG_GPExperiment &SIGEL_GP::SIG_GPManager::getCurrentExperiment()
{
  return currentExperiment;
};

void SIGEL_GP::SIG_GPManager::evolutionLoop() {
  int maxTouchsPerLoop = currentExperiment.gpParameter.getMaxTouchsPerLoop();
  int toDoSweepsPerLoop = currentExperiment.gpParameter.getToDoSweepsPerLoop();

  SIG_GPPopulation &pop = currentExperiment.population;  

  while ( !taskCanDoList.isEmpty() ) {
    stopIfNecessary( false );

    if (stopEvolutionNow)
      return;

    processInterfaceEvents();

    trainer->sweepToSpawn();

    for (int sweepCounter = 0;
      (sweepCounter < toDoSweepsPerLoop) && (!taskCanDoList.isEmpty()); sweepCounter++) {

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager sweeping the taskCanDoList (sweepCounter: "
          << sweepCounter
          << ")." << Qt::endl;
#endif

      int touchsCounter = 0;

      qsizetype canDoIdx = 0;   // was an iterator: Qt 2's list was linked

      while (canDoIdx < taskCanDoList.size()) {
      // Poll interval: long enough not to spin a core, short enough that
      // finished results do not wait. Interface events are processed after it.
#ifdef _WINDOWS
      Sleep(5);
#else
      usleep(5000);
#endif	
      processInterfaceEvents();

      if ((touchsCounter == maxTouchsPerLoop) && (maxTouchsPerLoop != -1)) {
        break;
      }

      SIG_GPTournament &actTour=*tours[ taskCanDoList.at( canDoIdx ) ];
      int actIndiNumber = actTour.indis.size();

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager inspecting tournament No. "
        << taskCanDoList.at( canDoIdx )
        << ".\n"
        << "justWaiting: "
        << actTour.justWaiting
        << "\n"
        << "depNumber: "
        << actTour.depNumber
        << "\n"
        << "indis:" << Qt::endl;
      for (int i = 0; i < actTour.indis.size(); i++)
        SIGEL_Tools::SIG_IO::cerr << "  indNumber: "
          << (*actTour.indis[ i ]).indNumber
          << "\n"
          << "    successor: "
          << (*actTour.indis[ i ]).successor
          << "\n"
          << "    fitTaskId: "
          << (*actTour.indis[ i ]).fitTaskId
          << Qt::endl;
#endif

      if (!actTour.justWaiting) {
#ifdef SIG_DEBUG
        SIGEL_Tools::SIG_IO::cerr << "Playing tournament "
          << taskCanDoList.at( canDoIdx )
          << Qt::endl;
#endif

      actTour.run();

      for (int i = 0; i < actIndiNumber; i++) {
        SIG_GPIndividual &actInd = pop.getIndividual( actTour.indis[i]->indNumber );

        if (actInd.upToDate()) {
#ifdef SIG_DEBUG
          SIGEL_Tools::SIG_IO::cerr << "Individual "
            << actInd.getPoolPos()
            << " is up to date." << Qt::endl;
#endif

          int actSuccessor = actTour.indis[i]->successor;
          if (actSuccessor != -1) {
            tours[ actSuccessor ]->depNumber -= 1;
            if( tours[ actSuccessor ]->depNumber == 0 ) {
              taskCanDoList << actSuccessor;
            };
          };
        }
        else {
#ifdef SIG_DEBUG
          SIGEL_Tools::SIG_IO::cerr << "Individual "
            << actInd.getPoolPos()
            << " is not up to date." << Qt::endl;
#endif

          actTour.justWaiting = true;
          actTour.indis[i]->fitTaskId = trainer->spawnTask( actInd );
          updateIndividualView( actTour.indis[i]->indNumber );
        };

        updateIndividualView( actTour.indis[i]->indNumber );

      };
    }
    else {
      actTour.justWaiting = false;

      for (int j=0; j<actIndiNumber; j++) {
        SIG_GPTournamentIndividual &actTourInd = *actTour.indis[j];

        if (actTourInd.fitTaskId != -1) {
          double actFitness = trainer->checkTask( actTourInd.fitTaskId );
          if (actFitness != -1) {
            SIG_GPIndividual &actInd = pop.getIndividual( actTourInd.indNumber );
            actInd.setFitness( actFitness );
            updateIndividualView( actTourInd.indNumber );
            actTourInd.fitTaskId = -1;

            int actSuccessor = actTourInd.successor;
            if (actSuccessor != -1) {
              tours[ actSuccessor ]->depNumber -= 1;
                if (tours[ actSuccessor ]->depNumber == 0)
                  taskCanDoList << actSuccessor;
            }; // if(actSucessor) -condition
          } // if(actFitness) -condition
         else
           actTour.justWaiting = true;
         };
       };
     }; // for (int j=0;...) - loop
        if (!actTour.justWaiting)
          taskCanDoList.removeAt( canDoIdx );   // next slides into canDoIdx
        else
          ++canDoIdx;

        touchsCounter++;
      };
    };
  };
};

void SIGEL_GP::SIG_GPManager::createTours(int quantity) {
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager creates "
    << quantity
    << " Tournaments." << Qt::endl;
#endif

  //The experiment's population
  SIG_GPPopulation &pop = currentExperiment.population;

  //The tournament set size is fixed
  // clear(): setAutoDelete made this delete every tournament. Real free.
  qDeleteAll( tours );
  tours.clear();
  // resize() grows with value-initialised (null) slots, as Qt 2 did.
  tours.resize(quantity);

  for(int i=0;i<quantity;i++) {
    QList< int > poolPositions( 2 );

    poolPositions[0] = randomizer.getRandomInt( pop.getSize() );
    poolPositions[1] = randomizer.getRandomInt( pop.getSize() - 1 );

    poolPositions[1] = (poolPositions[1] >= poolPositions[0]) ? poolPositions[1] + 1 : poolPositions[1];

    int totalProbCount =   currentExperiment.gpParameter.getReproductionProb()
                         + currentExperiment.gpParameter.getMutationProb()
                         + currentExperiment.gpParameter.getXoverProb();

    // ToDo: Throw Exception!
    if (!totalProbCount) {
      SIGEL_Tools::SIG_IO::cerr << "Cannot create tournaments - bad probabilities!" << Qt::endl;
      return;
    };

    int randomResult = randomizer.getRandomInt( totalProbCount ) + 1;

    SIG_GPTournament *actTour = 0;

    if (randomResult <= currentExperiment.gpParameter.getReproductionProb()) {

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager creates a Simple Tournament.\n"
        << "Player's positions are "
        << poolPositions[0]
        << " and "
        << poolPositions[1]
        << "." << Qt::endl;
#endif

      actTour = new SIG_GPSimpleTournament(randomizer,
        *trainer,
        pop,
        currentExperiment.gpParameter,
        *currentExperiment.robot.getLangParam(),
        poolPositions[0],
        poolPositions[1]);
    }
    else if ( randomResult <=   currentExperiment.gpParameter.getReproductionProb()
                                    + currentExperiment.gpParameter.getMutationProb() ) {
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager creates a Mutation Tournament.\n"
        << "Player's positions are "
        << poolPositions[0]
        << " and "
        << poolPositions[1]
        << "." << Qt::endl;
#endif

     actTour = new SIG_GPMutationTournament(randomizer,
       *trainer,
       pop,
       currentExperiment.gpParameter,
       *currentExperiment.robot.getLangParam(),
       poolPositions[0],
       poolPositions[1]);
    }
    else {
      int newPoolPos = randomizer.getRandomInt( pop.getSize() - 2 );

      std::sort( poolPositions.begin(), poolPositions.end() );

      newPoolPos = (newPoolPos >= poolPositions[0]) ? newPoolPos + 1 : newPoolPos;
      newPoolPos = (newPoolPos >= poolPositions[1]) ? newPoolPos + 1 : newPoolPos;

      poolPositions.resize( 3 );
      poolPositions[2] = newPoolPos;
      std::sort( poolPositions.begin(), poolPositions.end() );

      newPoolPos = randomizer.getRandomInt( pop.getSize() - 3 );

      newPoolPos = (newPoolPos >= poolPositions[0]) ? newPoolPos + 1 : newPoolPos;
      newPoolPos = (newPoolPos >= poolPositions[1]) ? newPoolPos + 1 : newPoolPos;
      newPoolPos = (newPoolPos >= poolPositions[2]) ? newPoolPos + 1 : newPoolPos;

      poolPositions.resize( 4 );
      poolPositions[3] = newPoolPos;

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager creates a Crossover Tournament.\n"
        << "Player's positions are "
        << poolPositions[0]
        << ", "
        << poolPositions[1]
        << ", "
        << poolPositions[2]
        << " and "
        << poolPositions[3]
        << "." << Qt::endl;
#endif

      actTour = new SIG_GPCrossOverTournament(randomizer,
        *trainer,
        pop,
        currentExperiment.gpParameter,
        *currentExperiment.robot.getLangParam(),
        poolPositions[0],
        poolPositions[1],
        poolPositions[2],
        poolPositions[3]);
    };

    // insert(): overwrite slot i, deleting any previous occupant. Qt 2's
    // insert did NOT shift, unlike QList::insert. The slot is null here.
    delete tours[ i ];
    tours[ i ] = actTour;
  };
};

namespace {

  /* Qt 2's QVector::isEmpty() was count()==0 -- no NON-NULL slots -- while
   * QList::isEmpty() is size()==0. They disagree between
   * "tours.clear(); tours.resize(quantity);" and the loop that fills the slots,
   * and after createTours' !totalProbCount early return, which leaves the
   * vector resized and entirely null.
   *
   * Unreachable at the four call sites today: start()'s two callers, main and
   * SIG_GUIGPExperiment::slotStartEvolution, each use a freshly built manager, so
   * tours is default-constructed at each entry to run(). Reproduced anyway,
   * so a caller that reuses a manager keeps 1.3's behaviour.
   */
  bool toursAreEmpty( QList< SIGEL_GP::SIG_GPTournament * > const &tours )
  {
    for (SIGEL_GP::SIG_GPTournament *t : tours)
      if (t)
        return false;
    return true;
  }


  /* fitTaskList held setAutoDelete(true): Qt 2's ~QList was the ONLY free, and it
   * ran on every exit -- including the two early returns inside the function.
   * QList frees nothing, so the guard restores that. Same shape as
   * DynaMechsLinkGuard in SIG_DynaMechsSimulationData.cpp.
   */
  struct FitTaskListGuard
    {
      QList< QList<int> * > *tasks;

      ~FitTaskListGuard() { if (tasks) qDeleteAll( *tasks ); }
    };

};

void SIGEL_GP::SIG_GPManager::evalNewIndis() {

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager evaluates new Individuals." << Qt::endl;
#endif

  QList< QList<int> * > fitTaskList;
  FitTaskListGuard fitTaskListGuard{ &fitTaskList };

  //The experiment's population
  SIG_GPPopulation &pop=currentExperiment.population;

  //The population size is determined
  int poolSize=pop.getSize();

  for (int i=0;i<poolSize;i++) {
    stopIfNecessary( true );

    if (stopEvolutionNow)
      return;

    processInterfaceEvents();

    SIG_GPIndividual &actInd=pop.getIndividual(i);

    bool upToDate = actInd.upToDate();

	// META-System:
	// individuals with negative fitness haven't been evaluated yet
	// or have been evaluated by the meta-system
	// these individuals should be evaluated again
    if (actInd.getFitness() < 0.0)
	  upToDate = false;

    //Update the fitness values for individuals that are not up to date
    if(!upToDate) {
#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager: Individual Number "
        << i
        << " hasn't an actual fitness value and will be evaluated." << Qt::endl;
#endif

      QList<int> *actFitTask = new QList<int>(2);
      (*actFitTask)[0] = trainer->spawnTask(actInd);
      (*actFitTask)[1] = actInd.getPoolPos();

      fitTaskList.append( actFitTask );
    };
  };

  while (!fitTaskList.isEmpty()) {

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager entering loop sweeping the fitTaskList" << Qt::endl;
#endif

    stopIfNecessary( true );

    if (stopEvolutionNow)
      return;

    processInterfaceEvents();

    trainer->sweepToSpawn();
    // first(): Qt 2 returned null on empty, Qt 6's first() is UB there.
    qsizetype fitCur = fitTaskList.isEmpty() ? -1 : 0;
    QList<int> *actFitTask = (fitCur < 0) ? 0 : fitTaskList.at( fitCur );
    QList<int> *prevFitTask = 0;

    while (actFitTask) {
      double actFitness = trainer->checkTask( (*actFitTask)[0] );

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager checking task "
        << (*actFitTask)[0]
        << ", has fitness "
        << actFitness
        << "." << Qt::endl;
#endif

      if (actFitness != -1) {

#ifdef SIG_DEBUG
        SIGEL_Tools::SIG_IO::cerr << "Fitness is ready." << Qt::endl;
#endif

        pop.getIndividual( (*actFitTask)[1] ).setFitness( actFitness );
        updateIndividualView( (*actFitTask)[1] );

        // remove(): setAutoDelete(true) made this the free.
        delete fitTaskList.takeAt( fitCur );
        // Qt 2's QList cursor after remove(): land on whatever slid in, else step back.
        if (fitCur >= fitTaskList.size())
          fitCur = fitTaskList.isEmpty() ? -1 : fitTaskList.size() - 1;
        actFitTask = (fitCur < 0) ? 0 : fitTaskList.at( fitCur );
        if (actFitTask == prevFitTask)
          break;
      }
      else {

#ifdef SIG_DEBUG
        SIGEL_Tools::SIG_IO::cerr << "Fitness is not yet ready." << Qt::endl;
#endif

        prevFitTask = actFitTask;
        // next(): a dead cursor stays dead and does NOT advance.
        if (fitCur < 0 || ++fitCur >= fitTaskList.size())
          { fitCur = -1; actFitTask = 0; }
        else
          actFitTask = fitTaskList.at( fitCur );
      };
    };
  };

  fitnessCalculated = true;
};

void SIGEL_GP::SIG_GPManager::calcInitTourSet() {

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager is calculating the initial set of active tournaments." << Qt::endl;
#endif

    taskCanDoList.clear();

    // The experiment's population
    SIG_GPPopulation &pop=getCurrentExperiment().population;

    QList<int> lastAccesses( pop.getSize() );

    for (int l=0; l < pop.getSize(); l++)
      lastAccesses[l] = -1;

    for(unsigned int i=0; i < tours.size(); i++) {

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager inspecting tournament "
				  << i << "." << Qt::endl;
#endif

        // The current tournament

	SIG_GPTournament &actTour=*tours[i];

	int actIndisNumber=actTour.indis.size();

	for (int j=0; j<actIndisNumber; j++)
	  {

#ifdef SIG_DEBUG
	    SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager inspecting tournament individual"
				      << j << "." << Qt::endl;
#endif

	    SIG_GPTournamentIndividual &actInd = *actTour.indis[j];
	    int lastAccess = lastAccesses[ actInd.indNumber ];

	    if (lastAccess != -1)
	      {
		SIG_GPTournament &prevTour = *tours[lastAccess];
		int prevIndisNumber = prevTour.indis.size();

		for (int k=0; k<prevIndisNumber; k++)
		  {
		    SIG_GPTournamentIndividual &prevInd = *prevTour.indis[k];
		    if ( prevInd.indNumber == actInd.indNumber )
		      {
			prevInd.successor = i;
		      };
		  };

		actTour.depNumber += 1;
	      };
	    lastAccesses[ actInd.indNumber ] = i;
	  };

	if (actTour.depNumber == 0)
	  taskCanDoList << i;
      };
  };

bool SIGEL_GP::SIG_GPManager::pvmIsLost() const
{
  return trainer && trainer->pvmLost;
};

SIGEL_GP::SIG_GPManager::TournamentProgress SIGEL_GP::SIG_GPManager::tournamentProgress() const
{
  // A tournament is unfinished while it waits for an earlier one, or while it
  // is in taskCanDoList: not played yet, or waiting for its results.
  const int plannedCount = int( tours.size() );
  const int waitingCount = int( std::count_if( tours.cbegin(), tours.cend(),
    []( const SIG_GPTournament *t ) { return t && t->depNumber > 0; } ) );
  const int queuedCount = int( taskCanDoList.size() );
  return { plannedCount - waitingCount - queuedCount, plannedCount };
};

void SIGEL_GP::SIG_GPManager::processInterfaceEvents()
{
  // Empty in the base: a manager with no interface has no events to process.
  // SIG_GUIGPManager overrides it.
};

void SIGEL_GP::SIG_GPManager::messageEvolutionStop() {
  // Sets the flag the evolution loop reads. Stops nothing itself; the loop
  // notices at its next check.
  stopEvolutionNow = true;
};

void SIGEL_GP::SIG_GPManager::updateIndividualView( int ) {
};

void SIGEL_GP::SIG_GPManager::stopIfNecessary(bool generationBreak) {
  if (checkTerminationConditions( generationBreak )) {
    trainer->stopTrainersSlaves();
    messageEvolutionStop();

	// and now eventually stop the Meta-GP-System
	currentExperiment.mtController->stopEvolution();
  };
};

bool SIGEL_GP::SIG_GPManager::checkTerminationConditions(bool generationBreak) {
  bool exitIsPermitted = (!currentExperiment.gpParameter.getSaveExit() || generationBreak);

  // ToDo: Check for maxFitness

  //The evolution loop is exited depending on the condition,
  //either on time or on reaching the maximum individual

  if (userTerminated)
    return true;

  if (!fitnessCalculated)
    return false;

  if (exitIsPermitted) {
    bool timeExpired = false;

    if (currentExperiment.gpParameter.getTerminationUsesDate())
      timeExpired =  ( currentExperiment.gpParameter.getTerminationTime() <= QDateTime::currentDateTime() );
    else {
      int durationHours =   ( currentExperiment.gpParameter.getTerminationDurationDays() * 24)
                                    + currentExperiment.gpParameter.getTerminationDurationHours();
      int durationMinutes =   ( durationHours * 60 )
                                    + currentExperiment.gpParameter.getTerminationDurationMinutes();
      int durationSeconds =   ( durationMinutes * 60 )
                                    + currentExperiment.gpParameter.getTerminationDurationSeconds();

      QDateTime terminationTime = startTime.addSecs( durationSeconds );

      timeExpired = ( terminationTime <= QDateTime::currentDateTime() );
    }

    bool generationsReached = ( currentExperiment.gpParameter.getTerminationGenerationNo() <= currentGenerationNo );

    switch (currentExperiment.gpParameter.getTerminationModel()) {
      case SIG_GPParameter::byTime:
        return timeExpired;
        break;
      case SIG_GPParameter::byGeneration:
        return generationsReached;
        break;
      case SIG_GPParameter::byTimeGeneration:
        return timeExpired || generationsReached;
        break;
      case SIG_GPParameter::byUser:
        return false;
        break;
    }
  }
  else
    return false;
};


void SIGEL_GP::SIG_GPManager::start()
{
	// The Meta system hands the same trainer to every run, so what the last one
	// learned about PVM must not end this one.
	if (trainer)
		trainer->pvmLost = false;

	if(currentExperiment.mtController->IsEnabled() && currentExperiment.mtController->UsedSystem() == CLASSIFIER_SUBST)
		run(dynamic_cast<MT_Classifier*>(currentExperiment.mtController->getClassifier()));
	else
		run();
};

void SIGEL_GP::SIG_GPManager::run() {

	// start the MT_GP-System only if the SIGEL-GP-System would start
	// Qt 2's isEmpty() was count()==0 -- NO NON-NULL SLOTS -- not size()==0.
	if(toursAreEmpty( tours ) && currentExperiment.getPopulation().getSize() > 3)
		currentExperiment.mtController->startEvolution();

#ifdef _WINDOWS
  HANDLE mutex = CreateMutex(NULL, false, NULL);
#else
  pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

  // init the condition variable
#ifdef _WINDOWS
  cond = CreateEvent(NULL, true, false, NULL);
#else
   pthread_cond_init(&cond, NULL);
#endif

  if (!toursAreEmpty( tours )) {
    SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager::run() was called more than once!" << Qt::endl;
    messageEvolutionStop();
  };

  if (currentExperiment.getPopulation().getSize() < 4) {
    SIGEL_Tools::SIG_IO::cerr << "Population contains less than 4 Individuals, cannot evolve!" << Qt::endl;
    messageEvolutionStop();
  };

  startTime = QDateTime::currentDateTime();

  for (;;) {
#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager entering endless for-loop." << Qt::endl;
#endif

    stopIfNecessary( true );

    if (stopEvolutionNow)
      return;

    // Every "resetGeneration" generations all fitness values are set to -1 so that evalNewIndis()
    // recomputes them. If the fitness function supports it, this allows a different target every "resetGeneration" generations
    // Apply the fitness criterion.
    // Do not divide by zero
    if ( currentExperiment.gpParameter.getResetEveryGeneration() != 0) {
      if ( (currentExperiment.population.getPoolGeneration() % currentExperiment.gpParameter.getResetEveryGeneration()) == 0)
        currentExperiment.population.resetAllFitnessValues();
    }
    // evaluate the individuals which have no fitness value
    evalNewIndis();

    stopIfNecessary( true );

    if (stopEvolutionNow)
      return;
    // creates a set of tournaments
    createTours( currentExperiment.gpParameter.getTournamentsPerGeneration() * currentExperiment.getPopulation().getSize() );
    // sorts the tournaments, how they should evolve on the pvm clients
    calcInitTourSet();
    // the heart of the genetic algorithm, it executes the tournaments
    evolutionLoop();

    if (stopEvolutionNow)
      return;

    // increment the generation, because there is one evolution-loop evolved
    // this gives us the current generation and it starts with 0 every time we start the evolution
    currentGenerationNo++;
    // this is the total amount of generations evolved, since the project is created
    currentExperiment.population.poolGeneration++;

    SIGEL_Tools::SIG_IO::cerr << "Computing Generation " << currentExperiment.population.poolGeneration << " (" << currentGenerationNo << " in this run)\t(" << (QDateTime::currentDateTime()).toString() << ")" << Qt::endl;

    // increment the age of the individuals
    for (int i=0; i < currentExperiment.population.getSize(); i++) {
      currentExperiment.population.getIndividual( i ).increaseAge();
      updateIndividualView( i );
    };

    int poolImageGeneration = currentExperiment.getGPParameter().getPoolImageGeneration();

    int poolGenerationNo = currentExperiment.population.poolGeneration;
    QDateTime generationBreak = QDateTime::currentDateTime();
    double bestFitness = currentExperiment.population.getBestFitness( true );
    double minFitness = currentExperiment.population.getWorstFitness( true );
    double averageFitness = currentExperiment.population.getAverageFitness();
    // write the stats of the evolution into a history
    SIG_GPExperimentHistoryEntry *newExpHistEntry = new SIG_GPExperimentHistoryEntry( poolGenerationNo,
      generationBreak,
      bestFitness,
      minFitness,
      averageFitness );

    currentExperiment.experimentHistory.append( newExpHistEntry );

	// tell the Meta-System that the current generation ended
	MT_Evaluator * MetaFitnessTrainer;
	MetaFitnessTrainer = dynamic_cast<MT_Evaluator*>(trainer);
	if (MetaFitnessTrainer !=0)
		MetaFitnessTrainer->nextSIGGeneration(averageFitness);

    // if the user wants a poolImage and it the generation where it should generate, then he gets it
    if ( poolImageGeneration && ((currentGenerationNo % poolImageGeneration) == 0) ) {
      QDir poolImageDir = currentExperiment.getGPParameter().getPoolImageDirectory();

      QString poolImageName =   poolImageDir.path()
                              + QString( "/" )
                              + currentExperiment.experimentName
                              + QString( "_PoolImage_" )
                              + QDateTime::currentDateTime().toString()
                              + ".pol";

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager saving poolImage, currentGenerationNo: "
          << currentGenerationNo
          << ", filename: "
          << poolImageName
          << Qt::endl;
#endif

      QFile poolImage( poolImageName );

      if (poolImage.open( QIODeviceBase::WriteOnly )) {
        QTextStream buffer( &poolImage );

        currentExperiment.getPopulation().writeToFile( buffer );
        poolImage.close();
      }
      else
        SIGEL_Tools::SIG_IO::cerr << "Could not save Pool under "
          << poolImageName
          << "!" << Qt::endl;
    };

    // autosave function	
    // checks whether to save the population or not
    // only save if getAutosave() not zero
    if (currentExperiment.environment.getAutosave()!=0) {
      // only save if the modulo rest is zero
      if ( (currentExperiment.population.poolGeneration%currentExperiment.environment.getAutosave())==0) {
        QFile file( currentExperiment.getPath() );
        if (file.open(QIODeviceBase::WriteOnly)) {
          QTextStream stream(&file);
          currentExperiment.saveExperiment(stream);
          file.close();
        }
        else {
          std::cerr << "could not autosave the experiment!" << endl;
        }
      }
    }

    // do we have a threaded server running for dyn. clients ?
    if (serverIsUp && currentGenerationNo%20==0) {
       fprintf(stderr, " Releasing Dynamic SIGEL-Clients:\n");

       // make the main thread running exclusively
#ifdef _WINDOWS
      WaitForSingleObject(mutex, INFINITE);
#else
      pthread_mutex_lock( &mutex );
#endif
      fprintf(stderr, "\t- Flushing all dynamic clients from PVM-Hosts list\n");
      trainer->flushAllDynHosts();

      fprintf(stderr, "\t- Asking server to disconnect the clients\n");
      disconnectClients = true;
      allDisconnected = false;

      // let's wait for server thread
#ifdef _WINDOWS
      while ( ! allDisconnected ) {
        WaitForSingleObject(cond, INFINITE);
      }
      ResetEvent(cond);
      ReleaseMutex(mutex);
#else
      while ( ! allDisconnected ) {
       pthread_cond_wait(&cond, &mutex);
      }
      pthread_mutex_unlock( &mutex );
#endif

      fprintf(stderr, "\n");
    }
  };
}



void SIGEL_GP::SIG_GPManager::RegisterDynPVMClients( void ) {
  enum { kSigelMasterRegPort = 6789, kSuicidalRequest   = 13 };

  fd_set mySet;
  struct sockaddr_in  sad, caddr;
  QList<int> clientSockets(0);
  //struct hostent *ptrh;
  struct protoent *ptrp;
  int i;
#ifdef _WINDOWS
  SOCKET socke, sdRecv;
  char myInt;
  int alen;
  HANDLE servMutex = CreateMutex(0, false, 0);
#else
  int socke, sdRecv, myInt;
  socklen_t alen;
  pthread_mutex_t servMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
  char clientName[256];
  QString client;
  struct timeval timeOut;

  // init some variables
  serverIsUp = true;

#ifdef _WINDOWS
  WSADATA SocketData;
  if(WSAStartup(MAKEWORD(1,1), &SocketData)){
    fprintf(stderr, "Error while negotiating the socket version.\n");
    exit(1);
  }
#endif	

  // Make a socket to listen to our clients;
  // init sockaddr struct: using Internet family, port kSigelMasterRegPort
  memset((char *)&sad, 0, sizeof(struct sockaddr_in));
  sad.sin_family = AF_INET;
  sad.sin_port = htons(kSigelMasterRegPort);
  sad.sin_addr.s_addr = INADDR_ANY;

  // map TCP protocol number
  if ((ptrp = getprotobyname("tcp")) == 0) {
    fprintf(stderr, "ERR:   Can't map 'tcp' to a protocol number\n");
#ifdef _WINDOWS
    WSACleanup();
#endif		
    exit(1);
  }

  // finally create the socket
  socke = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
#ifdef _WINDOWS
  if (socke == INVALID_SOCKET) {
    fprintf(stderr, "ERR:   Can't create socket\n");
    WSACleanup();
    exit(1);
  }
#else
  if (socke < 0) {
    fprintf(stderr, "ERR:   Can't create socket\n");
     exit(1);
  }
#endif

  // let's bind local address & socket
#ifdef _WINDOWS
  if ( bind(socke, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR ) {
    fprintf(stderr, "ERR:   Bind reported an error\n");
    WSACleanup();
     exit(1);
  }
#else
  if ( bind(socke, (struct sockaddr *)&sad, sizeof(sad)) < 0 ) {
    fprintf(stderr, "ERR:   Bind reported an error\n");
     exit(1);
  }
#endif

  // build the queue for incoming requests
#ifdef _WINDOWS
  if ( listen(socke, 32) == SOCKET_ERROR ) {
    fprintf(stderr, "ERR:   Listen failed\n");
    WSACleanup();
     exit(1);
  }
#else
  if ( listen(socke, 32) < 0 ) {
    fprintf(stderr, "ERR:   Listen failed\n");
     exit(1);
  }
#endif

  // accept() is blocking, but we want to wait in 10 sec. chunks;
  // this allows the main thread to adjust it's active-pvm-host list based on
  // our freshly registered clients. Also check all 10 seconds if the main
  // thread wants us to cancel all connections.
  timeOut.tv_sec  = 10;
  timeOut.tv_usec = 0;

  // to use select() we need to build a fs_set first
  FD_ZERO(&mySet);
  FD_SET(socke, &mySet);

  // (bounded) waiting for requests..
  fprintf(stderr, "Server is awaiting requests from dynamic clients on port %d..\n\n", kSigelMasterRegPort);

  // the (almost) endless server loop
  while ( true ) {
     // pselect returns zero when timeout occurs..
     select(socke+1, &mySet, NULL, NULL, &timeOut);

     // check what caused pselect() to exit
     if ( FD_ISSET(socke, &mySet) ) {
#ifdef _WINDOWS
      alen = sizeof(caddr);
#endif
      sdRecv = accept(socke, (sockaddr *)&caddr, &alen);

#ifdef _WINDOWS
      if (sdRecv == INVALID_SOCKET) {
        fprintf(stderr, "ERR:   accept() failed\n");
        WSACleanup();
        pvm_halt();
        exit(1);
      }
#else
      if (sdRecv < 0) {
        fprintf(stderr, "ERR:   accept() failed\n");
        exit(1);
      }
#endif

      // store socket for later disconnect
      clientSockets.resize( clientSockets.count()+1 );
#ifdef _WINDOWS
      clientSockets[(int)clientSockets.count()-1] = sdRecv;
#else
      clientSockets[clientSockets.count()-1] = sdRecv;
#endif

      i = recv(sdRecv, clientName, sizeof(clientName), 0);

      // remember client locally for later disconnect
      client = clientName;

      // tell the fitnesstrainer there's a fresh host
      trainer->addDynHost(client);
      fprintf(stderr, "\t(Servertask registered dyn. client \"%s\")\n", clientName);
    }

   // check if computation is finished and clients need to be disconnected
    if (disconnectClients) {
      // now make us running exclusively
#ifdef _WINDOWS
      WaitForSingleObject(servMutex, INFINITE);
#else
      pthread_mutex_lock( &servMutex );
#endif

      // iterate through list of connected sockets and cut connection;
      // be sure all clients have been removed from the pvmHost list !
      myInt = kSuicidalRequest;

      for (int i=0; i<clientSockets.count(); i++) {
        send( clientSockets[i], &myInt, sizeof(myInt), 0);
#ifdef _WINDOWS
        closesocket(clientSockets[i]);
#else
        close(clientSockets[i]);
#endif
      }
      fprintf(stderr, "\t(Servertask disconnected %d clients)\n", clientSockets.count());
      clientSockets.resize(0);

      // tell main thread to continue !
      disconnectClients = false;
      allDisconnected = true;

#ifdef _WINDOWS
      SetEvent(cond);
      ReleaseMutex(servMutex);
#else
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock( &servMutex );
#endif
    }

    // prepare next pselect() call
    FD_ZERO(&mySet);
    FD_SET(socke, &mySet);

    // set the next time-chunk to another 10 seconds
    timeOut.tv_sec  = 10;
    timeOut.tv_usec = 0;
  }

#ifdef _WINDOWS
  WSACleanup();
#endif
  fprintf(stderr, "SIGEL_GP::SIG_GPManager::RegisterDynPVMClients -- The server is exiting ! This should never ever happen ! !");
}

SIGEL_GP::SIG_GPManager::~SIG_GPManager()
{
	if(!currentExperiment.mtController->IsEnabled() || currentExperiment.mtController->UsedSystem() != EVALUATOR_SUBST){
		delete trainer;
	}

	// Qt 2's ~QVector freed whatever tournaments were still held. QList does not.
	qDeleteAll( tours );
};


//**************************** META method ************************
void SIGEL_GP::SIG_GPManager::run(MT_Classifier *MetaClassifier)
{
	/*	
		Meta ordering / changes
		0) as for SIGEL ...
		1) createTours
		2) MT_Classifier.preEvolution  --->classifies the SIGEL tournament; may shrink tours(!)
		3) calcInitTourSet()
		4) evalNeededIndis() --> new method: compute fitness only for those SIGEL individuals
		which still appear in tours!
		5) evolutionLoop(MT_Classifier) ------> new method: compute fitness only for those SIGEL individuals
		which still appear in tours! Use MT_Classifier for tournaments!
		6) MT_Classifier.nextSIGGeneration() 
		7) determine average fitness only from exactly computed SIGEL individuals  	
	*/

	/***************************************
	 *  tell the Meta-System to start the evolution
	 ****/

	// start the MT_GP-System only if the SIGEL-GP-System would start
	if(toursAreEmpty( tours ) && currentExperiment.getPopulation().getSize() > 3)
		currentExperiment.mtController->startEvolution();

	/***************************************
	 *  end of Meta-System specific part
	 ****/


#ifdef _WINDOWS
	HANDLE mutex = CreateMutex(NULL, false, NULL);
#else
  pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

   // init the condition variable
#ifdef _WINDOWS
	cond = CreateEvent(NULL, true, false, NULL);
#else
   pthread_cond_init(&cond, NULL);
#endif

    if (!toursAreEmpty( tours ))
    {	SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager::run() was called more than once!" << Qt::endl;
			messageEvolutionStop();
    };

    if (currentExperiment.getPopulation().getSize() < 4)
    {	SIGEL_Tools::SIG_IO::cerr << "Population contains less than 4 Individuals, cannot evolve!" << Qt::endl;
			messageEvolutionStop();
    };

    startTime = QDateTime::currentDateTime();

    for (;;)
      {
#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager entering endless for-loop." << Qt::endl;
#endif

	stopIfNecessary( true );

	if (stopEvolutionNow)
	  return;

	
// NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW
// NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW 
// NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW // NEW NEW NEW NEW NEW 
	// Find the population position of the best SIGEL individual  
	int PosBestSigelIndi =0;
	double BestFitness =-1.0;
	double PresentFitness =0.0;
	
	for(int k=0; k<currentExperiment.population.getSize(); k++)
	{
		PresentFitness= currentExperiment.population.getIndividualPointer(k)->getFitness();
		if(PresentFitness>BestFitness)
		{
			PosBestSigelIndi=k;
			BestFitness = PresentFitness;
		}

	}


	createTours( currentExperiment.gpParameter.getTournamentsPerGeneration() * currentExperiment.getPopulation().getSize() );

	MetaClassifier->preEvolution(&tours, PosBestSigelIndi);
	
	calcInitTourSet();

	evalNeededIndis();

	evolutionLoop(MetaClassifier);

	if (stopEvolutionNow)
	  return;

	currentGenerationNo++;
	currentExperiment.population.poolGeneration++;

	SIGEL_Tools::SIG_IO::cerr << "Computing Generation " << currentExperiment.population.poolGeneration << " (" << currentGenerationNo << " in this run)\t(" << (QDateTime::currentDateTime()).toString() << ")" << Qt::endl;

	for (int i=0; i < currentExperiment.population.getSize(); i++)
	  {
	    currentExperiment.population.getIndividual( i ).increaseAge();
	    updateIndividualView( i );
	  };

	int poolImageGeneration = currentExperiment.getGPParameter().getPoolImageGeneration();

	int poolGenerationNo = currentExperiment.population.poolGeneration;
	QDateTime generationBreak = QDateTime::currentDateTime();
	double bestFitness = currentExperiment.population.getBestFitness( true );
	double minFitness = currentExperiment.population.getWorstFitness( true );

	// double averageFitness = currentExperiment.population.getAverageFitness();
	// In the meta-classifier approach the average fitness is computed over those
	// SIG_Individuals whose fitness was computed exactly, by simulation
	double averageFitness = 0.0;
	double Fitt = 0.0;
	double NumOfCorrectFit = 0.0;

	for ( int i=0; i<currentExperiment.population.getSize(); i++)
	{
		Fitt= currentExperiment.population.getIndividualPointer(i)->getFitness();
		if (Fitt >= 0.0)
		{
			averageFitness = averageFitness + Fitt;
			NumOfCorrectFit++;
		}
	}
	if(NumOfCorrectFit > 0.0)
		averageFitness = averageFitness / NumOfCorrectFit;
	else
		averageFitness = -1000.0;
	
	MetaClassifier->nextSIGGeneration(averageFitness);

  	SIG_GPExperimentHistoryEntry *newExpHistEntry = new SIG_GPExperimentHistoryEntry( poolGenerationNo,
											  generationBreak,
											  bestFitness,
											  minFitness,
											  averageFitness );

	currentExperiment.experimentHistory.append( newExpHistEntry );

	if ( poolImageGeneration && ((currentGenerationNo % poolImageGeneration) == 0) ) {
		QDir poolImageDir = currentExperiment.getGPParameter().getPoolImageDirectory();

	  QString poolImageName =   poolImageDir.path()
	                            + QString( "/" )
                                    + currentExperiment.experimentName
	                            + QString( "_PoolImage_" )
                                    + QDateTime::currentDateTime().toString()
	                            + ".pol";

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager saving poolImage, currentGenerationNo: "
				      << currentGenerationNo
				      << ", filename: "
				      << poolImageName
				      << Qt::endl;
#endif

	  QFile poolImage( poolImageName );

	  if (poolImage.open( QIODeviceBase::WriteOnly )) {
			QTextStream buffer( &poolImage );

			currentExperiment.getPopulation().writeToFile( buffer );
			poolImage.close();
	  }
	  else
	    SIGEL_Tools::SIG_IO::cerr << "Could not save Pool under "
					<< poolImageName
					<< "!" << Qt::endl;
	  };

		// autosave function	
  		// checks whether to save the population or not
		// only save if getAutosave() not zero
		if (currentExperiment.environment.getAutosave()!=0) {
			// only save if the modulo rest is zero
			if ( (currentExperiment.population.poolGeneration%currentExperiment.environment.getAutosave())==0) {
				QFile file( currentExperiment.getPath() );
				if (file.open(QIODeviceBase::WriteOnly)) {
					QTextStream stream(&file);
					currentExperiment.saveExperiment(stream);
					file.close();
				}
				else {
					std::cerr << "could not autosave the experiment!" << endl;
				}
			}
		}

        // do we have a threaded server running for dyn. clients ?
        if (serverIsUp)
        {
           fprintf(stderr, " Releasing Dynamic SIGEL-Clients:\n");

           // make the main thread running exclusively
#ifdef _WINDOWS
				WaitForSingleObject(mutex, INFINITE);
#else
           pthread_mutex_lock( &mutex );
#endif
           fprintf(stderr, "\t- Flushing all dynamic clients from PVM-Hosts list\n");
           trainer->flushAllDynHosts();

           fprintf(stderr, "\t- Asking server to disconnect the clients\n");
           disconnectClients = true;
           allDisconnected = false;

           // let's wait for server thread..
#ifdef _WINDOWS
           while ( ! allDisconnected )

           { WaitForSingleObject(cond, INFINITE);
           }
           ResetEvent(cond);
           ReleaseMutex(mutex);
#else
           while ( ! allDisconnected )
           { pthread_cond_wait(&cond, &mutex);
           }
           pthread_mutex_unlock( &mutex );
#endif

           fprintf(stderr, "\n");
        }

      }
  }

//*****************************************************************
//**************************** META method ************************
//*****************************************************************
void SIGEL_GP::SIG_GPManager::evolutionLoop(MT_Classifier *MetaClassifier)
{
	/**
	*	Changes relative to the "normal" evolutionLoop():
	*	1) actTour.run(MetaClassifier); instead of actTour.run();
	*	2) if a tournament participant takes part in no further tournament,
	*		its fitness no longer needs computing. 
	*/


  int maxTouchsPerLoop = currentExperiment.gpParameter.getMaxTouchsPerLoop();
  int toDoSweepsPerLoop = currentExperiment.gpParameter.getToDoSweepsPerLoop();

  SIG_GPPopulation &pop = currentExperiment.population;  

  while ( !taskCanDoList.isEmpty() )
    {
      stopIfNecessary( false );

      if (stopEvolutionNow)
      	return;

      processInterfaceEvents();

      trainer->sweepToSpawn();

      for (int sweepCounter = 0;
      	   (sweepCounter < toDoSweepsPerLoop) && (!taskCanDoList.isEmpty());
      	   sweepCounter++)
    	{

#ifdef SIG_DEBUG
    	  SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager sweeping the taskCanDoList (sweepCounter: "
    				    << sweepCounter
    				    << ")." << Qt::endl;
#endif

    	  int touchsCounter = 0;

  	  qsizetype canDoIdx = 0;   // was an iterator: Qt 2's list was linked

  	  while (canDoIdx < taskCanDoList.size())
	    {
				// Poll interval: long enough not to spin a core, short enough that
				// finished results do not wait. Interface events are processed after it.
#ifdef _WINDOWS
				Sleep(5);
#else
				usleep(5000);
#endif				
				processInterfaceEvents();

	      if ((touchsCounter == maxTouchsPerLoop) && (maxTouchsPerLoop != -1))
				{  break;
				}

	      SIG_GPTournament &actTour=*tours[ taskCanDoList.at( canDoIdx ) ];
	      int actIndiNumber = actTour.indis.size();

#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "SIG_GPManager inspecting tournament No. "
					<< taskCanDoList.at( canDoIdx )
					<< ".\n"
					<< "justWaiting: "
					<< actTour.justWaiting
					<< "\n"
					<< "depNumber: "
					<< actTour.depNumber
					<< "\n"
					<< "indis:" << Qt::endl;
	      for (int i = 0; i < actTour.indis.size(); i++)
		SIGEL_Tools::SIG_IO::cerr << "  indNumber: "
					  << (*actTour.indis[ i ]).indNumber
					  << "\n"
					  << "    successor: "
					  << (*actTour.indis[ i ]).successor
					  << "\n"
					  << "    fitTaskId: "
					  << (*actTour.indis[ i ]).fitTaskId
					  << Qt::endl;

#endif

	      if (!actTour.justWaiting)
		{
#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "Playing tournament "
					    << taskCanDoList.at( canDoIdx )
					    << Qt::endl;
#endif
//*********************** META change 1)
		  actTour.run(MetaClassifier);

		  for (int i = 0; i < actIndiNumber; i++)
		    {
		      SIG_GPIndividual &actInd = pop.getIndividual( actTour.indis[i]->indNumber );

		      if (actInd.upToDate())
			{
#ifdef SIG_DEBUG
			  SIGEL_Tools::SIG_IO::cerr << "Individual "
						    << actInd.getPoolPos()
						    << " is up to date." << Qt::endl;
#endif

			  int actSuccessor = actTour.indis[i]->successor;
			  if (actSuccessor != -1)
			    {
			      tours[ actSuccessor ]->depNumber -= 1;
			      if( tours[ actSuccessor ]->depNumber == 0 )
				{
				  taskCanDoList << actSuccessor;
				};
			    };
			}
		      else
			{
#ifdef SIG_DEBUG
			  SIGEL_Tools::SIG_IO::cerr << "Individual "
						    << actInd.getPoolPos()
						    << " is not up to date." << Qt::endl;
#endif

//*********************** META change 2) :=  if(actTour.indis[i]->successor != -1)
			  if(actTour.indis[i]->successor != -1)
			  {
				  actTour.justWaiting = true;
				  actTour.indis[i]->fitTaskId = trainer->spawnTask( actInd );
				  updateIndividualView( actTour.indis[i]->indNumber );
			  }

			
			};

		      updateIndividualView( actTour.indis[i]->indNumber );

		    };
		}
	      else
		{
		  actTour.justWaiting = false;

		  for (int j=0; j<actIndiNumber; j++)
		    {
		      SIG_GPTournamentIndividual &actTourInd = *actTour.indis[j];

		      if (actTourInd.fitTaskId != -1)

			{
			  double actFitness = trainer->checkTask( actTourInd.fitTaskId );
			  if (actFitness != -1)
			    {

			      SIG_GPIndividual &actInd = pop.getIndividual( actTourInd.indNumber );
			      actInd.setFitness( actFitness );
			      updateIndividualView( actTourInd.indNumber );
			      actTourInd.fitTaskId = -1;

			      int actSuccessor = actTourInd.successor;
			      if (actSuccessor != -1)
				{
				  tours[ actSuccessor ]->depNumber -= 1;
				  if (tours[ actSuccessor ]->depNumber == 0)
				    taskCanDoList << actSuccessor;
				};
			    }
			  else
			    actTour.justWaiting = true;
			};
		    };
		};
	      if (!actTour.justWaiting)
      		taskCanDoList.removeAt( canDoIdx );   // next slides into canDoIdx
	      else
      		++canDoIdx;

	      touchsCounter++;
	    };
	};
    };
};


void SIGEL_GP::SIG_GPManager::evalNeededIndis()
{
  QList< QList<int> * > fitTaskList;
  FitTaskListGuard fitTaskListGuard{ &fitTaskList };

  //The experiment's population
  SIG_GPPopulation &pop=currentExperiment.population;

  //The population size is determined
  int poolSize=pop.getSize();


// ********************* META change
	QList<int> ToursParticipant;
	ToursParticipant.resize(poolSize); // Position i = number of tournaments individual i takes part in
	for(int l=0; l<poolSize;l++)
		ToursParticipant[l]=0;
int DebugInfo =0;

	SIG_GPTournamentIndividual * PresentIndi =0;
	SIG_GPTournament *PresentTour =0;
	for(int l=0; l<tours.size();l++)
	{	
		PresentTour = tours[l];
		for(int k=0; k<PresentTour->indis.size(); k++)
		{
			PresentIndi = PresentTour->indis[k];
			if(PresentIndi){
				DebugInfo = PresentIndi->indNumber;
				ToursParticipant[DebugInfo]= ToursParticipant[DebugInfo] +1;
			}
				
		}
	
	}
// ********************* End of the meta adaptation


  for (int i=0;i<poolSize;i++)
    {
      stopIfNecessary( true );

      if (stopEvolutionNow)
	return;

      processInterfaceEvents();

      SIG_GPIndividual &actInd=pop.getIndividual(i);

      bool upToDate = actInd.upToDate();

      //Update the fitness values for individuals that are up to date

// ********************* META change - only if the individual appears in the tournament schedule
	  if((!upToDate)&&(ToursParticipant[i]!=0))
		{
		  QList<int> *actFitTask = new QList<int>(2);
		  (*actFitTask)[0] = trainer->spawnTask(actInd);
		  (*actFitTask)[1] = actInd.getPoolPos();

		  fitTaskList.append( actFitTask );
		};
    };

  
  while (!fitTaskList.isEmpty())
  {
	  stopIfNecessary( true );

	if (stopEvolutionNow)
	  return;

	processInterfaceEvents();

	trainer->sweepToSpawn();
	// first(): Qt 2 returned null on empty, Qt 6's first() is UB there.
	qsizetype fitCur = fitTaskList.isEmpty() ? -1 : 0;
	QList<int> *actFitTask = (fitCur < 0) ? 0 : fitTaskList.at( fitCur );
	QList<int> *prevFitTask = 0;

	while (actFitTask)
	  {
	    double actFitness = trainer->checkTask( (*actFitTask)[0] );

		if (actFitness != -1)
	      {

			pop.getIndividual( (*actFitTask)[1] ).setFitness( actFitness );
		updateIndividualView( (*actFitTask)[1] );

		// remove(): setAutoDelete(true) made this the free.
		delete fitTaskList.takeAt( fitCur );
		// Qt 2's QList cursor after remove(): land on whatever slid in, else step back.
		if (fitCur >= fitTaskList.size())
		  fitCur = fitTaskList.isEmpty() ? -1 : fitTaskList.size() - 1;
		actFitTask = (fitCur < 0) ? 0 : fitTaskList.at( fitCur );
		if (actFitTask == prevFitTask)
		  break;
	      }
	    else
	      {


		prevFitTask = actFitTask;
		// next(): a dead cursor stays dead and does NOT advance.
		if (fitCur < 0 || ++fitCur >= fitTaskList.size())
		  { fitCur = -1; actFitTask = 0; }
		else
		  actFitTask = fitTaskList.at( fitCur );
	      };
	  };
   };

  fitnessCalculated = true;

};
