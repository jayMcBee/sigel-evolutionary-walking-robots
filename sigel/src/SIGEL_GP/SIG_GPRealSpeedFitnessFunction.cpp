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
#include "SIGEL_GP/SIG_GPRealSpeedFitnessFunction.h"

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Simulation/SIG_Simulation.h"

namespace SIGEL_GP
{

  SIG_GPRealSpeedFitnessFunction::SIG_GPRealSpeedFitnessFunction( SIGEL_Program::SIG_Program &program,
								  SIGEL_Robot::SIG_Robot &robot,
								  SIGEL_Environment::SIG_Environment &environment,
								  SIGEL_Simulation::SIG_SimulationParameters & simulationParameters )
    : SIG_GPFitnessFunction( program,
			     robot,
			     environment,
			     simulationParameters,
			     "RealSpeedFitnessFunction" )
  { };

  SIG_GPRealSpeedFitnessFunction::~SIG_GPRealSpeedFitnessFunction()
  { };

  double SIG_GPRealSpeedFitnessFunction::evalFitness() {
    double const recordingRate = 0.5;

    int const recordingFrequency = int( recordingRate / simparameter.getStepSize() );

    SIGEL_GP::SIG_GPFullDataRecorder recorder( recordingFrequency );

    SIGEL_Simulation::SIG_Simulation *simulation = new SIGEL_Simulation::SIG_Simulation( rob,
         environment,
         program,
         simparameter,
         recorder );

    double fitness = 0;

    try {
      simulation->start();
    }
    catch (SIGEL_Tools::SIG_Exception &e) {
    };

    // delete simulation;

    DL_vector *endPosition = new DL_vector();
    *endPosition = recorder.endPosition;

    DL_matrix *endRotation = new DL_matrix();
    *endRotation = recorder.endRotation;

    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );

    double totalDistance = 0;

    qsizetype recIdx = 0;
    DL_vector *actPosition = recorder.positions.value( recIdx );
    DL_matrix *actRotation = recorder.rotations.value( recIdx );

    DL_vector lastRealPosition = normalizeRobotPosition( *actPosition, *actRotation );

    while (actPosition) {
      DL_vector actRealPosition = normalizeRobotPosition( *actPosition, *actRotation );

      if (isValid( actRealPosition.x ) && isValid( actRealPosition.y ) && isValid( actRealPosition.z )) {
        DL_vector localDistanceVector = actRealPosition;
        localDistanceVector.minusis( &lastRealPosition );

        double localDistance = localDistanceVector.norm();

#ifdef SIG_DEBUG
        SIGEL_Tools::SIG_IO::cerr << "localDistance: "
          << localDistance
          << Qt::endl;
#endif

        totalDistance += localDistance;

        lastRealPosition = actRealPosition;
      }
      else {
        totalDistance = 0;
        break;
      };

      ++recIdx;
      actPosition = recorder.positions.value( recIdx );
      actRotation = recorder.rotations.value( recIdx );
    };

    int simulatedSeconds = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );

    fitness = totalDistance / simulatedSeconds;

    return fitness;
  };

}
