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
#include "SIGEL_RobotCheck/TST_SIG_RobotStartPose.h"

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_RobotCheck/SIG_LinkVolume.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QtTest>

#include <cmath>

namespace {

  const double epsilon = 1e-9;

  // The geometry owns its polygons and frees them. The corners go counter-clockwise
  // seen from outside the solid, so that each face normal points outwards.
  void addFace( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &corners )
  {
    SIGEL_Robot::SIG_Polygon *polygon = new SIGEL_Robot::SIG_Polygon( &geometry );
    for ( const SIG_Vector &corner : corners )
      polygon->appendVertex( corner );
  }

  // A straight prism: outline is its bottom face, counter-clockwise seen from above.
  void addPrism( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &outline, double height )
  {
    QList< SIG_Vector > bottom;
    QList< SIG_Vector > top;
    for ( const SIG_Vector &corner : outline ) {
      bottom.prepend( corner );
      top.append( SIG_Vector( corner.x, corner.y, corner.z + height ) );
    }
    addFace( geometry, bottom );
    addFace( geometry, top );
    for ( int i = 0; i < outline.count(); i++ ) {
      const SIG_Vector &from = outline.at( i );
      const SIG_Vector &to = outline.at( ( i + 1 ) % outline.count() );
      addFace( geometry, { from, to, SIG_Vector( to.x, to.y, to.z + height ), SIG_Vector( from.x, from.y, from.z + height ) } );
    }
  }

  void addBox( SIGEL_Robot::SIG_Geometry &geometry, SIG_Vector low, SIG_Vector high )
  {
    addPrism( geometry,
              { low, SIG_Vector( high.x, low.y, low.z ), SIG_Vector( high.x, high.y, low.z ), SIG_Vector( low.x, high.y, low.z ) },
              high.z - low.z );
  }

