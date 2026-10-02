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
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
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
  QList<Finding> warnings;
  QList<Finding> suggestions;

  try
    {
      SIGEL_Robot::SIG_Robot startRobot( robot );
      startRobot.prepareDynaMechs();
      StartPose startPose = startPoseOf( startRobot );

      QList<double> masses;
      for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
        {
          double mass;
          SIG_Vector centreOfMass;
          SIG_Matrix inertia;
          link->getPhysics( mass, centreOfMass, inertia );
          masses.append( mass );
        }

      bool everyLinkHasMass = adviseLinkWithoutMass( warnings, masses, startRobot );

      adviseLimitCannotHoldDrive( warnings );
      if ( everyLinkHasMass )
        adviseStrokeThrowsRobot( warnings, masses, startRobot );
      adviseSenseWithoutSensors( warnings );
      adviseAxisOffEdge( warnings, startRobot, startPose );
      adviseStartOutsideRange( warnings );

      adviseStartHeight( suggestions, startRobot, startPose );
      if ( everyLinkHasMass )
        adviseStepForGroundContact( suggestions, masses, startRobot );
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      warnings.clear();
      suggestions.clear();
      // The message's later lines name the source file that threw.
      warnings.append( Finding{ 0, tError, QString(), 0, 0, "The robot cannot be simulated: " + e.getMessage().section( '\n', 0, 0 ) } );
      adviseLimitCannotHoldDrive( warnings );
      adviseSenseWithoutSensors( warnings );
      adviseStartOutsideRange( warnings );
    }

  return warnings + suggestions;
}

bool SIGEL_GP::SIG_RobotAdvisor::adviseLinkWithoutMass( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  QStringList links;
  double lowest = 0;

  for ( int i = 0; i < masses.size(); i++ )
    {
      // Written this way round so that a mass that is not a number counts too.
      if ( !( masses[i] > 0 ) )
        {
          links.append( startRobot.getLinks()[i]->getName() );
          lowest = std::min( lowest, masses[i] );
        }
    }

  if ( links.isEmpty() )
    return true;

  QString text = QString( "Links with a mass of 0 or less: %1. The faces of their bodies probably face inwards. The simulation cannot work with them." )
    .arg( links.join( ", " ) );
  findings.append( Finding{ 10, tError, links.join( ", " ), lowest, 0, text } );
  return false;
}

