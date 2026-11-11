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
#include "SIGEL_Simulation/TST_SIG_DynaMechsSimulationData.h"

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "TST_FreeWatch.h"

#include <QtTest>

#include <dmIntegEuler.hpp>
#include <dmIntegRK4.hpp>
#include <dmIntegRK45.hpp>
#include <dmMDHLink.hpp>
#include <dmMobileBaseLink.hpp>
#include <dmPrismaticLink.hpp>
#include <dmRevoluteLink.hpp>

#include <limits>
#include <memory>

namespace
{

  // On the parent, a hinge at x 1 whose hand points along +x; on the child, a hinge at x -1 whose
  // hand points along +x. Together they put the child two units along x from the parent.
  const SIG_Vector besideOnPlusX[ 3 ] = { SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) };
  const SIG_Vector fromMinusX[ 3 ] = { SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) };

  // DynaMechs has no getter for the joint input and the joint limits of a link; pointers to the members read them.
  struct JointInputReader : public dmMDHLink
  {
    static constexpr Float dmMDHLink::*jointInput = &JointInputReader::m_joint_input;
    static constexpr Float dmMDHLink::*minimum = &JointInputReader::m_min_joint_pos;
    static constexpr Float dmMDHLink::*maximum = &JointInputReader::m_max_joint_pos;
    static constexpr Float dmLink::*spring = &JointInputReader::m_joint_limit_spring;
    static constexpr Float dmLink::*damper = &JointInputReader::m_joint_limit_damper;
  };

}

// The geometry owns its polygons and frees them. The corners go counter-clockwise
// seen from outside the solid, so that each face normal points outwards.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addFace( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &corners )
{
  SIGEL_Robot::SIG_Polygon *polygon = new SIGEL_Robot::SIG_Polygon( &geometry );
  for ( const SIG_Vector &corner : corners )
  {
    polygon->appendVertex( corner );
  }
}

// A box link of density 2 with a body and a material of its own; the robot owns all three.
SIGEL_Robot::SIG_Link *SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addBoxLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIG_Vector low, SIG_Vector high )
{
  const SIG_Vector c[ 8 ] = { low, SIG_Vector( high.x, low.y, low.z ), SIG_Vector( high.x, high.y, low.z ), SIG_Vector( low.x, high.y, low.z ),
                              SIG_Vector( low.x, low.y, high.z ), SIG_Vector( high.x, low.y, high.z ), high, SIG_Vector( low.x, high.y, high.z ) };
  SIGEL_Robot::SIG_Geometry *geometry = new SIGEL_Robot::SIG_Geometry();
  addFace( *geometry, { c[ 0 ], c[ 3 ], c[ 2 ], c[ 1 ] } );
  addFace( *geometry, { c[ 4 ], c[ 5 ], c[ 6 ], c[ 7 ] } );
  addFace( *geometry, { c[ 0 ], c[ 1 ], c[ 5 ], c[ 4 ] } );
  addFace( *geometry, { c[ 1 ], c[ 2 ], c[ 6 ], c[ 5 ] } );
  addFace( *geometry, { c[ 2 ], c[ 3 ], c[ 7 ], c[ 6 ] } );
  addFace( *geometry, { c[ 3 ], c[ 0 ], c[ 4 ], c[ 7 ] } );
  SIGEL_Robot::SIG_Body *body = new SIGEL_Robot::SIG_Body( &robot, name + ".wrl", "parts/" );
  body->setGeometry( geometry );
  SIGEL_Robot::SIG_Material *material = new SIGEL_Robot::SIG_Material( &robot, name );
  material->setDensity( 2 );
  SIGEL_Robot::SIG_Link *link = new SIGEL_Robot::SIG_Link( &robot, name, robot.getLinks().count() );
  link->setBody( body );
  link->setMaterial( material );
  robot.addBody( body );
  robot.addMaterial( material );
  robot.addLink( link );
  return link;
}

// A robot of one box link, prepared for the simulation.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::buildBox( SIGEL_Robot::SIG_Robot &robot )
{
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) ) );
  robot.prepareDynaMechs();
}

// A joint along y from base to arm, number number, with the range minimum to maximum.
SIGEL_Robot::SIG_Joint *SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addJoint( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, int number,
                                  SIGEL_Robot::SIG_Link *base, SIGEL_Robot::SIG_Link *arm, double minimum, double maximum )
{
  const QString name = "joint" + QString::number( number );
  if ( type == SIGEL_Robot::SIG_Joint::tRotationalJoint )
  {
    SIGEL_Robot::SIG_RotationalJoint *joint = new SIGEL_Robot::SIG_RotationalJoint( &robot, name, number );
    joint->setLeftLink( base );
    joint->setRightLink( arm );
    joint->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
    joint->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
    joint->setRange( minimum, maximum, 0 );
    robot.addJoint( joint );
    return joint;
  }
  SIGEL_Robot::SIG_TranslationalJoint *joint = new SIGEL_Robot::SIG_TranslationalJoint( &robot, name, number );
  joint->setLeftLink( base );
  joint->setRightLink( arm );
  joint->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
  joint->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  joint->setRange( minimum, maximum, 0 );
  robot.addJoint( joint );
  return joint;
}

