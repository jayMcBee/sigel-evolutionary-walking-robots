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
   * The individual, which conatins the robot control program.
   *
   */
 protected:
SIGEL_Program::SIG_Program & program;

  /**
   * The robot artichecture on which the robot control program shall be executed.
   *
   */
 protected:
SIGEL_Robot::SIG_Robot & rob;

  /**
   * The environment for the simulation run.
   *
   */
 protected:
SIGEL_Environment::SIG_Environment & environment;

  /**
   * The simulation parameter for the simulation run.
   *
   */
 protected:
SIGEL_Simulation::SIG_SimulationParameters & simparameter;

  /**
   * The name of the fitnessfunction.
   *
   */
 public:
QString const name;


/**
 * The function, which creates the object of a fitnessfunction with the needed data,
 * for the simulationrun.
 * @pre
 * The parameters has to be set, the GPFitnesstrainer takes controll about the
 * PVM-process. 
 * @post
 * An object of the FitnessFunction is created and the parameters are set to the attributes.
 * @param ind
 * The individual, which contains the robot controll program.
 * @param robot
 * The robot architecture of the robot, on which the robot controll
 * program shall be executed
 * @param environment
 * The environment of the simulationrun.
 * @param simparameter
 * Parameters for the simulationrun.
 */
 public: 
 SIG_GPFitnessFunction(SIGEL_Program::SIG_Program & program,
		       SIGEL_Robot::SIG_Robot & rob,
		       SIGEL_Environment::SIG_Environment &   environment,
		       SIGEL_Simulation::SIG_SimulationParameters & simparameter,
		       QString name);

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
 * An object of the fitnessfunction is created, the needed datas are set and everything is ready to run.
 * @post
 * The fitnessvalue is computed and returned, the object of the fitness function is destructed.
 * @return
 * A double is returned, which represents the fitnessvalue. If an error has occurred, the returnvalue of 
 * the double is -1.
 */
 public:
 virtual double evalFitness()=0;

  void setActGeneration(int _actGeneration) { actGeneration = _actGeneration;  }
  int getActGeneration() { return actGeneration; }
  void setResetEveryGeneration(int _resEvGen) {  resetEveryGeneration = _resEvGen;  }
  int getResetEveryGeneration() { return resetEveryGeneration; }

 protected:
 bool isValid( double value );

 protected:
 DL_vector normalizeRobotPosition( DL_vector originalPosition,  DL_matrix actualRobotRotation );

 /**
  * This variable holds the actual Generation.
  */
 protected:
  /**
    * this attribute specifies the current generation since this project was created.
    */
  int actGeneration;
  /**
    * Every resetEveryGeneration the fitness value of all individuals will be reseted. Thus there is
    * a new evalution of the fitness value. The fitness function could switch between different criteria
    * to calculate the fitness value. For a example look at the forceFitnessFunction.
    */
  int resetEveryGeneration;

};
}

#endif // SIGEL_GP_SIG_GPFITNESSFUNCTION_H
