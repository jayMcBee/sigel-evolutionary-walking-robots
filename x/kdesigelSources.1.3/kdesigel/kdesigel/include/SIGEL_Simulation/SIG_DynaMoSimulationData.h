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
#ifndef SIGEL_SIMULATION_SIG_DYNAMOSIMULATIONDATA_H
#define SIGEL_SIMULATION_SIG_DYNAMOSIMULATIONDATA_H

#include "SIGEL_Simulation/SIG_SimulationData.h"
#include "SIGEL_Simulation/SIG_DynaSystem.h"

namespace SIGEL_Simulation
{
  
  /**
   * This Class is used to wrap all relevant data for the simulation.
   * 
   * It contains the robot, the environment, the dynasystem and the parameters
   * as well as the timeframe. All these objects are public. So one has to
   * access them with this class.
   */
  class SIG_DynaMoSimulationData : public SIG_SimulationData
    { 

    public:

      /**
       * The dynamic system of the simulation.
       *
       * This is the only object of the simulation that really changes. It holds all
       * objects in an abstract form (points, orientation of the points, inertia tensors, etc.).
       */
      SIG_DynaSystem dynaSystem;

      /**
       * Constructor for SIG_DynaMoSimulationData
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
      SIG_DynaMoSimulationData( SIGEL_Robot::SIG_Robot const & robot,
				SIGEL_Environment::SIG_Environment const & environment,
				SIG_SimulationParameters const & simulationParameter);

      void simulationProgress();

      void setNewFrame( bool newValue );

      int gptestvalue;
    };
  
}

#endif // SIGEL_SIMULATION_SIG_DYNAMOSIMULATIONDATA_H