  // A link with a body and a material of its own; the robot owns all three.
  SIGEL_Robot::SIG_Link *addLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIGEL_Robot::SIG_Geometry *geometry, double density )
  {
    SIGEL_Robot::SIG_Body *body = new SIGEL_Robot::SIG_Body( &robot, name + ".wrl", "parts/" );
    body->setGeometry( geometry );
    SIGEL_Robot::SIG_Material *material = new SIGEL_Robot::SIG_Material( &robot, name );
    material->setDensity( density );
    SIGEL_Robot::SIG_Link *link = new SIGEL_Robot::SIG_Link( &robot, name, robot.getLinks().count() );
    link->setBody( body );
    link->setMaterial( material );
    robot.addBody( body );
    robot.addMaterial( material );
    robot.addLink( link );
    return link;
  }

  SIGEL_Robot::SIG_Link *addBoxLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIG_Vector low, SIG_Vector high, double density )
  {
    SIGEL_Robot::SIG_Geometry *geometry = new SIGEL_Robot::SIG_Geometry();
    addBox( *geometry, low, high );
    return addLink( robot, name, geometry, density );
  }

  // The joint between base and arm: on both, the axis runs along y and the hand points along x,
  // one unit from the axis base, so at a start value of 0 the joint puts the arm two units along
  // x from the base.
  void addArmJoint( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType jointType, SIGEL_Robot::SIG_Link *base, SIGEL_Robot::SIG_Link *arm, double startValue )
  {
    const SIG_Vector baseAxisBase( 1, 0, 0 );
    const SIG_Vector baseAxisDirection( 1, 1, 0 );
    const SIG_Vector baseHand( 2, 0, 0 );
    const SIG_Vector armAxisBase( -1, 0, 0 );
    const SIG_Vector armAxisDirection( -1, 1, 0 );
    const SIG_Vector armHand( 0, 0, 0 );

    if ( jointType == SIGEL_Robot::SIG_Joint::tRotationalJoint ) {
      SIGEL_Robot::SIG_RotationalJoint *joint = new SIGEL_Robot::SIG_RotationalJoint( &robot, "hinge", 0 );
      joint->setLeftLink( base );
      joint->setRightLink( arm );
      joint->setLeftPoints( baseAxisBase, baseAxisDirection, baseHand );
      joint->setRightPoints( armAxisBase, armAxisDirection, armHand );
      joint->setRange( -120, 120, startValue );
      robot.addJoint( joint );
      return;
    }

    SIGEL_Robot::SIG_TranslationalJoint *joint = new SIGEL_Robot::SIG_TranslationalJoint( &robot, "slider", 0 );
    joint->setLeftLink( base );
    joint->setRightLink( arm );
    joint->setLeftPoints( baseAxisBase, baseAxisDirection, baseHand );
    joint->setRightPoints( armAxisBase, armAxisDirection, armHand );
    joint->setRange( -1, 1, startValue );
    robot.addJoint( joint );
  }

  // In the model, base fills x -1..1, y -0.5..3, z -0.5..0.5 and, through the joint at a start
  // value of 0, arm fills x 1..3, y -2..0.25, z -0.25..0.25.
  void buildBaseAndArm( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType jointType, double startValue )
  {
    SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
    SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ), 4 );
    robot.setRootLink( base );
    addArmJoint( robot, jointType, base, arm, startValue );
  }

  // The volume of the box from low to high, as triangles.
  SIGEL_RobotCheck::SIG_LinkVolume boxVolume( SIG_Vector low, SIG_Vector high )
  {
    const SIG_Vector corners[ 8 ] = { low, SIG_Vector( high.x, low.y, low.z ), SIG_Vector( high.x, high.y, low.z ), SIG_Vector( low.x, high.y, low.z ),
                                      SIG_Vector( low.x, low.y, high.z ), SIG_Vector( high.x, low.y, high.z ), high, SIG_Vector( low.x, high.y, high.z ) };
    const int faces[ 6 ][ 4 ] = { { 0, 3, 2, 1 }, { 4, 5, 6, 7 }, { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 }, { 3, 0, 4, 7 } };
    SIGEL_RobotCheck::SIG_LinkVolume volume;
    for ( int face = 0; face < 6; face++ ) {
      volume.addTriangle( corners[ faces[ face ][ 0 ] ], corners[ faces[ face ][ 1 ] ], corners[ faces[ face ][ 2 ] ] );
      volume.addTriangle( corners[ faces[ face ][ 0 ] ], corners[ faces[ face ][ 2 ] ], corners[ faces[ face ][ 3 ] ] );
    }
    return volume;
  }

  // Empty if actual is (x, y, z), else what it is.
  QString vectorMismatch( SIG_Vector actual, double x, double y, double z )
  {
    if ( std::fabs( actual.x - x ) < epsilon && std::fabs( actual.y - y ) < epsilon && std::fabs( actual.z - z ) < epsilon )
      return QString();
    return QString( "is (%1, %2, %3), expected (%4, %5, %6)" )
      .arg( actual.x ).arg( actual.y ).arg( actual.z ).arg( x ).arg( y ).arg( z );
  }

  // Empty if, in the world, the link's vertices are the eight corners of the box from low
  // to high, each once; else what is wrong.
  QString boxMismatch( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector low, SIG_Vector high )
  {
    QList< bool > cornerSeen( 8, false );
    for ( SIG_Vector *vertex : link->getGeometry()->getVertices() ) {
      SIG_Vector world = startPose.toWorld( link, *vertex );
      int corner = 0;
      int weight = 1;
      for ( int axis = 0; axis < 3; axis++ ) {
        if ( std::fabs( world.get( axis ) - high.get( axis ) ) < epsilon )
          corner += weight;
        else if ( !( std::fabs( world.get( axis ) - low.get( axis ) ) < epsilon ) )
          return QString( "a vertex of %1 is at (%2, %3, %4)" ).arg( link->getName() ).arg( world.x ).arg( world.y ).arg( world.z );
        weight *= 2;
      }
      if ( cornerSeen[ corner ] )
        return QString( "two vertices of %1 are at (%2, %3, %4)" ).arg( link->getName() ).arg( world.x ).arg( world.y ).arg( world.z );
      cornerSeen[ corner ] = true;
    }
    if ( cornerSeen.contains( false ) )
      return QString( "%1 has a corner without a vertex" ).arg( link->getName() );
    return QString();
  }

}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aRobotTheSimulationTakesCanBeSimulatedAndHasNoRefusal()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -1, -1.5 ), SIG_Vector( 0.5, 1, 1.5 ), 2 ) );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  QCOMPARE( startPose.getRefusal(), QString() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::getMassesGivesEachLinksMassInTheOrderOfTheRobotsLinks()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  // Density times volume: 2 * ( 2 * 3.5 * 1 ) for base, 4 * ( 2 * 2.25 * 0.5 ) for arm.
  const QList< double > &masses = startPose.getMasses();
  QCOMPARE( masses.count(), 2 );
  QVERIFY2( std::fabs( masses[ 0 ] - 14 ) < epsilon, qPrintable( QString::number( masses[ 0 ] ) ) );
  QVERIFY2( std::fabs( masses[ 1 ] - 9 ) < epsilon, qPrintable( QString::number( masses[ 1 ] ) ) );
  QVERIFY( startPose.everyLinkHasMass() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aLinkWhoseFacesFaceInwardsStopsTheRobotFromBeingSimulated()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Geometry *inwards = new SIGEL_Robot::SIG_Geometry();

  // Low and high swapped on one axis turn every face of the box inwards.
  addBox( *inwards, SIG_Vector( -1, -2, 0.25 ), SIG_Vector( 1, 0.25, -0.25 ) );
  SIGEL_Robot::SIG_Link *arm = addLink( robot, "arm", inwards, 4 );
  robot.setRootLink( base );
  addArmJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, base, arm, 0 );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY( !startPose.canBeSimulated() );
  QVERIFY2( startPose.getRefusal().contains( "link arm" ), qPrintable( startPose.getRefusal() ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aLinkOfDensityZeroStopsTheRobotFromBeingSimulated()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ), 0 );
  robot.setRootLink( base );
  addArmJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, base, arm, 0 );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY( !startPose.canBeSimulated() );
  QVERIFY2( startPose.getRefusal().contains( "link arm" ), qPrintable( startPose.getRefusal() ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::getRobotGivesAPreparedCopyAndTheGivenRobotStaysAsItIs()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  const SIGEL_Robot::SIG_Robot &prepared = startPose.getRobot();
  QCOMPARE( prepared.getLinks().count(), 2 );
  QCOMPARE( prepared.getLinks()[ 0 ]->getName(), QString( "base" ) );
  QCOMPARE( prepared.getLinks()[ 1 ]->getName(), QString( "arm" ) );
  QCOMPARE( prepared.getRootLink(), prepared.getLinks()[ 0 ] );
  QCOMPARE( prepared.getJoints().count(), 1 );

  // A link gets a geometry of its own only when the robot is prepared for the simulation.
  QVERIFY( prepared.getLinks()[ 0 ]->getGeometry() != nullptr );
  QVERIFY( prepared.getLinks()[ 1 ]->getGeometry() != nullptr );
  QCOMPARE( robot.getLinks()[ 0 ]->getGeometry(), nullptr );
  QCOMPARE( robot.getLinks()[ 1 ]->getGeometry(), nullptr );
}

// The box lies off its origin, so its origin is not its centre of mass.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aRobotWithoutJointsHasItsOriginAtTheStartPositionInTheOrientationOfTheModel()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( 0, 0, 0 ), SIG_Vector( 1, 2, 3 ), 2 ) );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const SIGEL_Robot::SIG_Link *box = startPose.getRobot().getLinks()[ 0 ];
  QCOMPARE( vectorMismatch( startPose.getPosition( box ), 3, 4, 5 ), QString() );
  SIG_Matrix orientation = startPose.getOrientation( box );
  QCOMPARE( vectorMismatch( orientation.c0, 1, 0, 0 ), QString() );
  QCOMPARE( vectorMismatch( orientation.c1, 0, 1, 0 ), QString() );
  QCOMPARE( vectorMismatch( orientation.c2, 0, 0, 1 ), QString() );
}

