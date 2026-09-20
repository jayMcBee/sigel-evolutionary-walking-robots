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
#include "SIGEL_GP/SIG_GPFitnessFunction.h"

#include <cmath>

SIGEL_GP::SIG_GPFitnessFunction::SIG_GPFitnessFunction(
SIGEL_Program::SIG_Program & pprogram, 
SIGEL_Robot::SIG_Robot & prob,
SIGEL_Environment::SIG_Environment & penvironment, 
SIGEL_Simulation::SIG_SimulationParameters & psimparameter,
QString pname) : 
  program(pprogram),
  rob(prob),
  environment(penvironment),
  simparameter(psimparameter),
  name(pname)
  
{};

SIGEL_GP::SIG_GPFitnessFunction::~SIG_GPFitnessFunction() {};

bool SIGEL_GP::SIG_GPFitnessFunction::isValid( double value )
{
  return ( (value != HUGE_VAL) && (value != (-HUGE_VAL)) && (value==value) );
};

DL_vector SIGEL_GP::SIG_GPFitnessFunction::normalizeRobotPosition( DL_vector originalPosition,
								   DL_matrix actualRobotRotation )
{
  DL_vector robotsRealOrigin = rob.initialLocation;
  robotsRealOrigin.timesis( -1 );

  DL_vector normalizedPosition;

  actualRobotRotation.times( &robotsRealOrigin,
			     &normalizedPosition );

  normalizedPosition.plusis( &originalPosition );

  return normalizedPosition;
};
