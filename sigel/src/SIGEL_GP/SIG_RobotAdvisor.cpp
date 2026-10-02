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
#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <qdatetime.h>

#include <dmMDHLink.hpp>
#include <newmatap.h>

#include <algorithm>
#include <cmath>
#include <vector>

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
  QList<Finding> errors;
  QList<Finding> warnings;
  QList<Finding> suggestions;

  try
    {
      // The simulation ends the program on such a joint.
      for ( SIGEL_Robot::SIG_Joint *joint : robot.getJoints() )
        {
          if ( joint->getJointType() == SIGEL_Robot::SIG_Joint::tCylindricalJoint )
            throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "Joint " + joint->getName() + " is a cylindrical joint." );
        }

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

      bool everyLinkHasMass = adviseLinkWithoutMass( errors, masses, startRobot );

      adviseLimitCannotHoldDrive( warnings );
      if ( everyLinkHasMass )
        adviseStrokeThrowsRobot( warnings, masses, startRobot );
      adviseSenseWithoutSensors( warnings );
      if ( everyLinkHasMass )
        adviseLinksOverlap( warnings, masses, startRobot, startPose );
      adviseStartOutsideRange( warnings );

      adviseAxisOffEdge( suggestions, startRobot, startPose );
      adviseStartHeight( suggestions, startRobot, startPose );
      if ( everyLinkHasMass )
        {
          adviseStepForGroundContact( suggestions, masses, startRobot );
          adviseStepForJoints( suggestions, startPose );
          adviseHoldsStartPose( errors, warnings, suggestions, startRobot, startPose );
        }
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      errors.clear();
      warnings.clear();
      suggestions.clear();
      // The message's later lines name the source file that threw.
      errors.append( Finding{ 0, tError, QString(), 0, 0, "The robot cannot be simulated: " + e.getMessage().section( '\n', 0, 0 ) } );
      adviseLimitCannotHoldDrive( warnings );
      adviseSenseWithoutSensors( warnings );
      adviseStartOutsideRange( warnings );
    }

  return errors + warnings + suggestions;
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
  int jointsOffEdge = 0;
  int rotationalJoints = 0;

  for ( SIGEL_Robot::SIG_Joint *joint : startRobot.getJoints() )
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
      SIG_Vector axisPoint = startPose.positions[ successor->getNumber() ];
      SIG_Vector axis = startPose.orientations[ successor->getNumber() ].c2;

      for ( SIGEL_Robot::SIG_Link *link : { predecessor, successor } )
        {
          double offEdge = offAxis( startPose, link, axisPoint, axis ) / size * 100;
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
    return;

  // A robot built this way throughout would give a list of every joint.
  if ( jointsOffEdge == rotationalJoints && rotationalJoints > 1 )
    pairs = QStringList( QString( "all %1 joints" ).arg( rotationalJoints ) );

  QString text = QString( "No edge of the link lies on the joint's axis (up to %1 % of the robot's size away): %2. Look at the robot to see that the parts neither overlap nor float apart when the joint turns." )
    .arg( worstOffEdge, 0, 'g', 3 ).arg( pairs.join( ", " ) );
  findings.append( Finding{ 4, tSuggestion, pairs.join( ", " ), worstOffEdge, axisOffEdgePercent, text } );
}

