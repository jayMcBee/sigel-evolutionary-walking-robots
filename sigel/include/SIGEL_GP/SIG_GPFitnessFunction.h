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
#ifndef SIGEL_GP_SIG_GPFITNESSFUNCTION_H
#define SIGEL_GP_SIG_GPFITNESSFUNCTION_H



#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"


#include <qstring.h>

namespace SIGEL_GP
{

/**
 * This abstract class represents the structure of the different fitnessclasses
 * which computes the fitness of an robot controll program.
 *
 */
class SIG_GPFitnessFunction{

/**
 * The destructor of the fitnessfunction.
 * @pre 
 * The computation of the fitnessvalue is done.
 * @post
 * The object of the fitnessfunction is destructed.
 */
 public:
 virtual ~SIG_GPFitnessFunction();


/**
 * This operation activates the computation of the fitnessvalue. It is virtual, this means 
 * that the implemantation is for every fitnessfunction different. 
 * @pre 
 * An object of the fitnessfunction is created and everything is ready to run.
 * @post
 * The fitnessvalue is computed and returned, the object of the fitness function is destructed.
 * @param program
 * The individual's robot control program.
 * @param rob
 * The robot architecture on which the robot control program is executed.
 * @param environment
 * The environment of the simulation run.
 * @param simparameter
 * Parameters for the simulation run.
 * @return
 * A double is returned, which represents the fitnessvalue. If an error has occurred, the returnvalue of 
 * the double is -1.
 */
 public:
 virtual double evalFitness( SIGEL_Program::SIG_Program &program,
                             SIGEL_Robot::SIG_Robot &rob,
                             SIGEL_Environment::SIG_Environment &environment,
                             SIGEL_Simulation::SIG_SimulationParameters &simparameter ) const = 0;

/**
 * The key of this fitness function in experiment files and in the PVM data.
 */
 public:
 virtual QString serializedId() const = 0;

/**
 * The name the user sees, e.g. "Force".
 */
 public:
 virtual QString name() const = 0;

  void setActGeneration(int _actGeneration) { actGeneration = _actGeneration;  }
  int getActGeneration() { return actGeneration; }
  void setResetEveryGeneration(int _resEvGen) {  resetEveryGeneration = _resEvGen;  }
  int getResetEveryGeneration() { return resetEveryGeneration; }

 protected:
 bool isValid( double value ) const;

 protected:
 DL_vector normalizeRobotPosition( DL_vector originalPosition,  DL_matrix actualRobotRotation, const SIGEL_Robot::SIG_Robot &rob ) const;

 /**
  * This variable holds the actual Generation.
  */
 protected:
  /**
    * this attribute specifies the current generation since this project was created.
    */
  int actGeneration = 0;
  /**
    * Every resetEveryGeneration the fitness value of all individuals will be reseted. Thus there is
    * a new evalution of the fitness value. The fitness function could switch between different criteria
    * to calculate the fitness value. For a example look at the forceFitnessFunction.
    */
  int resetEveryGeneration = 0;

};
}

#endif // SIGEL_GP_SIG_GPFITNESSFUNCTION_H