// The box lies off its origin, so its origin is not its centre of mass.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::toWorldPutsEachVertexWhereTheModelHasItMovedByTheStartPosition()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( 0, 0, 0 ), SIG_Vector( 1, 2, 3 ), 2 ) );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const SIGEL_Robot::SIG_Link *box = startPose.getRobot().getLinks()[ 0 ];
  QCOMPARE( boxMismatch( startPose, box, SIG_Vector( 3, 4, 5 ), SIG_Vector( 4, 6, 8 ) ), QString() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::eachLinkOfARobotWithARotationalJointIsWhereTheModelHasItMovedByTheStartPosition()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::eachLinkOfARobotWithATranslationalJointIsWhereTheModelHasItMovedByTheStartPosition()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
}

// In the model, a start value of 90 turns the arm by -90 degrees about the axis direction +y,
// so that it points along +z from the axis instead of along +x.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aRotationalJointThatStartsTurnedTurnsTheLinkBehindItAboutTheJointAxis()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 90 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 3.75, 2, 5 ), SIG_Vector( 4.25, 4.25, 7 ) ), QString() );
}

// In the model, a start value moves the arm along the axis direction, +y.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aTranslationalJointThatStartsMovedMovesTheLinkBehindItAlongTheJointAxis()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 0.5 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 4, 2.5, 4.75 ), SIG_Vector( 6, 4.75, 5.25 ) ), QString() );
}