void SIGEL_GP::SIG_RobotAdvisor::adviseLinksOverlap( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const
{
  const QList<SIGEL_Robot::SIG_Link *> &links = startRobot.getLinks();

  // Each link's triangles and the box around them, in the start pose.
  QList< QList<Triangle> > triangles;
  QList<SIG_Vector> lowCorners;
  QList<SIG_Vector> highCorners;
  for ( SIGEL_Robot::SIG_Link *link : links )
    {
      triangles.append( trianglesAtStart( startPose, link ) );

      SIG_Vector low( 0, 0, 0 );
      SIG_Vector high( 0, 0, 0 );
      bool first = true;
      for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
        {
          SIG_Vector corner = atStart( startPose, link, *vertex );
          for ( int axis = 0; axis < 3; axis++ )
            {
              if ( first || corner.get( axis ) < low.get( axis ) )
                low.set( axis, corner.get( axis ) );
              if ( first || corner.get( axis ) > high.get( axis ) )
                high.set( axis, corner.get( axis ) );
            }
          first = false;
        }
      lowCorners.append( low );
      highCorners.append( high );
    }

  // The same sample points on every call, so that the advice does not change between calls.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );

  QStringList pairs;
  double worstOverlap = 0;

  for ( int i = 0; i < links.size(); i++ )
    {
      for ( int j = i + 1; j < links.size(); j++ )
        {
          // The box that both links' boxes share.
          SIG_Vector low( 0, 0, 0 );
          SIG_Vector high( 0, 0, 0 );
          double boxVolume = 1;
          for ( int axis = 0; axis < 3; axis++ )
            {
              low.set( axis, std::max( lowCorners[i].get( axis ), lowCorners[j].get( axis ) ) );
              high.set( axis, std::min( highCorners[i].get( axis ), highCorners[j].get( axis ) ) );
              boxVolume *= std::max( 0.0, high.get( axis ) - low.get( axis ) );
            }
          if ( boxVolume == 0 )
            continue;

          int insideBoth = 0;
          for ( int sample = 0; sample < overlapSamples; sample++ )
            {
              // SIG_Randomizer gives numbers below 32768.
              SIG_Vector point( low.x + ( high.x - low.x ) * randomizer.getRandomInt( 32768 ) / 32767.0,
                                low.y + ( high.y - low.y ) * randomizer.getRandomInt( 32768 ) / 32767.0,
                                low.z + ( high.z - low.z ) * randomizer.getRandomInt( 32768 ) / 32767.0 );
              if ( isInside( triangles[i], point ) && isInside( triangles[j], point ) )
                insideBoth++;
            }

          double overlapVolume = boxVolume * insideBoth / overlapSamples;
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
    return;

  QString text = QString( "Links that overlap at the start pose (up to %1 % of the smaller link's volume): %2. The simulation lets links pass through each other, so the robot is not the one that was drawn." )
    .arg( worstOverlap, 0, 'g', 3 ).arg( pairs.join( ", " ) );
  findings.append( Finding{ 5, tWarning, pairs.join( ", " ), worstOverlap, overlapPercent, text } );
}

QList<SIGEL_GP::SIG_RobotAdvisor::Triangle> SIGEL_GP::SIG_RobotAdvisor::trianglesAtStart( const StartPose &startPose, const SIGEL_Robot::SIG_Link *link ) const
{
  QList<Triangle> triangles;

  SIGEL_Robot::SIG_GeometryIterator polygons( link->getGeometry() );
  while ( polygons )
    {
      const SIGEL_Robot::SIG_Polygon &polygon = polygons.iterate();

      // A polygon with more than three corners becomes a fan of triangles.
      for ( int i = 2; i < polygon.getNumVertices(); i++ )
        {
          triangles.append( Triangle{ atStart( startPose, link, polygon.getVertex( 0 ) ),
                                      atStart( startPose, link, polygon.getVertex( i - 1 ) ),
                                      atStart( startPose, link, polygon.getVertex( i ) ) } );
        }
    }

  return triangles;
}

bool SIGEL_GP::SIG_RobotAdvisor::isInside( const QList<Triangle> &triangles, const SIG_Vector &point ) const
{
  // A ray from a point inside a closed mesh leaves through an odd number of triangles.
  // The ray's direction is slanted so that it does not run along the faces of a box.
  const double dx = 0.5377, dy = 0.2131, dz = 0.8157;

  int crossings = 0;
  for ( const Triangle &triangle : triangles )
    {
      double e1x = triangle.b.x - triangle.a.x, e1y = triangle.b.y - triangle.a.y, e1z = triangle.b.z - triangle.a.z;
      double e2x = triangle.c.x - triangle.a.x, e2y = triangle.c.y - triangle.a.y, e2z = triangle.c.z - triangle.a.z;

      double px = dy * e2z - dz * e2y, py = dz * e2x - dx * e2z, pz = dx * e2y - dy * e2x;
      double determinant = e1x * px + e1y * py + e1z * pz;
      if ( determinant == 0 )
        continue;

      double tx = point.x - triangle.a.x, ty = point.y - triangle.a.y, tz = point.z - triangle.a.z;
      double u = ( tx * px + ty * py + tz * pz ) / determinant;
      if ( u < 0 || u > 1 )
        continue;

      double qx = ty * e1z - tz * e1y, qy = tz * e1x - tx * e1z, qz = tx * e1y - ty * e1x;
      double v = ( dx * qx + dy * qy + dz * qz ) / determinant;
      if ( v < 0 || u + v > 1 )
        continue;

      double distance = ( e2x * qx + e2y * qy + e2z * qz ) / determinant;
      if ( distance > 0 )
        crossings++;
    }

  return crossings % 2 == 1;
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

      startPose.jointMobility = jointMobilityOf( simulationData );
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      dmEnvironment::setEnvironment( environmentBefore );
      throw;
    }

  dmEnvironment::setEnvironment( environmentBefore );
  return startPose;
}

void SIGEL_GP::SIG_RobotAdvisor::adviseStepForJoints( QList<Finding> &findings, const StartPose &startPose ) const
{
  double mobility = startPose.jointMobility;
  if ( !( mobility > 0 ) )
    return;

  // The rates at which joint friction, the limit damper and the limit spring act on the joints.
  double frictionRate = simulationParameter.getJointFrictionU_c() * mobility;
  double damperRate = simulationParameter.getJointLimitsB_damper() * mobility;
  double springRate = std::sqrt( simulationParameter.getJointLimitsK_spring() * mobility );

  double fastestRate = std::max( { frictionRate, damperRate, springRate } );
  if ( !( fastestRate > 0 ) )
    return;

  double suggestedStep = jointStability / fastestRate;
  if ( simulationParameter.getStepSize() <= suggestedStep )
    return;

  QString text = QString( "Step size %1 is above the suggested %2 for the joints, limited by the joint-limit spring. Runs may still work; the spring acts only past a limit." );
  if ( fastestRate == damperRate )
    text = QString( "Step size %1 is above the suggested %2 for the joints, limited by the joint-limit damper. Runs may still work; the damper acts only past a limit." );
  if ( fastestRate == frictionRate )
    {
      text = QString( "Step size %1 is above the suggested %2 for the joints, limited by joint friction. It is close to the limit: the simulation works but has no margin." );
      if ( simulationParameter.getStepSize() * frictionRate > jointFrictionInstability )
        text = QString( "Step size %1 is above the suggested %2 for the joints, limited by joint friction. Expect scores of 0 from a simulation that breaks down; lower the step size or the joint friction." );
    }

  findings.append( Finding{ 9, tSuggestion, QString(), simulationParameter.getStepSize(), suggestedStep,
                            text.arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ) } );
}

double SIGEL_GP::SIG_RobotAdvisor::jointMobilityOf( SIGEL_Simulation::SIG_DynaMechsSimulationData &simulationData ) const
{
  for ( SIGEL_Robot::SIG_Joint *joint : robot.getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        return 0;
    }

  dmArticulation &system = simulationData.dynaMechsSystem;
  int stateSize = system.getNumDOFs();

  // Where each joint sits in the system's packed state.
  QList<dmLink *> jointLinks;
  QList<int> jointOffsets;
  int offset = 0;
  for ( unsigned int i = 0; i < system.getNumLinks(); i++ )
    {
      dmLink *link = system.getLink( i );
      if ( link->getNumDOFs() == 1 )
        {
          jointLinks.append( link );
          jointOffsets.append( offset );
        }
      offset += link->getNumDOFs();
    }

  int jointCount = jointLinks.size();
  if ( jointCount == 0 )
    return 0;

  std::vector<Float> position( stateSize );
  std::vector<Float> velocity( stateSize );
  system.getState( position.data(), velocity.data() );
  std::fill( velocity.begin(), velocity.end(), 0 );

  Float noTorque = 0;
  Float unitTorque = 1;
  for ( dmLink *link : jointLinks )
    link->setJointInput( &noTorque );

  // The same poses on every call, so that the advice does not change between calls.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  double mobility = 0;

  for ( int pose = 0; pose <= randomPoses; pose++ )
    {
      if ( pose > 0 )
        {
          for ( int j = 0; j < jointCount; j++ )
            {
              Float minimum, maximum, spring, damper;
              static_cast<dmMDHLink *>( jointLinks[j] )->getJointLimits( &minimum, &maximum, &spring, &damper );

              // A joint without limits can stand at any angle.
              if ( maximum - minimum > 2 * M_PI )
                {
                  minimum = -M_PI;
                  maximum = M_PI;
                }

              // SIG_Randomizer gives numbers below 32768.
              position[ jointOffsets[j] ] = minimum + ( maximum - minimum ) * randomizer.getRandomInt( 32768 ) / 32767.0;
            }
        }

      system.setState( position.data(), velocity.data() );

      std::vector<Float> state( position );
      state.insert( state.end(), velocity.begin(), velocity.end() );
      std::vector<Float> atRest( 2 * stateSize );
      system.ABDynamics( state.data(), atRest.data() );

      // One unit of torque on a joint, less the robot at rest, gives one column of the inverse mass matrix.
      NEWMAT::Matrix inverseMass( jointCount, jointCount );
      for ( int j = 0; j < jointCount; j++ )
        {
          std::vector<Float> pushed( 2 * stateSize );
          jointLinks[j]->setJointInput( &unitTorque );
          system.ABDynamics( state.data(), pushed.data() );
          jointLinks[j]->setJointInput( &noTorque );

          for ( int i = 0; i < jointCount; i++ )
            inverseMass( i + 1, j + 1 ) = pushed[ stateSize + jointOffsets[i] ] - atRest[ stateSize + jointOffsets[i] ];
        }

      NEWMAT::SymmetricMatrix symmetric( jointCount );
      for ( int i = 1; i <= jointCount; i++ )
        {
          for ( int j = 1; j <= i; j++ )
            symmetric( i, j ) = ( inverseMass( i, j ) + inverseMass( j, i ) ) / 2;
        }

      NEWMAT::DiagonalMatrix eigenvalues( jointCount );
      NEWMAT::EigenValues( symmetric, eigenvalues );
      mobility = std::max( mobility, static_cast<double>( eigenvalues( jointCount ) ) );
    }

  return mobility;
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
      double lowest, highest;
      extentAtStart( startRobot, startPose, axis, lowest, highest );
      size = std::max( size, highest - lowest );
    }

  return size;
}

