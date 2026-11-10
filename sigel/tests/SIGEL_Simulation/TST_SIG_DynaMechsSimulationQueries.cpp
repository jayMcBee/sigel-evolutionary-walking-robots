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
#include "SIGEL_Simulation/TST_SIG_DynaMechsSimulationQueries.h"

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
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
#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

#include <QTime>
#include <QtTest>

#include <dmMDHLink.hpp>
#include <dmMobileBaseLink.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>


// The geometry owns its polygons and frees them. The corners go counter-clockwise
// seen from outside the solid, so that each face normal points outwards.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::addFace( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &corners )
{
  SIGEL_Robot::SIG_Polygon *polygon = new SIGEL_Robot::SIG_Polygon( &geometry );
  for ( const SIG_Vector &corner : corners )
  {
    polygon->appendVertex( corner );
  }
}

// A box link of density 2 with a body and a material of its own; the robot owns all three.
SIGEL_Robot::SIG_Link *SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::addBoxLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIG_Vector low, SIG_Vector high )
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

// A joint along y with its axis at x 1 on the left link and at x -1 on the right link,
// so that at the start value 0 the right link is two units along x from the left one.
SIGEL_Robot::SIG_Joint *SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::addJoint( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, int number,
                                  SIGEL_Robot::SIG_Link *left, SIGEL_Robot::SIG_Link *right, double minimum, double maximum, double start )
{
  const QString name = "joint" + QString::number( number );
  if ( type == SIGEL_Robot::SIG_Joint::tRotationalJoint )
  {
    SIGEL_Robot::SIG_RotationalJoint *joint = new SIGEL_Robot::SIG_RotationalJoint( &robot, name, number );
    joint->setLeftLink( left );
    joint->setRightLink( right );
    joint->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
    joint->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
    joint->setRange( minimum, maximum, start );
    robot.addJoint( joint );
    return joint;
  }
  SIGEL_Robot::SIG_TranslationalJoint *joint = new SIGEL_Robot::SIG_TranslationalJoint( &robot, name, number );
  joint->setLeftLink( left );
  joint->setRightLink( right );
  joint->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 2, 0, 0 ) );
  joint->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  joint->setRange( minimum, maximum, start );
  robot.addJoint( joint );
  return joint;
}

// In the model, base fills x -1..1, y -0.5..3, z -0.5..0.5 and, at the start value 0, arm fills x 1..3, y -2..0.25, z -0.25..0.25.
// The robot is not yet prepared.
SIGEL_Robot::SIG_Joint *SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::addBaseAndArm( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type,
                                       double minimum, double maximum, double start )
{
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  return addJoint( robot, type, 0, base, arm, minimum, maximum, start );
}

int SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::reading( const SIG_DynaMechsSimulationQueries &queries, int sensorNumber, int registerSize )
{
  QList< SIG_Register > registers( 4, SIG_Register( registerSize ) );
  queries.sense( sensorNumber, registers );
  return registers[ 0 ].getValue();
}

QString SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::matrixMismatch( SIG_Matrix actual, SIG_Matrix expected )
{
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      if ( std::fabs( actual.get( r, c ) - expected.get( r, c ) ) > 1e-9 )
      {
        return QString( "(%1,%2) is %3, expected %4" ).arg( r ).arg( c ).arg( actual.get( r, c ) ).arg( expected.get( r, c ) );
      }
    }
  }
  return QString();
}

SIG_Matrix SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::product( SIG_Matrix left, SIG_Matrix right )
{
  SIG_Matrix result = SIG_Matrix();
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      double sum = 0;
      for ( int k = 0; k < 3; k++ )
      {
        sum += left.get( r, k ) * right.get( k, c );
      }
      result.set( r, c, sum );
    }
  }
  return result;
}

SIG_Matrix SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::transposed( SIG_Matrix matrix )
{
  SIG_Matrix result = SIG_Matrix();
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      result.set( r, c, matrix.get( c, r ) );
    }
  }
  return result;
}

SIG_Matrix SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::turnAbout( int axis, double degrees )
{
  const double cosine = std::cos( degrees * M_PI / 180 );
  const double sine = std::sin( degrees * M_PI / 180 );
  const int next = ( axis + 1 ) % 3;
  const int last = ( axis + 2 ) % 3;
  SIG_Matrix turn = SIG_Matrix();
  turn.set( axis, axis, 1 );
  turn.set( next, next, cosine );
  turn.set( last, last, cosine );
  turn.set( next, last, -sine );
  turn.set( last, next, sine );
  return turn;
}

int SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::tiltReading( bool pitch, SIG_Matrix orientation )
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
  sensor->setLink( box );
  if ( pitch )
  {
    sensor->SetPitchType();
  }
  else
  {
    sensor->SetRollType();
  }
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  robot.initialOrientation = orientation;
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  return reading( simulationQueries, 0, 8 );
}

// How far the origin of link 1 lies from the origin of link 0 along the y axis of link 0.
double SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::armAlongBaseY( const SIG_DynaMechsSimulationQueries &queries )
{
  SIG_Vector base = queries.getLinkPosition( 0 );
  SIG_Vector arm = queries.getLinkPosition( 1 );
  SIG_Matrix orientation = queries.getLinkOrientation( 0 );
  return ( arm.x - base.x ) * orientation.get( 0, 1 ) + ( arm.y - base.y ) * orientation.get( 1, 1 ) + ( arm.z - base.z ) * orientation.get( 2, 1 );
}

// The readings of the sensor numbers from the register's bottom up, one for each sensor, on a base with a
// slider arm started at 0.5, high above the floor without gravity, after one step. Kind 0 is a joint sensor on the
// slider, 1 a contact sensor on the base and 2 a pitch sensor on the base; the sensors are numbered as listed.
QList< int > SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::sensorReadings( const QList< int > &kinds )
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0.5 );
  SIGEL_Robot::SIG_Link *base = robot.getLinks()[ 0 ];
  for ( int number = 0; number < kinds.size(); number++ )
  {
    if ( kinds[ number ] == 0 )
    {
      SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", number );
      sensor->setJoint( joint );
      robot.addSensor( sensor );
    }
    else if ( kinds[ number ] == 1 )
    {
      SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", number );
      sensor->setLink( base );
      robot.addSensor( sensor );
    }
    else
    {
      SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", number );
      sensor->setLink( base );
      sensor->SetPitchType();
      robot.addSensor( sensor );
    }
  }
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  simulationData.simulationProgress();
  QList< int > readings;
  for ( int number = 0; number < kinds.size(); number++ )
  {
    readings.append( reading( simulationQueries, -128 + number, 8 ) );
  }
  return readings;
}

// Empty if the box around link number in the world is box, given as lowest x, y, z and highest x, y, z; else what it is.
QString SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::worldBoxMismatch( const SIGEL_Robot::SIG_Robot &robot, const SIG_DynaMechsSimulationQueries &queries, int number,
                          const QList< double > &box )
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
  for ( int r = 0; r < 3; r++ )
  {
    if ( std::fabs( lowest[ r ] - box[ r ] ) > 1e-6 || std::fabs( highest[ r ] - box[ r + 3 ] ) > 1e-6 )
    {
      return QString( "link %1 is from ( %2, %3, %4 ) to ( %5, %6, %7 )" ).arg( number )
        .arg( lowest[ 0 ] ).arg( lowest[ 1 ] ).arg( lowest[ 2 ] ).arg( highest[ 0 ] ).arg( highest[ 1 ] ).arg( highest[ 2 ] );
    }
  }
  return QString();
}

