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

#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Simulation/SIG_DynaMechsCommandInterface.h"

SIGEL_Simulation::SIG_Simulation::SIG_Simulation(SIGEL_Robot::SIG_Robot const & robot,
						 SIGEL_Environment::SIG_Environment const & environment,
						 SIGEL_Program::SIG_Program const & robotProgram,
						 SIG_SimulationParameters const & simulationParameter,
						 SIG_Recorder & theRecorder) :
  recorder(theRecorder)
{
  simulationData = std::make_unique< SIG_DynaMechsSimulationData >( robot, environment, simulationParameter );
  simulationQueries = std::make_unique< SIG_DynaMechsSimulationQueries >( *simulationData );
  commandInterface = std::make_unique< SIG_DynaMechsCommandInterface >( *simulationData );

  recorder.setSimulationQueries( *simulationQueries );
  recorder.init();

  interpreter = std::make_unique< SIG_Interpreter >( *robot.getLangParam(),
						     robotProgram,
						     *commandInterface,
						     *simulationQueries );

};

SIGEL_Simulation::SIG_Simulation::~SIG_Simulation() = default;

// The boundary below is deliberate. Every caller is a fitness function that
// catches SIG_Exception, so an escaping one becomes a wrong fitness, not a crash.
void SIGEL_Simulation::SIG_Simulation::start()
{
  try {
  // max is the time which is specified in "Simulation Parameters"-"General Settings"-"Time To Simulate"
  QTime max=simulationData->simulationParameter.getTimeToSimulate();
  QTime act=simulationQueries->getCurrentSimulationWholeSeconds();

  // make a timestep in our simulation until simulation time is over
  // or the premature termination method tells us to
  // stop -- inherit class and define this method to do so
  do {
     makeTimeSteps(1);
     act=simulationQueries->getCurrentSimulationWholeSeconds();

     // premature means "early"
     if ( prematureTermination() ) {
        break;
     }
  }
  while (act<max);

  recorder.finish();
  }
  catch (...) {
    // Any exception terminates, deliberately: the alternative is a fitness
    // function silently scoring a partial run.
    std::terminate();
  }
};


// The boundary below is deliberate. Every caller is a fitness function that
// catches SIG_Exception, so an escaping one becomes a wrong fitness, not a crash.
void SIGEL_Simulation::SIG_Simulation::makeTimeSteps(int numTimeSteps)
{
  try {
  for(int i=0;i<numTimeSteps;i++)  {
      interpreter->interprete( simulationData->simulationParameter.getStepSize() );

      simulationData->simulationProgress();

      simulationQueries->checkDynas();

      simulationData->actualFrame++;

      recorder.record();
    };
  }
  catch (...) {
    // Any exception terminates, deliberately: the alternative is a fitness
    // function silently scoring a partial run.
    std::terminate();
  }
};
