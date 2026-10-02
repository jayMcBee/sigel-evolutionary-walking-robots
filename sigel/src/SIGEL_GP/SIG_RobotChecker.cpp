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
#include "SIGEL_GP/SIG_RobotChecker.h"

#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Simulation/SIG_JointMobility.h"
#include "SIGEL_Simulation/SIG_RestRun.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <qdatetime.h>

#include <algorithm>
#include <cmath>

SIGEL_GP::SIG_RobotChecker::SIG_RobotChecker( const SIGEL_Robot::SIG_Robot &robot,
                                              const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                                              const SIGEL_Environment::SIG_Environment &environment,
                                              const SIGEL_GP::SIG_GPParameter &gpParameter )
  : robot( robot ),
    simulationParameter( simulationParameter ),
    environment( environment ),
    gpParameter( gpParameter )
{
}

QList<SIGEL_GP::SIG_RobotChecker::Finding> SIGEL_GP::SIG_RobotChecker::check() const
{
  QList<Finding> errors;
  QList<Finding> warnings;
  QList<Finding> suggestions;

  try
    {
      SIGEL_Simulation::SIG_StartPose startPose( robot, environment, simulationParameter );

      bool everyLinkHasMass = checkLinkWithoutMass( errors, startPose );

      checkLimitCannotHoldDrive( warnings );
      if ( everyLinkHasMass )
        checkStrokeThrowsRobot( warnings, startPose );
      checkSenseWithoutSensors( warnings );
      if ( everyLinkHasMass )
        checkLinksOverlap( warnings, startPose );
      checkStartOutsideRange( warnings );

      checkAxisOffEdge( suggestions, startPose );
      checkStartHeight( suggestions, startPose );
      if ( everyLinkHasMass )
        {
          checkStepForGroundContact( suggestions, startPose );
          checkStepForJoints( suggestions, startPose );
          checkHoldsStartPose( errors, warnings, suggestions, startPose );
        }
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      errors.clear();
      warnings.clear();
      suggestions.clear();
      // The message's later lines name the source file that threw.
      errors.append( Finding{ 0, tError, "Robot cannot be simulated", QString(), e.getMessage().section( '\n', 0, 0 ) + " No program can be scored.",
                              "Correct the robot model.", 0, 0 } );
      checkLimitCannotHoldDrive( warnings );
      checkSenseWithoutSensors( warnings );
      checkStartOutsideRange( warnings );
    }

  return errors + warnings + suggestions;
}

bool SIGEL_GP::SIG_RobotChecker::checkLinkWithoutMass( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
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
    return true;

  findings.append( Finding{ 10, tError, "Link without mass", links.join( ", " ),
                            QString( "The mass of these links is 0 or less (lowest %1), so the simulation cannot work with them." ).arg( lowest ),
                            "The faces of their bodies probably face inwards. Turn them outwards.", lowest, 0 } );
  return false;
}