QString SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::vectorMismatch( SIG_Vector actual, SIG_Vector expected, double tolerance )
{
  if ( std::fabs( actual.x - expected.x ) <= tolerance && std::fabs( actual.y - expected.y ) <= tolerance && std::fabs( actual.z - expected.z ) <= tolerance )
  {
    return QString();
  }
  return QString( "( %1, %2, %3 ), expected ( %4, %5, %6 )" ).arg( actual.x ).arg( actual.y ).arg( actual.z )
    .arg( expected.x ).arg( expected.y ).arg( expected.z );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getCurrentSimulationSecondsIsTheFrameTimesTheStepSize_data()
{
  QTest::addColumn< int >( "frame" );
  QTest::addColumn< double >( "stepSize" );
  QTest::addColumn< double >( "seconds" );

  QTest::newRow( "at the start" ) << 0 << 0.02 << 0.0;
  QTest::newRow( "125 steps of 0.02" ) << 125 << 0.02 << 2.5;
  QTest::newRow( "3000 steps of 0.005" ) << 3000 << 0.005 << 15.0;
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getCurrentSimulationSecondsIsTheFrameTimesTheStepSize()
{
  QFETCH( int, frame );
  QFETCH( double, stepSize );
  QFETCH( double, seconds );
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setStepSize( stepSize );
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.actualFrame = frame;

  QVERIFY2( std::fabs( simulationQueries.getCurrentSimulationSeconds() - seconds ) < 1e-9,
            qPrintable( QString::number( simulationQueries.getCurrentSimulationSeconds(), 'g', 17 ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getCurrentSimulationWholeSecondsCutsTheTimeToWholeSeconds_data()
{
  QTest::addColumn< int >( "frame" );
  QTest::addColumn< double >( "stepSize" );
  QTest::addColumn< QTime >( "wholeSeconds" );

  QTest::newRow( "at the start" ) << 0 << 0.02 << QTime( 0, 0, 0 );
  QTest::newRow( "2.5 seconds" ) << 125 << 0.02 << QTime( 0, 0, 2 );
  QTest::newRow( "2.98 seconds" ) << 149 << 0.02 << QTime( 0, 0, 2 );
  QTest::newRow( "3 seconds" ) << 150 << 0.02 << QTime( 0, 0, 3 );
  QTest::newRow( "65 seconds" ) << 3250 << 0.02 << QTime( 0, 1, 5 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getCurrentSimulationWholeSecondsCutsTheTimeToWholeSeconds()
{
  QFETCH( int, frame );
  QFETCH( double, stepSize );
  QFETCH( QTime, wholeSeconds );
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setStepSize( stepSize );
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.actualFrame = frame;

  QCOMPARE( simulationQueries.getCurrentSimulationWholeSeconds(), wholeSeconds );
}

// A robot without joints keeps the frame of its model, so the origin of its root link is at the start position.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkPositionGivesTheStartPositionForARobotWithoutJoints()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( 0, 0, 0 ), SIG_Vector( 1, 2, 3 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( vectorMismatch( simulationQueries.getLinkPosition( 0 ), SIG_Vector( 3, 4, 5 ), 1e-9 ), QString() );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkPositionAndOrientationPutEachLinkWhereTheSimulationHasIt_data()
{
  QTest::addColumn< double >( "start" );
  QTest::addColumn< bool >( "turned" );
  QTest::addColumn< QList< double > >( "baseBox" );
  QTest::addColumn< QList< double > >( "armBox" );

  // In the model, base fills x -1..1, y -0.5..3, z -0.5..0.5 and arm x 1..3, y -2..0.25, z -0.25..0.25; the start position adds ( 3, 4, 5 ).
  // A start value of 90 turns the arm by -90 degrees about its axis along +y at x 1, so that it points along +z.
  // The quarter turn of the robot about y takes x to -z and z to x.
  QTest::newRow( "in the pose of the model" ) << 0.0 << false << QList< double >{ 2, 3.5, 4.5, 4, 7, 5.5 } << QList< double >{ 4, 2, 4.75, 6, 4.25, 5.25 };
  QTest::newRow( "arm turned by its joint" ) << 90.0 << false << QList< double >{ 2, 3.5, 4.5, 4, 7, 5.5 } << QList< double >{ 3.75, 2, 5, 4.25, 4.25, 7 };
  QTest::newRow( "robot turned a quarter about y" ) << 0.0 << true << QList< double >{ 2.5, 3.5, 4, 3.5, 7, 6 } << QList< double >{ 2.75, 2, 2, 3.25, 4.25, 4 };
}

// SIG_SimulationVisualisation draws each vertex of a link at the link's position plus its orientation
// times the vertex, with the vertices as SIG_Robot::prepareDynaMechs leaves them.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkPositionAndOrientationPutEachLinkWhereTheSimulationHasIt()
{
  QFETCH( double, start );
  QFETCH( bool, turned );
  QFETCH( QList< double >, baseBox );
  QFETCH( QList< double >, armBox );
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, start );
  robot.prepareDynaMechs();
  if ( turned )
  {
    robot.initialOrientation = turnAbout( 1, 90 );
  }
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 0, baseBox ), QString() );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, 1, armBox ), QString() );
}

// One second of free fall under the default gravity of 9.81 drops the box by 4.905; the simulation is not exact, so 1 % off is allowed.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkPositionFollowsTheRobotWhenItFalls()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 20, 10 ) );
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setStepSize( 0.02 );
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  for ( int step = 0; step < 50; step++ )
  {
    simulationData.simulationProgress();
    simulationData.actualFrame++;
  }

  QCOMPARE( vectorMismatch( simulationQueries.getLinkPosition( 0 ), SIG_Vector( 10, 20 - 4.905, 10 ), 0.05 ), QString() );
}

// A robot without joints keeps the frame of its model, so its root link is not turned.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkOrientationGivesNoTurnForARobotWithoutJoints()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( 0, 0, 0 ), SIG_Vector( 1, 2, 3 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  SIG_Matrix noTurn;
  noTurn.makeone();
  QCOMPARE( matrixMismatch( simulationQueries.getLinkOrientation( 0 ), noTurn ), QString() );
}

// Without gravity, the robot spins about the vertical at 1 radian per second for 0.5 seconds. A second
// arm on -x makes the vertical a free axis of the robot, so it spins steadily. The sliders along y hold
// the arms in the turn, so the root's orientation turns the arm's offset from the root at the start
// into its offset now, and the arm turns from its start orientation as the root does.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkOrientationFollowsTheRobotWhenItTurns()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0 );
  SIGEL_Robot::SIG_Link *other = addBoxLink( robot, "other", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_TranslationalJoint *otherSlider = new SIGEL_Robot::SIG_TranslationalJoint( &robot, "joint1", 1 );
  otherSlider->setLeftLink( robot.getLinks()[ 0 ] );
  otherSlider->setRightLink( other );
  otherSlider->setLeftPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( -2, 0, 0 ) );
  otherSlider->setRightPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  otherSlider->setRange( -1, 1, 0 );
  robot.addJoint( otherSlider );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setStepSize( 0.02 );
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  const SIG_Vector baseAtStart = simulationQueries.getLinkPosition( 0 );
  const SIG_Vector armAtStart = simulationQueries.getLinkPosition( 1 );
  const double start[ 3 ] = { armAtStart.x - baseAtStart.x, armAtStart.y - baseAtStart.y, armAtStart.z - baseAtStart.z };
  SIG_Matrix armOrientationAtStart = simulationQueries.getLinkOrientation( 1 );
  dmRigidBody *root = simulationData.dynaMechsLinks[ 0 ]->dynaMechsLink;
  Float state[ 7 ];
  Float velocity[ 6 ];
  static_cast< dmMobileBaseLink * >( root )->getState( state, velocity );
  Float spin[ 6 ] = { 0, 0, 1, 0, 0, 0 };
  static_cast< dmMobileBaseLink * >( root )->setState( state, spin );

  // The integrator copies the state of the system when it takes the system.
  simulationData.dynaMechsIntegrator->setSystem( &simulationData.dynaMechsSystem );

  for ( int step = 0; step < 25; step++ )
  {
    simulationData.simulationProgress();
  }

  SIG_Matrix orientation = simulationQueries.getLinkOrientation( 0 );
  const double cosine = ( orientation.get( 0, 0 ) + orientation.get( 2, 2 ) ) / 2;
  QVERIFY2( std::fabs( orientation.get( 1, 1 ) - 1 ) < 1e-6 && std::fabs( cosine - std::cos( 0.5 ) ) < 0.01,
            qPrintable( QString( "the root is turned by %1 degrees, y stays at %2" ).arg( std::acos( std::min( 1.0, cosine ) ) * 180 / M_PI ).arg( orientation.get( 1, 1 ) ) ) );
  SIG_Vector base = simulationQueries.getLinkPosition( 0 );
  SIG_Vector arm = simulationQueries.getLinkPosition( 1 );
  double now[ 3 ] = { 0, 0, 0 };
  for ( int r = 0; r < 3; r++ )
  {
    for ( int c = 0; c < 3; c++ )
    {
      now[ r ] += orientation.get( r, c ) * start[ c ];
    }
  }
  const SIG_Vector offset( now[ 0 ], now[ 1 ], now[ 2 ] );
  QCOMPARE( vectorMismatch( SIG_Vector( arm.x - base.x, arm.y - base.y, arm.z - base.z ), offset, 1e-3 ), QString() );
  SIG_Matrix armTurned = product( orientation, armOrientationAtStart );
  QCOMPARE( matrixMismatch( simulationQueries.getLinkOrientation( 1 ), armTurned ), QString() );
}

