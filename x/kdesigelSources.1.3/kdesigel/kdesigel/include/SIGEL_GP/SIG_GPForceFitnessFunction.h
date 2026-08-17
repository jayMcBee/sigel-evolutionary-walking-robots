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
#ifndef SIGEL_GP_SIG_GPFORCEFITNESSFUNCTION_H
#define SIGEL_GP_SIG_GPFORCEFITNESSFUNCTION_H


#include "SIGEL_GP/SIG_GPFitnessFunction.h"

namespace SIGEL_GP
{

/**
 * This abstract class represents the structure of the different fitnessclasses
 * which computes the fitness of an robot controll programm.
 *
 */
class SIG_GPForceFitnessFunction : public SIG_GPFitnessFunction
{

/**
 * The function, which creates the object of a fitnessfunction with the needed data,
 * for the simulationrun.
 * @pre
 * The parameters has to be set, the GPFitnesstrainer takes controll about the
 * PVM-Prozess.
 * @post
 * An object of the FitnessFunction is created and the parameters are set to the attributes.
 * @param ind
 * The individual, which contains the robot controll programm.
 * @param robot
 * The robot architecture of the robot, on which the robot controll
 * programm shall be executed
 * @param environment
 * The environment of the simulationrun.
 * @param simparameter
 * Parameters for the simulationrun.
  */
 public:
  SIG_GPForceFitnessFunction(SIGEL_Program::SIG_Program & program,
			      SIGEL_Robot::SIG_Robot & rob,
			      SIGEL_Environment::SIG_Environment &  environment,
			      SIGEL_Simulation::SIG_SimulationParameters & simparameter);

/**
 * The destructor of the fitnessfunction.
 * @pre
 * The computation of the fitnessvalue is done.
 * @post
 * The object of the fitnessfunction is destructed.
 */
 public:
virtual ~SIG_GPForceFitnessFunction();


/**
 * This operation activates the computation of the fitnessvalue. It is virtual, this means
 * that the implemantation is for every fitnessfunction different.
 * @pre
 * An object of the fitnessfunction is created, the needed datas are set and everything is ready to run.
 * @post
 * The fitnessvalue is computed and returned, the object of the fitness function is destructed.
 * @return
 * A double is returned, which represents the fitnessvalue. If an error has occured, the returnvalue of
 * the double is -1.
 */
public:
double evalFitness();

};
}

#endif // SIGEL_GP_SIG_GPFORCEFITNESSFUNCTION_H