void SIGEL_GP::SIG_RobotChecker::checkLimitCannotHoldDrive( QList<Finding> &findings ) const
{
  double spring = simulationParameter.getJointLimitsK_spring();
  if ( spring <= 0 )
    {
      findings.append( Finding{ 1, tWarning, "Joint limit cannot hold its drive", QString(),
                                QString( "The joint-limit spring is %1, so no joint limit holds a drive." ).arg( spring ),
                                "Raise the joint-limit spring.", spring, 0 } );
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

  findings.append( Finding{ 1, tWarning, "Joint limit cannot hold its drive", drives.join( ", " ),
                            QString( "These drives can push their joint up to %1 degrees past its limit, more than the joint's range of %2 degrees. The joint may fold through or spin freely." ).arg( worstPastLimit, 0, 'f', 0 ).arg( worstRange ),
                            "Lower the drive force or raise the joint-limit spring.", worstPastLimit, worstRange } );
}

void SIGEL_GP::SIG_RobotChecker::checkStrokeThrowsRobot( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  double totalMass = 0;
  for ( double mass : startPose.getMasses() )
    totalMass += mass;

  // Without gravity nothing has a weight to lift.
  double weight = totalMass * std::abs( environment.getGravity().y );
  if ( weight == 0 )
    return;

  double startHeight = environment.getStartPosition().y - floorLevel;
  double throwHeight = throwStartHeights * startHeight;

  QStringList drives;
  double highestStroke = 0;

  for ( SIGEL_Robot::SIG_Drive *drive : robot.getDrives() )
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

  findings.append( Finding{ 2, tWarning, "Drive too strong for the robot's weight", drives.join( ", " ),
                            QString( "At full force, one turn of the joint through its range can lift the whole robot up to %1 high. The robot starts at height %2. Random programs throw it into the air." ).arg( highestStroke, 0, 'g', 3 ).arg( startHeight ),
                            "Lower the drive force.", highestStroke, throwHeight } );
}

void SIGEL_GP::SIG_RobotChecker::checkSenseWithoutSensors( QList<Finding> &findings ) const
{
  // With probability 0 no program contains a SENSE.
  if (    robot.getSensors().isEmpty()
       && robot.getLangParam()->hasCommand( "SENSE" )
       && gpParameter.getProbability( SIGEL_Program::SENSE ) > 0 )
    findings.append( Finding{ 3, tWarning, "SENSE without sensors", QString(),
                              "The robot has no sensors but SENSE is allowed, so every SENSE does nothing.",
                              "Give the robot sensors, or take SENSE out of the language parameters.", 0, 0 } );
}

void SIGEL_GP::SIG_RobotChecker::checkAxisOffEdge( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
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

  findings.append( Finding{ 4, tSuggestion, "No link edge on the joint's axis", pairs.join( ", " ),
                            QString( "The joint's axis is up to %1 % of the robot's size away from the nearest edge of the link. The parts may overlap or float apart when the joint turns." ).arg( worstOffEdge, 0, 'g', 3 ),
                            "Look at the robot. A pin in a fork is sound.", worstOffEdge, axisOffEdgePercent } );
}

void SIGEL_GP::SIG_RobotChecker::checkLinksOverlap( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  const QList<SIGEL_Robot::SIG_Link *> &links = startPose.getRobot().getLinks();
  const QList<double> &masses = startPose.getMasses();

  QList<SIGEL_Robot::SIG_TriangleMesh> meshes;
  for ( SIGEL_Robot::SIG_Link *link : links )
    meshes.append( startPose.getMesh( link ) );

  // The same sample points on every call, so that the findings do not change between calls.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );

  QStringList pairs;
  double worstOverlap = 0;

  for ( int i = 0; i < links.size(); i++ )
    {
      for ( int j = i + 1; j < links.size(); j++ )
        {
          double overlapVolume = meshes[i].sharedVolume( meshes[j], overlapSamples, randomizer );
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

  findings.append( Finding{ 5, tWarning, "Links overlap", pairs.join( ", " ),
                            QString( "At the start pose these links share up to %1 % of the smaller link's volume. The simulation lets links pass through each other." ).arg( worstOverlap, 0, 'g', 3 ),
                            "Move the links apart in the robot model.", worstOverlap, overlapPercent } );
}

void SIGEL_GP::SIG_RobotChecker::checkStartOutsideRange( QList<Finding> &findings ) const
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

  findings.append( Finding{ 6, tWarning, "Joint starts outside its range", joints.join( ", " ),
                            "The start angle of these joints is outside their own range, so the limit spring kicks them at the first step.",
                            "Put init between minimal and maximal.", firstInitial, firstLimit } );
}

bool SIGEL_GP::SIG_RobotChecker::startsOutsideRange( double minimum, double maximum, double initial ) const
{
  // A joint whose minimum equals its maximum has no limit.
  if ( minimum == maximum )
    return false;

  return initial < minimum || initial > maximum;
}

void SIGEL_GP::SIG_RobotChecker::checkStartHeight( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  // A face that lies on the floor is below it by rounding only.
  double belowFloor = floorLevel - 1e-9 * startPose.getSize();

  int verticesBelowFloor = 0;
  double lowest = floorLevel;
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
    return;

  double startHeight = environment.getStartPosition().y;
  findings.append( Finding{ 7, tSuggestion, "Robot starts inside the floor", lowestLink,
                            QString( "Start height %1 puts %2 vertices below the floor, the lowest on this link. The floor throws the robot up at the first step." ).arg( startHeight ).arg( verticesBelowFloor ),
                            QString( "Set the start height to %1 or more." ).arg( startHeight + floorLevel - lowest ), lowest, floorLevel } );
}

void SIGEL_GP::SIG_RobotChecker::checkStepForGroundContact( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  const QList<double> &masses = startPose.getMasses();
  if ( masses.isEmpty() )
    return;

  int lightest = std::min_element( masses.begin(), masses.end() ) - masses.begin();
  double lightestMass = masses[ lightest ];
  QString lightestLink = startPose.getRobot().getLinks()[ lightest ]->getName();

  // The fastest rate of the ground's penalty springs and dampers on the lightest link.
  double fastestRate = std::max( { std::sqrt( environment.getGroundNormalSpringConstant() / lightestMass ),
                                   std::sqrt( environment.getGroundPlanarSpringConstant() / lightestMass ),
                                   environment.getGroundNormalDamperConstant() / lightestMass,
                                   environment.getGroundPlanarDamperConstant() / lightestMass } );
  double suggestedStep = groundContactStability / fastestRate;

  if ( simulationParameter.getStepSize() > suggestedStep )
    {
      findings.append( Finding{ 8, tSuggestion, "Step size for ground contact", lightestLink,
                                QString( "Step size %1 is above the suggested %2 for ground contact on the lightest link (mass %3). Runs may still work; if they do not, robots fly off or score 0." ).arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( lightestMass, 0, 'g', 3 ),
                                "Lower the step size.", simulationParameter.getStepSize(), suggestedStep } );
    }
}

void SIGEL_GP::SIG_RobotChecker::checkStepForJoints( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  SIGEL_Simulation::SIG_JointMobility jointMobility( startPose.getRobot(), environment, simulationParameter );
  double mobility = jointMobility.getLargest();
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

  QString limitedBy = "the joint-limit spring";
  QString effect = "Runs may still work; the spring acts only past a limit.";
  QString advice = "Lower the step size or the joint-limit spring.";
  if ( fastestRate == damperRate )
    {
      limitedBy = "the joint-limit damper";
      effect = "Runs may still work; the damper acts only past a limit.";
      advice = "Lower the step size or the joint-limit damper.";
    }
  if ( fastestRate == frictionRate )
    {
      limitedBy = "joint friction";
      effect = "The step is close to the limit: the simulation works but has no margin.";
      if ( simulationParameter.getStepSize() * frictionRate > jointFrictionInstability )
        effect = "The simulation breaks down, and programs score 0.";
      advice = "Lower the step size or the joint friction.";
    }

  findings.append( Finding{ 9, tSuggestion, "Step size for the joints", QString(),
                            QString( "Step size %1 is above the suggested %2 for the joints, limited by %3. " ).arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( limitedBy ) + effect,
                            advice, simulationParameter.getStepSize(), suggestedStep } );
}

double SIGEL_GP::SIG_RobotChecker::offAxis( const SIGEL_Simulation::SIG_StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector axisPoint, SIG_Vector axis ) const
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

void SIGEL_GP::SIG_RobotChecker::checkHoldsStartPose( QList<Finding> &errors, QList<Finding> &warnings, QList<Finding> &suggestions, const SIGEL_Simulation::SIG_StartPose &startPose ) const
{
  // Axis 1 is y, which points up.
  double lowest, highest;
  startPose.getExtent( 1, lowest, highest );
  double height = highest - lowest;

  double seconds = std::min( restSeconds, static_cast<double>( QTime( 0, 0 ).secsTo( simulationParameter.getTimeToSimulate() ) ) );

  // Placed on the floor, so that a fall from the start height does not count as sinking.
  SIGEL_Simulation::SIG_RestRun restRun( robot, environment, simulationParameter, floorLevel - lowest + restClearance, seconds );

  if ( restRun.hasBrokenDown() )
    {
      errors.append( Finding{ 11, tError, "Simulation breaks at rest", QString(),
                              QString( "With no drive active the simulation breaks after %1 s, so no program can be scored." ).arg( restRun.getBrokeAfter() ),
                              "Lower the step size.", restRun.getBrokeAfter(), 0 } );
      return;
    }

  QString loosestJoint = restRun.getLoosestJoint();
  double largestTurn = restRun.getLargestTurn();
  double sink = restRun.getSink();
  double sinkPercent = sink / height * 100;

  if ( largestTurn > collapsesDegrees || sinkPercent > collapsesSinkPercent )
    {
      warnings.append( Finding{ 11, tWarning, "Robot does not hold its start pose", loosestJoint,
                                QString( "With no drive active this joint turns %1 degrees and the body sinks %2 (%3 % of the robot's height). Force drives apply no force between MOVEs, so only joint limits hold a stance." ).arg( largestTurn, 0, 'f', 0 ).arg( sink, 0, 'g', 3 ).arg( sinkPercent, 0, 'f', 0 ),
                                "Put the start angles on the limits that carry the weight, or use servo drives.", largestTurn, collapsesDegrees } );
    }
  else if ( largestTurn > settlesDegrees )
    {
      suggestions.append( Finding{ 11, tSuggestion, "Robot settles before it rests", loosestJoint,
                                   QString( "With no drive active this joint turns %1 degrees before the robot rests, so every run starts with this motion." ).arg( largestTurn, 0, 'f', 0 ),
                                   "Put the start angles where the robot rests.", largestTurn, settlesDegrees } );
    }
}