// A base and an arm with a joint along y between them; the robot is not yet prepared.
SIGEL_Robot::SIG_Joint *SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addBaseAndArm( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, double minimum, double maximum )
{
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  return addJoint( robot, type, 0, base, arm, minimum, maximum );
}

SIGEL_Robot::SIG_Drive *SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addDrive( SIGEL_Robot::SIG_Robot &robot, int number, SIGEL_Robot::SIG_Joint *joint, SIGEL_Robot::SIG_Drive::DriveMode mode )
{
  SIGEL_Robot::SIG_Drive *drive = new SIGEL_Robot::SIG_Drive( &robot, "drive" + QString::number( number ), number );
  drive->setJoint( joint );
  drive->setMode( mode );
  drive->setForces( -10, 10 );
  robot.addDrive( drive );
  return drive;
}

// A base and an arm with a hinge along y between them, and a drive of the mode on the hinge.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::buildBaseAndArmWithADrive( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Drive::DriveMode mode )
{
  SIGEL_Robot::SIG_Joint *hinge = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  addDrive( robot, 0, hinge, mode );
  robot.prepareDynaMechs();
}

// A rotational joint along y from left to right: on each link, the axis base, a second point
// of the axis and the hand. At a start value of 0 the joint lines up the hands.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::addHinge( SIGEL_Robot::SIG_Robot &robot, int number, SIGEL_Robot::SIG_Link *left, const SIG_Vector ( &leftPoints )[ 3 ],
               SIGEL_Robot::SIG_Link *right, const SIG_Vector ( &rightPoints )[ 3 ] )
{
  SIGEL_Robot::SIG_RotationalJoint *joint = new SIGEL_Robot::SIG_RotationalJoint( &robot, "hinge" + QString::number( number ), number );
  joint->setLeftLink( left );
  joint->setRightLink( right );
  joint->setLeftPoints( leftPoints[ 0 ], leftPoints[ 1 ], leftPoints[ 2 ] );
  joint->setRightPoints( rightPoints[ 0 ], rightPoints[ 1 ], rightPoints[ 2 ] );
  joint->setRange( -120, 120, 0 );
  robot.addJoint( joint );
}

// The lowest and highest corner, in the world, of the box around link number of robot.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::worldBox( const SIGEL_Robot::SIG_Robot &robot, SIGEL_Simulation::SIG_DynaMechsSimulationQueries &queries, int number,
               SIG_Vector &low, SIG_Vector &high )
{
  SIG_Vector position = queries.getLinkPosition( number );
  SIG_Matrix orientation = queries.getLinkOrientation( number );
  double lowest[ 3 ] = { std::numeric_limits< double >::max(), std::numeric_limits< double >::max(), std::numeric_limits< double >::max() };
  double highest[ 3 ] = { std::numeric_limits< double >::lowest(), std::numeric_limits< double >::lowest(), std::numeric_limits< double >::lowest() };
  for ( SIG_Vector *vertex : robot.getLinks()[ number ]->getGeometry()->getVertices() )
  {
    for ( int r = 0; r < 3; r++ )
    {
      double world = position.get( r );
      for ( int c = 0; c < 3; c++ )
      {
        world += orientation.get( r, c ) * vertex->get( c );
      }
      lowest[ r ] = std::min( lowest[ r ], world );
      highest[ r ] = std::max( highest[ r ], world );
    }
  }
  low = SIG_Vector( lowest[ 0 ], lowest[ 1 ], lowest[ 2 ] );
  high = SIG_Vector( highest[ 0 ], highest[ 1 ], highest[ 2 ] );
}

