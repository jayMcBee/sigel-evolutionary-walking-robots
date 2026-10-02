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
#include "SIGEL_GP/SIG_RobotAdvisor.h"

#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <algorithm>
#include <cmath>

SIGEL_GP::SIG_RobotAdvisor::SIG_RobotAdvisor( const SIGEL_Robot::SIG_Robot &robot,
                                              const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                                              const SIGEL_Environment::SIG_Environment &environment,
                                              const SIGEL_GP::SIG_GPParameter &gpParameter )
  : robot( robot ),
    simulationParameter( simulationParameter ),
    environment( environment ),
    gpParameter( gpParameter )
{
}

QList<SIGEL_GP::SIG_RobotAdvisor::Finding> SIGEL_GP::SIG_RobotAdvisor::advise() const
{
  QList<Finding> findings;

  adviseLimitCannotHoldDrive( findings );
  adviseSenseWithoutSensors( findings );
  adviseStartOutsideRange( findings );

  try
    {
      SIGEL_Robot::SIG_Robot startRobot( robot );
      startRobot.instantiateGeometries();
      startRobot.initiate();

      adviseAxisOffEdge( findings, startRobot );
      adviseStartHeight( findings, startRobot );
      adviseStepForGroundContact( findings, startRobot );
      adviseStrokeThrowsRobot( findings, startRobot );
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      findings.append( Finding{ 0, QString(), 0, 0, "The robot cannot be put into its start pose: " + e.getMessage() } );
    }

  return findings;
}