void SIGEL_GP::SIG_RobotAdvisor::adviseLimitCannotHoldDrive( QList<Finding> &findings ) const
{
  double spring = simulationParameter.getJointLimitsK_spring();
  if ( spring <= 0 )
    {
      findings.append( Finding{ 1, tWarning, QString(), spring, 0, "The joint-limit spring is 0 or less, so no joint limit holds a drive. Raise the joint-limit spring." } );
      return;
    }

  QStringList drives;
  double worstPastLimit = 0;
  double worstRange = 0;

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
      double pastLimit = drive->getMaxForce() / spring * 180 / M_PI;
      double range = rotationalJoint->getMax() - rotationalJoint->getMin();

      if ( pastLimit > range )
        {
          drives.append( drive->getName() );
          if ( pastLimit - range > worstPastLimit - worstRange )
            {
              worstPastLimit = pastLimit;
              worstRange = range;
            }
        }
    }

  if ( drives.isEmpty() )
    return;

  QString text = QString( "Drives that can push their joint further past its limit than the joint's range (up to %1 degrees past, range %2): %3. The joint may fold through or spin freely. Lower the drive force or raise the joint-limit spring." )
    .arg( worstPastLimit, 0, 'f', 0 ).arg( worstRange ).arg( drives.join( ", " ) );
  findings.append( Finding{ 1, tWarning, drives.join( ", " ), worstPastLimit, worstRange, text } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStrokeThrowsRobot( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  double totalMass = 0;
  for ( double mass : masses )
    totalMass += mass;

  // Without gravity nothing has a weight to lift.
  double weight = totalMass * std::abs( environment.getGravity().y );
  if ( weight == 0 )
    return;

  double startHeight = environment.getStartPosition().y - floorLevel;
  double throwHeight = throwStartHeights * startHeight;

  QStringList drives;
  double highestStroke = 0;

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
          drives.append( drive->getName() );
          highestStroke = std::max( highestStroke, strokeHeight );
        }
    }

  if ( drives.isEmpty() )
    return;

  QString text = QString( "Drives strong enough to throw the robot (one stroke can lift it up to %1 high; the start height is %2): %3. Expect random programs to fling it. Lower the drive force." )
    .arg( highestStroke, 0, 'g', 3 ).arg( startHeight ).arg( drives.join( ", " ) );
  findings.append( Finding{ 2, tWarning, drives.join( ", " ), highestStroke, throwHeight, text } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseSenseWithoutSensors( QList<Finding> &findings ) const
{
  // With probability 0 no program contains a SENSE.
  if (    robot.getSensors().isEmpty()
       && robot.getLangParam()->hasCommand( "SENSE" )
       && gpParameter.getProbability( SIGEL_Program::SENSE ) > 0 )
    findings.append( Finding{ 3, tWarning, QString(), 0, 0, "The robot has no sensors but SENSE is allowed; every SENSE does nothing." } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseAxisOffEdge( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const
{
  double size = sizeAtStart( startRobot, startPose );

  QStringList pairs;
  double worstOffEdge = 0;

  for ( SIGEL_Robot::SIG_Joint *joint : startRobot.getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        continue;

      SIGEL_Robot::SIG_Link *predecessor;
      double a, alpha, d, theta, screwD, screwTheta;
      joint->getMDH( predecessor, a, alpha, d, theta, screwD, screwTheta );
      SIGEL_Robot::SIG_Link *successor = joint->otherSide( predecessor );

      // The simulation turns the joint about the z axis of the successor's frame.
      SIG_Vector axisPoint = startPose.positions[ successor->getNumber() ];
      SIG_Vector axis = startPose.orientations[ successor->getNumber() ].c2;

      for ( SIGEL_Robot::SIG_Link *link : { predecessor, successor } )
        {
          double offEdge = offAxis( startPose, link, axisPoint, axis ) / size * 100;
          if ( offEdge > axisOffEdgePercent )
            {
              pairs.append( joint->getName() + " on " + link->getName() );
              worstOffEdge = std::max( worstOffEdge, offEdge );
            }
        }
    }

  if ( pairs.isEmpty() )
    return;

  QString text = QString( "Joints whose axis does not lie on an edge of a link they join (up to %1 % of the robot's size away): %2. The parts will overlap or float apart when the joint turns." )
    .arg( worstOffEdge, 0, 'g', 3 ).arg( pairs.join( ", " ) );
  findings.append( Finding{ 4, tWarning, pairs.join( ", " ), worstOffEdge, axisOffEdgePercent, text } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStartOutsideRange( QList<Finding> &findings ) const
{
  QStringList joints;
  double firstInitial = 0;
  double firstLimit = 0;

  for ( SIGEL_Robot::SIG_Joint *joint : robot.getJoints() )
    {
      double minimum = 0;
      double maximum = 0;
      double initial = 0;
      bool outside = false;

      switch ( joint->getJointType() )
        {
        case SIGEL_Robot::SIG_Joint::tRotationalJoint:
          {
            const SIGEL_Robot::SIG_RotationalJoint *rotationalJoint = static_cast<const SIGEL_Robot::SIG_RotationalJoint *>( joint );
            minimum = rotationalJoint->getMin();
            maximum = rotationalJoint->getMax();
            initial = rotationalJoint->getIni();
            outside = startsOutsideRange( minimum, maximum, initial );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tTranslationalJoint:
          {
            const SIGEL_Robot::SIG_TranslationalJoint *translationalJoint = static_cast<const SIGEL_Robot::SIG_TranslationalJoint *>( joint );
            minimum = translationalJoint->getMin();
            maximum = translationalJoint->getMax();
            initial = translationalJoint->getIni();
            outside = startsOutsideRange( minimum, maximum, initial );
          }
          break;

        case SIGEL_Robot::SIG_Joint::tCylindricalJoint:
          {
            const SIGEL_Robot::SIG_CylindricalJoint *cylindricalJoint = static_cast<const SIGEL_Robot::SIG_CylindricalJoint *>( joint );
            minimum = cylindricalJoint->getMinRot();
            maximum = cylindricalJoint->getMaxRot();
            initial = cylindricalJoint->getIniRot();
            outside = startsOutsideRange( minimum, maximum, initial );
            if ( !outside )
              {
                minimum = cylindricalJoint->getMinTrans();
                maximum = cylindricalJoint->getMaxTrans();
                initial = cylindricalJoint->getIniTrans();
                outside = startsOutsideRange( minimum, maximum, initial );
              }
          }
          break;

        case SIGEL_Robot::SIG_Joint::tGlueJoint:
          break;
        }

      if ( outside )
        {
          if ( joints.isEmpty() )
            {
              firstInitial = initial;
              firstLimit = initial < minimum ? minimum : maximum;
            }
          joints.append( QString( "%1 (init %2, range %3..%4)" ).arg( joint->getName() ).arg( initial ).arg( minimum ).arg( maximum ) );
        }
    }

  if ( joints.isEmpty() )
    return;

  QString text = QString( "Joints that start outside their own range: %1. The limit spring kicks them at the first step." )
    .arg( joints.join( ", " ) );
  findings.append( Finding{ 6, tWarning, joints.join( ", " ), firstInitial, firstLimit, text } );
}

bool SIGEL_GP::SIG_RobotAdvisor::startsOutsideRange( double minimum, double maximum, double initial ) const
{
  // A joint whose minimum equals its maximum has no limit.
  if ( minimum == maximum )
    return false;

  return initial < minimum || initial > maximum;
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStartHeight( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const
{
  // A face that lies on the floor is below it by rounding only.
  double belowFloor = floorLevel - 1e-9 * sizeAtStart( startRobot, startPose );

  int verticesBelowFloor = 0;
  double lowest = floorLevel;
  QString lowestLink;

  for ( SIGEL_Robot::SIG_Link *link : startRobot.getLinks() )
    {
      for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
        {
          double height = atStart( startPose, link, *vertex ).y;
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
    return;

  double startHeight = environment.getStartPosition().y;
  QString text = QString( "Start height %1 puts %2 vertices below the floor (lowest on link %3); the floor throws the robot up at the first step. The lowest safe start height is %4." )
    .arg( startHeight ).arg( verticesBelowFloor ).arg( lowestLink ).arg( startHeight + floorLevel - lowest );
  findings.append( Finding{ 7, tSuggestion, lowestLink, lowest, floorLevel, text } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStepForGroundContact( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  if ( masses.isEmpty() )
    return;

  int lightest = std::min_element( masses.begin(), masses.end() ) - masses.begin();
  double lightestMass = masses[ lightest ];
  QString lightestLink = startRobot.getLinks()[ lightest ]->getName();

  // The fastest rate of the ground's penalty springs and dampers on the lightest link.
  double fastestRate = std::max( { std::sqrt( environment.getGroundNormalSpringConstant() / lightestMass ),
                                   std::sqrt( environment.getGroundPlanarSpringConstant() / lightestMass ),
                                   environment.getGroundNormalDamperConstant() / lightestMass,
                                   environment.getGroundPlanarDamperConstant() / lightestMass } );
  double suggestedStep = groundContactStability / fastestRate;

  if ( simulationParameter.getStepSize() > suggestedStep )
    {
      QString text = QString( "Step size %1 is above the suggested %2 for ground contact (lightest link %3, mass %4). Runs may still work; if robots fly off or scores are 0, lower the step size." )
        .arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( lightestLink ).arg( lightestMass, 0, 'g', 3 );
      findings.append( Finding{ 8, tSuggestion, lightestLink, simulationParameter.getStepSize(), suggestedStep, text } );
    }
}

SIGEL_GP::SIG_RobotAdvisor::StartPose SIGEL_GP::SIG_RobotAdvisor::startPoseOf( const SIGEL_Robot::SIG_Robot &startRobot ) const
{
  // The simulation data makes its own environment the global one; other code still needs the one from before.
  dmEnvironment *environmentBefore = dmEnvironment::getEnvironment();

  StartPose startPose;
  try
    {
      SIGEL_Simulation::SIG_DynaMechsSimulationData simulationData( startRobot, environment, simulationParameter );
      SIGEL_Simulation::SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

      for ( int i = 0; i < simulationQueries.getLinkCount(); i++ )
        {
          startPose.positions.append( simulationQueries.getLinkPosition( i ) );
          startPose.orientations.append( simulationQueries.getLinkOrientation( i ) );
        }
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      dmEnvironment::setEnvironment( environmentBefore );
      throw;
    }

  dmEnvironment::setEnvironment( environmentBefore );
  return startPose;
}

SIG_Vector SIGEL_GP::SIG_RobotAdvisor::atStart( const StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector point ) const
{
  SIG_Matrix orientation = startPose.orientations[ link->getNumber() ];
  SIG_Vector position = startPose.positions[ link->getNumber() ];

  SIG_Vector inWorld;
  orientation.times( &point, &inWorld );
  inWorld.plusis( &position );
  return inWorld;
}

double SIGEL_GP::SIG_RobotAdvisor::offAxis( const StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector axisPoint, SIG_Vector axis ) const
{
  // An axis on an edge has two vertices on it, so the second nearest tells.
  QList<double> distances;
  for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
    {
      SIG_Vector fromAxisPoint = atStart( startPose, link, *vertex );
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

double SIGEL_GP::SIG_RobotAdvisor::sizeAtStart( const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const
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
              double coordinate = atStart( startPose, link, *vertex ).get( axis );
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
