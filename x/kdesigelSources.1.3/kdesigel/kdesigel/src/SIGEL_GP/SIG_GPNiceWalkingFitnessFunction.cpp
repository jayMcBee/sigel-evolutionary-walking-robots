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
#include "SIGEL_GP/SIG_GPNiceWalkingFitnessFunction.h"

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Simulation/SIG_Simulation.h"

namespace SIGEL_GP
{

  SIG_GPNiceWalkingFitnessFunction::SIG_GPNiceWalkingFitnessFunction( SIGEL_Program::SIG_Program &program,
								  SIGEL_Robot::SIG_Robot &robot,
								  SIGEL_Environment::SIG_Environment &environment,
								  SIGEL_Simulation::SIG_SimulationParameters & simulationParameters )
    : SIG_GPFitnessFunction( program,
			     robot,
			     environment,
			     simulationParameters,
			     "NiceWalkingFitnessFunction" )
  { };

  SIG_GPNiceWalkingFitnessFunction::~SIG_GPNiceWalkingFitnessFunction()
  { };

  double SIG_GPNiceWalkingFitnessFunction::evalFitness()
  {
    double const toleranceBandWidth = 0.5;

    SIGEL_GP::SIG_GPFullDataRecorder recorder( 1 );

    SIGEL_Simulation::SIG_Simulation *simulation = new SIGEL_Simulation::SIG_Simulation( rob,
											 environment,
											 program,
											 simparameter,
											 recorder );

    double fitness = 0;

    try
      {
	simulation->start();
      }
    catch (SIGEL_Tools::SIG_Exception &e)
      { };

    // delete simulation;

    DL_vector realStartPosition = normalizeRobotPosition( *recorder.positions.first(),
							  *recorder.rotations.first() );

    DL_vector realEndPosition = normalizeRobotPosition( recorder.endPosition,
							recorder.endRotation );

    double const optimalHeight = realStartPosition.y;

    double const minimalHeight = optimalHeight - toleranceBandWidth;
    double const maximalHeight = optimalHeight + toleranceBandWidth;

    DL_vector distanceVector = realStartPosition;
    distanceVector.minusis( &realEndPosition );

    double distance = distanceVector.norm();

    int simulatedSeconds = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );

    fitness = distance / simulatedSeconds;

    DL_vector *endPosition = new DL_vector();
    *endPosition = recorder.endPosition;

    DL_matrix *endRotation = new DL_matrix();
    *endRotation = recorder.endRotation;

    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );

    DL_vector *actPosition = recorder.positions.first();
    DL_matrix *actRotation = recorder.rotations.first();

    while (actPosition)
      {
	DL_vector actRealPosition = normalizeRobotPosition( *actPosition,
							    *actRotation );

	if ( !(isValid( actRealPosition.x ) && isValid( actRealPosition.y ) && isValid( actRealPosition.z )) )
	  {
	    fitness = 0;
	    break;
	  };

	if ((actRealPosition.y < minimalHeight) || (actRealPosition.y > maximalHeight))
	  {
	    fitness = 0;
	    break;
	  };

	actPosition = recorder.positions.next();
	actRotation = recorder.rotations.next();
      };

    return fitness;
  };

}