void SIGEL_GP::SIG_RobotAdvisor::extentAtStart( const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose, int axis, double &lowest, double &highest ) const
{
  bool first = true;
  lowest = 0;
  highest = 0;

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
}

void SIGEL_GP::SIG_RobotAdvisor::adviseHoldsStartPose( QList<Finding> &errors, QList<Finding> &warnings, QList<Finding> &suggestions, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const
{
  double lowest, highest;
  extentAtStart( startRobot, startPose, 1, lowest, highest );
  double height = highest - lowest;

  // Placed on the floor, so that a fall from the start height does not count as sinking.
  SIGEL_Robot::SIG_Robot restRobot( robot );
  restRobot.prepareDynaMechs();
  // prepareDynaMechs sets the robot's location, so the lift comes after it.
  restRobot.initialLocation.y += floorLevel - lowest + restClearance;

  QString loosestJoint;
  double largestTurn = 0;
  double sink = 0;
  double brokeAfter = -1;

  // The simulation data makes its own environment the global one; other code still needs the one from before.
  dmEnvironment *environmentBefore = dmEnvironment::getEnvironment();
  {
    SIGEL_Simulation::SIG_DynaMechsSimulationData simulationData( restRobot, environment, simulationParameter );
    SIGEL_Simulation::SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

    QList<SIG_Matrix> orientationsAtStart;
    for ( int i = 0; i < simulationQueries.getLinkCount(); i++ )
      orientationsAtStart.append( simulationQueries.getLinkOrientation( i ) );

    int rootNumber = simulationQueries.getRootNumber();
    double heightAtStart = simulationQueries.getLinkPosition( rootNumber ).y;

    double seconds = std::min( restSeconds, static_cast<double>( QTime( 0, 0 ).secsTo( simulationParameter.getTimeToSimulate() ) ) );

    // No program runs, so no drive ever moves.
    while ( simulationQueries.getCurrentSimulationSeconds() < seconds )
      {
        simulationData.setNewFrame( true );
        simulationData.simulationProgress();
        simulationData.actualFrame++;

        SIG_Vector rootPosition = simulationQueries.getLinkPosition( rootNumber );
        if ( !std::isfinite( rootPosition.x ) || !std::isfinite( rootPosition.y ) || !std::isfinite( rootPosition.z ) )
          {
            brokeAfter = simulationQueries.getCurrentSimulationSeconds();
            break;
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
  dmEnvironment::setEnvironment( environmentBefore );

  if ( brokeAfter >= 0 )
    {
      QString text = QString( "The robot breaks the simulation after %1 s with no drive active. Lower the step size." ).arg( brokeAfter );
      errors.append( Finding{ 11, tError, QString(), brokeAfter, 0, text } );
      return;
    }

  double sinkPercent = sink / height * 100;

  if ( largestTurn > collapsesDegrees || sinkPercent > collapsesSinkPercent )
    {
      QString text = QString( "With no drive active the robot does not hold its start pose: joint %1 turns %2 degrees and the body sinks %3 (%4 % of the robot's height). Force drives are limp between MOVEs, so only joint limits hold a stance. Put the start angles on the limits that carry the weight, or use servo drives." )
        .arg( loosestJoint ).arg( largestTurn, 0, 'f', 0 ).arg( sink, 0, 'g', 3 ).arg( sinkPercent, 0, 'f', 0 );
      warnings.append( Finding{ 11, tWarning, loosestJoint, largestTurn, collapsesDegrees, text } );
    }
  else if ( largestTurn > settlesDegrees )
    {
      QString text = QString( "With no drive active the robot settles before it rests: joint %1 turns %2 degrees." )
        .arg( loosestJoint ).arg( largestTurn, 0, 'f', 0 );
      suggestions.append( Finding{ 11, tSuggestion, loosestJoint, largestTurn, settlesDegrees, text } );
    }
}

double SIGEL_GP::SIG_RobotAdvisor::turnBetween( SIG_Matrix left, SIG_Matrix right, SIG_Matrix leftAtStart, SIG_Matrix rightAtStart ) const
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
