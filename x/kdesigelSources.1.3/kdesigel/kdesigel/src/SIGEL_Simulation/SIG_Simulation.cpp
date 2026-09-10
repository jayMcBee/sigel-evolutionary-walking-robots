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
#include "SIGEL_Simulation/SIG_Simulation.h"
#include <exception>
#include <iostream>

#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Simulation/SIG_DynaMechsCommandInterface.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_IO.h"

SIGEL_Simulation::SIG_Simulation::SIG_Simulation(SIGEL_Robot::SIG_Robot const & robot,
						 SIGEL_Environment::SIG_Environment const & environment,
						 SIGEL_Program::SIG_Program const & robotProgram,
						 SIG_SimulationParameters const & simulationParameter,
						 SIG_Recorder & theRecorder) :
  recorder(theRecorder),
  stopSimulation(false),
  QObject()
{
  switch (simulationParameter.getSimulationLibrary())
    {
    case SIG_SimulationParameters::DynaMechs:
      {
	SIG_DynaMechsSimulationData *dynaMechsSimulationData = new SIG_DynaMechsSimulationData( robot,
												environment,
												simulationParameter );

	simulationData = dynaMechsSimulationData;
	simulationQueries = new SIG_DynaMechsSimulationQueries( *dynaMechsSimulationData );
	commandInterface = new SIG_DynaMechsCommandInterface( *dynaMechsSimulationData );
      };
      break;

    // The Dynamo backend was deleted on 2026-08-28 -- physics_backends.md.
    // SIMULATIONLIBRARY 0 therefore names a simulator that no longer exists,
    // and this case must not be allowed to fall through: the three interface
    // pointers below would stay uninitialised, and silently constructing a
    // DynaMechs simulation instead would answer with a fitness from a
    // different physics engine than the file asked for. All 14 shipped
    // experiments carry SIMULATIONLIBRARY 1.
    //
    // The printed line is load-bearing, not decoration, and an earlier
    // version of this comment had the reason backwards. The throw does clear
    // all six fitness functions, which construct SIG_Simulation OUTSIDE their
    // own try block -- but one frame further out sigel_slave.cpp:361-367
    // wraps evalFitness() in catch (SIG_Exception &) { fitnessValue = 0; }.
    // So under PVM the throw is swallowed and the individual scores 0.0 as
    // though it had been evaluated, which is exactly the failure
    // SIG_GPSimpleRecorder.cpp, init describes. The message is then the only
    // evidence that reaches anyone. It goes to std::cerr rather than
    // SIG_IO::cerr because SIG_IO buffers and flushes on destruction
    // (PORTING.md 10). Under sigel_eval, which has no such catch, the throw
    // reaches terminate() and aborts.
    default:
      std::cerr << "SIG_Simulation: SIMULATIONLIBRARY "
		<< static_cast<int>( simulationParameter.getSimulationLibrary() )
		<< " selects the Dynamo backend, which was removed. Only"
		   " SIMULATIONLIBRARY 1 (DynaMechs) is supported."
		<< std::endl;
      throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__,
					"SIMULATIONLIBRARY selects the removed Dynamo"
					" backend; only DynaMechs (SIMULATIONLIBRARY 1)"
					" is supported" );
    };

  recorder.setSimulationQueries( *simulationQueries );
  recorder.init();

  interpreter = new SIG_Interpreter( *robot.getLangParam(),
				     robotProgram,
				     *commandInterface,
				     *simulationQueries );

};

SIGEL_Simulation::SIG_Simulation::~SIG_Simulation()
{ };

// NOTE: in 2003 this carried throw(SIG_SimulationCannotSolveException).
// C++17 removed dynamic exception specifications, but removing it outright
// would change behaviour: SIG_Recorder and the simulation backend can throw
// other SIG_Exception subclasses through this frame, which the old
// specification
// turned into terminate(). Every caller is a GP fitness function that does
// catch (SIG_Exception &) { }, so without the boundary those become a
// silently wrong fitness value instead of a crash. The guarantee is kept
// explicitly below.
void SIGEL_Simulation::SIG_Simulation::start()
{
  try {
  // max is the time which is specified in "Simulation Parameters"-"General Settings"-"Time To Simulate"
  QTime max=simulationData->simulationParameter.getTimeToSimulate();
  QTime act=simulationQueries->getActualSimulationTime();

  // make a timestep in our simulation until simulation time is over
  // or the premature termination method tells us to
  // stop -- inherit class and define this method to do so
  do {
     makeTimeSteps(1);
     act=simulationQueries->getActualSimulationTime();

     // premature means "early"
     if ( prematureTermination() ) {
        break;
     }
  }
  while (act<max);

  recorder.finish();
  }
  catch (SIGEL_Simulation::SIG_SimulationCannotSolveException &) {
    throw;                     // the one type the 2003 specification allowed
  }
  catch (...) {
    // Anything else reached terminate() in 2003. Preserved deliberately: the
    // alternative is a fitness function silently scoring a partial run.
    std::terminate();
  }
};


// NOTE: in 2003 this carried throw(SIG_SimulationCannotSolveException).
// C++17 removed dynamic exception specifications, but removing it outright
// would change behaviour: SIG_Recorder and the simulation backend can throw
// other SIG_Exception subclasses through this frame, which the old
// specification
// turned into terminate(). Every caller is a GP fitness function that does
// catch (SIG_Exception &) { }, so without the boundary those become a
// silently wrong fitness value instead of a crash. The guarantee is kept
// explicitly below.
void SIGEL_Simulation::SIG_Simulation::makeTimeSteps(int numTimeSteps)
{
  try {
  for(int i=0;i<numTimeSteps;i++)  {
      interpreter->interprete( simulationData->simulationParameter.getStepSize() );

      simulationData->setNewFrame( true );
		
      simulationData->simulationProgress();

      simulationQueries->checkDynas();

      // UNREACHABLE since 2026-08-28. slotDynamoMessage was the only writer
      // of stopSimulation and the Dynamo signal that invoked it is deleted,
      // so this is the sole throw site of SIG_SimulationCannotSolveException
      // in the tree and it can no longer fire. Left in place: removing it
      // would change the exception surface of a class Phase C still has to
      // port. See physics_backends.md, "Dead but not deleted".
      if (stopSimulation)
        throw SIG_SimulationCannotSolveException( __FILE__, __LINE__,
                                                                                                  "Dynamo produced an Cannot Solve Constraints Error" );

      simulationData->actualFrame++;

      recorder.record();
    };
  }
  catch (SIGEL_Simulation::SIG_SimulationCannotSolveException &) {
    throw;                     // the one type the 2003 specification allowed
  }
  catch (...) {
    // Anything else reached terminate() in 2003. Preserved deliberately: the
    // alternative is a fitness function silently scoring a partial run.
    std::terminate();
  }
};

void SIGEL_Simulation::SIG_Simulation::slotDynamoMessage(QString theMessage)
{
  // Process theMessage; currently it always aborts
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cout << theMessage;
#endif

  stopSimulation=true;
};


