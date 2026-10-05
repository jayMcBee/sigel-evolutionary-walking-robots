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
#ifndef SIGEL_ROBOTCHECK_SIG_JOINTAXISCHECK_H
#define SIGEL_ROBOTCHECK_SIG_JOINTAXISCHECK_H

#include "SIGEL_RobotCheck/SIG_RobotIssue.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds joints whose axis passes outside a link that they join.
   * A link that is moved along the axis is not found.
   */
  class SIG_JointAxisCheck
  {

  public:

    SIG_JointAxisCheck( const SIG_RobotStartPose &startPose );

    QList<SIG_RobotIssue> issues() const;

  private:

    // The shipped robots are at 0.08 % or less, and a robot whose axis is off its link on purpose at 0.56 %.
    // A model whose parts float apart is at 2 %.
    static constexpr double axisOutsideLinkPercent = 1.0;

    const SIG_RobotStartPose &startPose;

  };

}

#endif // SIGEL_ROBOTCHECK_SIG_JOINTAXISCHECK_H
