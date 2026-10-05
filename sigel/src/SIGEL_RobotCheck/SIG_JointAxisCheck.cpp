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
#include "SIGEL_RobotCheck/SIG_JointAxisCheck.h"

#include <qstringlist.h>

#include <algorithm>

SIGEL_RobotCheck::SIG_JointAxisCheck::SIG_JointAxisCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose )
  : startPose( startPose )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_JointAxisCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  if ( !startPose.canBeSimulated() )
    return issues;

  double size = startPose.getSize();

  QStringList pairs;
  double worstOutside = 0;

  for ( SIGEL_Robot::SIG_Joint *joint : startPose.getRobot().getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      SIGEL_Robot::SIG_Link *predecessor;
      double a, alpha, d, theta, screwD, screwTheta;
      joint->getMDH( predecessor, a, alpha, d, theta, screwD, screwTheta );
      SIGEL_Robot::SIG_Link *successor = joint->otherSide( predecessor );

      // The simulation turns the joint about the z axis of the successor's frame.
      SIG_Vector axisPoint = startPose.getPosition( successor );
      SIG_Vector axis = startPose.getOrientation( successor ).c2;

      for ( SIGEL_Robot::SIG_Link *link : { predecessor, successor } )
        {
          double outside = startPose.getLinkVolume( link ).distanceToLine( axisPoint, axis ) / size * 100;
          if ( outside > axisOutsideLinkPercent )
            {
              pairs.append( joint->getName() + " on " + link->getName() );
              worstOutside = std::max( worstOutside, outside );
            }
        }
    }

  if ( pairs.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tJointAxis, SIG_RobotIssue::tSuggestion, "Joint axis passes outside a link", pairs.join( ", " ),
                                 QString( "For each pair, the joint's axis passes outside the link, by up to %1 % of the robot's size. The link is not at its joint, so the parts float apart." ).arg( worstOutside, 0, 'g', 3 ),
                                 "Move the joint or the link in the robot model, so that the axis touches the link." } );

  return issues;
}
