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
#include "SIGEL_Robot/TST_SIG_Geometry.h"

#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

#include <QTextStream>
#include <QtTest>

#include <memory>

namespace {

  // The geometry that the unstreaming constructor reads from in; null, with what it
  // throws in thrown, if it throws.
  std::unique_ptr< SIGEL_Robot::SIG_Geometry > unstreamedGeometry( QTextStream &in, QString &thrown )
  {
    try {
      return std::make_unique< SIGEL_Robot::SIG_Geometry >( in );
    }
    catch ( const SIGEL_Tools::SIG_Exception &e ) {
      thrown = e.getMessage();
    }
    catch ( ... ) {
      thrown = "an exception that is not a SIG_Exception";
    }
    return nullptr;
  }

}

void SIGEL_Robot::TST_SIG_Geometry::aNewGeometryHasNoVerticesAndNoPolygons()
{
  SIG_Geometry geometry;

  QCOMPARE( geometry.getNumVertices(), 0 );
  QCOMPARE( geometry.getNumPolygons(), 0 );
  QVERIFY( geometry.getVertices().isEmpty() );
  QVERIFY( geometry.getPolygons().isEmpty() );
}

void SIGEL_Robot::TST_SIG_Geometry::getOrAddVertexAddsANewPointAtTheNextIndex()
{
  SIG_Geometry geometry;

  QCOMPARE( geometry.getOrAddVertex( SIG_Vector( 1, 2, 3 ) ), 0 );
  QCOMPARE( geometry.getOrAddVertex( SIG_Vector( 4, 5, 6 ) ), 1 );

  QCOMPARE( geometry.getNumVertices(), 2 );
  SIG_Vector first = geometry.getVertex( 0 );
  QCOMPARE( first.x, 1.0 );
  QCOMPARE( first.y, 2.0 );
  QCOMPARE( first.z, 3.0 );
  SIG_Vector second = geometry.getVertex( 1 );
  QCOMPARE( second.x, 4.0 );
  QCOMPARE( second.y, 5.0 );
  QCOMPARE( second.z, 6.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::getOrAddVertexGivesAKnownPointItsIndexAndAddsNothing()
{
  SIG_Geometry geometry;
  geometry.getOrAddVertex( SIG_Vector( 1, 2, 3 ) );
  geometry.getOrAddVertex( SIG_Vector( 4, 5, 6 ) );

  QCOMPARE( geometry.getOrAddVertex( SIG_Vector( 4, 5, 6 ) ), 1 );
  QCOMPARE( geometry.getOrAddVertex( SIG_Vector( 1, 2, 3 ) ), 0 );

  QCOMPARE( geometry.getNumVertices(), 2 );
}

void SIGEL_Robot::TST_SIG_Geometry::getOrAddVertexAddsAPointThatDiffersOnlyInZ()
{
  SIG_Geometry geometry;
  geometry.getOrAddVertex( SIG_Vector( 1, 2, 3 ) );

  QCOMPARE( geometry.getOrAddVertex( SIG_Vector( 1, 2, 4 ) ), 1 );

  QCOMPARE( geometry.getNumVertices(), 2 );
  QCOMPARE( geometry.getVertex( 1 ).z, 4.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::polygonsThatShareACornerShareItsVertex()
{
  SIG_Geometry geometry;

  // The geometry owns its polygons and frees them.
  SIG_Polygon *first = new SIG_Polygon( &geometry );
  first->appendVertex( SIG_Vector( 0, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1, 1, 0 ) );
  SIG_Polygon *second = new SIG_Polygon( &geometry );
  second->appendVertex( SIG_Vector( 0, 0, 0 ) );
  second->appendVertex( SIG_Vector( 1, 1, 0 ) );
  second->appendVertex( SIG_Vector( 0, 1, 0 ) );

  QCOMPARE( geometry.getNumVertices(), 4 );
  QCOMPARE( first->getVertexIndex( 0 ), 0 );
  QCOMPARE( first->getVertexIndex( 1 ), 1 );
  QCOMPARE( first->getVertexIndex( 2 ), 2 );
  QCOMPARE( second->getVertexIndex( 0 ), 0 );
  QCOMPARE( second->getVertexIndex( 1 ), 2 );
  QCOMPARE( second->getVertexIndex( 2 ), 3 );
  SIG_Vector corner = second->getVertex( 2 );
  QCOMPARE( corner.x, 0.0 );
  QCOMPARE( corner.y, 1.0 );
  QCOMPARE( corner.z, 0.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::getPolygonsListsThePolygonsInTheOrderTheyWereMade()
{
  SIG_Geometry geometry;
  SIG_Polygon *first = new SIG_Polygon( &geometry );
  SIG_Polygon *second = new SIG_Polygon( &geometry );
  SIG_Polygon *third = new SIG_Polygon( &geometry );

  QCOMPARE( geometry.getNumPolygons(), 3 );
  QCOMPARE( geometry.getPolygons().count(), 3 );
  QCOMPARE( geometry.getPolygons().value( 0 ), first );
  QCOMPARE( geometry.getPolygons().value( 1 ), second );
  QCOMPARE( geometry.getPolygons().value( 2 ), third );
}

void SIGEL_Robot::TST_SIG_Geometry::getVerticesListsTheVerticesInIndexOrder()
{
  SIG_Geometry geometry;
  geometry.getOrAddVertex( SIG_Vector( 1, 2, 3 ) );
  geometry.getOrAddVertex( SIG_Vector( 4, 5, 6 ) );
  geometry.getOrAddVertex( SIG_Vector( 7, 8, 9 ) );

  const QList< SIG_Vector * > &vertices = geometry.getVertices();

  QCOMPARE( vertices.count(), 3 );
  QCOMPARE( vertices.value( 0 )->x, 1.0 );
  QCOMPARE( vertices.value( 0 )->y, 2.0 );
  QCOMPARE( vertices.value( 0 )->z, 3.0 );
  QCOMPARE( vertices.value( 1 )->x, 4.0 );
  QCOMPARE( vertices.value( 1 )->y, 5.0 );
  QCOMPARE( vertices.value( 1 )->z, 6.0 );
  QCOMPARE( vertices.value( 2 )->x, 7.0 );
  QCOMPARE( vertices.value( 2 )->y, 8.0 );
  QCOMPARE( vertices.value( 2 )->z, 9.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::translateMovesEveryVertexByTheDirection()
{
  SIG_Geometry geometry;
  SIG_Polygon *polygon = new SIG_Polygon( &geometry );
  polygon->appendVertex( SIG_Vector( 0, 0, 0 ) );
  polygon->appendVertex( SIG_Vector( 1, -1, 2 ) );

  geometry.translate( SIG_Vector( 1, 2, 3 ) );

  SIG_Vector first = geometry.getVertex( 0 );
  QCOMPARE( first.x, 1.0 );
  QCOMPARE( first.y, 2.0 );
  QCOMPARE( first.z, 3.0 );
  SIG_Vector second = polygon->getVertex( 1 );
  QCOMPARE( second.x, 2.0 );
  QCOMPARE( second.y, 1.0 );
  QCOMPARE( second.z, 5.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::rotateMultipliesEveryVertexByTheMatrix()
{
  SIG_Geometry geometry;
  SIG_Polygon *polygon = new SIG_Polygon( &geometry );
  polygon->appendVertex( SIG_Vector( 1, 2, 3 ) );
  polygon->appendVertex( SIG_Vector( 0, 1, 0 ) );

  // Not symmetric, so that the matrix and its transpose give different points.
  SIG_Matrix matrix;
  matrix.set( 0, 0, 1 );
  matrix.set( 0, 1, 2 );
  matrix.set( 0, 2, 3 );
  matrix.set( 1, 0, 4 );
  matrix.set( 1, 1, 5 );
  matrix.set( 1, 2, 6 );
  matrix.set( 2, 0, 7 );
  matrix.set( 2, 1, 8 );
  matrix.set( 2, 2, 9 );

  geometry.rotate( matrix );

  SIG_Vector first = geometry.getVertex( 0 );
  QCOMPARE( first.x, 14.0 );
  QCOMPARE( first.y, 32.0 );
  QCOMPARE( first.z, 50.0 );
  SIG_Vector second = polygon->getVertex( 1 );
  QCOMPARE( second.x, 2.0 );
  QCOMPARE( second.y, 5.0 );
  QCOMPARE( second.z, 8.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::copyConstructorCopiesTheVerticesAndThePolygons()
{
  SIG_Geometry original;
  SIG_Polygon *first = new SIG_Polygon( &original );
  first->appendVertex( SIG_Vector( 0, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1, 1, 2 ) );
  SIG_Polygon *second = new SIG_Polygon( &original );
  second->appendVertex( SIG_Vector( 1, 1, 2 ) );
  second->appendVertex( SIG_Vector( 0, 1, 0 ) );
  second->appendVertex( SIG_Vector( 0, 0, 0 ) );

  SIG_Geometry copy( &original );

  QCOMPARE( copy.getNumVertices(), 4 );
  for ( int i = 0; i < 4; i++ ) {
    QVERIFY( copy.getVertices().value( i ) != original.getVertices().value( i ) );
    SIG_Vector copied = copy.getVertex( i );
    SIG_Vector kept = original.getVertex( i );
    QVERIFY( copied.equal( &kept ) );
  }
  QCOMPARE( copy.getNumPolygons(), 2 );
  QCOMPARE( copy.getPolygons().count(), 2 );
  const SIG_Polygon *copiedFirst = copy.getPolygons().value( 0 );
  const SIG_Polygon *copiedSecond = copy.getPolygons().value( 1 );
  QVERIFY( copiedFirst != first );
  QVERIFY( copiedSecond != second );
  QCOMPARE( copiedFirst->getNumVertices(), 3 );
  QCOMPARE( copiedFirst->getVertexIndex( 0 ), 0 );
  QCOMPARE( copiedFirst->getVertexIndex( 1 ), 1 );
  QCOMPARE( copiedFirst->getVertexIndex( 2 ), 2 );
  QCOMPARE( copiedSecond->getNumVertices(), 3 );
  QCOMPARE( copiedSecond->getVertexIndex( 0 ), 2 );
  QCOMPARE( copiedSecond->getVertexIndex( 1 ), 3 );
  QCOMPARE( copiedSecond->getVertexIndex( 2 ), 0 );
}

void SIGEL_Robot::TST_SIG_Geometry::aCopyAndItsOriginalChangeIndependently()
{
  SIG_Geometry original;
  SIG_Polygon *polygon = new SIG_Polygon( &original );
  polygon->appendVertex( SIG_Vector( 1, 2, 3 ) );
  SIG_Geometry copy( &original );

  copy.translate( SIG_Vector( 10, 10, 10 ) );
  copy.getOrAddVertex( SIG_Vector( 7, 8, 9 ) );
  new SIG_Polygon( &copy );

  QCOMPARE( original.getNumVertices(), 1 );
  QCOMPARE( original.getNumPolygons(), 1 );
  SIG_Vector unmoved = polygon->getVertex( 0 );
  QCOMPARE( unmoved.x, 1.0 );
  QCOMPARE( unmoved.y, 2.0 );
  QCOMPARE( unmoved.z, 3.0 );

  // The copied polygon reads its point from the copy.
  QCOMPARE( copy.getPolygons().count(), 2 );
  SIG_Vector moved = copy.getPolygons().value( 0 )->getVertex( 0 );
  QCOMPARE( moved.x, 11.0 );
  QCOMPARE( moved.y, 12.0 );
  QCOMPARE( moved.z, 13.0 );
}

void SIGEL_Robot::TST_SIG_Geometry::writeToFileTransferWritesTheVerticesAndThenThePolygons()
{
  SIG_Geometry geometry;
  SIG_Polygon *first = new SIG_Polygon( &geometry );
  first->appendVertex( SIG_Vector( 0, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1.5, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1.5, 2, -3 ) );
  SIG_Polygon *second = new SIG_Polygon( &geometry );
  second->appendVertex( SIG_Vector( 1.5, 2, -3 ) );
  second->appendVertex( SIG_Vector( 0, 0, 0 ) );
  second->appendVertex( SIG_Vector( 0, 2, 0 ) );
  second->appendVertex( SIG_Vector( 1.5, 0, 0 ) );
  QString text;
  QTextStream stream( &text );

  geometry.writeToFileTransfer( stream );
  stream.flush();

  // The keyword Geometry, the number of vertices, each vertex, the number of polygons, then one line per polygon.
  QCOMPARE( text, QString( "Geometry 4 0 0 0 1.5 0 0 1.5 2 -3 0 2 0 2\nPolygon 3 0 1 2\nPolygon 4 2 0 3 1\n" ) );
}

void SIGEL_Robot::TST_SIG_Geometry::writeToFileTransferOfAnEmptyGeometryWritesTwoZeroCounts()
{
  SIG_Geometry geometry;
  QString text;
  QTextStream stream( &text );

  geometry.writeToFileTransfer( stream );
  stream.flush();

  QCOMPARE( text, QString( "Geometry 0 0\n" ) );
}

void SIGEL_Robot::TST_SIG_Geometry::unstreamingConstructorReadsWhatWriteToFileTransferWroteAndStopsAfterIt()
{
  SIG_Geometry source;
  SIG_Polygon *first = new SIG_Polygon( &source );
  first->appendVertex( SIG_Vector( 0, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1.5, 0, 0 ) );
  first->appendVertex( SIG_Vector( 1.5, 2, -3 ) );
  SIG_Polygon *second = new SIG_Polygon( &source );
  second->appendVertex( SIG_Vector( 1.5, 2, -3 ) );
  second->appendVertex( SIG_Vector( 0, 2, 0 ) );
  second->appendVertex( SIG_Vector( 0, 0, 0 ) );
  QString text;
  QTextStream out( &text );
  source.writeToFileTransfer( out );
  out << "Link";
  out.flush();
  QTextStream in( &text );
  QString thrown;

  std::unique_ptr< SIG_Geometry > read = unstreamedGeometry( in, thrown );
  QString next;
  in >> next;

  // Moved, so that a read polygon that takes its points from the source gives wrong ones.
  source.translate( SIG_Vector( 10, 10, 10 ) );

  QCOMPARE( thrown, QString() );
  QCOMPARE( next, QString( "Link" ) );
  QCOMPARE( read->getNumVertices(), 4 );
  SIG_Vector corner = read->getVertex( 2 );
  QCOMPARE( corner.x, 1.5 );
  QCOMPARE( corner.y, 2.0 );
  QCOMPARE( corner.z, -3.0 );
  QCOMPARE( read->getNumPolygons(), 2 );
  QCOMPARE( read->getPolygons().count(), 2 );
  const SIG_Polygon *readSecond = read->getPolygons().value( 1 );
  QCOMPARE( readSecond->getNumVertices(), 3 );
  QCOMPARE( readSecond->getVertexIndex( 0 ), 2 );
  QCOMPARE( readSecond->getVertexIndex( 1 ), 3 );
  QCOMPARE( readSecond->getVertexIndex( 2 ), 0 );
  SIG_Vector readCorner = readSecond->getVertex( 1 );
  QCOMPARE( readCorner.x, 0.0 );
  QCOMPARE( readCorner.y, 2.0 );
  QCOMPARE( readCorner.z, 0.0 );

  QString written;
  QTextStream stream( &written );
  read->writeToFileTransfer( stream );
  stream.flush();
  QCOMPARE( written + "Link", text );
}

void SIGEL_Robot::TST_SIG_Geometry::unstreamingConstructorThrowsWhenTheRecordIsNotAGeometry()
{
  QString text = "Body 0 0\n";
  QTextStream stream( &text, QIODevice::ReadOnly );

  QVERIFY_THROWS_EXCEPTION( SIG_UnstreamingError, SIG_Geometry geometry( stream ) );
}
