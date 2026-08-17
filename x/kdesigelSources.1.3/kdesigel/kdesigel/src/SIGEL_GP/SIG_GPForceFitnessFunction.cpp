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

SIGEL_GP::SIG_GPForceFitnessFunction::SIG_GPForceFitnessFunction(SIGEL_Program::SIG_Program & program,
      SIGEL_Robot::SIG_Robot & rob,
      SIGEL_Environment::SIG_Environment & environment,
      SIGEL_Simulation::SIG_SimulationParameters & simparameter)
 : SIG_GPFitnessFunction(program,
      rob,
      environment,
      simparameter,
      "ForceFitnessFunction")
{
};

SIGEL_GP::SIG_GPForceFitnessFunction::~SIG_GPForceFitnessFunction() {

};

double SIGEL_GP::SIG_GPForceFitnessFunction::evalFitness() {
  SIGEL_GP::SIG_GPFullDataRecorder recorder(1);

  SIGEL_Simulation::SIG_Simulation *simulation = new SIGEL_Simulation::SIG_Simulation( rob, environment, program, simparameter, recorder );

  // fuer die NiceWalkingFitnessFunction
  double const toleranceBandWidth = 0.5;

  double fitness = 0;
  vector<double> variance;
  unsigned int frames;
  double momentX, momentY, momentZ;

  // start the simulation
  try {
    simulation->start();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
  };

  // delete simulation;

    DL_vector realStartPosition = normalizeRobotPosition( *recorder.positions.first(),  *recorder.rotations.first() );
    DL_vector realEndPosition = normalizeRobotPosition( recorder.endPosition, recorder.endRotation );

    double const optimalHeight = realStartPosition.y;
    double const minimalHeight = optimalHeight - toleranceBandWidth;
    double const maximalHeight = optimalHeight + toleranceBandWidth;

    DL_vector distanceVector = realStartPosition;
    distanceVector.minusis( &realEndPosition );

    double distance = distanceVector.norm();

    int simulatedSeconds = QTime().secsTo( simparameter.getTimeToSimulate() );

    double fitnessGes = distance / simulatedSeconds;

// falls der Fitnesswert resetet wurde, dann berechne die Kraft
//  if ( (resetEveryGeneration != 0) && (actGeneration%(resetEveryGeneration*5) < resetEveryGeneration)) {

      // die Liste listForces geht über die verschiedenen frames
      // innerhalb der Liste stehen arrays, die den 6-dim Vektor für jedes einzelne Gelenk beinhalten
      vector<double*>* usedForce = recorder.listForces.first() ;

      frames = recorder.listForces.count()-1;

      // der erste wert ist immer mist, somit hol ich mir gleich den zweiten aus der liste
      // while schleife holt immer den naechsten Frame
      while ( (usedForce = recorder.listForces.next()) != 0 ) {
        double betrag = 0;
        double durchschnittProGelenk = 0;
        vector<double> betraege;
        // usedForce.size() gibt die Anzahl der Gelenke an
        for (unsigned int i=0;i<(*usedForce).size();++i) {
          momentX = (*usedForce)[i][0];
          momentY = (*usedForce)[i][1];
          momentZ = (*usedForce)[i][2];
          betrag = sqrt( pow(momentX,2) + pow(momentY,2) + pow(momentZ,2) );
          betraege.push_back(betrag);

          // berechne die durchschnittliche Kraft pro Gelenk
          durchschnittProGelenk += betrag/((*usedForce).size());
        } // ende for-schleife

        double varianz = 0;
        for (unsigned int i=0;i<(*usedForce).size();++i) {
          varianz += fabs(betraege[i]-durchschnittProGelenk);
        }
        variance.push_back(varianz);
      } // ende while-schleife [hole naechsten Frame]

      // den Durchschnitt der Varianz über alle Frames
      for (unsigned int i=0;i<(variance.size());++i) {
        fitness += variance[i]/(frames*distance)*simulatedSeconds;
      }

     if (finite(fitness)!=0) {
        // Reziprokwert berechnen (falls nicht durch 0 geteilt wird), da Maximierungsproblem
        fitness!=0 ? fitness = 1/fitness : fitness=0;
      }
      else {
        // falls Kraft Unendlich oder NaN - Fitnesswert null
        fitness = 0;
      }

//  } // grosse if schleife
//  else { // ansonsten berechne die Geschwindgkeit
// einen Teil des Blocks habe ich nach oben verschoben

    DL_vector *endPosition = new DL_vector();
    *endPosition = recorder.endPosition;

    DL_matrix *endRotation = new DL_matrix();
    *endRotation = recorder.endRotation;

    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );

    DL_vector *actPosition = recorder.positions.first();
    DL_matrix *actRotation = recorder.rotations.first();

    while (actPosition) {
      DL_vector actRealPosition = normalizeRobotPosition( *actPosition, *actRotation );

      if ( !(isValid( actRealPosition.x ) && isValid( actRealPosition.y ) && isValid( actRealPosition.z )) ) {
        fitness = 0;
        break;
      };

      if ((actRealPosition.y < minimalHeight) || (actRealPosition.y > maximalHeight)) {
        fitness = 0;
        break;
      };

      actPosition = recorder.positions.next();
      actRotation = recorder.rotations.next();
    };
//  }

  // now we have to clean up the memory
  // the forces-Array are created with "new" in dmArticulation::getForces()
  vector<double*>* usedForceLoes = recorder.listForces.first() ;
  do {
   for (unsigned int i=0; i<usedForceLoes->size(); ++i) {
     delete[] (*usedForceLoes)[i];
   }
   delete usedForceLoes;
  } while ( (usedForceLoes = recorder.listForces.next()) != 0 );

  return fitness;
};
