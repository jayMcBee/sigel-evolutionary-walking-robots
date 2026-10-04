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
#include "SIGEL_RobotCheck/SIG_StartHeightCheck.h"

SIGEL_RobotCheck::SIG_StartHeightCheck::SIG_StartHeightCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose,
                                                              const SIGEL_Environment::SIG_Environment &environment )
  : startPose( startPose ),
    environment( environment )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_StartHeightCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  if ( !startPose.canBeSimulated() )
    return issues;

  // A face that lies on the floor is below it by rounding only.
  double belowFloor = SIG_RobotStartPose::floorLevel - 1e-9 * startPose.getSize();

  int verticesBelowFloor = 0;
  double lowest = SIG_RobotStartPose::floorLevel;
  QString lowestLink;

  for ( SIGEL_Robot::SIG_Link *link : startPose.getRobot().getLinks() )
    {
      for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
        {
          double height = startPose.toWorld( link, *vertex ).y;
          if ( height < belowFloor )
            verticesBelowFloor++;
          if ( height < lowest )
            {
              lowest = height;
              lowestLink = link->getName();
            }
        }
    }

  if ( verticesBelowFloor == 0 )
    return issues;

  double startHeight = environment.getStartPosition().y;
  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tStartHeight, SIG_RobotIssue::tSuggestion, "Robot starts inside the floor", lowestLink,
                                 QString( "Start height %1 puts %2 vertices below the floor, the lowest on this link. The floor throws the robot up at the first step." ).arg( startHeight ).arg( verticesBelowFloor ),
                                 QString( "Set the start height to %1 or more." ).arg( startPose.getSafeStartHeight() ) } );

  return issues;
}
