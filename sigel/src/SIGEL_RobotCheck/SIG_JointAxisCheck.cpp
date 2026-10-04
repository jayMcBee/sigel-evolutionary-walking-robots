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
  double worstOffEdge = 0;
  int jointsOffEdge = 0;
  int rotationalJoints = 0;

  for ( SIGEL_Robot::SIG_Joint *joint : startPose.getRobot().getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      rotationalJoints++;
      bool offAnEdge = false;

      SIGEL_Robot::SIG_Link *predecessor;
      double a, alpha, d, theta, screwD, screwTheta;
      joint->getMDH( predecessor, a, alpha, d, theta, screwD, screwTheta );
      SIGEL_Robot::SIG_Link *successor = joint->otherSide( predecessor );

      // The simulation turns the joint about the z axis of the successor's frame.
      SIG_Vector axisPoint = startPose.getPosition( successor );
      SIG_Vector axis = startPose.getOrientation( successor ).c2;

      for ( SIGEL_Robot::SIG_Link *link : { predecessor, successor } )
        {
          double offEdge = offAxis( link, axisPoint, axis ) / size * 100;
          if ( offEdge > axisOffEdgePercent )
            {
              pairs.append( joint->getName() + " on " + link->getName() );
              worstOffEdge = std::max( worstOffEdge, offEdge );
              offAnEdge = true;
            }
        }

      if ( offAnEdge )
        jointsOffEdge++;
    }

  if ( pairs.isEmpty() )
    return issues;

  // A robot built this way throughout would give a list of every joint.
  if ( jointsOffEdge == rotationalJoints && rotationalJoints > 1 )
    pairs = QStringList( QString( "all %1 joints" ).arg( rotationalJoints ) );

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tJointAxis, SIG_RobotIssue::tSuggestion, "No link edge on the joint's axis", pairs.join( ", " ),
                                 QString( "The joint's axis is up to %1 % of the robot's size away from the nearest edge of the link. The parts may overlap or float apart when the joint turns." ).arg( worstOffEdge, 0, 'g', 3 ),
                                 "Look at the robot. A pin in a fork is sound." } );

  return issues;
}

double SIGEL_RobotCheck::SIG_JointAxisCheck::offAxis( const SIGEL_Robot::SIG_Link *link, SIG_Vector axisPoint, SIG_Vector axis ) const
{
  // An axis on an edge has two vertices on it, so the second nearest tells.
  QList<double> distances;
  for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
    {
      SIG_Vector fromAxisPoint = startPose.toWorld( link, *vertex );
      fromAxisPoint.minusis( &axisPoint );
      SIG_Vector acrossAxis;
      fromAxisPoint.crossprod( &axis, &acrossAxis );
      distances.append( acrossAxis.norm() );
    }

  if ( distances.size() < 2 )
    return 0;

  std::sort( distances.begin(), distances.end() );
  return distances[1];
}