void SIGEL_GP::SIG_RobotAdvisor::adviseAxisOffEdge( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  double size = sizeAtStart( startRobot );

  for ( SIGEL_Robot::SIG_Joint *joint : startRobot.getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );
      adviseAxisOffEdge( findings, startRobot, joint, joint->getLeftLink(), rotationalJoint->getLeftBase(), rotationalJoint->getLeftDir(), size );
      adviseAxisOffEdge( findings, startRobot, joint, joint->getRightLink(), rotationalJoint->getRightBase(), rotationalJoint->getRightDir(), size );
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseAxisOffEdge( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot, const SIGEL_Robot::SIG_Joint *joint, const SIGEL_Robot::SIG_Link *link, SIG_Vector base, SIG_Vector dir, double size ) const
{
  SIG_Vector axisPoint = atStart( link, base );
  SIG_Vector axis = atStart( link, dir );
  axis.minusis( &axisPoint );
  axis.normalize();

  // An axis on an edge has two vertices on it, so the second nearest tells.
  QList<double> distances;
  for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
    {
      SIG_Vector fromAxisPoint = atStart( link, *vertex );
      fromAxisPoint.minusis( &axisPoint );
      SIG_Vector offAxis;
      fromAxisPoint.crossprod( &axis, &offAxis );
      distances.append( offAxis.norm() );
    }

  if ( distances.size() < 2 )
    return;

  std::sort( distances.begin(), distances.end() );
  double offEdge = distances[1] / size * 100;

  if ( offEdge > axisOffEdgePercent )
    {
      QString text = QString( "Joint %1: the axis does not lie on an edge of link %2; the parts will overlap or float apart when it turns." )
        .arg( joint->getName() ).arg( link->getName() );
      findings.append( Finding{ 4, joint->getName(), offEdge, axisOffEdgePercent, text } );
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStepForGroundContact( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  SIGEL_Robot::SIG_Link *lightestLink = nullptr;
  double lightestMass = 0;

  for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
    {
      double mass;
      SIG_Vector centreOfMass;
      SIG_Matrix inertia;
      link->getPhysics( mass, centreOfMass, inertia );

      if ( !lightestLink || mass < lightestMass )
        {
          lightestLink = link;
          lightestMass = mass;
        }
    }

  if ( !lightestLink )
    return;

  // The fastest rate of the ground's penalty springs and dampers on the lightest link.
  double fastestRate = std::max( { std::sqrt( environment.getGroundNormalSpringConstant() / lightestMass ),
                                   std::sqrt( environment.getGroundPlanarSpringConstant() / lightestMass ),
                                   environment.getGroundNormalDamperConstant() / lightestMass,
                                   environment.getGroundPlanarDamperConstant() / lightestMass } );
  double suggestedStep = groundContactStability / fastestRate;

  if ( simulationParameter.getStepSize() > suggestedStep )
    {
      QString text = QString( "Step size %1 is above the suggested %2 for ground contact (lightest link %3, mass %4)." )
        .arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( lightestLink->getName() ).arg( lightestMass, 0, 'g', 3 );
      findings.append( Finding{ 8, lightestLink->getName(), simulationParameter.getStepSize(), suggestedStep, text } );
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStartHeight( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  // A face that lies on the floor is below it by rounding only.
  double belowFloor = floorLevel - 1e-9 * sizeAtStart( startRobot );

  int verticesBelowFloor = 0;
  double lowest = floorLevel;

  for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
    {
      for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
        {
          double height = atStart( link, *vertex ).y;
          if ( height < belowFloor )
            verticesBelowFloor++;
          lowest = std::min( lowest, height );
        }
    }

  if ( verticesBelowFloor > 0 )
    {
      double startHeight = environment.getStartPosition().y;
      QString text = QString( "Start height %1 puts %2 vertices below the floor; the lowest safe start height is %3." )
        .arg( startHeight ).arg( verticesBelowFloor ).arg( startHeight + floorLevel - lowest );
      findings.append( Finding{ 7, QString(), lowest, floorLevel, text } );
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStrokeThrowsRobot( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  double totalMass = 0;
  for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
    {
      double mass;
      SIG_Vector centreOfMass;
      SIG_Matrix inertia;
      link->getPhysics( mass, centreOfMass, inertia );
      totalMass += mass;
    }

  double weight = totalMass * std::abs( environment.getGravity().y );
  double throwHeight = throwStartHeights * ( environment.getStartPosition().y - floorLevel );

  for ( SIGEL_Robot::SIG_Drive *drive : startRobot.getDrives() )
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
          QString text = QString( "Drive %1 is strong enough to throw the robot (about %2 high). Expect random programs to fling it; lower the drive force." )
            .arg( drive->getName() ).arg( strokeHeight, 0, 'g', 3 );
          findings.append( Finding{ 2, drive->getName(), strokeHeight, throwHeight, text } );
        }
    }
}

SIG_Vector SIGEL_GP::SIG_RobotAdvisor::atStart( const SIGEL_Robot::SIG_Link *link, SIG_Vector point ) const
{
  SIG_Vector linkLocation;
  SIG_Matrix linkOrientation;
  link->getInitialLocation( linkLocation, linkOrientation );

  SIG_Vector inRobot;
  linkOrientation.times( &point, &inRobot );
  inRobot.plusis( &linkLocation );

  SIG_Matrix robotOrientation = robot.initialOrientation;
  SIG_Vector robotLocation = robot.initialLocation;
  SIG_Vector startPosition = environment.getStartPosition();

  SIG_Vector inWorld;
  robotOrientation.times( &inRobot, &inWorld );
  inWorld.plusis( &robotLocation );
  inWorld.plusis( &startPosition );

  return inWorld;
}

double SIGEL_GP::SIG_RobotAdvisor::sizeAtStart( const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  double size = 0;

  for ( int axis = 0; axis < 3; axis++ )
    {
      bool first = true;
      double lowest = 0;
      double highest = 0;

      for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
        {
          for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
            {
              double coordinate = atStart( link, *vertex ).get( axis );
              if ( first || coordinate < lowest )
                lowest = coordinate;
              if ( first || coordinate > highest )
                highest = coordinate;
              first = false;
            }
        }

      size = std::max( size, highest - lowest );
    }

  return size;
}

void SIGEL_GP::SIG_RobotAdvisor::adviseLimitCannotHoldDrive( QList<Finding> &findings ) const
{
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
      double pastLimit = drive->getMaxForce() / simulationParameter.getJointLimitsK_spring() * 180 / M_PI;
      double range = rotationalJoint->getMax() - rotationalJoint->getMin();

      if ( pastLimit > range )
        {
          QString text = QString( "Drive %1 can push joint %2 about %3 degrees past its limit; the joint may fold through or spin freely. Lower the drive force or raise the joint-limit spring." )
            .arg( drive->getName() ).arg( joint->getName() ).arg( pastLimit, 0, 'f', 0 );
          findings.append( Finding{ 1, drive->getName(), pastLimit, range, text } );
        }
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseSenseWithoutSensors( QList<Finding> &findings ) const
{
  // With probability 0 no program contains a SENSE.
  if (    robot.getSensors().isEmpty()
       && robot.getLangParam()->hasCommand( "SENSE" )
       && gpParameter.getProbability( SIGEL_Program::SENSE ) > 0 )
    findings.append( Finding{ 3, QString(), 0, 0, "The robot has no sensors but SENSE is allowed; every SENSE does nothing." } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStartOutsideRange( QList<Finding> &findings ) const
{
  for ( SIGEL_Robot::SIG_Joint *joint : robot.getJoints() )
    {
      switch ( joint->getJointType() )
        {
        case SIGEL_Robot::SIG_Joint::tRotationalJoint:
          {
            const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );
            adviseStartOutsideRange( findings, joint->getName(), rotationalJoint->getMin(), rotationalJoint->getMax(), rotationalJoint->getIni() );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tTranslationalJoint:
          {
            const SIGEL_Robot::SIG_TranslationalJoint *translationalJoint = static_cast<const SIGEL_Robot::SIG_TranslationalJoint *>( joint );
            adviseStartOutsideRange( findings, joint->getName(), translationalJoint->getMin(), translationalJoint->getMax(), translationalJoint->getIni() );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tCylindricalJoint:
          {
            const SIGEL_Robot::SIG_CylindricalJoint *cylindricalJoint = static_cast<const SIGEL_Robot::SIG_CylindricalJoint *>( joint );
            adviseStartOutsideRange( findings, joint->getName(), cylindricalJoint->getMinRot(), cylindricalJoint->getMaxRot(), cylindricalJoint->getIniRot() );
            adviseStartOutsideRange( findings, joint->getName(), cylindricalJoint->getMinTrans(), cylindricalJoint->getMaxTrans(), cylindricalJoint->getIniTrans() );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tGlueJoint:
          break;
        }
    }
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStartOutsideRange( QList<Finding> &findings, const QString &jointName, double minimum, double maximum, double initial ) const
{
  // A joint whose minimum equals its maximum has no limit.
  if ( minimum == maximum )
    return;

  if ( initial < minimum || initial > maximum )
    {
      QString text = QString( "Joint %1 starts outside its own range (init %2, range %3..%4); the limit spring kicks it at the first step." )
        .arg( jointName ).arg( initial ).arg( minimum ).arg( maximum );
      findings.append( Finding{ 6, jointName, initial, initial < minimum ? minimum : maximum, text } );
    }
}
