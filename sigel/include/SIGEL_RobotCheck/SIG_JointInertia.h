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
#ifndef SIGEL_ROBOTCHECK_SIG_JOINTINERTIA_H
#define SIGEL_ROBOTCHECK_SIG_JOINTINERTIA_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

namespace SIGEL_RobotCheck
{

  /**
   * The smallest inertia that a motion of a robot's joints has to turn, with
   * the rest of the robot free to move: the inverse of the largest eigenvalue
   * of the joints' part of the inverse mass matrix, over the start pose and
   * many poses inside the joint ranges. The robot must be prepared for the
   * simulation. The constructor throws a SIG_Exception for a robot that has
   * no joints or a joint that does not turn.
   */
  class SIG_JointInertia
  {
  public:

    SIG_JointInertia( const SIGEL_Robot::SIG_Robot &preparedRobot,
                       const SIGEL_Environment::SIG_Environment &environment,
                       const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter );

    double getSmallest() const;

  private:

    // The start pose and this many random poses inside the joint ranges.
    static constexpr int randomPoses = 150;

    double smallest;
  };

}

#endif // SIGEL_ROBOTCHECK_SIG_JOINTINERTIA_H
