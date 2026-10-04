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
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"

#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <newmat.h>

#include <algorithm>

SIGEL_RobotCheck::SIG_RobotStartPose::SIG_RobotStartPose( const SIGEL_Robot::SIG_Robot &robot,
                                                          const SIGEL_Environment::SIG_Environment &environment,
                                                          const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : startRobot( robot ),
    startHeight( environment.getStartPosition().y )
{
  try
    {
      // The simulation ends the program on any other kind of joint.
      for ( SIGEL_Robot::SIG_Joint *joint : startRobot.getJoints() )
        {
          if (    joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint
               && joint->getJointType() != SIGEL_Robot::SIG_Joint::tTranslationalJoint )
            throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "Joint " + joint->getName() + " is neither a rotational nor a translational joint." );
        }

      startRobot.prepareDynaMechs();

      for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
        {
          double mass;
          SIG_Vector centreOfMass;
          SIG_Matrix inertia;
          link->getPhysics( mass, centreOfMass, inertia );
          masses.append( mass );
        }

      SIGEL_Simulation::SIG_DynaMechsSimulationData simulationData( startRobot, environment, simulationParameter );
      SIGEL_Simulation::SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

      for ( int i = 0; i < simulationQueries.getLinkCount(); i++ )
        {
          positions.append( simulationQueries.getLinkPosition( i ) );
          orientations.append( simulationQueries.getLinkOrientation( i ) );
        }
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      // The message's later lines name the source file that threw.
      refusal = e.getMessage().section( '\n', 0, 0 );
    }
  catch ( NEWMAT::Exception & )
    {
      // newmat throws for a robot whose geometry gives a matrix it cannot invert.
      refusal = QString( NEWMAT::Exception::what() ).section( '\n', 0, 0 ).trimmed();
    }
}

bool SIGEL_RobotCheck::SIG_RobotStartPose::canBeSimulated() const
{
  return refusal.isEmpty();
}

QString SIGEL_RobotCheck::SIG_RobotStartPose::getRefusal() const
{
  return refusal;
}

bool SIGEL_RobotCheck::SIG_RobotStartPose::everyLinkHasMass() const
{
  if ( !canBeSimulated() )
    return false;

  for ( double mass : masses )
    {
      // Written this way round so that a mass that is not a number counts too.
      if ( !( mass > 0 ) )
        return false;
    }

  return true;
}

const SIGEL_Robot::SIG_Robot &SIGEL_RobotCheck::SIG_RobotStartPose::getRobot() const
{
  return startRobot;
}

const QList<double> &SIGEL_RobotCheck::SIG_RobotStartPose::getMasses() const
{
  return masses;
}

SIG_Vector SIGEL_RobotCheck::SIG_RobotStartPose::getPosition( const SIGEL_Robot::SIG_Link *link ) const
{
  return positions[ link->getNumber() ];
}

SIG_Matrix SIGEL_RobotCheck::SIG_RobotStartPose::getOrientation( const SIGEL_Robot::SIG_Link *link ) const
{
  return orientations[ link->getNumber() ];
}

SIG_Vector SIGEL_RobotCheck::SIG_RobotStartPose::toWorld( const SIGEL_Robot::SIG_Link *link, SIG_Vector point ) const
{
  SIG_Matrix orientation = getOrientation( link );
  SIG_Vector position = getPosition( link );

  SIG_Vector inWorld;
  orientation.times( &point, &inWorld );
  inWorld.plusis( &position );
  return inWorld;
}

SIGEL_RobotCheck::SIG_LinkVolume SIGEL_RobotCheck::SIG_RobotStartPose::getLinkVolume( const SIGEL_Robot::SIG_Link *link ) const
{
  SIG_LinkVolume volume;

  SIGEL_Robot::SIG_GeometryIterator polygons( link->getGeometry() );
  while ( polygons )
    {
      const SIGEL_Robot::SIG_Polygon &polygon = polygons.iterate();

      // A polygon with more than three corners becomes a fan of triangles.
      for ( int i = 2; i < polygon.getNumVertices(); i++ )
        {
          volume.addTriangle( toWorld( link, polygon.getVertex( 0 ) ),
                              toWorld( link, polygon.getVertex( i - 1 ) ),
                              toWorld( link, polygon.getVertex( i ) ) );
        }
    }

  return volume;
}

void SIGEL_RobotCheck::SIG_RobotStartPose::getExtent( int axis, double &lowest, double &highest ) const
{
  bool first = true;
  lowest = 0;
  highest = 0;

  for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
    {
      for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
        {
          double coordinate = toWorld( link, *vertex ).get( axis );
          if ( first || coordinate < lowest )
            lowest = coordinate;
          if ( first || coordinate > highest )
            highest = coordinate;
          first = false;
        }
    }
}

double SIGEL_RobotCheck::SIG_RobotStartPose::getSize() const
{
  double size = 0;

  for ( int axis = 0; axis < 3; axis++ )
    {
      double lowest, highest;
      getExtent( axis, lowest, highest );
      size = std::max( size, highest - lowest );
    }

  return size;
}

double SIGEL_RobotCheck::SIG_RobotStartPose::getSafeStartHeight() const
{
  // Axis 1 is y, which points up.
  double lowest, highest;
  getExtent( 1, lowest, highest );

  return startHeight + floorLevel - lowest;
}
