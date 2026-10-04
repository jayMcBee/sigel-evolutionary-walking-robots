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
#include "SIGEL_RobotCheck/SIG_DriveStrengthCheck.h"

#include "SIGEL_Robot/SIG_RotationalJoint.h"

#include <qstringlist.h>

#include <algorithm>
#include <cmath>

SIGEL_RobotCheck::SIG_DriveStrengthCheck::SIG_DriveStrengthCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose,
                                                                  const SIGEL_Environment::SIG_Environment &environment )
  : startPose( startPose ),
    environment( environment )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_DriveStrengthCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  // The check computes with the links' masses.
  if ( !startPose.everyLinkHasMass() )
    return issues;

  double totalMass = 0;
  for ( double mass : startPose.getMasses() )
    totalMass += mass;

  // Without gravity nothing has a weight to lift.
  double weight = totalMass * std::abs( environment.getGravity().y );
  if ( weight == 0 )
    return issues;

  // The start height in the experiment is a setting; this one is the robot's own.
  double startHeight = startPose.getSafeStartHeight();
  double throwHeight = throwStartHeights * startHeight;

  QStringList drives;
  double highestStroke = 0;

  for ( SIGEL_Robot::SIG_Drive *drive : startPose.getRobot().getDrives() )
    {
      const SIGEL_Robot::SIG_Joint *joint = drive->getJoint();
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );
      if ( rotationalJoint->getMin() == rotationalJoint->getMax() )
        continue;

      // The height one full-force stroke through the joint's range lifts the whole robot.
      double range = ( rotationalJoint->getMax() - rotationalJoint->getMin() ) * M_PI / 180;
      double strokeHeight = drive->getMaxForce() * range / weight;

      if ( strokeHeight > throwHeight )
        {
          drives.append( drive->getName() );
          highestStroke = std::max( highestStroke, strokeHeight );
        }
    }

  if ( drives.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tDriveStrength, SIG_RobotIssue::tWarning, "Drive too strong for the robot's weight", drives.join( ", " ),
                                 QString( "At full force, one turn of the joint through its range can lift the whole robot up to %1 high. Standing on the floor, the robot has a start height of %2. Random programs throw it into the air." ).arg( highestStroke, 0, 'g', 3 ).arg( startHeight ),
                                 "Lower the drive force." } );

  return issues;
}
