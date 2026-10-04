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
#include "SIGEL_RobotCheck/SIG_LinkMassCheck.h"

#include <qstringlist.h>

#include <algorithm>

SIGEL_RobotCheck::SIG_LinkMassCheck::SIG_LinkMassCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose )
  : startPose( startPose )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_LinkMassCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  if ( !startPose.canBeSimulated() )
    return issues;

  const QList<double> &masses = startPose.getMasses();
  QStringList links;
  double lowest = 0;

  for ( int i = 0; i < masses.size(); i++ )
    {
      // Written this way round so that a mass that is not a number counts too.
      if ( !( masses[i] > 0 ) )
        {
          links.append( startPose.getRobot().getLinks()[i]->getName() );
          lowest = std::min( lowest, masses[i] );
        }
    }

  if ( links.isEmpty() )
    return issues;

  issues.append( SIG_RobotIssue{ SIG_RobotIssue::tLinkMass, SIG_RobotIssue::tError, "Link without mass", links.join( ", " ),
                                 QString( "The mass of these links is 0 or less (lowest %1), so the simulation cannot work with them." ).arg( lowest ),
                                 "The faces of their bodies probably face inwards. Turn them outwards." } );

  return issues;
}