// Empty if the world box of link number is the box from low to high, else what it is.
QString SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::worldBoxMismatch( const SIGEL_Robot::SIG_Robot &robot, SIGEL_Simulation::SIG_DynaMechsSimulationQueries &queries, int number,
                          SIG_Vector low, SIG_Vector high )
{
  SIG_Vector actualLow;
  SIG_Vector actualHigh;
  worldBox( robot, queries, number, actualLow, actualHigh );
  for ( int r = 0; r < 3; r++ )
  {
    if ( std::fabs( actualLow.get( r ) - low.get( r ) ) > 1e-6 || std::fabs( actualHigh.get( r ) - high.get( r ) ) > 1e-6 )
    {
      return QString( "link %1 is from (%2, %3, %4) to (%5, %6, %7)" ).arg( number )
        .arg( actualLow.x ).arg( actualLow.y ).arg( actualLow.z ).arg( actualHigh.x ).arg( actualHigh.y ).arg( actualHigh.z );
    }
  }
  return QString();
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainGivesTheHeightsOfTheFloorFunction()
{
  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "0.5*x-10*z" );

  // The function's x counts along the first index, its z along the second.
  QCOMPARE( loadedHeight( environment, 0, 0 ), 0.0 );
  QCOMPARE( loadedHeight( environment, 0, 3 ), -30.0 );
  QCOMPARE( loadedHeight( environment, 2, 1 ), -9.0 );
  QCOMPARE( loadedHeight( environment, 3, 3 ), -28.5 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainGivesTheHeightsOfThePictureFile()
{
  QTemporaryDir folder;
  QVERIFY( folder.isValid() );
  QFile pictureFile( folder.filePath( "floor.pgm" ) );
  QVERIFY( pictureFile.open( QIODevice::WriteOnly ) );
  pictureFile.write( "P2\n# a comment line\n3 2\n255\n0 51 255\n255 0 102\n" );
  pictureFile.close();

  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorPictureFile( pictureFile.fileName() );
  environment.setFloorFuncSelected( false );

  dmEnvironment dynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 3 );
  QCOMPARE( secondDimension, 2 );

  // A grey value of the maximum is a height of 5.
  QCOMPARE( heights[ 0 ][ 0 ], 0.0 );
  QCOMPARE( heights[ 0 ][ 1 ], 5.0 );
  QCOMPARE( heights[ 1 ][ 0 ], 1.0 );
  QCOMPARE( heights[ 2 ][ 1 ], 2.0 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainGivesTheDimensionsOfTheFloor()
{
  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorDimensionX( 7 );
  environment.setFloorDimensionZ( 7 );

  dmEnvironment dynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 7 );
  QCOMPARE( secondDimension, 7 );
  QCOMPARE( spacing, 1.0 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::eachLoadHasATerrainFileOfItsOwnAndLeavesNoneBehind()
{
  SIGEL_Environment::SIG_Environment environment;
  dmEnvironment firstDynaMechsEnvironment;
  dmEnvironment secondDynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), firstDynaMechsEnvironment );
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), secondDynaMechsEnvironment );

  const QString firstTerrainFile = QFile::decodeName( firstDynaMechsEnvironment.getTerrainFilename() );
  const QString secondTerrainFile = QFile::decodeName( secondDynaMechsEnvironment.getTerrainFilename() );

  QVERIFY( firstTerrainFile != secondTerrainFile );
  QVERIFY( !QFile::exists( firstTerrainFile ) );
  QVERIFY( !QFile::exists( secondTerrainFile ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainNeedsNoSigelRoot()
{
  const QByteArray sigelRoot = qgetenv( "SIGEL_ROOT" );
  const bool sigelRootWasSet = qEnvironmentVariableIsSet( "SIGEL_ROOT" );
  qunsetenv( "SIGEL_ROOT" );

  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorFunction( "2" );
  double height = 0;
  QString thrown;

  // Caught here, so that SIGEL_ROOT is back for the tests after this one whatever loadTerrain does.
  try
  {
    height = loadedHeight( environment, 5, 5 );
  }
  catch ( const SIGEL_Tools::SIG_Exception &e )
  {
    thrown = e.getMessage();
  }
  catch ( ... )
  {
    thrown = "an exception that is not a SIG_Exception";
  }

  if ( sigelRootWasSet )
  {
    qputenv( "SIGEL_ROOT", sigelRoot );
  }

  QCOMPARE( thrown, QString() );
  QCOMPARE( height, 2.0 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainGivesEachHeightOfASymmetricFloorFunction()
{
  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "x*z+x+z" );

  dmEnvironment dynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 4 );
  QCOMPARE( secondDimension, 4 );

  for (int first = 0; first < 4; ++first)
  {
    for (int second = 0; second < 4; ++second)
    {
      QCOMPARE( heights[ first ][ second ], static_cast< double >( first * second + first + second ) );
    }
  }
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadTerrainGivesTheDimensionsOfAFloorThatIsNotSquare()
{
  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorDimensionX( 3 );
  environment.setFloorDimensionZ( 5 );
  environment.setFloorFunction( "2" );

  dmEnvironment dynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), dynaMechsEnvironment );

  verifyFloorOfOneHeight( dynaMechsEnvironment, 3, 5, 2.0 );
}

double SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::loadedHeight( const SIGEL_Environment::SIG_Environment &environment, int first, int second ) const
{
  dmEnvironment dynaMechsEnvironment;
  SIG_DynaMechsSimulationData::loadTerrain( environment.getTerrain(), dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  return heights[ first ][ second ];
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::verifyFloorOfOneHeight( dmEnvironment &dynaMechsEnvironment, int firstDimension, int secondDimension, double height ) const
{
  int loadedFirstDimension = 0;
  int loadedSecondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( loadedFirstDimension, loadedSecondDimension, spacing );

  QCOMPARE( loadedFirstDimension, firstDimension );
  QCOMPARE( loadedSecondDimension, secondDimension );

  for (int first = 0; first < firstDimension; ++first)
  {
    for (int second = 0; second < secondDimension; ++second)
    {
      QCOMPARE( heights[ first ][ second ], height );
    }
  }
}

// SIGEL's y axis points up, DynaMechs' z axis does. Both are right-handed, so SIGEL's z is DynaMechs' -y.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesDynaMechsTheGravityInItsAxes()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0.5, -9.81, 0.25 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  CartesianVector gravity;
  simulationData.dynaMechsEnvironment.getGravity( gravity );
  QCOMPARE( gravity[ 0 ], 0.5 );
  QCOMPARE( gravity[ 1 ], -0.25 );
  QCOMPARE( gravity[ 2 ], -9.81 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesDynaMechsTheGroundConstants()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  environment.setGroundPlanarSpringConstant( 1234.5 );
  environment.setGroundNormalSpringConstant( 2345.5 );
  environment.setGroundPlanarDamperConstant( 12.5 );
  environment.setGroundNormalDamperConstant( 23.5 );
  environment.setFrictionCoeff_u_s( 0.8 );
  environment.setFrictionCoeff_u_k( 0.6 );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  const dmEnvironment &ground = simulationData.dynaMechsEnvironment;
  QCOMPARE( ground.getGroundPlanarSpringConstant(), 1234.5 );
  QCOMPARE( ground.getGroundNormalSpringConstant(), 2345.5 );
  QCOMPARE( ground.getGroundPlanarDamperConstant(), 12.5 );
  QCOMPARE( ground.getGroundNormalDamperConstant(), 23.5 );
  QCOMPARE( ground.getGroundStaticFrictionCoeff(), 0.8 );
  QCOMPARE( ground.getGroundKineticFrictionCoeff(), 0.6 );
}

// In DynaMechs' axes: SIGEL ( 4, 4.5, 5 ) is DynaMechs ( 4, -5, 4.5 ).
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsTheRootLinkAtTheStartPositionPlusTheInitialLocationOfTheRobot()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  robot.initialLocation = SIG_Vector( 1, 0.5, 0 );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  dmRigidBody *root = simulationData.dynaMechsLinks[ robot.getRootLink()->getNumber() ]->dynaMechsLink;
  QVERIFY( dynamic_cast< dmMobileBaseLink * >( root ) );
  Float state[ 7 ];
  Float velocity[ 6 ];
  static_cast< dmMobileBaseLink * >( root )->getState( state, velocity );
  QCOMPARE( state[ 4 ], 4.0 );
  QCOMPARE( state[ 5 ], -5.0 );
  QCOMPARE( state[ 6 ], 4.5 );
}

// DynaMechs has one current environment, so a second simulation would take it from the first.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorRefusesASecondSimulationInTheSameProcess()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData first( robot, environment, simulationParameters );

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, SIG_DynaMechsSimulationData second( robot, environment, simulationParameters ) );

  QCOMPARE( dmEnvironment::getEnvironment(), &first.dynaMechsEnvironment );
}

// DynaMechs must not keep the address of an environment that is gone.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorMakesItsEnvironmentTheOneOfDynaMechsAndTheDestructorTakesItOut()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  QCOMPARE( dmEnvironment::getEnvironment(), nullptr );

  {
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    QCOMPARE( dmEnvironment::getEnvironment(), &simulationData.dynaMechsEnvironment );
  }

  QCOMPARE( dmEnvironment::getEnvironment(), nullptr );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::simulationProgressSetsTheForceOfADriveToZeroWhenItsTimeAccountRunsOut_data()
{
  QTest::addColumn< int >( "mode" );

  QTest::newRow( "force" ) << int( SIGEL_Robot::SIG_Drive::tForceMode );
  QTest::newRow( "simple servo" ) << int( SIGEL_Robot::SIG_Drive::tServoSimpleMode );
}

// A drive acts for the time that its command gives, then lets the joint go.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::simulationProgressSetsTheForceOfADriveToZeroWhenItsTimeAccountRunsOut()
{
  QFETCH( int, mode );
  SIGEL_Robot::SIG_Robot robot;
  buildBaseAndArmWithADrive( robot, static_cast< SIGEL_Robot::SIG_Drive::DriveMode >( mode ) );
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  dmMDHLink *hinge = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( hinge );
  Float force = 5;
  hinge->setJointInput( &force );

  // One and a half steps: the force acts through the first step and ends in the second.
  simulationData.driveForcesTimeAccounts[ 0 ] = 1.5 * simulationParameters.getStepSize();

  simulationData.simulationProgress();
  QCOMPARE( hinge->*JointInputReader::jointInput, 5.0 );

  simulationData.simulationProgress();
  QCOMPARE( hinge->*JointInputReader::jointInput, 0.0 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorTurnsTheRootLinkByTheInitialOrientationOfTheRobot_data()
{
  QTest::addColumn< double >( "axisX" );
  QTest::addColumn< double >( "axisY" );
  QTest::addColumn< double >( "axisZ" );
  QTest::addColumn< double >( "degrees" );

  // Turns of large and small angles about axes off the coordinate axes, so that every way of
  // turning a matrix into a quaternion is used; a half turn about x puts the robot on its back.
  QTest::newRow( "quarter turn about y" ) << 0.0 << 1.0 << 0.0 << 90.0;
  QTest::newRow( "quarter turn about x" ) << 1.0 << 0.0 << 0.0 << 90.0;
  QTest::newRow( "half turn about x" ) << 1.0 << 0.0 << 0.0 << 180.0;
  QTest::newRow( "150 degrees about an axis near y" ) << 0.2 << 1.0 << 0.3 << 150.0;
  QTest::newRow( "quarter turn about an axis near x" ) << 1.0 << -0.3 << 0.2 << 90.0;
  QTest::newRow( "100 degrees about an axis near x" ) << 1.0 << -0.2 << 0.0 << 100.0;
  QTest::newRow( "150 degrees about an axis between -y and z" ) << 0.3 << -1.0 << 1.0 << 150.0;
}

// The turn by the right-hand rule: cos * 1 + sin * [ a ]x + ( 1 - cos ) a a^T for the unit axis a.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorTurnsTheRootLinkByTheInitialOrientationOfTheRobot()
{
  QFETCH( double, axisX );
  QFETCH( double, axisY );
  QFETCH( double, axisZ );
  QFETCH( double, degrees );
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  const double length = std::sqrt( axisX * axisX + axisY * axisY + axisZ * axisZ );
  const double a[ 3 ] = { axisX / length, axisY / length, axisZ / length };
  const double cosine = std::cos( degrees * M_PI / 180 );
  const double sine = std::sin( degrees * M_PI / 180 );
  const double cross[ 3 ][ 3 ] = { { 0, -a[ 2 ], a[ 1 ] }, { a[ 2 ], 0, -a[ 0 ] }, { -a[ 1 ], a[ 0 ], 0 } };
  SIG_Matrix turn;
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      turn.set( r, c, ( r == c ? cosine : 0 ) + sine * cross[ r ][ c ] + ( 1 - cosine ) * a[ r ] * a[ c ] );
    }
  }
  robot.initialOrientation = turn;
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  SIG_Matrix orientation = simulationQueries.getLinkOrientation( robot.getRootLink()->getNumber() );
  QString mismatch;
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      if ( mismatch.isEmpty() && std::fabs( orientation.get( r, c ) - turn.get( r, c ) ) > 1e-9 )
      {
        mismatch = QString( "(%1,%2) is %3, expected %4" ).arg( r ).arg( c ).arg( orientation.get( r, c ) ).arg( turn.get( r, c ) );
      }
    }
  }
  QCOMPARE( mismatch, QString() );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorTakesTheIntegratorOfTheSimulationParameters_data()
{
  QTest::addColumn< int >( "integrator" );

  QTest::newRow( "Euler" ) << int( SIG_SimulationParameters::Euler );
  QTest::newRow( "RungeKutta4" ) << int( SIG_SimulationParameters::RungeKutta4 );
  QTest::newRow( "RungeKutta45" ) << int( SIG_SimulationParameters::RungeKutta45 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorTakesTheIntegratorOfTheSimulationParameters()
{
  QFETCH( int, integrator );
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setDynaMechsIntegrator( static_cast< SIG_SimulationParameters::DynaMechsIntegrator >( integrator ) );

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  dmIntegrator *made = simulationData.dynaMechsIntegrator.get();
  QCOMPARE( dynamic_cast< dmIntegEuler * >( made ) != nullptr, integrator == SIG_SimulationParameters::Euler );
  QCOMPARE( dynamic_cast< dmIntegRK45 * >( made ) != nullptr, integrator == SIG_SimulationParameters::RungeKutta45 );
  QCOMPARE( dynamic_cast< dmIntegRK4 * >( made ) != nullptr && dynamic_cast< dmIntegRK45 * >( made ) == nullptr,
            integrator == SIG_SimulationParameters::RungeKutta4 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesTheLinkOfAJointItsKindLimitsAndFriction_data()
{
  QTest::addColumn< int >( "jointType" );
  QTest::addColumn< double >( "minimum" );
  QTest::addColumn< double >( "maximum" );
  QTest::addColumn< double >( "start" );
  QTest::addColumn< double >( "unit" );

  // The robot file gives the range of a rotational joint in degrees, DynaMechs takes radians.
  QTest::newRow( "rotational" ) << int( SIGEL_Robot::SIG_Joint::tRotationalJoint ) << -120.0 << 120.0 << 0.0 << M_PI / 180;
  QTest::newRow( "rotational, started at 30" ) << int( SIGEL_Robot::SIG_Joint::tRotationalJoint ) << -120.0 << 120.0 << 30.0 << M_PI / 180;
  QTest::newRow( "translational" ) << int( SIGEL_Robot::SIG_Joint::tTranslationalJoint ) << -1.0 << 1.0 << 0.0 << 1.0;
  QTest::newRow( "translational, started at 0.5" ) << int( SIGEL_Robot::SIG_Joint::tTranslationalJoint ) << -1.0 << 1.0 << 0.5 << 1.0;
}

// The joint limits, their spring and damper, and the joint friction come from the robot and the simulation parameters.
// The limits lie the range's ends away from the start value, so they are that far from the start position in
// DynaMechs. Whether DynaMechs counts the joint in the robot file's direction or against it is not fixed.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesTheLinkOfAJointItsKindLimitsAndFriction()
{
  QFETCH( int, jointType );
  QFETCH( double, minimum );
  QFETCH( double, maximum );
  QFETCH( double, start );
  QFETCH( double, unit );
  const SIGEL_Robot::SIG_Joint::JointType type = static_cast< SIGEL_Robot::SIG_Joint::JointType >( jointType );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, type, minimum, maximum );
  if ( type == SIGEL_Robot::SIG_Joint::tRotationalJoint )
  {
    static_cast< SIGEL_Robot::SIG_RotationalJoint * >( joint )->setRange( minimum, maximum, start );
  }
  else
  {
    static_cast< SIGEL_Robot::SIG_TranslationalJoint * >( joint )->setRange( minimum, maximum, start );
  }
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setJointLimitsK_spring( 123.5 );
  simulationParameters.setJointLimitsB_damper( 4.5 );
  simulationParameters.setJointFrictionU_c( 0.25 );

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  dmRigidBody *arm = simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink;
  if ( type == SIGEL_Robot::SIG_Joint::tRotationalJoint )
  {
    QVERIFY( dynamic_cast< dmRevoluteLink * >( arm ) );
  }
  else
  {
    QVERIFY( dynamic_cast< dmPrismaticLink * >( arm ) );
  }
  dmMDHLink *link = static_cast< dmMDHLink * >( arm );
  Float position[ 1 ];
  Float velocity[ 1 ];
  link->getState( position, velocity );
  const double lower = link->*JointInputReader::minimum - position[ 0 ];
  const double upper = link->*JointInputReader::maximum - position[ 0 ];
  const bool alongTheRobotFile = std::fabs( lower - ( minimum - start ) * unit ) < 1e-9 && std::fabs( upper - ( maximum - start ) * unit ) < 1e-9;
  const bool againstTheRobotFile = std::fabs( lower + ( maximum - start ) * unit ) < 1e-9 && std::fabs( upper + ( minimum - start ) * unit ) < 1e-9;
  QVERIFY2( alongTheRobotFile || againstTheRobotFile,
            qPrintable( QString( "the limits lie %1 and %2 from the start position" ).arg( lower ).arg( upper ) ) );
  QCOMPARE( link->*JointInputReader::spring, 123.5 );
  QCOMPARE( link->*JointInputReader::damper, 4.5 );
  QCOMPARE( link->getJointFriction(), 0.25 );
}

// SIGEL takes equal limits as a joint without limits.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesAJointWithEqualLimitsNoLimits()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setJointLimitsK_spring( 123.5 );
  simulationParameters.setJointLimitsB_damper( 4.5 );

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  dmMDHLink *link = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( link );
  QCOMPARE( link->*JointInputReader::minimum, std::numeric_limits< double >::lowest() );
  QCOMPARE( link->*JointInputReader::maximum, std::numeric_limits< double >::max() );
  QCOMPARE( link->*JointInputReader::spring, 0.0 );
  QCOMPARE( link->*JointInputReader::damper, 0.0 );
}

// DynaMechs builds a tree of links, so a link with two joints to it cannot be simulated.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorRefusesALinkThatTwoJointsReach()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  addJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 1, robot.getLinks()[ 0 ], robot.getLinks()[ 1 ], -90, 90 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  bool refused = false;
  bool otherException = false;

  try
  {
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  }
  catch ( const SIGEL_Tools::SIG_Exception & )
  {
    refused = true;
  }
  catch ( ... )
  {
    otherException = true;
  }

  QVERIFY( !otherException );
  QVERIFY( refused );
  QCOMPARE( dmEnvironment::getEnvironment(), nullptr );
}

// Each drive and each sensor is in its list at its number, for the commands and queries of the program.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorListsTheDrivesAndSensorsAtTheirNumbers()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *hinge = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  SIGEL_Robot::SIG_Drive *servo = addDrive( robot, 1, hinge, SIGEL_Robot::SIG_Drive::tServoSimpleMode );
  SIGEL_Robot::SIG_Drive *force = addDrive( robot, 0, hinge, SIGEL_Robot::SIG_Drive::tForceMode );
  SIGEL_Robot::SIG_PitchRollSensor *pitchRoll = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "pitchRoll", 2 );
  pitchRoll->setLink( robot.getLinks()[ 0 ] );
  robot.addSensor( pitchRoll );
  SIGEL_Robot::SIG_ContactSensor *contact = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  contact->setLink( robot.getLinks()[ 1 ] );
  robot.addSensor( contact );
  SIGEL_Robot::SIG_JointSensor *angle = new SIGEL_Robot::SIG_JointSensor( &robot, "angle", 1 );
  angle->setJoint( hinge );
  robot.addSensor( angle );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  QCOMPARE( simulationData.drives.size(), 2 );
  QCOMPARE( simulationData.drives[ 0 ], force );
  QCOMPARE( simulationData.drives[ 1 ], servo );
  QCOMPARE( simulationData.sensors.size(), 3 );
  QCOMPARE( simulationData.sensors[ 0 ], contact );
  QCOMPARE( simulationData.sensors[ 1 ], angle );
  QCOMPARE( simulationData.sensors[ 2 ], pitchRoll );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorListsOrRefusesARelativeOrAbsoluteDrive_data()
{
  QTest::addColumn< int >( "mode" );

  QTest::newRow( "relative" ) << int( SIGEL_Robot::SIG_Drive::tRelativeMode );
  QTest::newRow( "absolute" ) << int( SIGEL_Robot::SIG_Drive::tAbsoluteMode );
}

