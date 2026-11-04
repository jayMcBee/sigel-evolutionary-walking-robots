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
#include "SIGEL_Robot/TST_SIG_Mirtich.h"

#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Mirtich.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

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

  // The tetrahedron with the corners 0, (a, 0, 0), (0, b, 0) and (0, 0, c).
  void addTetrahedron( SIGEL_Robot::SIG_Geometry &geometry, double a, double b, double c )
  {
    const SIG_Vector origin( 0, 0, 0 );
    const SIG_Vector onX( a, 0, 0 );
    const SIG_Vector onY( 0, b, 0 );
    const SIG_Vector onZ( 0, 0, c );
    addFace( geometry, { origin, onY, onX } );
    addFace( geometry, { origin, onX, onZ } );
    addFace( geometry, { origin, onZ, onY } );
    addFace( geometry, { onX, onY, onZ } );
  }

  // Empty if actual is (x, y, z), else what it is.
  QString vectorMismatch( SIG_Vector actual, double x, double y, double z )
  {
    if ( std::fabs( actual.x - x ) < epsilon && std::fabs( actual.y - y ) < epsilon && std::fabs( actual.z - z ) < epsilon )
      return QString();
    return QString( "is (%1, %2, %3), expected (%4, %5, %6)" )
      .arg( actual.x ).arg( actual.y ).arg( actual.z ).arg( x ).arg( y ).arg( z );
  }

  // Empty if actual is expected, else where it differs first.
  QString matrixMismatch( SIG_Matrix actual, double const ( &expected )[ 3 ][ 3 ] )
  {
    for ( int r = 0; r < 3; r++ )
      for ( int c = 0; c < 3; c++ )
        if ( !( std::fabs( actual.get( r, c ) - expected[ r ][ c ] ) < epsilon ) )
          return QString( "(%1,%2) is %3, expected %4" ).arg( r ).arg( c ).arg( actual.get( r, c ) ).arg( expected[ r ][ c ] );
    return QString();
  }

}

