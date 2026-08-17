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

#ifndef SIGEL_SIMULATION_SIG_ERTSIMULATION_H
#define SIGEL_SIMULATION_SIG_ERTSIMULATION_H

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Simulation/SIG_Interpreter.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <math.h>
#include <cmath>


namespace SIGEL_Simulation
{

  /**
   * SIG_EarlyRunTermSimulation inherits from SIG_Simulation and defines
   * a simulation variant where the evaluation of individuals is stopped
   * once their height drops below 1/2 of the start height (regarding the
   * Y-position of the root link)
   */
  class SIG_EarlyRunTermSimulation : public SIG_Simulation
  {

    public:
      /**
       * The constructor of SIG_EarlyRunTermSimulation
       */
       SIG_EarlyRunTermSimulation( SIGEL_Robot::SIG_Robot const & robot,
		      SIGEL_Environment::SIG_Environment const & environment,
		      SIGEL_Program::SIG_Program const & robotProgram,
		      SIG_SimulationParameters const & simulationParameter,
		      SIG_Recorder & theRecorder);

      ~SIG_EarlyRunTermSimulation()     {}

      /**
      * Returns true if the position of our root link
      * drops below 1/2 of the start height.
      */
       bool prematureTermination( void );

       /**
       * For a given frequency of the SIG_GPFullDataRecorder object this
       * method computes the max. number of frames recorded by the recorder
       * using the current max. simulation time as stored in our member
       * 'simulationData'.
       */
       int getMaxRecorderSteps(int inRecFrequency);

       /**
       * Contains the startheight of our robot.
       */
       double ertStartHeight;

       /**
       * Our pointer to the current recorder.
       */
       SIGEL_GP::SIG_GPFullDataRecorder *ertRecorder;

       /**
       * Our pointer to the current robot.
       */
       SIGEL_Robot::SIG_Robot *ertRobot;

       /**
       * Stolen from GP_FitnessFunction.
       */
       protected:
        DL_vector normalizeRobotPosition( DL_vector originalPosition,
				   DL_matrix actualRobotRotation );

    };

}

#endif // SIGEL_SIMULATION_SIG_SIMULATION_H
