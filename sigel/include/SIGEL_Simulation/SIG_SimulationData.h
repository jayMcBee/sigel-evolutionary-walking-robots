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
#ifndef SIGEL_SIMULATION_SIG_SIMULATIONDATA_H
#define SIGEL_SIMULATION_SIG_SIMULATIONDATA_H

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

namespace SIGEL_Simulation
{
  
  /**
   * This Class is used to wrap all relevant data for the simulation.
   * 
   * It contains the robot, the environment, the dynasystem and the parameters
   * as well as the timeframe. All these objects are public. So one has to
   * access them with this class.
   */
  class SIG_SimulationData
    { 
    public:
      /**
       * The robot which is simulated.
       *
       * It can be accessed via this variable.
       */
      SIGEL_Robot::SIG_Robot const & robot;

      /**
       * The environment in which the simulation takes place.
       *
       * It can be accessed via this variable.
       */
      SIGEL_Environment::SIG_Environment const & environment;

      /**
       * The Parameters of the simulation.
       *
       * They can be accessed with help of this variable.
       */
      SIG_SimulationParameters const & simulationParameter;

      /**
       * The number of the frame that is actually simulated. Is initially 0.
       */
      int actualFrame;

      /**
       * Constructor for SIG_SimulationData
       *
       * SIG_SimulationData is constructed by the SIG_Simulation object.
       * @param robot The robot object which is simulated is passed to the
       *              SIG_SimulationData object from the SIG_Simulation object.
       * @param environment The environment in which the robot is simulated.
       *                    It is passed to the SIG_SimulationData object
       *                    from the SIG_Simulation object.
       * @param simulationParameter The parameters which specify the circumstances under
       *                            which the simulation takes place. Passed from the
       *                            SIG_Simulation object.
       */
      SIG_SimulationData( SIGEL_Robot::SIG_Robot const & robot,
			  SIGEL_Environment::SIG_Environment const & environment,
			  SIG_SimulationParameters const & simulationParameter);

      virtual void simulationProgress() = 0;

      virtual void setNewFrame( bool newValue ) = 0;

    };
  
}

#endif // SIGEL_SIMULATION_SIG_SIMULATIONDATA_H