// The robot language has drives of four modes; constructorListsTheDrivesAndSensorsAtTheirNumbers
// covers force and simple servo drives. A drive that the simulation does not list would do nothing
// on MOVE, without a word, so it must be listed or the robot refused.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorListsOrRefusesARelativeOrAbsoluteDrive()
{
  QFETCH( int, mode );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *hinge = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  SIGEL_Robot::SIG_Drive *drive = addDrive( robot, 0, hinge, static_cast< SIGEL_Robot::SIG_Drive::DriveMode >( mode ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  bool refused = false;
  bool listed = false;

  try
  {
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    listed = simulationData.drives.value( 0 ) == drive;
  }
  catch ( const SIGEL_Tools::SIG_Exception & )
  {
    refused = true;
  }

  QVERIFY( listed || refused );
}

// A box above the floor falls under gravity, and the link positions follow.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::simulationProgressMovesTheRobot()
{
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 0, 3, 0 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  const double before = simulationQueries.getLinkPosition( 0 ).y;

  for ( int step = 0; step < 10; step++ )
  {
    simulationData.simulationProgress();
  }

  const double after = simulationQueries.getLinkPosition( 0 ).y;
  QVERIFY2( after < before, qPrintable( QString( "from %1 to %2" ).arg( before ).arg( after ) ) );
}

// In the model, base fills x -1..1, y -0.5..3, z -0.5..0.5; the start position moves everything by ( 3, 4, 5 ).
// A link with two joints to further links, as a body with two legs: the second leg hangs on a screw offset.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsBothLegsOfABodyWithTwoLegsWhereTheModelHasThem()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *right = addBoxLink( robot, "right", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_Link *left = addBoxLink( robot, "left", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  addHinge( robot, 0, base, besideOnPlusX, right, fromMinusX );
  const SIG_Vector besideOnMinusX[ 3 ] = { SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( -2, 0, 0 ) };
  const SIG_Vector fromPlusX[ 3 ] = { SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 0, 0, 0 ) };
  addHinge( robot, 1, base, besideOnMinusX, left, fromPlusX );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 2, SIG_Vector( 0, 2, 4.75 ), SIG_Vector( 2, 4.25, 5.25 ) ), QString() );
}

