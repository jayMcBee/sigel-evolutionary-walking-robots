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
#include "SIGEL_Simulation/SIG_RestRun.h"

#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Simulation/SIG_DynaMechsEnvironmentKeeper.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"

#include <algorithm>
#include <cmath>

SIGEL_Simulation::SIG_RestRun::SIG_RestRun( const SIGEL_Robot::SIG_Robot &robot,
                                            const SIGEL_Environment::SIG_Environment &environment,
                                            const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                                            double lift,
                                            double seconds )
  : largestTurn( 0 ),
    sink( 0 ),
    brokeAfter( -1 )
{
  SIGEL_Robot::SIG_Robot restRobot( robot );
  restRobot.prepareDynaMechs();
  // prepareDynaMechs sets the robot's location, so the lift comes after it.
  restRobot.initialLocation.y += lift;

  SIG_DynaMechsEnvironmentKeeper environmentKeeper;
  SIG_DynaMechsSimulationData simulationData( restRobot, environment, simulationParameter );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QList<SIG_Matrix> orientationsAtStart;
  for ( int i = 0; i < simulationQueries.getLinkCount(); i++ )
    orientationsAtStart.append( simulationQueries.getLinkOrientation( i ) );

  int rootNumber = simulationQueries.getRootNumber();
  double heightAtStart = simulationQueries.getLinkPosition( rootNumber ).y;

  // No interpreter runs, so no drive ever moves.
  while ( simulationQueries.getCurrentSimulationSeconds() < seconds )
    {
      simulationData.setNewFrame( true );
      simulationData.simulationProgress();
      simulationData.actualFrame++;

      SIG_Vector rootPosition = simulationQueries.getLinkPosition( rootNumber );
      if ( !std::isfinite( rootPosition.x ) || !std::isfinite( rootPosition.y ) || !std::isfinite( rootPosition.z ) )
        {
          brokeAfter = simulationQueries.getCurrentSimulationSeconds();
          return;
        }
      sink = heightAtStart - rootPosition.y;

      for ( SIGEL_Robot::SIG_Joint *joint : restRobot.getJoints() )
        {
          int left = joint->getLeftLink()->getNumber();
          int right = joint->getRightLink()->getNumber();
          double turn = turnBetween( simulationQueries.getLinkOrientation( left ), simulationQueries.getLinkOrientation( right ),
                                     orientationsAtStart[ left ], orientationsAtStart[ right ] );
          if ( turn > largestTurn )
            {
              largestTurn = turn;
              loosestJoint = joint->getName();
            }
        }
    }
}

double SIGEL_Simulation::SIG_RestRun::getLargestTurn() const
{
  return largestTurn;
}

QString SIGEL_Simulation::SIG_RestRun::getLoosestJoint() const
{
  return loosestJoint;
}

double SIGEL_Simulation::SIG_RestRun::getSink() const
{
  return sink;
}

bool SIGEL_Simulation::SIG_RestRun::hasBrokenDown() const
{
  return brokeAfter >= 0;
}

double SIGEL_Simulation::SIG_RestRun::getBrokeAfter() const
{
  return brokeAfter;
}

double SIGEL_Simulation::SIG_RestRun::turnBetween( SIG_Matrix left, SIG_Matrix right, SIG_Matrix leftAtStart, SIG_Matrix rightAtStart ) const
{
  // The trace of the rotation that takes the right link, seen from the left one, from its start to now.
  double trace = 0;
  for ( int i = 0; i < 3; i++ )
    {
      for ( int j = 0; j < 3; j++ )
        {
          double now = 0;
          double atStart = 0;
          for ( int k = 0; k < 3; k++ )
            {
              now += left.get( k, i ) * right.get( k, j );
              atStart += leftAtStart.get( k, i ) * rightAtStart.get( k, j );
            }
          trace += now * atStart;
        }
    }

  return std::acos( std::clamp( ( trace - 1 ) / 2, -1.0, 1.0 ) ) * 180 / M_PI;
}
