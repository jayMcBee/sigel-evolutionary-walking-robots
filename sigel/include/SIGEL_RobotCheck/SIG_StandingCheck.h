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
#ifndef SIGEL_ROBOTCHECK_SIG_STANDINGCHECK_H
#define SIGEL_ROBOTCHECK_SIG_STANDINGCHECK_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_RobotCheck/SIG_RobotIssue.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds a robot that does not hold its start pose when no drive is active,
   * and a simulation that breaks down then.
   */
  class SIG_StandingCheck
  {
  public:

    SIG_StandingCheck( const SIGEL_Robot::SIG_Robot &robot,
                       const SIG_RobotStartPose &startPose,
                       const SIGEL_Environment::SIG_Environment &environment,
                       const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter );

    QList<SIG_RobotIssue> issues() const;

  private:

    bool hasForceDrive() const;

    // The robot is left alone for this long, a hair above the floor. Ten measured robots all rest by then.
    static constexpr double restSeconds = 5.0;
    static constexpr double restClearance = 1e-6;
    // From few robots. Robots that stand turn a joint by 3.9 degrees at most and sink 2 % of their height;
    // two that settle turn 19 to 21 degrees; the one that collapses turns 52 degrees and sinks 43 %.
    static constexpr double settlesDegrees = 10.0;
    static constexpr double collapsesDegrees = 30.0;
    static constexpr double collapsesSinkPercent = 25.0;

    const SIGEL_Robot::SIG_Robot &robot;
    const SIG_RobotStartPose &startPose;
    const SIGEL_Environment::SIG_Environment &environment;
    const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter;
  };

}

#endif // SIGEL_ROBOTCHECK_SIG_STANDINGCHECK_H