// The root is the second link of the robot here, so its number is 1, and it is the base that fills x 2..4, y 3.5..7, z 4.5..5.5.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getRootNumberGivesTheNumberOfTheRootLink()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  robot.setRootLink( base );
  addJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0, base, arm, -120, 120, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 3, 4, 5 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( simulationQueries.getRootNumber(), 1 );
  QCOMPARE( worldBoxMismatch( robot, simulationQueries, simulationQueries.getRootNumber(), { 2, 3.5, 4.5, 4, 7, 5.5 } ), QString() );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getLinkCountGivesTheNumberOfLinksOfTheRobot()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ) );
  addJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 1, robot.getLinks()[ 1 ], tip, -120, 120, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  QCOMPARE( simulationQueries.getLinkCount(), 3 );
}

// The caller owns the list and each array of six values in it.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getUsedForcesGivesOneForceForEachLink()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ) );
  addJoint( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 1, robot.getLinks()[ 1 ], tip, -120, 120, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  simulationData.simulationProgress();

  std::vector< double * > *forces = simulationQueries.getUsedForces();

  QVERIFY( forces );
  const qsizetype count = static_cast< qsizetype >( forces->size() );
  for ( double *force : *forces )
  {
    delete[] force;
  }
  delete forces;
  QCOMPARE( count, 3 );
}

// Without gravity, high above the floor and with no drive, nothing acts on any link. Only the base is
// read: DynaMechs computes no values for a link without children, so the arm's memory holds whatever it held.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getUsedForcesGivesZeroForARobotThatNothingPushes()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  simulationData.simulationProgress();

  std::vector< double * > *forces = simulationQueries.getUsedForces();

  QVERIFY( forces );
  QString mismatch;
  for ( std::size_t link = 0; link < forces->size(); link++ )
  {
    for ( int i = 0; i < 6; i++ )
    {
      if ( link == 0 && mismatch.isEmpty() && std::fabs( ( *forces )[ link ][ i ] ) > 1e-9 )
      {
        mismatch = QString( "value %1 of link %2 is %3" ).arg( i ).arg( link ).arg( ( *forces )[ link ][ i ] );
      }
    }
    delete[] ( *forces )[ link ];
  }
  const qsizetype count = static_cast< qsizetype >( forces->size() );
  delete forces;
  QCOMPARE( count, 2 );
  QCOMPARE( mismatch, QString() );
}

// Without gravity and high above the floor, the hinge's motor turns the arm with a torque of 5
// about the hinge axis, and nothing else acts. So the arm takes a moment of 5 about the hinge axis.
// DynaMechs gives the values of a link in the link's own axes: the z axis of the link behind a joint
// is the joint axis.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getUsedForcesGivesTheMomentOfAJointOnTheLinkItTurns()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  dmMDHLink *hinge = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( hinge );
  Float torque = 5;
  hinge->setJointInput( &torque );
  simulationData.simulationProgress();

  std::vector< double * > *forces = simulationQueries.getUsedForces();

  QVERIFY( forces );
  QString values;
  double moment = 0;
  for ( std::size_t link = 0; link < forces->size(); link++ )
  {
    values += QString( "link %1:" ).arg( link );
    for ( int i = 0; i < 6; i++ )
    {
      values += QString( " %1" ).arg( ( *forces )[ link ][ i ] );
    }
    values += "; ";
  }
  if ( forces->size() == 2 )
  {
    moment = std::fabs( ( *forces )[ 1 ][ 2 ] );
  }
  for ( double *force : *forces )
  {
    delete[] force;
  }
  delete forces;
  QVERIFY2( std::fabs( moment - 5 ) < 0.05, qPrintable( values ) );
}

