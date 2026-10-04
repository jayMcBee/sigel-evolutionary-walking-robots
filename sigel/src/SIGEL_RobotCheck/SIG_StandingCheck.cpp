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
#include "SIGEL_RobotCheck/SIG_StandingCheck.h"

#include "SIGEL_RobotCheck/SIG_UnpoweredRun.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <qdatetime.h>

#include <algorithm>

SIGEL_RobotCheck::SIG_StandingCheck::SIG_StandingCheck( const SIGEL_Robot::SIG_Robot &robot,
                                                        const SIGEL_RobotCheck::SIG_RobotStartPose &startPose,
                                                        const SIGEL_Environment::SIG_Environment &environment,
                                                        const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : robot( robot ),
    startPose( startPose ),
    environment( environment ),
    simulationParameter( simulationParameter )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_StandingCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  // The check computes with the links' masses.
  if ( !startPose.everyLinkHasMass() )
    return issues;

  // Axis 1 is y, which points up.
  double lowest, highest;
  startPose.getExtent( 1, lowest, highest );
  double height = highest - lowest;

  double seconds = std::min( restSeconds, static_cast<double>( QTime( 0, 0 ).secsTo( simulationParameter.getTimeToSimulate() ) ) );

  try
    {
      // Placed on the floor, so that a fall from the start height does not count as sinking.
      SIG_UnpoweredRun unpoweredRun( robot, environment, simulationParameter, SIG_RobotStartPose::floorLevel - lowest + restClearance, seconds );

      if ( unpoweredRun.hasBrokenDown() )
        {
          issues.append( SIG_RobotIssue{ SIG_RobotIssue::tStanding, SIG_RobotIssue::tError, "Simulation breaks at rest", QString(),
                                         QString( "With no drive active the simulation breaks after %1 s, so no program can be scored." ).arg( unpoweredRun.getBrokeAfter() ),
                                         "Lower the step size." } );
          return issues;
        }

      QString loosestJoint = unpoweredRun.getLoosestJoint();
      double largestTurn = unpoweredRun.getLargestTurn();
      double sink = unpoweredRun.getSink();
      double sinkPercent = sink / height * 100;

      if ( largestTurn > collapsesDegrees || sinkPercent > collapsesSinkPercent )
        {
          // A body can sink with no joint turning.
          QString analysis = QString( "With no drive active the body sinks %1 (%2 % of the robot's height)." ).arg( sink, 0, 'g', 3 ).arg( sinkPercent, 0, 'f', 0 );
          if ( !loosestJoint.isEmpty() )
            analysis = QString( "With no drive active this joint turns %1 degrees and the body sinks %2 (%3 % of the robot's height)." ).arg( largestTurn, 0, 'f', 0 ).arg( sink, 0, 'g', 3 ).arg( sinkPercent, 0, 'f', 0 );

          QString advice = "Put the start angles on the limits that carry the weight.";
          if ( hasForceDrive() )
            {
              analysis += " Force drives apply no force between MOVEs, so only joint limits hold a stance.";
              advice = "Put the start angles on the limits that carry the weight, or use servo drives.";
            }

          issues.append( SIG_RobotIssue{ SIG_RobotIssue::tStanding, SIG_RobotIssue::tWarning, "Robot does not hold its start pose", loosestJoint, analysis, advice } );
        }
      else if ( largestTurn > settlesDegrees )
        {
          issues.append( SIG_RobotIssue{ SIG_RobotIssue::tStanding, SIG_RobotIssue::tSuggestion, "Robot settles before it rests", loosestJoint,
                                         QString( "With no drive active this joint turns %1 degrees before the robot rests, so every run starts with this motion." ).arg( largestTurn, 0, 'f', 0 ),
                                         "Put the start angles where the robot rests." } );
        }
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      // The message's later lines name the source file that threw.
      issues.append( SIG_RobotIssue::cannotBeSimulated( e.getMessage().section( '\n', 0, 0 ) ) );
    }

  return issues;
}

bool SIGEL_RobotCheck::SIG_StandingCheck::hasForceDrive() const
{
  for ( SIGEL_Robot::SIG_Drive *drive : robot.getDrives() )
    {
      if ( drive->getMode() == SIGEL_Robot::SIG_Drive::tForceMode )
        return true;
    }

  return false;
}