// The root is link 1 and the right link of its joint: masses and placements follow the link number.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aRootThatIsNotTheFirstLinkGivesEachLinkItsOwnMassAndPlace()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ), 4 );
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  robot.setRootLink( base );
  SIGEL_Robot::SIG_RotationalJoint *hinge = new SIGEL_Robot::SIG_RotationalJoint( &robot, "hinge", 0 );
  hinge->setLeftLink( arm );
  hinge->setRightLink( base );
  hinge->setLeftPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  hinge->setRightPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
  hinge->setRange( -120, 120, 0 );
  robot.addJoint( hinge );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( startPose.getMasses().count(), 2 );
  QVERIFY2( std::fabs( startPose.getMasses()[ 0 ] - 9 ) < epsilon, qPrintable( QString::number( startPose.getMasses()[ 0 ] ) ) );
  QVERIFY2( std::fabs( startPose.getMasses()[ 1 ] - 14 ) < epsilon, qPrintable( QString::number( startPose.getMasses()[ 1 ] ) ) );
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );

  // The arm is behind the joint, so its frame has its origin and z axis on the axis through ( 4, 4, 5 ) along y.
  SIG_Vector origin = startPose.getPosition( links[ 0 ] );
  QVERIFY2( std::fabs( origin.x - 4 ) < epsilon && std::fabs( origin.z - 5 ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( origin.x ).arg( origin.y ).arg( origin.z ) ) );
  SIG_Vector zAxis = startPose.getOrientation( links[ 0 ] ).c2;
  QVERIFY2( std::fabs( zAxis.x ) < epsilon && std::fabs( std::fabs( zAxis.y ) - 1 ) < epsilon && std::fabs( zAxis.z ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( zAxis.x ).arg( zAxis.y ).arg( zAxis.z ) ) );
}

// A chain base, arm, tip: on arm, the axis of shoulder is at x -1 and the axis of elbow at x 1,
// both along y. Elbow starts at 90, which turns tip by -90 degrees about +y, so that it points
// along +z from the elbow axis, which in the world runs through ( 6, 4, 5 ).
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aLinkTwoJointsFromTheRootIsWhereTheModelHasItMovedByTheStartPosition()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ), 4 );
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ), 4 );
  robot.setRootLink( base );
  SIGEL_Robot::SIG_RotationalJoint *shoulder = new SIGEL_Robot::SIG_RotationalJoint( &robot, "shoulder", 0 );
  shoulder->setLeftLink( base );
  shoulder->setRightLink( arm );
  shoulder->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
  shoulder->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  shoulder->setRange( -120, 120, 0 );
  robot.addJoint( shoulder );
  SIGEL_Robot::SIG_RotationalJoint *elbow = new SIGEL_Robot::SIG_RotationalJoint( &robot, "elbow", 1 );
  elbow->setLeftLink( arm );
  elbow->setRightLink( tip );
  elbow->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
  elbow->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  elbow->setRange( -120, 120, 90 );
  robot.addJoint( elbow );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  const QList< SIGEL_Robot::SIG_Link * > &links = startPose.getRobot().getLinks();
  QCOMPARE( boxMismatch( startPose, links[ 0 ], SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 1 ], SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
  QCOMPARE( boxMismatch( startPose, links[ 2 ], SIG_Vector( 5.75, 3, 5 ), SIG_Vector( 6.25, 4, 7 ) ), QString() );
  SIG_Vector origin = startPose.getPosition( links[ 2 ] );
  QVERIFY2( std::fabs( origin.x - 6 ) < epsilon && std::fabs( origin.z - 5 ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( origin.x ).arg( origin.y ).arg( origin.z ) ) );
  SIG_Vector zAxis = startPose.getOrientation( links[ 2 ] ).c2;
  QVERIFY2( std::fabs( zAxis.x ) < epsilon && std::fabs( std::fabs( zAxis.y ) - 1 ) < epsilon && std::fabs( zAxis.z ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( zAxis.x ).arg( zAxis.y ).arg( zAxis.z ) ) );
}