// The inertia tensor is I = integral of ( |r|^2 * 1 - r * r^T ) dm: moments of inertia on the
// diagonal, products of inertia with a minus sign off it, as DynaMechs' setInertiaParameters takes it.
void SIGEL_Robot::TST_SIG_Mirtich::aBoxCentredOnTheOriginHasMassDensityTimesVolumeAndTheBoxInertia()
{
  SIG_Geometry geometry;
  addBox( geometry, SIG_Vector( -0.5, -1, -1.5 ), SIG_Vector( 0.5, 1, 1.5 ) );
  SIG_Mirtich mirtich( &geometry, "box" );
  double mass = 0;
  SIG_Vector centreOfMass( 9, 9, 9 );
  SIG_Matrix inertia;

  mirtich.computePhysics( 2, mass, centreOfMass, inertia );

  // A box of sides 1, 2 and 3 has volume 6; Ixx = m ( 2^2 + 3^2 ) / 12 and so on.
  QVERIFY( std::fabs( mass - 12 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, 0, 0, 0 ), QString() );
  const double expected[ 3 ][ 3 ] = { { 13, 0, 0 }, { 0, 10, 0 }, { 0, 0, 5 } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

void SIGEL_Robot::TST_SIG_Mirtich::aBoxOffTheOriginHasItsCentreOfMassAtTheBoxCentre()
{
  SIG_Geometry geometry;
  addBox( geometry, SIG_Vector( 0.5, 1, 1.5 ), SIG_Vector( 1.5, 3, 4.5 ) );
  SIG_Mirtich mirtich( &geometry, "box" );
  double mass = 0;
  SIG_Vector centreOfMass( 9, 9, 9 );
  SIG_Matrix inertia;

  mirtich.computePhysics( 2, mass, centreOfMass, inertia );

  QVERIFY( std::fabs( mass - 12 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, 1, 2, 3 ), QString() );
}

// DynaMechs' setInertiaParameters takes the inertia about the origin of the link, not about the
// centre of mass: the box inertia plus m ( |d|^2 * 1 - d * d^T ) for the centre d = ( 1, 2, 3 ).
void SIGEL_Robot::TST_SIG_Mirtich::theInertiaTensorOfABoxOffTheOriginIsAboutTheOriginOfTheGeometry()
{
  SIG_Geometry geometry;
  addBox( geometry, SIG_Vector( 0.5, 1, 1.5 ), SIG_Vector( 1.5, 3, 4.5 ) );
  SIG_Mirtich mirtich( &geometry, "box" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;

  mirtich.computePhysics( 2, mass, centreOfMass, inertia );

  const double expected[ 3 ][ 3 ] = { { 169, -24, -36 }, { -24, 130, -72 }, { -36, -72, 65 } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

void SIGEL_Robot::TST_SIG_Mirtich::aTetrahedronHasItsMassCentreOfMassAndInertia_data()
{
  QTest::addColumn< double >( "a" );
  QTest::addColumn< double >( "b" );
  QTest::addColumn< double >( "c" );

  // The slanted face has the normal ( bc, ac, ab ); its largest part picks the projection plane.
  QTest::newRow( "slanted face mostly along x" ) << 1.0 << 2.0 << 3.0;
  QTest::newRow( "slanted face mostly along y" ) << 3.0 << 1.0 << 2.0;
  QTest::newRow( "slanted face mostly along z" ) << 2.0 << 3.0 << 1.0;
}

void SIGEL_Robot::TST_SIG_Mirtich::aTetrahedronHasItsMassCentreOfMassAndInertia()
{
  QFETCH( double, a );
  QFETCH( double, b );
  QFETCH( double, c );
  SIG_Geometry geometry;
  addTetrahedron( geometry, a, b, c );
  SIG_Mirtich mirtich( &geometry, "tetrahedron" );
  const double density = 1.5;
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;

  mirtich.computePhysics( density, mass, centreOfMass, inertia );

  // The integrals of x^2, xy and so on over the tetrahedron, times the density.
  const double xx = density * a * a * a * b * c / 60;
  const double yy = density * a * b * b * b * c / 60;
  const double zz = density * a * b * c * c * c / 60;
  const double xy = density * a * a * b * b * c / 120;
  const double yz = density * a * b * b * c * c / 120;
  const double zx = density * a * a * b * c * c / 120;
  QVERIFY( std::fabs( mass - density * a * b * c / 6 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, a / 4, b / 4, c / 4 ), QString() );
  const double expected[ 3 ][ 3 ] = { { yy + zz, -xy, -zx }, { -xy, zz + xx, -yz }, { -zx, -yz, xx + yy } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

// The L-shaped prism is the box [0,3]x[0,1]x[0,1] plus the box [0,1]x[1,2]x[0,1]; the
// expected values are the sums of the integrals over the two boxes.
void SIGEL_Robot::TST_SIG_Mirtich::aSolidWithFacesOfSixCornersThatAreNotConvexHasItsMassCentreOfMassAndInertia()
{
  SIG_Geometry geometry;
  addPrism( geometry,
            { SIG_Vector( 0, 0, 0 ), SIG_Vector( 3, 0, 0 ), SIG_Vector( 3, 1, 0 ),
              SIG_Vector( 1, 1, 0 ), SIG_Vector( 1, 2, 0 ), SIG_Vector( 0, 2, 0 ) },
            1 );
  SIG_Mirtich mirtich( &geometry, "L" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;

  mirtich.computePhysics( 1, mass, centreOfMass, inertia );

  QVERIFY( std::fabs( mass - 4 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, 1.25, 0.75, 0.5 ), QString() );
  const double expected[ 3 ][ 3 ] = { { 14.0 / 3, -3, -2.5 }, { -3, 32.0 / 3, -1.5 }, { -2.5, -1.5, 38.0 / 3 } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

void SIGEL_Robot::TST_SIG_Mirtich::aSecondCallWithAnotherDensityScalesMassAndInertiaAndKeepsTheCentreOfMass()
{
  SIG_Geometry geometry;
  addTetrahedron( geometry, 1, 2, 3 );
  SIG_Mirtich mirtich( &geometry, "tetrahedron" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;
  mirtich.computePhysics( 1, mass, centreOfMass, inertia );

  mirtich.computePhysics( 4, mass, centreOfMass, inertia );

  // At density 1 the tetrahedron has mass 1 and the integrals x^2 0.1, y^2 0.4, z^2 0.9, xy 0.1, yz 0.3, zx 0.15.
  QVERIFY( std::fabs( mass - 4 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, 0.25, 0.5, 0.75 ), QString() );
  const double expected[ 3 ][ 3 ] = { { 5.2, -0.4, -0.6 }, { -0.4, 4, -1.2 }, { -0.6, -1.2, 2 } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

void SIGEL_Robot::TST_SIG_Mirtich::afterInvalidateTheResultFollowsTheMovedGeometry()
{
  SIG_Geometry geometry;
  addBox( geometry, SIG_Vector( -0.5, -1, -1.5 ), SIG_Vector( 0.5, 1, 1.5 ) );
  SIG_Mirtich mirtich( &geometry, "box" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;
  mirtich.computePhysics( 2, mass, centreOfMass, inertia );
  geometry.translate( SIG_Vector( 1, 2, 3 ) );

  mirtich.invalidate();
  mirtich.computePhysics( 2, mass, centreOfMass, inertia );

  QVERIFY( std::fabs( mass - 12 ) < epsilon );
  QCOMPARE( vectorMismatch( centreOfMass, 1, 2, 3 ), QString() );
  const double expected[ 3 ][ 3 ] = { { 169, -24, -36 }, { -24, 130, -72 }, { -36, -72, 65 } };
  QCOMPARE( matrixMismatch( inertia, expected ), QString() );
}

void SIGEL_Robot::TST_SIG_Mirtich::getT0GivesTheVolume()
{
  SIG_Geometry geometry;
  addBox( geometry, SIG_Vector( 0.5, 1, 1.5 ), SIG_Vector( 1.5, 3, 4.5 ) );
  SIG_Mirtich mirtich( &geometry, "box" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;

  mirtich.computePhysics( 2, mass, centreOfMass, inertia );

  QVERIFY( std::fabs( mirtich.getT0() - 6 ) < epsilon );
}

void SIGEL_Robot::TST_SIG_Mirtich::aGeometryWithoutVolumeThrowsSIG_CannotMirtichNamingTheGeometry()
{
  SIG_Geometry geometry;
  SIG_Mirtich mirtich( &geometry, "wheel(body)" );
  double mass = 0;
  SIG_Vector centreOfMass;
  SIG_Matrix inertia;
  QString thrown;

  try {
    mirtich.computePhysics( 2, mass, centreOfMass, inertia );
  }
  catch ( const SIG_CannotMirtich &e ) {
    thrown = e.getMessage();
  }

  QVERIFY2( thrown.contains( "\"wheel(body)\"" ), qPrintable( thrown ) );
}
