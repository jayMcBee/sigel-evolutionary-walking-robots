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
#ifndef SIGEL_ROBOTCHECK_SIG_GROUNDSTEPSIZECHECK_H
#define SIGEL_ROBOTCHECK_SIG_GROUNDSTEPSIZECHECK_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_RobotCheck/SIG_RobotIssue.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds a step size that is too large for the ground contact of the
   * lightest link.
   */
  class SIG_GroundStepSizeCheck
  {

  public:

    SIG_GroundStepSizeCheck( const SIG_RobotStartPose &startPose,
                             const SIGEL_Environment::SIG_Environment &environment,
                             const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter );

    QList<SIG_RobotIssue> issues() const;

  private:

    // Single bodies go unstable from 0.5; the shipped robots show the first NaN at 1.5.
    static constexpr double groundContactStability = 0.75;

    const SIG_RobotStartPose &startPose;
    const SIGEL_Environment::SIG_Environment &environment;
    const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter;

  };

}

#endif // SIGEL_ROBOTCHECK_SIG_GROUNDSTEPSIZECHECK_H
