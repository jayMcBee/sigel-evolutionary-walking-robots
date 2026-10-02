/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_SIMULATION_SIG_JOINTMOBILITY_H
#define SIGEL_SIMULATION_SIG_JOINTMOBILITY_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

namespace SIGEL_Simulation
{

  /**
   * How easily a torque on a robot's joints accelerates them: the largest
   * eigenvalue of the joints' part of the inverse mass matrix, over the start
   * pose and many poses inside the joint ranges. It is 0 for a robot that has
   * no joints or a joint that does not turn. The robot must be prepared for
   * the simulation.
   */
  class SIG_JointMobility
  {

  public:

    SIG_JointMobility( const SIGEL_Robot::SIG_Robot &preparedRobot,
                       const SIGEL_Environment::SIG_Environment &environment,
                       const SIG_SimulationParameters &simulationParameter );

    double getLargest() const;

  private:

    // The start pose and this many random poses inside the joint ranges.
    static constexpr int randomPoses = 150;

    double largest;

  };

}

#endif // SIGEL_SIMULATION_SIG_JOINTMOBILITY_H
