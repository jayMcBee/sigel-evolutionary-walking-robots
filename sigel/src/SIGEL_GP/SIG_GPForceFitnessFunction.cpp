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
#include "SIGEL_GP/SIG_GPForceFitnessFunction.h"

#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Link.h"

#include <math.h>
#include <qdatetime.h>

double SIGEL_GP::SIG_GPForceFitnessFunction::evalFitness( SIGEL_Program::SIG_Program &program,
                                                          SIGEL_Robot::SIG_Robot &rob,
                                                          SIGEL_Environment::SIG_Environment &environment,
                                                          SIGEL_Simulation::SIG_SimulationParameters &simparameter ) const {
  SIGEL_GP::SIG_GPFullDataRecorder recorder(1);

  SIGEL_Simulation::SIG_Simulation simulation( rob, environment, program, simparameter, recorder );

  // For the NiceWalkingFitnessFunction
  double const toleranceBandWidth = 0.5;

  double fitness = 0;
  vector<double> absDeviations;
  unsigned int frames;
  double momentX, momentY, momentZ;

  // start the simulation
  try {
    simulation.start();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    SIGEL_Tools::SIG_IO::cerr << e.getMessage() << Qt::flush;
  };

    SIG_Vector realStartPosition = normalizeRobotPosition( *recorder.positions.value( 0 ),  *recorder.rotations.value( 0 ), rob );
    SIG_Vector realEndPosition = normalizeRobotPosition( recorder.endPosition, recorder.endRotation, rob );

    double const optimalHeight = realStartPosition.y;
    double const minimalHeight = optimalHeight - toleranceBandWidth;
    double const maximalHeight = optimalHeight + toleranceBandWidth;

    SIG_Vector distanceVector = realStartPosition;
    distanceVector.minusis( &realEndPosition );

    double distance = distanceVector.norm();

    int simulatedSeconds = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );

    double fitnessGes = distance / simulatedSeconds;

// If the fitness value was reset, compute the force
//  if ( (resetEveryGeneration != 0) && (actGeneration%(resetEveryGeneration*5) < resetEveryGeneration)) {

      // The list listForces runs over the individual frames
      // the list holds arrays containing the 6-dimensional vector for each individual joint
      qsizetype forceIdx = 0;
      vector<double*>* usedForce = recorder.listForces.value( forceIdx );

      // count() was uint and is now qsizetype, so this narrows where it did
      // not. The value is identical either way -- empty gives 4294967295 in
      // both, n>=1 gives n-1 -- and the cast says the narrowing is meant.
      frames = static_cast< unsigned int >( recorder.listForces.count() ) - 1;

      // The first value is always garbage, so take the second from the list straight away
      // The while loop always fetches the next frame
      while ( (usedForce = recorder.listForces.value( ++forceIdx )) != nullptr ) {
        double momentMagnitude = 0;
        double averageMomentPerJoint = 0;
        vector<double> momentMagnitudes;
        // usedForce.size() gives the number of joints
        for (unsigned int i=0;i<(*usedForce).size();++i) {
          momentX = (*usedForce)[i][0];
          momentY = (*usedForce)[i][1];
          momentZ = (*usedForce)[i][2];
          momentMagnitude = sqrt( pow(momentX,2) + pow(momentY,2) + pow(momentZ,2) );
          momentMagnitudes.push_back(momentMagnitude);

          // Compute the average force per joint
          averageMomentPerJoint += momentMagnitude/((*usedForce).size());
        } // end of for loop

        double absDeviationSum = 0;
        for (unsigned int i=0;i<(*usedForce).size();++i) {
          absDeviationSum += fabs(momentMagnitudes[i]-averageMomentPerJoint);
        }
        absDeviations.push_back(absDeviationSum);
      } // end of while loop [fetch next frame]

      // The mean over all frames, divided by the average speed
      for (unsigned int i=0;i<(absDeviations.size());++i) {
        fitness += absDeviations[i]/(frames*distance)*simulatedSeconds;
      }

     if (finite(fitness)!=0) {
        // Compute the reciprocal (guarding against division by zero), since this is a maximisation problem
        fitness!=0 ? fitness = 1/fitness : fitness=0;
      }
      else {
        // If the force is infinite or NaN, the fitness value is zero
        fitness = 0;
      }

//  } // large if block
//  else { // otherwise compute the velocity
// I moved part of this block further up

    SIG_Vector *endPosition = new SIG_Vector();
    *endPosition = recorder.endPosition;

    SIG_Matrix *endRotation = new SIG_Matrix();
    *endRotation = recorder.endRotation;

    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );

    qsizetype recIdx = 0;
    SIG_Vector *actPosition = recorder.positions.value( recIdx );
    SIG_Matrix *actRotation = recorder.rotations.value( recIdx );

    while (actPosition) {
      SIG_Vector actRealPosition = normalizeRobotPosition( *actPosition, *actRotation, rob );

      if ( !(isValid( actRealPosition.x ) && isValid( actRealPosition.y ) && isValid( actRealPosition.z )) ) {
        fitness = 0;
        break;
      };

      if ((actRealPosition.y < minimalHeight) || (actRealPosition.y > maximalHeight)) {
        fitness = 0;
        break;
      };

      ++recIdx;
      actPosition = recorder.positions.value( recIdx );
      actRotation = recorder.rotations.value( recIdx );
    };
//  }

  // now we have to clean up the memory
  // the forces-Array are created with "new" in dmArticulation::getForces()
  for (vector<double*> *usedForceLoes : recorder.listForces) {
    for (unsigned int i=0; i<usedForceLoes->size(); ++i)
      delete[] (*usedForceLoes)[i];
    delete usedForceLoes;
  }

  return fitness;
};
