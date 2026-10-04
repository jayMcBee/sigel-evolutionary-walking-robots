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
#include "SIGEL_RobotCheck/SIG_JointStartCheck.h"

#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"

#include <qstringlist.h>

SIGEL_RobotCheck::SIG_JointStartCheck::SIG_JointStartCheck( const SIGEL_Robot::SIG_Robot &robot )
  : robot( robot )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_JointStartCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  QStringList joints;

  for ( SIGEL_Robot::SIG_Joint *joint : robot.getJoints() )
    {
      double minimum = 0;
      double maximum = 0;
      double initial = 0;
      bool outside = false;

      switch ( joint->getJointType() )
        {
        case SIGEL_Robot::SIG_Joint::tRotationalJoint:
          {
            const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );
            minimum = rotationalJoint->getMin();
            maximum = rotationalJoint->getMax();
            initial = rotationalJoint->getIni();
            outside = startsOutsideRange( minimum, maximum, initial );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tTranslationalJoint:
          {
            const SIGEL_Robot::SIG_TranslationalJoint *translationalJoint = static_cast<const SIGEL_Robot::SIG_TranslationalJoint *>( joint );
            minimum = translationalJoint->getMin();
            maximum = translationalJoint->getMax();
            initial = translationalJoint->getIni();
            outside = startsOutsideRange( minimum, maximum, initial );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tCylindricalJoint:
          {
            const SIGEL_Robot::SIG_CylindricalJoint *cylindricalJoint = static_cast<const SIGEL_Robot::SIG_CylindricalJoint *>( joint );
            minimum = cylindricalJoint->getMinRot();
            maximum = cylindricalJoint->getMaxRot();
            initial = cylindricalJoint->getIniRot();
            outside = startsOutsideRange( minimum, maximum, initial );
            if ( !outside )
              {
                minimum = cylindricalJoint->getMinTrans();
                maximum = cylindricalJoint->getMaxTrans();
                initial = cylindricalJoint->getIniTrans();
                outside = startsOutsideRange( minimum, maximum, initial );
              }
          }
          break;

        case SIGEL_Robot::SIG_Joint::tGlueJoint:
          break;
        }

      if ( outside )
        joints.append( QString( "%1 (init %2, range %3..%4)" ).arg( joint->getName() ).arg( initial ).arg( minimum ).arg( maximum ) );
    }

  if ( joints.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tJointStart, SIG_RobotIssue::tWarning, "Joint starts outside its range", joints.join( ", " ),
                                 "The start angle of these joints is outside their own range, so the limit spring kicks them at the first step.",
                                 "Put init between minimal and maximal." } );

  return issues;
}

bool SIGEL_RobotCheck::SIG_JointStartCheck::startsOutsideRange( double minimum, double maximum, double initial ) const
{
  // A joint whose minimum equals its maximum has no limit.
  if ( minimum == maximum )
    return false;

  return initial < minimum || initial > maximum;
}