// A robot file may give the child of a joint as its left link and the parent as its right one.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsALinkWhereTheModelHasItWhenItsJointHasTheRootOnTheRight()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  addHinge( robot, 0, arm, fromMinusX, base, besideOnPlusX );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
}

// A chain base, arm, tip, each two units along x from the one before.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsEachLinkOfAChainWhereTheModelHasIt()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ) );
  robot.setRootLink( base );
  addHinge( robot, 0, base, besideOnPlusX, arm, fromMinusX );
  addHinge( robot, 1, arm, besideOnPlusX, tip, fromMinusX );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 2, SIG_Vector( 6, 3, 4.75 ), SIG_Vector( 8, 4, 5.25 ) ), QString() );
}

// A walking robot: a hip along y between body and thigh, and a knee along x between thigh and shin.
// On the thigh the knee's axis runs through ( 0, -2, 0.2 ) along x, so that it passes the hip axis
// and does not cut it, and its hand points along -y. On the shin it runs through ( 0, 1, 0.2 ) along
// x with the hand along -y, so the shin hangs below the thigh.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsTheLinksOfALegWithAHipAndAKneeOnCrossingAxesWhereTheModelHasThem()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *thigh = addBoxLink( robot, "thigh", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_Link *shin = addBoxLink( robot, "shin", SIG_Vector( -0.25, -1, -0.25 ), SIG_Vector( 0.25, 1, 0.25 ) );
  robot.setRootLink( base );
  addHinge( robot, 0, base, besideOnPlusX, thigh, fromMinusX );
  const SIG_Vector kneeOnThigh[ 3 ] = { SIG_Vector( 0, -2, 0.2 ), SIG_Vector( 1, -2, 0.2 ), SIG_Vector( 0, -3, 0.2 ) };
  const SIG_Vector kneeOnShin[ 3 ] = { SIG_Vector( 0, 1, 0.2 ), SIG_Vector( 1, 1, 0.2 ), SIG_Vector( 0, 0, 0.2 ) };
  addHinge( robot, 1, thigh, kneeOnThigh, shin, kneeOnShin );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 4, 2, 4.75 ), SIG_Vector( 6, 4.25, 5.25 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 2, SIG_Vector( 4.75, 0, 4.75 ), SIG_Vector( 5.25, 2, 5.25 ) ), QString() );
}