// The simulation turns the link behind a rotational joint about the z axis of its frame.
void SIGEL_RobotCheck::TST_SIG_RobotStartPose::theLinkBehindARotationalJointHasItsOriginAndItsZAxisOnTheJointAxis()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );

  // In the world, the axis is the line through ( 4, 4, 5 ) along y; the sense of the z axis is free.
  const SIGEL_Robot::SIG_Link *arm = startPose.getRobot().getLinks()[ 1 ];
  SIG_Vector origin = startPose.getPosition( arm );
  QVERIFY2( std::fabs( origin.x - 4 ) < epsilon && std::fabs( origin.z - 5 ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( origin.x ).arg( origin.y ).arg( origin.z ) ) );
  SIG_Vector zAxis = startPose.getOrientation( arm ).c2;
  QVERIFY2( std::fabs( zAxis.x ) < epsilon && std::fabs( std::fabs( zAxis.y ) - 1 ) < epsilon && std::fabs( zAxis.z ) < epsilon,
            qPrintable( QString( "(%1, %2, %3)" ).arg( zAxis.x ).arg( zAxis.y ).arg( zAxis.z ) ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::getExtentGivesTheLowestAndHighestCoordinateOfAllLinksInTheWorld()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( -10, 4, -10 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  double lowest = 0;
  double highest = 0;

  // On x and y, the lowest coordinate is on one link and the highest on the other. On x and z
  // every coordinate is below 0, on y every coordinate is above 0.
  startPose.getExtent( 0, lowest, highest );
  QCOMPARE( vectorMismatch( SIG_Vector( lowest, highest, 0 ), -11, -7, 0 ), QString() );
  startPose.getExtent( 1, lowest, highest );
  QCOMPARE( vectorMismatch( SIG_Vector( lowest, highest, 0 ), 2, 7, 0 ), QString() );
  startPose.getExtent( 2, lowest, highest );
  QCOMPARE( vectorMismatch( SIG_Vector( lowest, highest, 0 ), -10.5, -9.5, 0 ), QString() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::getSizeIsTheLargestOfTheThreeExtents()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );

  // The extents are 4 along x, 5 along y and 1 along z.
  QVERIFY2( std::fabs( startPose.getSize() - 5 ) < epsilon, qPrintable( QString::number( startPose.getSize() ) ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::atTheSafeStartHeightTheLowestVertexIsOnTheFloor()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );

  // The lowest vertex of the model is on the arm, 2 below the origin.
  const double safeStartHeight = startPose.getSafeStartHeight();
  QVERIFY2( std::fabs( safeStartHeight - 2 ) < epsilon, qPrintable( QString::number( safeStartHeight ) ) );
  environment.setStartPosition( SIG_Vector( 3, safeStartHeight, 5 ) );
  SIG_RobotStartPose onTheFloor( robot, environment, simulationParameters );
  QVERIFY2( onTheFloor.canBeSimulated(), qPrintable( onTheFloor.getRefusal() ) );
  double lowest = 1;
  double highest = 0;
  onTheFloor.getExtent( 1, lowest, highest );
  QVERIFY2( std::fabs( lowest - SIG_RobotStartPose::floorLevel ) < epsilon, qPrintable( QString::number( lowest ) ) );
  QVERIFY2( std::fabs( onTheFloor.getSafeStartHeight() - 2 ) < epsilon, qPrintable( QString::number( onTheFloor.getSafeStartHeight() ) ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::getLinkVolumeHoldsTheWholeLinkWhereTheSimulationPutsIt()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Geometry *sixCorners = new SIGEL_Robot::SIG_Geometry();

  // The arm box of buildBaseAndArm, with six corners on its top and bottom faces. The outline
  // starts in the middle of an edge: SIG_Mirtich takes a face's normal from its first three
  // corners, so these must not be on one line, and no triangle of a fan from there is flat.
  addPrism( *sixCorners,
            { SIG_Vector( 1, -1, -0.25 ), SIG_Vector( 1, 0.25, -0.25 ), SIG_Vector( 0, 0.25, -0.25 ),
              SIG_Vector( -1, 0.25, -0.25 ), SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, -2, -0.25 ) },
            0.5 );
  SIGEL_Robot::SIG_Link *arm = addLink( robot, "arm", sixCorners, 4 );
  robot.setRootLink( base );
  addArmJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, base, arm, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY2( startPose.canBeSimulated(), qPrintable( startPose.getRefusal() ) );
  SIG_LinkVolume volume = startPose.getLinkVolume( startPose.getRobot().getLinks()[ 1 ] );

  // The arm fills x 4..6, y 2..4.25, z 4.75..5.25 in the world, a volume of 2.25. Every sample
  // point is inside that box, so it must be inside the link's volume too.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  const double shared = volume.sharedWith( boxVolume( SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), 1000, randomizer );
  QVERIFY2( std::fabs( shared - 2.25 ) < epsilon, qPrintable( QString::number( shared ) ) );

  // A box one unit larger on every side, which also holds part of the base, shares with the
  // link's volume no more than the arm.
  const double sharedWithLarger = volume.sharedWith( boxVolume( SIG_Vector( 3, 1, 3.75 ), SIG_Vector( 7, 5.25, 6.25 ) ), 1000, randomizer );
  QVERIFY2( std::fabs( sharedWithLarger - 2.25 ) < epsilon, qPrintable( QString::number( sharedWithLarger ) ) );
  const double beside = volume.distanceToLine( SIG_Vector( 8, 3, 5 ), SIG_Vector( 0, 0, 1 ) );
  QVERIFY2( std::fabs( beside - 2 ) < epsilon, qPrintable( QString::number( beside ) ) );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::aJointThatIsNeitherRotationalNorTranslationalIsRefusedInOneLineThatNamesTheJoint()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Link *lid = addBoxLink( robot, "lid", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 0.5, 0.5 ), 2 );
  robot.setRootLink( base );
  SIGEL_Robot::SIG_GlueJoint *glue = new SIGEL_Robot::SIG_GlueJoint( &robot, "glue", 0 );
  glue->setLeftLink( base );
  glue->setRightLink( lid );
  glue->setPlaneA( SIG_Vector( 0, 3, 0 ), SIG_Vector( 1, 3, 0 ), SIG_Vector( 0, 3, 1 ) );
  glue->setPlaneB( SIG_Vector( 0, -0.5, 0 ), SIG_Vector( 1, -0.5, 0 ), SIG_Vector( 0, -0.5, 1 ) );
  robot.addJoint( glue );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY( !startPose.canBeSimulated() );
  QCOMPARE( startPose.getRefusal(), QString( "Joint glue is neither a rotational nor a translational joint." ) );
  QVERIFY( !startPose.everyLinkHasMass() );
}

void SIGEL_RobotCheck::TST_SIG_RobotStartPose::jointAxesThatCutInALinkAreRefusedInOneLineThatNamesTheLink()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ), 2 );
  SIGEL_Robot::SIG_Link *middle = addBoxLink( robot, "middle", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 0.5, 0.5 ), 2 );
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 0.5, 0.5 ), 2 );
  robot.setRootLink( base );

  // On middle, the axis of first runs along z and the axis of second along y, both through ( -1, 0, 0 ).
  SIGEL_Robot::SIG_RotationalJoint *first = new SIGEL_Robot::SIG_RotationalJoint( &robot, "first", 0 );
  first->setLeftLink( base );
  first->setRightLink( middle );
  first->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 0, 1 ), SIG_Vector( 2, 0, 0 ) );
  first->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 0, 1 ), SIG_Vector( 0, 0, 0 ) );
  first->setRange( -90, 90, 0 );
  robot.addJoint( first );
  SIGEL_Robot::SIG_RotationalJoint *second = new SIGEL_Robot::SIG_RotationalJoint( &robot, "second", 1 );
  second->setLeftLink( middle );
  second->setRightLink( tip );
  second->setLeftPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  second->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  second->setRange( -90, 90, 0 );
  robot.addJoint( second );
  SIGEL_Environment::SIG_Environment environment;
  SIGEL_Simulation::SIG_SimulationParameters simulationParameters;

  SIG_RobotStartPose startPose( robot, environment, simulationParameters );

  QVERIFY( !startPose.canBeSimulated() );

  // The first line of what SIG_WrongKinematicsException says.
  QCOMPARE( startPose.getRefusal(), QString( "WrongKinematicsException: Axes of adjacent joints meeting in link middle cut!" ) );
  QVERIFY( !startPose.everyLinkHasMass() );
}
