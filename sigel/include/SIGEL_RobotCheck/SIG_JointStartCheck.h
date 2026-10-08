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
#ifndef SIGEL_ROBOTCHECK_SIG_JOINTSTARTCHECK_H
#define SIGEL_ROBOTCHECK_SIG_JOINTSTARTCHECK_H

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_RobotCheck/SIG_RobotIssue.h"

#include <QList>

namespace SIGEL_RobotCheck
{

  /**
   * Finds joints that start outside their own range.
   */
  class SIG_JointStartCheck
  {
  public:

    SIG_JointStartCheck( const SIGEL_Robot::SIG_Robot &robot );

    QList<SIG_RobotIssue> issues() const;

  private:

    bool startsOutsideRange( double minimum, double maximum, double initial ) const;

    const SIGEL_Robot::SIG_Robot &robot;
  };

}

#endif // SIGEL_ROBOTCHECK_SIG_JOINTSTARTCHECK_H