// The simulation owns the links it builds for DynaMechs.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::destructorFreesTheLinks()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  auto simulationData = std::make_unique< SIG_DynaMechsSimulationData >( robot, environment, simulationParameters );
  QCOMPARE( simulationData->dynaMechsLinks.size(), 2 );
  bool rootFreed;
  bool armFreed;

  {
    SIGEL_Tools::TST_FreeWatch rootWatch( simulationData->dynaMechsLinks[ 0 ] );
    simulationData.reset();
    rootFreed = rootWatch.freed();
  }

  // One watch at a time: a second simulation for the arm.
  simulationData = std::make_unique< SIG_DynaMechsSimulationData >( robot, environment, simulationParameters );
  {
    SIGEL_Tools::TST_FreeWatch armWatch( simulationData->dynaMechsLinks[ 1 ] );
    simulationData.reset();
    armFreed = armWatch.freed();
  }

  QVERIFY( rootFreed );
  QVERIFY( armFreed );
}

// The simulation's floor is the environment's: the floor function on the master, the terrain
// that the master sent on a slave.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesDynaMechsTheFloorOfTheEnvironment_data()
{
  QTest::addColumn< bool >( "terrainGiven" );

  QTest::newRow( "floor function" ) << false;
  QTest::newRow( "terrain given by setTerrain" ) << true;
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorGivesDynaMechsTheFloorOfTheEnvironment()
{
  QFETCH( bool, terrainGiven );
  SIGEL_Robot::SIG_Robot robot;
  buildBox( robot );
  SIGEL_Environment::SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 3 );
  environment.setFloorFunction( terrainGiven ? "7" : "x+10*z" );
  if ( terrainGiven )
  {
    SIGEL_Environment::SIG_Terrain terrain;
    QVERIFY( terrain.setFromFunction( "x+10*z", 4, 3 ) );
    environment.setTerrain( terrain );
  }
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = simulationData.dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );
  QCOMPARE( firstDimension, 4 );
  QCOMPARE( secondDimension, 3 );
  QCOMPARE( heights[ 3 ][ 2 ], 23.0 );
  QCOMPARE( heights[ 1 ][ 0 ], 1.0 );
}

