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
#ifndef SIGEL_ROBOTCHECK_SIG_DRIVESTRENGTHCHECK_H
#define SIGEL_ROBOTCHECK_SIG_DRIVESTRENGTHCHECK_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_RobotCheck/SIG_RobotIssue.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds drives that are strong enough to throw the robot into the air.
   */
  class SIG_DriveStrengthCheck
  {
  public:

    SIG_DriveStrengthCheck( const SIG_RobotStartPose &startPose,
                            const SIGEL_Environment::SIG_Environment &environment );

    QList<SIG_RobotIssue> issues() const;

  private:

    // In start heights of the robot standing on the floor: the two shipped robots that are thrown
    // are at 13.1 and 14.9, the highest of the five that are not at 6.0.
    static constexpr double throwStartHeights = 8.0;

    const SIG_RobotStartPose &startPose;
    const SIGEL_Environment::SIG_Environment &environment;
  };

}

#endif // SIGEL_ROBOTCHECK_SIG_DRIVESTRENGTHCHECK_H
