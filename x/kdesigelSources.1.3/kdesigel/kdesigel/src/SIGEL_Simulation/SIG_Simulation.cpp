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

#include "SIGEL_Simulation/SIG_DynaMoSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMoSimulationQueries.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Simulation/SIG_DynaMoCommandInterface.h"
#include "SIGEL_Simulation/SIG_DynaMechsCommandInterface.h"
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
    case SIG_SimulationParameters::DynaMo:
      {
	SIG_DynaMoSimulationData *dynaMoSimulationData = new SIG_DynaMoSimulationData( robot,
										       environment,
										       simulationParameter );

	simulationData = dynaMoSimulationData;
	simulationQueries = new SIG_DynaMoSimulationQueries( *dynaMoSimulationData );
	commandInterface = new SIG_DynaMoCommandInterface( *dynaMoSimulationData );

#ifdef SIG_DEBUG 
// dynamic_cast<SIG_DynaMoSimulationQueries*>(simulationQueries)->printDynaDatas();
#endif
	connect( &dynaMoSimulationData->dynaSystem,
		 SIGNAL(signalDynamoMessage(QString)),
		 SLOT(slotDynamoMessage(QString)) );
      };
      break;
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

void SIGEL_Simulation::SIG_Simulation::start()
  throw(SIGEL_Simulation::SIG_SimulationCannotSolveException)
{
  // max is the time which is specified in "Simulation Parameters"-"General Settings"-"Time To Simulate"
  QTime max=simulationData->simulationParameter.getTimeToSimulate();
  QTime act=simulationQueries->getActualSimulationTime();

  // make a timestep in our simulation until simulation time is over
  // or the premature termination method tells us to
  // stop -- inherit class and define this method to do so
  do {
     makeTimeSteps(1);
     act=simulationQueries->getActualSimulationTime();

     // premature means "fuehzeitig"
     if ( prematureTermination() ) {
        break;
     }
  }
  while (act<max);

  recorder.finish();
}


void SIGEL_Simulation::SIG_Simulation::makeTimeSteps(int numTimeSteps)
  throw(SIGEL_Simulation::SIG_SimulationCannotSolveException)
{
  for(int i=0;i<numTimeSteps;i++)  {
      interpreter->interprete( simulationData->simulationParameter.getStepSize() );

      simulationData->setNewFrame( true );
		
      simulationData->simulationProgress();

#ifdef SIG_DEBUG
// dynamic_cast<SIG_DynaMoSimulationQueries*>(simulationQueries)->printDynaDatas();
#endif
      simulationQueries->checkDynas();

      if (stopSimulation)
        throw SIG_SimulationCannotSolveException( __FILE__, __LINE__,
                                                                                                  "Dynamo produced an Cannot Solve Constraints Error" );

      simulationData->actualFrame++;

      recorder.record();
    };
};

void SIGEL_Simulation::SIG_Simulation::slotDynamoMessage(QString theMessage)
{
  // Process theMessage; currently it always aborts
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cout << theMessage;
#endif

  stopSimulation=true;
};


