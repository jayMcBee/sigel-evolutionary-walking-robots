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
#ifndef SIGEL_ROBOTCHECK_SIG_ROBOTISSUE_H
#define SIGEL_ROBOTCHECK_SIG_ROBOTISSUE_H

#include <qstring.h>

namespace SIGEL_RobotCheck
{

  /**
   * One issue that the robot check raises: the parts of the robot it is
   * about, what was measured and what that does to a run, and what to change.
   */
  struct SIG_RobotIssue
  {
    // The check that raised the issue, in the order in which issues are listed.
    enum Check { tCannotBeSimulated, tLimitHoldsDrive, tDriveStrength, tJointAxis, tLinkOverlap, tJointStart,
                 tStartHeight, tGroundStepSize, tJointStepSize, tIntegrator, tLinkMass, tStanding };

    // An error is a robot that cannot run at all.
    enum Kind { tError, tWarning, tSuggestion };

    // Errors come before warnings and those before suggestions; the check orders the rest.
    bool operator<( const SIG_RobotIssue &other ) const;

    // The issue of a robot that the simulation cannot take, with the simulation's reason.
    static SIG_RobotIssue cannotBeSimulated( const QString &reason );

    Check check;
    Kind kind;
    QString title;
    QString part;
    QString analysis;
    QString advice;
  };

}

#endif // SIGEL_ROBOTCHECK_SIG_ROBOTISSUE_H