// In the model, a translational joint's start value moves the link behind it along the axis, here +y.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsTheLinkBehindATranslationalJointWhereItsStartValuePutsIt()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  SIGEL_Robot::SIG_TranslationalJoint *slider = new SIGEL_Robot::SIG_TranslationalJoint( &robot, "slider", 0 );
  slider->setLeftLink( base );
  slider->setRightLink( arm );
  slider->setLeftPoints( besideOnPlusX[ 0 ], besideOnPlusX[ 1 ], besideOnPlusX[ 2 ] );
  slider->setRightPoints( fromMinusX[ 0 ], fromMinusX[ 1 ], fromMinusX[ 2 ] );
  slider->setRange( -1, 1, 0.5 );
  robot.addJoint( slider );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 4, 2.5, 4.75 ), SIG_Vector( 6, 4.75, 5.25 ) ), QString() );
}

// In the model, a start value of 90 turns the arm by -90 degrees about the axis direction +y,
// so that it points along +z from the axis instead of along +x.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::constructorPutsTheLinkBehindARotationalJointWhereItsStartValuePutsIt()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *hinge = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120 );
  static_cast< SIGEL_Robot::SIG_RotationalJoint * >( hinge )->setRange( -120, 120, 90 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, SIG_Vector( 2, 3.5, 4.5 ), SIG_Vector( 4, 7, 5.5 ) ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, SIG_Vector( 3.75, 2, 5 ), SIG_Vector( 4.25, 4.25, 7 ) ), QString() );
}

