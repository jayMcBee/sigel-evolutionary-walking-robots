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
#ifndef SIGEL_GP_SIG_GPSTEPPERFITNESSFUNCTION_H
#define SIGEL_GP_SIG_GPSTEPPERFITNESSFUNCTION_H

#include "SIGEL_GP/SIG_GPFitnessFunction.h"

namespace SIGEL_GP
{
/**
 * This fitnessfunction is designed for legged (especially humanoid, 2 leg) robots to
 * encourage the use of their feet to evolve a gait.
 */
  class SIG_GPStepperFitnessFunction : public SIG_GPFitnessFunction
    {

    public:

      SIG_GPStepperFitnessFunction(SIGEL_Program::SIG_Program &program,
				      SIGEL_Robot::SIG_Robot &robot,
				      SIGEL_Environment::SIG_Environment &environment,
				      SIGEL_Simulation::SIG_SimulationParameters & simulationParameters );

      ~SIG_GPStepperFitnessFunction();

      double evalFitness();

    };

}

#endif // SIGEL_GP_SIG_GPSTEPPERFITNESSFUNCTION_H
