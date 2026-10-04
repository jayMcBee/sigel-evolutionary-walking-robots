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
#include "SIGEL_RobotCheck/SIG_LinkOverlapCheck.h"

#include "SIGEL_RobotCheck/SIG_LinkVolume.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <qstringlist.h>

#include <algorithm>

SIGEL_RobotCheck::SIG_LinkOverlapCheck::SIG_LinkOverlapCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose )
  : startPose( startPose )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_LinkOverlapCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  // The check computes with the links' masses.
  if ( !startPose.everyLinkHasMass() )
    return issues;

  const QList<SIGEL_Robot::SIG_Link *> &links = startPose.getRobot().getLinks();
  const QList<double> &masses = startPose.getMasses();

  QList<SIG_LinkVolume> volumes;
  for ( SIGEL_Robot::SIG_Link *link : links )
    volumes.append( startPose.getLinkVolume( link ) );

  // The same sample points on every call, so that the issues do not change between calls.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );

  QStringList pairs;
  double worstOverlap = 0;

  for ( int i = 0; i < links.size(); i++ )
    {
      for ( int j = i + 1; j < links.size(); j++ )
        {
          double overlapVolume = volumes[i].sharedWith( volumes[j], overlapSamples, randomizer );
          double smallerVolume = std::min( masses[i] / links[i]->getMaterial()->getDensity(),
                                           masses[j] / links[j]->getMaterial()->getDensity() );
          double overlap = overlapVolume / smallerVolume * 100;

          if ( overlap > overlapPercent )
            {
              pairs.append( links[i]->getName() + " and " + links[j]->getName() );
              worstOverlap = std::max( worstOverlap, overlap );
            }
        }
    }

  if ( pairs.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tLinkOverlap, SIG_RobotIssue::tWarning, "Links overlap", pairs.join( ", " ),
                                 QString( "At the start pose these links share up to %1 % of the smaller link's volume. The simulation lets links pass through each other." ).arg( worstOverlap, 0, 'g', 3 ),
                                 "Move the links apart in the robot model." } );

  return issues;
}
