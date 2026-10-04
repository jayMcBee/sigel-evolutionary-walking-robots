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
#include "SIGEL_RobotCheck/SIG_LimitHoldsDriveCheck.h"

#include "SIGEL_Robot/SIG_RotationalJoint.h"

#include <qstringlist.h>

#include <cmath>

SIGEL_RobotCheck::SIG_LimitHoldsDriveCheck::SIG_LimitHoldsDriveCheck( const SIGEL_Robot::SIG_Robot &robot,
                                                                      const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : robot( robot ),
    simulationParameter( simulationParameter )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_LimitHoldsDriveCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  double spring = simulationParameter.getJointLimitsK_spring();
  if ( spring <= 0 )
    {
      issues.append( SIG_RobotIssue{ SIG_RobotIssue::tLimitHoldsDrive, SIG_RobotIssue::tWarning, "Joint limit cannot hold its drive", QString(),
                                     QString( "The joint-limit spring is %1, so no joint limit holds a drive." ).arg( spring ),
                                     "Raise the joint-limit spring." } );
      return issues;
    }

  QStringList drives;
  double worstPastLimit = 0;
  double worstRange = 0;

  for ( SIGEL_Robot::SIG_Drive *drive : robot.getDrives() )
    {
      const SIGEL_Robot::SIG_Joint *joint = drive->getJoint();
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );

      // A joint whose minimum equals its maximum has no limit to push past.
      if ( rotationalJoint->getMin() == rotationalJoint->getMax() )
        continue;

      // The limit spring balances the drive this far past the limit, in degrees.
      double pastLimit = drive->getMaxForce() / spring * 180 / M_PI;
      double range = rotationalJoint->getMax() - rotationalJoint->getMin();

      if ( pastLimit > range )
        {
          drives.append( drive->getName() );
          if ( pastLimit > worstPastLimit )
            {
              worstPastLimit = pastLimit;
              worstRange = range;
            }
        }
    }

  if ( drives.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tLimitHoldsDrive, SIG_RobotIssue::tWarning, "Joint limit cannot hold its drive", drives.join( ", " ),
                                 QString( "These drives can push their joint up to %1 degrees past its limit, more than the joint's range of %2 degrees. The joint may fold through or spin freely." ).arg( worstPastLimit, 0, 'f', 0 ).arg( worstRange ),
                                 "Lower the drive force or raise the joint-limit spring." } );

  return issues;
}
