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
#ifndef SIGEL_ROBOTCHECK_SIG_LINKOVERLAPCHECK_H
#define SIGEL_ROBOTCHECK_SIG_LINKOVERLAPCHECK_H

#include "SIGEL_RobotCheck/SIG_RobotIssue.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds links whose volumes overlap at the start pose.
   */
  class SIG_LinkOverlapCheck
  {

  public:

    SIG_LinkOverlapCheck( const SIG_RobotStartPose &startPose );

    QList<SIG_RobotIssue> issues() const;

  private:

    // The shipped robots and a robot of pins in forks overlap by 0.03 % at most; a model whose links do overlap by 5.8 %.
    static constexpr double overlapPercent = 1.0;
    static constexpr int overlapSamples = 20000;

    const SIG_RobotStartPose &startPose;

  };

}

#endif // SIGEL_ROBOTCHECK_SIG_LINKOVERLAPCHECK_H