// The same for a slider with a push of 5: the arm takes a force of 5 along the slider axis, the z axis of its
// link, and the base a force of 5 along its own y axis, the vertical, along which the slider axis runs. The base
// starts in the robot file's frame, whose y axis points up.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::getUsedForcesGivesTheForceOfASliderOnTheLinkItMoves()
{
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0 );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  dmMDHLink *slider = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( slider );
  Float push = 5;
  slider->setJointInput( &push );
  simulationData.simulationProgress();

  std::vector< double * > *forces = simulationQueries.getUsedForces();

  QVERIFY( forces );
  QString values;
  double baseForce = 0;
  double force = 0;
  for ( std::size_t link = 0; link < forces->size(); link++ )
  {
    values += QString( "link %1:" ).arg( link );
    for ( int i = 0; i < 6; i++ )
    {
      values += QString( " %1" ).arg( ( *forces )[ link ][ i ] );
    }
    values += "; ";
  }
  if ( forces->size() == 2 )
  {
    baseForce = std::fabs( ( *forces )[ 0 ][ 4 ] );
    force = std::fabs( ( *forces )[ 1 ][ 5 ] );
  }
  for ( double *each : *forces )
  {
    delete[] each;
  }
  delete forces;
  QVERIFY2( std::fabs( baseForce - 5 ) < 0.05, qPrintable( values ) );
  QVERIFY2( std::fabs( force - 5 ) < 0.05, qPrintable( values ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheJointPositionInTheRangeOfTheRegister_data()
{
  QTest::addColumn< int >( "jointType" );
  QTest::addColumn< double >( "minimum" );
  QTest::addColumn< double >( "maximum" );
  QTest::addColumn< double >( "start" );
  QTest::addColumn< int >( "registerSize" );
  QTest::addColumn< int >( "expected" );

  // The joint's range maps onto the register's range: its minimum to the bottom, its maximum to the top.
  const int rotational = SIGEL_Robot::SIG_Joint::tRotationalJoint;
  QTest::newRow( "rotational, at the middle" ) << rotational << -120.0 << 120.0 << 0.0 << 8 << 0;
  const int translational = SIGEL_Robot::SIG_Joint::tTranslationalJoint;
  QTest::newRow( "translational, three quarters up" ) << translational << -1.0 << 1.0 << 0.5 << 8 << 64;
  QTest::newRow( "translational, range off the middle" ) << translational << -0.5 << 1.5 << 0.0 << 8 << -64;
  QTest::newRow( "translational, at its minimum" ) << translational << -1.0 << 1.0 << -1.0 << 8 << -128;
  QTest::newRow( "translational, at its maximum" ) << translational << -1.0 << 1.0 << 1.0 << 8 << 127;
  QTest::newRow( "translational, 16-bit register" ) << translational << -1.0 << 1.0 << 0.5 << 16 << 16384;
}

// The register holds whole numbers, so the reading may be one off.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheJointPositionInTheRangeOfTheRegister()
{
  QFETCH( int, jointType );
  QFETCH( double, minimum );
  QFETCH( double, maximum );
  QFETCH( double, start );
  QFETCH( int, registerSize );
  QFETCH( int, expected );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, static_cast< SIGEL_Robot::SIG_Joint::JointType >( jointType ), minimum, maximum, start );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "angle", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  const int value = reading( simulationQueries, 0, registerSize );
  QVERIFY2( std::abs( value - expected ) <= 1, qPrintable( QString( "reads %1, expected %2" ).arg( value ).arg( expected ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseWritesTheReadingIntoTheFirstRegisterOnly()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0.5 );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  QList< SIG_Register > registers( 4, SIG_Register( 8 ) );
  registers[ 0 ].loadValue( -100 );
  registers[ 1 ].loadValue( 5 );
  registers[ 2 ].loadValue( -3 );
  registers[ 3 ].loadValue( 7 );

  simulationQueries.sense( 0, registers );

  QVERIFY2( std::abs( registers[ 0 ].getValue() - 64 ) <= 1, qPrintable( QString::number( registers[ 0 ].getValue() ) ) );
  QCOMPARE( registers[ 1 ].getValue(), 5 );
  QCOMPARE( registers[ 2 ].getValue(), -3 );
  QCOMPARE( registers[ 3 ].getValue(), 7 );
}

// Without gravity, a push on the slider moves the arm along it. Its position on the slider is how far
// the arm has moved along the base's y axis from the start value 0, and the reading must follow it.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheJointPositionAfterTheJointHasMoved()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0 );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  dmMDHLink *slider = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( slider );
  const double atStart = armAlongBaseY( simulationQueries );
  Float push = 10;
  slider->setJointInput( &push );

  for ( int step = 0; step < 25; step++ )
  {
    simulationData.simulationProgress();
  }

  const double moved = armAlongBaseY( simulationQueries ) - atStart;
  QVERIFY2( std::fabs( moved ) > 0.1 && std::fabs( moved ) < 0.9, qPrintable( QString( "the arm moved %1" ).arg( moved ) ) );
  const int expected = static_cast< int >( -128 + ( moved + 1 ) / 2 * 256 );
  const int value = reading( simulationQueries, 0, 8 );
  QVERIFY2( std::abs( value - expected ) <= 1, qPrintable( QString( "reads %1, expected %2" ).arg( value ).arg( expected ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsAJointWithTheRootOnTheRight_data()
{
  QTest::addColumn< int >( "jointType" );
  QTest::addColumn< double >( "start" );
  QTest::addColumn< int >( "expected" );

  const int rotational = SIGEL_Robot::SIG_Joint::tRotationalJoint;
  QTest::newRow( "rotational, three quarters up" ) << rotational << 60.0 << 64;
}

// A robot file may name the child of a joint as its left link and the root as its right one. The reading follows the robot file all the same.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsAJointWithTheRootOnTheRight()
{
  QFETCH( int, jointType );
  QFETCH( double, start );
  QFETCH( int, expected );
  const SIGEL_Robot::SIG_Joint::JointType type = static_cast< SIGEL_Robot::SIG_Joint::JointType >( jointType );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  const double range = type == SIGEL_Robot::SIG_Joint::tRotationalJoint ? 120 : 1;
  SIGEL_Robot::SIG_Joint *joint = addJoint( robot, type, 0, arm, base, -range, range, start );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "sensor", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  const int value = reading( simulationQueries, 0, 8 );
  QVERIFY2( std::abs( value - expected ) <= 1, qPrintable( QString( "reads %1, expected %2" ).arg( value ).arg( expected ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheJointThatTheSensorIsOn_data()
{
  QTest::addColumn< int >( "jointNumber" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "joint 0, to link 2" ) << 0 << 64;
  QTest::newRow( "joint 1, to link 1" ) << 1 << -64;
}

// A legged robot has several joints on its root, and the robot file may list the links in any order.
// Here joint 0 is the slider to link 2 on +x, started at 0.5, and joint 1 the slider to link 1 on -x, started at -0.5.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheJointThatTheSensorIsOn()
{
  QFETCH( int, jointNumber );
  QFETCH( int, expected );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
  SIGEL_Robot::SIG_Link *left = addBoxLink( robot, "left", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  SIGEL_Robot::SIG_Link *right = addBoxLink( robot, "right", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
  robot.setRootLink( base );
  SIGEL_Robot::SIG_Joint *joints[ 2 ] = { addJoint( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 0, base, right, -1, 1, 0.5 ), nullptr };
  SIGEL_Robot::SIG_TranslationalJoint *leftSlider = new SIGEL_Robot::SIG_TranslationalJoint( &robot, "joint1", 1 );
  leftSlider->setLeftLink( base );
  leftSlider->setRightLink( left );
  leftSlider->setLeftPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 1, 0 ), SIG_Vector( -2, 0, 0 ) );
  leftSlider->setRightPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 1, 0 ), SIG_Vector( 0, 0, 0 ) );
  leftSlider->setRange( -1, 1, -0.5 );
  robot.addJoint( leftSlider );
  joints[ 1 ] = leftSlider;
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", 0 );
  sensor->setJoint( joints[ jointNumber ] );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  const int value = reading( simulationQueries, 0, 8 );
  QVERIFY2( std::abs( value - expected ) <= 1, qPrintable( QString( "reads %1, expected %2" ).arg( value ).arg( expected ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsAJointPushedPastItsLimitAsTheEndOfTheRegister_data()
{
  QTest::addColumn< double >( "push" );

  QTest::newRow( "pushed one way" ) << 1000.0;
  QTest::newRow( "pushed the other way" ) << -1000.0;
}

// The joint limits of DynaMechs are springs, so a hard push takes the slider past its end. The reading stays at the register's end on that side.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsAJointPushedPastItsLimitAsTheEndOfTheRegister()
{
  QFETCH( double, push );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -0.2, 0.2, 0 );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, 0, 0 ) );
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  dmMDHLink *slider = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( slider );
  const double atStart = armAlongBaseY( simulationQueries );
  Float input = push;
  slider->setJointInput( &input );

  for ( int step = 0; step < 25; step++ )
  {
    simulationData.simulationProgress();
  }

  const double moved = armAlongBaseY( simulationQueries ) - atStart;
  QVERIFY2( std::fabs( moved ) > 0.25, qPrintable( QString( "the arm moved %1" ).arg( moved ) ) );
  QCOMPARE( reading( simulationQueries, 0, 8 ), moved > 0 ? 127 : -128 );
}

// A joint with equal limits has no limits, as a wheel. Its position repeats every full turn, and a
// quarter turn is a different position. The joint is set to each angle in DynaMechs.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsAJointWithoutLimitsByItsAngle()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, 0, 0, 0 );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "wheel", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  dmMDHLink *wheel = dynamic_cast< dmMDHLink * >( simulationData.dynaMechsLinks[ 1 ]->dynaMechsLink );
  QVERIFY( wheel );
  Float start[ 1 ];
  Float velocity[ 1 ];
  wheel->getState( start, velocity );
  const double turns[ 3 ] = { 0, M_PI / 2, 2 * M_PI };
  int readings[ 3 ] = { 0, 0, 0 };

  for ( int k = 0; k < 3; k++ )
  {
    Float angle[ 1 ] = { start[ 0 ] + turns[ k ] };
    wheel->setState( angle, velocity );
    readings[ k ] = reading( simulationQueries, 0, 8 );
  }

  const QString message = QString( "at 0, 90 and 360 degrees it reads %1, %2 and %3" ).arg( readings[ 0 ] ).arg( readings[ 1 ] ).arg( readings[ 2 ] );
  QVERIFY2( std::abs( readings[ 0 ] - readings[ 2 ] ) <= 1, qPrintable( message ) );
  QVERIFY2( std::abs( readings[ 0 ] - readings[ 1 ] ) > 1, qPrintable( message ) );
}

// A slider with equal limits has no limits, so it has no range to read; sense leaves the register as it was.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseLeavesTheRegisterForASliderWithoutLimits()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *joint = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 0, 0, 0 );
  SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide", 0 );
  sensor->setJoint( joint );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  QList< SIG_Register > registers( 1, SIG_Register( 8 ) );
  registers[ 0 ].loadValue( 37 );

  simulationQueries.sense( 0, registers );

  QCOMPARE( registers[ 0 ].getValue(), 37 );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsContactWithTheGroundAsTheTopOfTheRegister_data()
{
  QTest::addColumn< int >( "registerSize" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "8-bit register" ) << 8 << 127;
  QTest::newRow( "16-bit register" ) << 16 << 32767;
}

// The box starts with its bottom 0.05 into the flat floor, so it touches the ground in the first step.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsContactWithTheGroundAsTheTopOfTheRegister()
{
  QFETCH( int, registerSize );
  QFETCH( int, expected );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  sensor->setLink( box );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 0.45, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.simulationProgress();

  QCOMPARE( reading( simulationQueries, 0, registerSize ), expected );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsNoContactAsTheBottomOfTheRegister_data()
{
  QTest::addColumn< int >( "registerSize" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "8-bit register" ) << 8 << -128;
  QTest::newRow( "16-bit register" ) << 16 << -32768;
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsNoContactAsTheBottomOfTheRegister()
{
  QFETCH( int, registerSize );
  QFETCH( int, expected );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  sensor->setLink( box );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 5, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.simulationProgress();

  QCOMPARE( reading( simulationQueries, 0, registerSize ), expected );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheContactOfALinkBehindAJoint_data()
{
  QTest::addColumn< int >( "linkNumber" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "arm on the floor" ) << 1 << 127;
  QTest::newRow( "base above the floor" ) << 0 << -128;
}

// The arm hangs 1.5 lower than the base; it starts with its bottom 0.05 into the floor and the base 1.45 above it.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheContactOfALinkBehindAJoint()
{
  QFETCH( int, linkNumber );
  QFETCH( int, expected );
  SIGEL_Robot::SIG_Robot robot;
  addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
  SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  sensor->setLink( robot.getLinks()[ linkNumber ] );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 1.95, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.simulationProgress();

  QCOMPARE( reading( simulationQueries, 0, 8 ), expected );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsContactOfALinkWithOnlyPartOfItOnTheGround_data()
{
  QTest::addColumn< bool >( "corner" );

  QTest::newRow( "an edge down" ) << false;
  QTest::newRow( "a corner down" ) << true;
}

// A walking foot often touches the ground with an edge or a corner only. The box is tilted and starts with its lowest point 0.02 into the floor.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsContactOfALinkWithOnlyPartOfItOnTheGround()
{
  QFETCH( bool, corner );
  SIG_Matrix tilt = turnAbout( 2, -45 );
  if ( corner )
  {
    tilt = product( turnAbout( 0, 20 ), turnAbout( 2, 30 ) );
  }
  double lowest = 0;
  for ( double x : { -0.5, 0.5 } )
  {
    for ( double y : { -0.5, 0.5 } )
    {
      for ( double z : { -0.5, 0.5 } )
      {
        lowest = std::min( lowest, tilt.get( 1, 0 ) * x + tilt.get( 1, 1 ) * y + tilt.get( 1, 2 ) * z );
      }
    }
  }
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  sensor->setLink( box );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  robot.initialOrientation = tilt;
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, -lowest - 0.02, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  simulationData.simulationProgress();

  QCOMPARE( reading( simulationQueries, 0, 8 ), 127 );
}

// A foot lands and lifts off. The box falls from 0.5 above the floor and lands within a second; then it is
// thrown up at 3 per second and leaves the ground. The reading follows each change.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheContactOfALinkAsItLandsAndLiftsOff()
{
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_ContactSensor *sensor = new SIGEL_Robot::SIG_ContactSensor( &robot, "contact", 0 );
  sensor->setLink( box );
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 1, 10 ) );
  SIG_SimulationParameters simulationParameters;
  simulationParameters.setStepSize( 0.02 );
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  simulationData.simulationProgress();
  QCOMPARE( reading( simulationQueries, 0, 8 ), -128 );
  bool landed = false;

  for ( int step = 0; step < 50 && !landed; step++ )
  {
    simulationData.simulationProgress();
    landed = reading( simulationQueries, 0, 8 ) == 127;
  }

  QVERIFY( landed );
  dmRigidBody *root = simulationData.dynaMechsLinks[ 0 ]->dynaMechsLink;
  Float state[ 7 ];
  Float velocity[ 6 ];
  static_cast< dmMobileBaseLink * >( root )->getState( state, velocity );
  Float throwUp[ 6 ] = { 0, 0, 0, 0, 0, 3 };
  static_cast< dmMobileBaseLink * >( root )->setState( state, throwUp );

  // The integrator copies the state of the system when it takes the system.
  simulationData.dynaMechsIntegrator->setSystem( &simulationData.dynaMechsSystem );
  const double heightOnTheGround = simulationQueries.getLinkPosition( 0 ).y;
  bool lifted = false;
  for ( int step = 0; step < 10 && !lifted; step++ )
  {
    simulationData.simulationProgress();
    lifted = reading( simulationQueries, 0, 8 ) == -128;
  }

  QVERIFY2( simulationQueries.getLinkPosition( 0 ).y > heightOnTheGround + 0.01, qPrintable( QString( "the box rose from %1 to %2" ).arg( heightOnTheGround ).arg( simulationQueries.getLinkPosition( 0 ).y ) ) );
  QVERIFY( lifted );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsALevelLinkAsTheMiddleOfTheRegister_data()
{
  QTest::addColumn< bool >( "pitch" );

  QTest::newRow( "pitch" ) << true;
  QTest::newRow( "roll" ) << false;
}

// A pitch or roll sensor reads -90 to 90 degrees over the register's range, so a link that is not tilted reads the middle.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsALevelLinkAsTheMiddleOfTheRegister()
{
  QFETCH( bool, pitch );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
  robot.setRootLink( box );
  SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
  sensor->setLink( box );
  if ( pitch )
  {
    sensor->SetPitchType();
  }
  else
  {
    sensor->SetRollType();
  }
  robot.addSensor( sensor );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;

  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );

  const int value = reading( simulationQueries, 0, 8 );
  QVERIFY2( std::abs( value ) <= 1, qPrintable( QString( "reads %1" ).arg( value ) ) );
}

// A tilt of 30 degrees is 30 / 180 of the register's range, 42.67, off the middle. SIGEL fixes no front
// of a robot, so which level axis is the pitch axis is open; but the pitch sensor reads the turns about
// one of x and z, the roll sensor those about the other, and each ignores the other axis.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsATiltAboutOneLevelAxisOnlyAndPitchAndRollUseDifferentAxes()
{
  int tiltAxis[ 2 ] = { -1, -1 };
  QString readings;
  for ( int kind = 0; kind < 2; kind++ )
  {
    const bool pitch = kind == 0;
    for ( int axis = 0; axis < 3; axis += 2 )
    {
      const int up = tiltReading( pitch, turnAbout( axis, 30 ) );
      const int down = tiltReading( pitch, turnAbout( axis, -30 ) );
      readings += QString( "%1 about %2: %3 and %4; " ).arg( pitch ? "pitch" : "roll" ).arg( axis == 0 ? "x" : "z" ).arg( up ).arg( down );
      const bool followsTheTilt = std::abs( std::abs( up ) - 42.67 ) <= 1.5 && std::abs( up + down ) <= 1;
      const bool ignoresTheTilt = std::abs( up ) <= 1 && std::abs( down ) <= 1;
      if ( followsTheTilt )
      {
        tiltAxis[ kind ] = tiltAxis[ kind ] == -1 ? axis : 3;
      }
      else if ( !ignoresTheTilt )
      {
        tiltAxis[ kind ] = 3;
      }
    }
  }

  const bool oneAxisEach = tiltAxis[ 0 ] >= 0 && tiltAxis[ 0 ] < 3 && tiltAxis[ 1 ] >= 0 && tiltAxis[ 1 ] < 3 && tiltAxis[ 0 ] != tiltAxis[ 1 ];
  QVERIFY2( oneAxisEach, qPrintable( readings ) );
}

// A 16-bit register has 256 times the steps of an 8-bit one, so a tilt sensor reads 256 times as much
// on it, up to the rest of a step. Each sensor reads at least one of these tilts off the middle and
// short of the ends.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsATiltOnTheScaleOfTheRegister()
{
  QString mismatch;
  bool offTheMiddle[ 2 ] = { false, false };
  for ( int kind = 0; kind < 2; kind++ )
  {
    for ( int axis = 0; axis < 3; axis += 2 )
    {
      for ( double degrees : { -30.0, 30.0 } )
      {
        SIG_Matrix tilt = turnAbout( axis, degrees );
        QList< int > readings;
        for ( int registerSize : { 8, 16 } )
        {
          SIGEL_Robot::SIG_Robot robot;
          SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
          robot.setRootLink( box );
          SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
          sensor->setLink( box );
          if ( kind == 0 )
          {
            sensor->SetPitchType();
          }
          else
          {
            sensor->SetRollType();
          }
          robot.addSensor( sensor );
          robot.prepareDynaMechs();
          robot.initialOrientation = tilt;
          SIGEL_Environment::SIG_Environment environment;
          environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
          SIG_SimulationParameters simulationParameters;
          SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
          SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
          readings.append( reading( simulationQueries, 0, registerSize ) );
        }
        if ( std::abs( readings[ 0 ] ) > 1 && std::abs( readings[ 0 ] ) < 127 )
        {
          offTheMiddle[ kind ] = true;
        }
        if ( mismatch.isEmpty() && std::abs( readings[ 1 ] - 256 * readings[ 0 ] ) > 256 )
        {
          mismatch = QString( "%1 at %2 degrees about %3 reads %4 on 8 bits and %5 on 16 bits" ).arg( kind == 0 ? "pitch" : "roll" ).arg( degrees )
            .arg( axis == 0 ? "x" : "z" ).arg( readings[ 0 ] ).arg( readings[ 1 ] );
        }
      }
    }
  }

  QCOMPARE( mismatch, QString() );
  QVERIFY2( offTheMiddle[ 0 ] && offTheMiddle[ 1 ], "a sensor reads no tilt of 30 degrees between the middle and the ends of the register" );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheSameTiltWhenTheRobotTurnsAboutTheVertical_data()
{
  QTest::addColumn< bool >( "pitch" );
  QTest::addColumn< int >( "tiltAxis" );
  QTest::addColumn< double >( "degrees" );

  // A tilt axis of -1 is no tilt.
  QTest::newRow( "pitch, level, turned 45 degrees" ) << true << -1 << 45.0;
  QTest::newRow( "pitch, level, turned 90 degrees" ) << true << -1 << 90.0;
  QTest::newRow( "pitch, tilted about x, turned 90 degrees" ) << true << 0 << 90.0;
  QTest::newRow( "pitch, tilted about z, turned 90 degrees" ) << true << 2 << 90.0;
  QTest::newRow( "roll, level, turned 45 degrees" ) << false << -1 << 45.0;
  QTest::newRow( "roll, level, turned 90 degrees" ) << false << -1 << 90.0;
  QTest::newRow( "roll, tilted about x, turned 90 degrees" ) << false << 0 << 90.0;
  QTest::newRow( "roll, tilted about z, turned 90 degrees" ) << false << 2 << 90.0;
}

// A robot that turns while it walks keeps its tilt: the box is tilted by 30 degrees, then turned about the vertical.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheSameTiltWhenTheRobotTurnsAboutTheVertical()
{
  QFETCH( bool, pitch );
  QFETCH( int, tiltAxis );
  QFETCH( double, degrees );
  SIG_Matrix tilt = turnAbout( 1, 0 );
  if ( tiltAxis >= 0 )
  {
    tilt = turnAbout( tiltAxis, 30 );
  }

  const int unturned = tiltReading( pitch, tilt );
  const int turned = tiltReading( pitch, product( turnAbout( 1, degrees ), tilt ) );

  QVERIFY2( std::abs( turned - unturned ) <= 1, qPrintable( QString( "reads %1, unturned %2" ).arg( turned ).arg( unturned ) ) );
}

// A robot that has fallen over is tilted past a quarter turn. A tilt sensor reads at most 90 degrees.
// About its own level axis, a quarter turn one way and one the other way read the two ends of the
// register, and a tilt of 120 or 150 degrees reads as the quarter turn that way. About the other
// level axis a quarter turn stands the sensor's axis upright, so that axis is not checked.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsATiltPastAQuarterTurnAsAQuarterTurn()
{
  QString mismatch;
  for ( int kind = 0; kind < 2 && mismatch.isEmpty(); kind++ )
  {
    const QString name = kind == 0 ? "pitch" : "roll";
    int ownAxis = -1;
    for ( int axis = 0; axis < 3; axis += 2 )
    {
      const int up = tiltReading( kind == 0, turnAbout( axis, 90 ) );
      const int down = tiltReading( kind == 0, turnAbout( axis, -90 ) );
      const bool topAndBottom = std::abs( up - 127 ) <= 1 && std::abs( down + 128 ) <= 1;
      const bool bottomAndTop = std::abs( up + 128 ) <= 1 && std::abs( down - 127 ) <= 1;
      if ( topAndBottom || bottomAndTop )
      {
        ownAxis = axis;
      }
    }
    if ( ownAxis < 0 )
    {
      mismatch = QString( "%1 reads no quarter turn one way and the other way as the two ends of the register" ).arg( name );
      break;
    }
    for ( int sign = -1; sign <= 1; sign += 2 )
    {
      const int quarter = tiltReading( kind == 0, turnAbout( ownAxis, sign * 90 ) );
      for ( double degrees : { 120.0, 150.0 } )
      {
        const int past = tiltReading( kind == 0, turnAbout( ownAxis, sign * degrees ) );
        if ( mismatch.isEmpty() && std::abs( past - quarter ) > 1 )
        {
          mismatch = QString( "%1 reads %2 at %3 degrees and %4 at %5 degrees" ).arg( name ).arg( past ).arg( sign * degrees ).arg( quarter ).arg( sign * 90 );
        }
      }
    }
  }

  QCOMPARE( mismatch, QString() );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltThatTheSimulationHasMade_data()
{
  QTest::addColumn< bool >( "pitch" );
  QTest::addColumn< double >( "spin" );

  QTest::newRow( "pitch, one way" ) << true << 1.0;
  QTest::newRow( "pitch, the other way" ) << true << -1.0;
  QTest::newRow( "roll, one way" ) << false << 1.0;
  QTest::newRow( "roll, the other way" ) << false << -1.0;
}

// Without gravity, the box spins about a level axis for 0.5 seconds. Its tilt sensor then reads what it
// reads on a box that starts in the orientation the spin has given it, and not what a level box reads.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltThatTheSimulationHasMade()
{
  QFETCH( bool, pitch );
  QFETCH( double, spin );
  SIG_Matrix orientation;
  int value = 0;

  // One simulation at a time: this one ends before tiltReading makes its own.
  {
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Link *box = addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) );
    robot.setRootLink( box );
    SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
    sensor->setLink( box );
    if ( pitch )
    {
      sensor->SetPitchType();
    }
    else
    {
      sensor->SetRollType();
    }
    robot.addSensor( sensor );
    robot.prepareDynaMechs();
    SIGEL_Environment::SIG_Environment environment;
    environment.setGravity( SIG_Vector( 0, 0, 0 ) );
    environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
    SIG_SimulationParameters simulationParameters;
    simulationParameters.setStepSize( 0.02 );
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
    dmRigidBody *root = simulationData.dynaMechsLinks[ 0 ]->dynaMechsLink;
    Float state[ 7 ];
    Float velocity[ 6 ];
    static_cast< dmMobileBaseLink * >( root )->getState( state, velocity );
    Float spinVelocity[ 6 ] = { spin, spin, 0, 0, 0, 0 };
    static_cast< dmMobileBaseLink * >( root )->setState( state, spinVelocity );

    // The integrator copies the state of the system when it takes the system.
    simulationData.dynaMechsIntegrator->setSystem( &simulationData.dynaMechsSystem );
    for ( int step = 0; step < 25; step++ )
    {
      simulationData.simulationProgress();
    }
    orientation = simulationQueries.getLinkOrientation( 0 );
    value = reading( simulationQueries, 0, 8 );
  }

  QVERIFY2( orientation.get( 1, 1 ) < std::cos( 20 * M_PI / 180 ), qPrintable( QString( "the box is tilted by %1 degrees" ).arg( std::acos( orientation.get( 1, 1 ) ) * 180 / M_PI ) ) );
  QCOMPARE( value, tiltReading( pitch, orientation ) );
  SIG_Matrix noTurn;
  noTurn.makeone();
  const int level = tiltReading( pitch, noTurn );
  QVERIFY2( std::abs( value - level ) > 1, qPrintable( QString( "reads %1, a level box %2" ).arg( value ).arg( level ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltOfALinkBehindAJoint_data()
{
  QTest::addColumn< bool >( "pitch" );

  QTest::newRow( "pitch" ) << true;
  QTest::newRow( "roll" ) << false;
}

// Robots carry tilt sensors on limbs too. In the pose of the model both links are level, so a sensor on the arm reads what one on the base reads.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltOfALinkBehindAJoint()
{
  QFETCH( bool, pitch );
  int readings[ 2 ] = { 0, 0 };
  for ( int linkNumber = 0; linkNumber < 2; linkNumber++ )
  {
    SIGEL_Robot::SIG_Robot robot;
    addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tRotationalJoint, -120, 120, 0 );
    SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
    sensor->setLink( robot.getLinks()[ linkNumber ] );
    if ( pitch )
    {
      sensor->SetPitchType();
    }
    else
    {
      sensor->SetRollType();
    }
    robot.addSensor( sensor );
    robot.prepareDynaMechs();
    SIGEL_Environment::SIG_Environment environment;
    environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
    SIG_SimulationParameters simulationParameters;
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
    readings[ linkNumber ] = reading( simulationQueries, 0, 8 );
  }

  QVERIFY2( std::abs( readings[ 1 ] - readings[ 0 ] ) <= 1, qPrintable( QString( "the arm reads %1, the base %2" ).arg( readings[ 1 ] ).arg( readings[ 0 ] ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltOfALinkThatItsJointHasTilted_data()
{
  QTest::addColumn< bool >( "pitch" );
  QTest::addColumn< double >( "start" );

  QTest::newRow( "pitch, 30" ) << true << 30.0;
  QTest::newRow( "pitch, -30" ) << true << -30.0;
  QTest::newRow( "roll, 30" ) << false << 30.0;
  QTest::newRow( "roll, -30" ) << false << -30.0;
}

// A hip with a level axis tilts the leg behind it: along z for the pitch, along x for the roll. With the start
// value 30 or -30 the arm is turned from its pose at 0 by some turn; its tilt sensor then reads what one on a
// box turned that way reads, and not what it reads at 0.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsTheTiltOfALinkThatItsJointHasTilted()
{
  QFETCH( bool, pitch );
  QFETCH( double, start );
  SIG_Matrix armOrientation[ 2 ];
  int armReadings[ 2 ] = { 0, 0 };
  const double starts[ 2 ] = { 0, start };
  for ( int run = 0; run < 2; run++ )
  {
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Link *base = addBoxLink( robot, "base", SIG_Vector( -1, -0.5, -0.5 ), SIG_Vector( 1, 3, 0.5 ) );
    SIGEL_Robot::SIG_Link *arm = addBoxLink( robot, "arm", SIG_Vector( -1, -2, -0.25 ), SIG_Vector( 1, 0.25, 0.25 ) );
    robot.setRootLink( base );
    SIGEL_Robot::SIG_RotationalJoint *hip = new SIGEL_Robot::SIG_RotationalJoint( &robot, "hip", 0 );
    hip->setLeftLink( base );
    hip->setRightLink( arm );
    if ( pitch )
    {
      hip->setLeftPoints( SIG_Vector( 1, 0, 0 ), SIG_Vector( 1, 0, 1 ), SIG_Vector( 2, 0, 0 ) );
      hip->setRightPoints( SIG_Vector( -1, 0, 0 ), SIG_Vector( -1, 0, 1 ), SIG_Vector( 0, 0, 0 ) );
    }
    else
    {
      // Off the base's z axis, which an axis along x through the base's origin would cut.
      hip->setLeftPoints( SIG_Vector( 1, 0.5, 0 ), SIG_Vector( 2, 0.5, 0 ), SIG_Vector( 1, 0.5, 1 ) );
      hip->setRightPoints( SIG_Vector( -1, 0.5, 0 ), SIG_Vector( 0, 0.5, 0 ), SIG_Vector( -1, 0.5, 1 ) );
    }
    hip->setRange( -120, 120, starts[ run ] );
    robot.addJoint( hip );
    SIGEL_Robot::SIG_PitchRollSensor *sensor = new SIGEL_Robot::SIG_PitchRollSensor( &robot, "tilt", 0 );
    sensor->setLink( arm );
    if ( pitch )
    {
      sensor->SetPitchType();
    }
    else
    {
      sensor->SetRollType();
    }
    robot.addSensor( sensor );
    robot.prepareDynaMechs();
    SIGEL_Environment::SIG_Environment environment;
    environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
    SIG_SimulationParameters simulationParameters;
    SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
    SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
    armOrientation[ run ] = simulationQueries.getLinkOrientation( 1 );
    armReadings[ run ] = reading( simulationQueries, 0, 8 );
  }

  SIG_Matrix turn = product( armOrientation[ 1 ], transposed( armOrientation[ 0 ] ) );
  QVERIFY2( turn.get( 1, 1 ) < std::cos( 20 * M_PI / 180 ), qPrintable( QString( "the arm is tilted by %1 degrees" ).arg( std::acos( std::min( 1.0, turn.get( 1, 1 ) ) ) * 180 / M_PI ) ) );
  const int expected = tiltReading( pitch, turn );
  QVERIFY2( std::abs( armReadings[ 1 ] - expected ) <= 1, qPrintable( QString( "the arm reads %1, a box turned so %2" ).arg( armReadings[ 1 ] ).arg( expected ) ) );
  QVERIFY2( std::abs( armReadings[ 1 ] - armReadings[ 0 ] ) > 1, qPrintable( QString( "the arm reads %1 tilted and %2 at the start value 0" ).arg( armReadings[ 1 ] ).arg( armReadings[ 0 ] ) ) );
}

void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseTakesTheSensorsInTurnAsTheSensorNumberCounts_data()
{
  QTest::addColumn< int >( "registerSize" );

  QTest::newRow( "8-bit register" ) << 8;
  QTest::newRow( "16-bit register" ) << 16;
}

// Three joint sensors on sliders that read the middle, three quarters and a quarter of the register. The sensor
// number is a register value, and from the register's bottom to its top each next value takes the next sensor, round and round.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseTakesTheSensorsInTurnAsTheSensorNumberCounts()
{
  QFETCH( int, registerSize );
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Joint *first = addBaseAndArm( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, -1, 1, 0 );
  SIGEL_Robot::SIG_Link *arm = robot.getLinks()[ 1 ];
  SIGEL_Robot::SIG_Link *tip = addBoxLink( robot, "tip", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ) );
  SIGEL_Robot::SIG_Link *end = addBoxLink( robot, "end", SIG_Vector( -1, -1, -0.25 ), SIG_Vector( 1, 0, 0.25 ) );
  SIGEL_Robot::SIG_Joint *second = addJoint( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 1, arm, tip, -1, 1, 0.5 );
  SIGEL_Robot::SIG_Joint *third = addJoint( robot, SIGEL_Robot::SIG_Joint::tTranslationalJoint, 2, tip, end, -1, 1, -0.5 );
  const QList< SIGEL_Robot::SIG_Joint * > joints = { first, second, third };
  for ( int number = 0; number < 3; number++ )
  {
    SIGEL_Robot::SIG_JointSensor *sensor = new SIGEL_Robot::SIG_JointSensor( &robot, "slide" + QString::number( number ), number );
    sensor->setJoint( joints[ number ] );
    robot.addSensor( sensor );
  }
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  const int quarter = 1 << ( registerSize - 3 );
  const QList< int > sensorReadings = { 0, 2 * quarter, -2 * quarter };
  const int bottom = -( 1 << ( registerSize - 1 ) );
  const int top = ( 1 << ( registerSize - 1 ) ) - 1;
  QString mismatch;

  for ( int value = bottom; value < top && mismatch.isEmpty(); value++ )
  {
    const int now = sensorReadings.indexOf( reading( simulationQueries, value, registerSize ) );
    const int next = sensorReadings.indexOf( reading( simulationQueries, value + 1, registerSize ) );
    if ( now < 0 || next != ( now + 1 ) % 3 )
    {
      mismatch = QString( "sensor number %1 reads %2, %3 reads %4" ).arg( value ).arg( reading( simulationQueries, value, registerSize ) )
        .arg( value + 1 ).arg( reading( simulationQueries, value + 1, registerSize ) );
    }
  }

  QCOMPARE( mismatch, QString() );
}

// A robot with a joint sensor, a contact sensor and a pitch sensor, numbered 0, 1 and 2. Each sensor reads
// in it what it reads as the only sensor of the same robot, and the sensor numbers take them in turn.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseReadsEachSensorOfARobotWithSensorsOfEveryType()
{
  QList< int > alone;
  for ( int kind = 0; kind < 3; kind++ )
  {
    alone.append( sensorReadings( { kind } ).value( 0 ) );
  }
  QVERIFY2( alone[ 0 ] != alone[ 1 ] && alone[ 1 ] != alone[ 2 ] && alone[ 0 ] != alone[ 2 ],
            qPrintable( QString( "alone they read %1, %2 and %3" ).arg( alone[ 0 ] ).arg( alone[ 1 ] ).arg( alone[ 2 ] ) ) );

  const QList< int > together = sensorReadings( { 0, 1, 2 } );

  const int first = alone.indexOf( together.value( 0 ) );
  const bool inTurn = first >= 0 && together.value( 1 ) == alone[ ( first + 1 ) % 3 ] && together.value( 2 ) == alone[ ( first + 2 ) % 3 ];
  QVERIFY2( inTurn, qPrintable( QString( "together they read %1, %2 and %3; alone %4, %5 and %6" ).arg( together.value( 0 ) ).arg( together.value( 1 ) )
            .arg( together.value( 2 ) ).arg( alone[ 0 ] ).arg( alone[ 1 ] ).arg( alone[ 2 ] ) ) );
}

// With no sensor there is nothing to read.
void SIGEL_Simulation::TST_SIG_DynaMechsSimulationQueries::senseLeavesTheRegistersAloneWhenTheRobotHasNoSensor()
{
  SIGEL_Robot::SIG_Robot robot;
  robot.setRootLink( addBoxLink( robot, "box", SIG_Vector( -0.5, -0.5, -0.5 ), SIG_Vector( 0.5, 0.5, 0.5 ) ) );
  robot.prepareDynaMechs();
  SIGEL_Environment::SIG_Environment environment;
  environment.setStartPosition( SIG_Vector( 10, 10, 10 ) );
  SIG_SimulationParameters simulationParameters;
  SIG_DynaMechsSimulationData simulationData( robot, environment, simulationParameters );
  SIG_DynaMechsSimulationQueries simulationQueries( simulationData );
  QList< SIG_Register > registers( 4, SIG_Register( 8 ) );
  registers[ 0 ].loadValue( -100 );
  registers[ 1 ].loadValue( 5 );
  registers[ 2 ].loadValue( -3 );
  registers[ 3 ].loadValue( 7 );

  simulationQueries.sense( 0, registers );

  QCOMPARE( registers[ 0 ].getValue(), -100 );
  QCOMPARE( registers[ 1 ].getValue(), 5 );
  QCOMPARE( registers[ 2 ].getValue(), -3 );
  QCOMPARE( registers[ 3 ].getValue(), 7 );
}