// The step size the user sets is the time of one step: the drive's time account drops by it,
// and a falling box falls further in a longer step.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationData::simulationProgressStepsByTheStepSizeOfTheSimulationParameters()
{
  double fallen[ 2 ] = { 0, 0 };
  const double stepSizes[ 2 ] = { 0.01, 0.04 };
  for ( int run = 0; run < 2; run++ )
  {
    SIGEL_Robot::SIG_Robot robot;
    buildBaseAndArmWithADrive( robot, SIGEL_Robot::SIG_Drive::tForceMode );
    SIGEL_Environment::SIG_Environment environment;
    environment.setStartPosition( SIG_Vector( 0, 5, 0 ) );
    SIG_SimulationParameters simulationParameters;
    simulationParameters.setStepSize( stepSizes[ run ] );
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
    simulationData.driveForcesTimeAccounts[ 0 ] = 1;
    const double before = simulationQueries.getLinkPosition( 0 ).y;

    simulationData.simulationProgress();

    QCOMPARE( simulationData.driveForcesTimeAccounts[ 0 ], 1 - stepSizes[ run ] );
    fallen[ run ] = before - simulationQueries.getLinkPosition( 0 ).y;
  }

  QVERIFY2( fallen[ 0 ] > 0 && fallen[ 1 ] > fallen[ 0 ], qPrintable( QString( "fell %1 and %2" ).arg( fallen[ 0 ] ).arg( fallen[ 1 ] ) ) );
}
