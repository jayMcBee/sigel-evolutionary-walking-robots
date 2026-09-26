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
#include "SIGEL_GP/SIG_GPSimpleFitnessFunction.h"

#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_GP/SIG_GPSimpleRecorder.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <qdatetime.h>

SIGEL_GP::SIG_GPSimpleFitnessFunction::~SIG_GPSimpleFitnessFunction()
{ };

double SIGEL_GP::SIG_GPSimpleFitnessFunction::evalFitness( SIGEL_Program::SIG_Program &program,
                                                           SIGEL_Robot::SIG_Robot &rob,
                                                           SIGEL_Environment::SIG_Environment &environment,
                                                           SIGEL_Simulation::SIG_SimulationParameters &simparameter ) const {
  SIGEL_GP::SIG_GPSimpleRecorder recorder;

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

  DL_vector startPosition = normalizeRobotPosition( recorder.start, recorder.startRotation, rob );

  DL_vector endPosition = normalizeRobotPosition( recorder.end, recorder.endRotation, rob );

  if (isValid( endPosition.x ) && isValid( endPosition.y ) && isValid( endPosition.z )) {
    DL_vector distanceVector = endPosition;

    distanceVector.minusis( &startPosition );

    double distance = distanceVector.norm();

    int simulatedSeconds = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );

    fitness = distance / simulatedSeconds;
  }
  else
    fitness = 0;

  return fitness;
};
