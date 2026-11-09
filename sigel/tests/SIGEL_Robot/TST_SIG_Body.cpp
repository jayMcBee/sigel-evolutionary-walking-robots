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
#include "SIGEL_Robot/TST_SIG_Body.h"

#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "TST_FreeWatch.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

#include <cmath>
#include <memory>
#include <type_traits>

namespace
{

  // A file whose only shape a test reads may load as an empty geometry for want of faces, so
  // such a test puts a cornerShape( 5, 5, 5 ) in front of it: this is its area.
  const double besideTheShapeArea = 0.5;

  const double epsilon = 1e-5;
  const char *const quarterTurnAboutZ = "0 0 1 1.5707963";

}

QString SIGEL_Robot::TST_SIG_Body::faceSetShape( const QString &points, const QString &indices )
{
  return "Shape { geometry IndexedFaceSet { coord Coordinate { point [ " + points + " ] }\n"
         "                                  coordIndex [ " + indices + " ] } }\n";
}

QString SIGEL_Robot::TST_SIG_Body::triangleShape()
{
  return faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 1, 2, -1" );
}

// A triangle whose first corner is ( x, y, z ), so the transforms can be read off
// that vertex. The other corners are one unit along x and along y from it.
QString SIGEL_Robot::TST_SIG_Body::cornerShape( double x, double y, double z )
{
  const QString points = QString( "%1 %2 %3, %4 %2 %3, %1 %5 %3" ).arg( x ).arg( y ).arg( z ).arg( x + 1 ).arg( y + 1 );
  return faceSetShape( points, "0, 1, 2, -1" );
}

QString SIGEL_Robot::TST_SIG_Body::transformed( const QString &fields, const QString &children )
{
  return "Transform { " + fields + " children [ " + children + " ] }\n";
}

// Writes contents to name in folder and returns that file's path.
QString SIGEL_Robot::TST_SIG_Body::writeFile( const QTemporaryDir &folder, const QString &contents, const QString &name )
{
  QString fileName = folder.filePath( name );
  QFile file( fileName );
  if ( !folder.isValid() || !file.open( QIODevice::WriteOnly ) )
  {
    QTest::qFail( qPrintable( "cannot write " + fileName ), __FILE__, __LINE__ );
  }
  else
  {
    file.write( contents.toUtf8() );
  }
  return fileName;
}

QString SIGEL_Robot::TST_SIG_Body::writeVRMLFile( const QTemporaryDir &folder, const QString &body, const QString &name )
{
  return writeFile( folder, "#VRML V2.0 utf8\n" + body, name );
}

// What load throws, empty if it throws nothing. Catching everything here keeps
// an exception from escaping the test function, which ends the whole test run.
QString SIGEL_Robot::TST_SIG_Body::thrownByLoad( SIGEL_Robot::SIG_Body &body )
{
  try
  {
    body.load();
  }
  catch ( const SIGEL_Tools::SIG_Exception &e )
  {
    return e.getMessage();
  }
  catch ( ... )
  {
    return "an exception that is not a SIG_Exception";
  }
  return QString();
}

// The body that the unstreaming constructor reads from in; null, with what it
// throws in thrown, if it throws.
std::unique_ptr< SIGEL_Robot::SIG_Body > SIGEL_Robot::TST_SIG_Body::unstreamedBody( SIGEL_Robot::SIG_Robot &robot, QTextStream &in, QString &thrown )
{
  try
  {
    return std::make_unique< SIGEL_Robot::SIG_Body >( &robot, in );
  }
  catch ( const SIGEL_Tools::SIG_Exception &e )
  {
    thrown = e.getMessage();
  }
  catch ( ... )
  {
    thrown = "an exception that is not a SIG_Exception";
  }
  return nullptr;
}

// VRML 97 takes a comma as white space, so the USE is a copy of S beside the Transform, at the
// place of S. CyberVRML97 reads it into the Transform, which would move it by 5 along x.
QString SIGEL_Robot::TST_SIG_Body::useDirectlyAfterTransformEnd()
{
  return "DEF S " + triangleShape() + "Transform { translation 5 0 0 } USE S\n";
}

bool SIGEL_Robot::TST_SIG_Body::isNear( SIG_Vector v, double x, double y, double z )
{
  return std::fabs( v.get( 0 ) - x ) < epsilon
    && std::fabs( v.get( 1 ) - y ) < epsilon
    && std::fabs( v.get( 2 ) - z ) < epsilon;
}

QString SIGEL_Robot::TST_SIG_Body::toString( SIG_Vector v )
{
  return QString( "(%1, %2, %3)" ).arg( v.get( 0 ) ).arg( v.get( 1 ) ).arg( v.get( 2 ) );
}

QString SIGEL_Robot::TST_SIG_Body::toString( const QList< SIG_Vector > &vertices )
{
  QStringList texts;
  for ( const SIG_Vector &v : vertices )
  {
    texts.append( toString( v ) );
  }
  return "[" + texts.join( ", " ) + "]";
}

// Empty for a null geometry, so that a missing one fails the comparison instead of crashing.
QList< SIG_Vector > SIGEL_Robot::TST_SIG_Body::verticesOf( const SIGEL_Robot::SIG_Geometry *geometry )
{
  QList< SIG_Vector > vertices;
  for ( int i = 0; geometry && i < geometry->getNumVertices(); i++ )
  {
    vertices.append( geometry->getVertex( i ) );
  }
  return vertices;
}

// The corners of each face of the geometry that a body loads from the VRML file body, in the
// order of each face. VRML 97 fixes the order of a face's corners, not that of the body's vertices.
QList< QList< SIG_Vector > > SIGEL_Robot::TST_SIG_Body::loadedFaces( const QString &body )
{
  QTemporaryDir folder;
  SIGEL_Robot::SIG_Robot robot;
  SIGEL_Robot::SIG_Body loaded( &robot, writeVRMLFile( folder, body ), "" );
  const QString thrown = thrownByLoad( loaded );
  if ( !thrown.isEmpty() )
  {
    QTest::qFail( qPrintable( "load threw: " + thrown ), __FILE__, __LINE__ );
    return {};
  }
  if ( !loaded.getGeometry() )
  {
    QTest::qFail( "load left the body without geometry", __FILE__, __LINE__ );
    return {};
  }
  QList< QList< SIG_Vector > > faces;
  for ( const SIGEL_Robot::SIG_Polygon *polygon : loaded.getGeometry()->getPolygons() )
  {
    QList< SIG_Vector > corners;
    for ( int i = 0; i < polygon->getNumVertices(); i++ )
    {
      corners.append( polygon->getVertex( i ) );
    }
    faces.append( corners );
  }
  return faces;
}

QString SIGEL_Robot::TST_SIG_Body::toString( const QList< QList< SIG_Vector > > &faces )
{
  QStringList texts;
  for ( const QList< SIG_Vector > &corners : faces )
  {
    texts.append( toString( corners ) );
  }
  return "[" + texts.join( ", " ) + "]";
}

// For the faces of one cornerShape.
bool SIGEL_Robot::TST_SIG_Body::isTheFirstCornerOfTheOnlyFaceNear( const QList< QList< SIG_Vector > > &faces, double x, double y, double z )
{
  return faces.size() == 1 && faces.first().size() == 3 && isNear( faces.first().first(), x, y, z );
}

// Whether one of the faces has its first corner at ( x, y, z ).
bool SIGEL_Robot::TST_SIG_Body::hasAFaceThatStartsNear( const QList< QList< SIG_Vector > > &faces, double x, double y, double z )
{
  for ( const QList< SIG_Vector > &corners : faces )
  {
    if ( !corners.isEmpty() && isNear( corners.first(), x, y, z ) )
    {
      return true;
    }
  }
  return false;
}

// The vertices without those of the cornerShape( 5, 5, 5 ) that a test puts beside its shape.
QList< SIG_Vector > SIGEL_Robot::TST_SIG_Body::withoutTheBesideShape( const QList< SIG_Vector > &vertices )
{
  QList< SIG_Vector > kept;
  for ( const SIG_Vector &vertex : vertices )
  {
    if ( !isNear( vertex, 5, 5, 5 ) && !isNear( vertex, 6, 5, 5 ) && !isNear( vertex, 5, 6, 5 ) )
    {
      kept.append( vertex );
    }
  }
  return kept;
}

// Whether v lies on the surface of the Cylinder of loadMakesFacesOnTheSurfaceOfACurvedShape.
bool SIGEL_Robot::TST_SIG_Body::isOnTheSurface( SIG_Vector v )
{
  const double fromTheAxis = std::sqrt( v.x * v.x + v.z * v.z );
  const bool onTheSide = std::fabs( fromTheAxis - 1 ) < epsilon && std::fabs( v.y ) < 1 + epsilon;
  const bool onACap = std::fabs( std::fabs( v.y ) - 1 ) < epsilon && fromTheAxis < 1 + epsilon;
  return onTheSide || onACap;
}

// The sum of the areas of the faces, each a flat polygon.
double SIGEL_Robot::TST_SIG_Body::surfaceArea( const SIGEL_Robot::SIG_Geometry *geometry )
{
  double area = 0;
  for ( const SIGEL_Robot::SIG_Polygon *polygon : geometry->getPolygons() )
  {
    double x = 0;
    double y = 0;
    double z = 0;
    for ( int i = 0; i < polygon->getNumVertices(); i++ )
    {
      SIG_Vector a = polygon->getVertex( i );
      SIG_Vector b = polygon->getVertex( ( i + 1 ) % polygon->getNumVertices() );
      x += a.y * b.z - a.z * b.y;
      y += a.z * b.x - a.x * b.z;
      z += a.x * b.y - a.y * b.x;
    }
    area += std::sqrt( x * x + y * y + z * z ) / 2;
  }
  return area;
}

// The corners of each face, as points. VRML 97 does not fix in which order a body keeps its
// vertices, so the load tests compare points, not vertex indices.
QList< QStringList > SIGEL_Robot::TST_SIG_Body::facePoints( const SIGEL_Robot::SIG_Geometry *geometry )
{
  QList< QStringList > faces;
  for ( const SIGEL_Robot::SIG_Polygon *polygon : geometry->getPolygons() )
  {
    QStringList face;
    for ( int i = 0; i < polygon->getNumVertices(); i++ )
    {
      face.append( toString( polygon->getVertex( i ) ) );
    }
    faces.append( face );
  }
  return faces;
}

// The faces as indices into the geometry's vertices, as the transfer format writes them.
QList< QList< int > > SIGEL_Robot::TST_SIG_Body::faceIndices( const SIGEL_Robot::SIG_Geometry *geometry )
{
  QList< QList< int > > faces;
  for ( const SIGEL_Robot::SIG_Polygon *polygon : geometry->getPolygons() )
  {
    QList< int > face;
    for ( int i = 0; i < polygon->getNumVertices(); i++ )
    {
      face.append( polygon->getVertexIndex( i ) );
    }
    faces.append( face );
  }
  return faces;
}

void SIGEL_Robot::TST_SIG_Body::constructorStoresTheNameAndLeavesTheBodyWithoutGeometry()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );

  QCOMPARE( body.getName(), QString( "arm.wrl" ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::bodyCannotBeCopyConstructed()
{
  // The body owns its geometry, so a copy would delete it a second time.
  QVERIFY( !std::is_copy_constructible_v< SIG_Body > );
}

void SIGEL_Robot::TST_SIG_Body::bodyCannotBeCopyAssigned()
{
  // The body owns its geometry, so an assignment would leak the target's and
  // delete the source's a second time.
  QVERIFY( !std::is_copy_assignable_v< SIG_Body > );
}

// The robot builder gives a directory that ends with "/".
void SIGEL_Robot::TST_SIG_Body::getGeometryFileGivesTheRelativeNameInTheDirectory()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );

  QCOMPARE( body.getGeometryFile(), QString( "parts/arm.wrl" ) );
}

void SIGEL_Robot::TST_SIG_Body::getGeometryFileReturnsAnAbsoluteNameWithoutTheDirectory()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "/models/arm.wrl", "parts/" );

  QCOMPARE( body.getGeometryFile(), QString( "/models/arm.wrl" ) );
}

void SIGEL_Robot::TST_SIG_Body::setGeometryGivesTheBodyThatGeometry()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );

  // The body owns the geometry from here on and frees it.
  SIG_Geometry *geometry = new SIG_Geometry();

  body.setGeometry( geometry );

  QCOMPARE( body.getGeometry(), geometry );
}

void SIGEL_Robot::TST_SIG_Body::setGeometryFreesTheGeometryItReplaces()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  SIG_Geometry *first = new SIG_Geometry();
  SIG_Geometry *second = new SIG_Geometry();
  body.setGeometry( first );
  bool freed;

  {
    SIGEL_Tools::TST_FreeWatch watch( first );
    body.setGeometry( second );
    freed = watch.freed();
  }

  // Left to the test only if the body did not free it, so a failure leaks nothing.
  if ( !freed )
  {
    delete first;
  }

  QVERIFY( freed );
  QCOMPARE( body.getGeometry(), second );
}

void SIGEL_Robot::TST_SIG_Body::destructorFreesTheGeometry()
{
  SIG_Robot robot;
  SIG_Geometry *geometry = new SIG_Geometry();
  bool freed;

  {
    SIGEL_Tools::TST_FreeWatch watch( geometry );
    {
      SIG_Body body( &robot, "arm.wrl", "parts/" );
      body.setGeometry( geometry );
    }
    freed = watch.freed();
  }

  // Left to the test only if the body did not free it, so a failure leaks nothing.
  if ( !freed )
  {
    delete geometry;
  }

  QVERIFY( freed );
}

void SIGEL_Robot::TST_SIG_Body::setGeometryOfTheGeometryItHoldsKeepsIt()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  SIG_Geometry *geometry = new SIG_Geometry();
  new SIG_Polygon( geometry );
  body.setGeometry( geometry );
  bool freed;

  {
    SIGEL_Tools::TST_FreeWatch watch( geometry );
    body.setGeometry( geometry );
    freed = watch.freed();
  }

  QVERIFY( !freed );
  QCOMPARE( body.getGeometry(), geometry );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::writeToFileTransferWritesNForABodyWithoutGeometry()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  QString text;
  QTextStream stream( &text );

  body.writeToFileTransfer( stream );
  stream.flush();

  QCOMPARE( text, QString( "Body arm.wrl parts/ n\n" ) );
}

void SIGEL_Robot::TST_SIG_Body::writeToFileTransferWritesYAndThenTheGeometry()
{
  QTemporaryDir folder;
  SIG_Robot robot;
  SIG_Body body( &robot, writeVRMLFile( folder, triangleShape() ), "parts/" );
  QCOMPARE( thrownByLoad( body ), QString() );
  QVERIFY( body.getGeometry() );
  QString text;
  QTextStream stream( &text );
  QString geometryText;
  QTextStream geometryStream( &geometryText );

  body.writeToFileTransfer( stream );
  stream.flush();
  body.getGeometry()->writeToFileTransfer( geometryStream );
  geometryStream.flush();

  QVERIFY( !geometryText.isEmpty() );
  QCOMPARE( text, "Body " + body.getName() + " parts/ y\n" + geometryText );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsWhatWriteToFileTransferWroteAndStopsAfterIt()
{
  QTemporaryDir folder;
  SIG_Robot robot;
  SIG_Body source( &robot, writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 1 1 0, 0.5 2 -3", "0, 1, 2, -1, 2, 3, 0" ) ), "" );
  QCOMPARE( thrownByLoad( source ), QString() );
  QVERIFY( source.getGeometry() );

  // A fixed directory, so the expected path does not depend on the temporary folder.
  SIG_Body written( &robot, "arm.wrl", "parts/" );
  written.setGeometry( new SIG_Geometry( source.getGeometry() ) );
  QString text;
  QTextStream out( &text );
  written.writeToFileTransfer( out );
  out << "Material";
  out.flush();
  QTextStream in( &text );
  QString keyword;
  in >> keyword;

  QString thrown;

  std::unique_ptr< SIG_Body > read = unstreamedBody( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( keyword, QString( "Body" ) );
  QCOMPARE( read->getName(), QString( "arm.wrl" ) );
  QCOMPARE( read->getGeometryFile(), QString( "parts/arm.wrl" ) );
  QVERIFY( read->getGeometry() );
  QCOMPARE( read->getGeometry()->getNumVertices(), 4 );
  for ( int i = 0; i < 4; i++ )
  {
    QVERIFY2( isNear( read->getGeometry()->getVertex( i ),
                      source.getGeometry()->getVertex( i ).get( 0 ),
                      source.getGeometry()->getVertex( i ).get( 1 ),
                      source.getGeometry()->getVertex( i ).get( 2 ) ),
              qPrintable( toString( read->getGeometry()->getVertex( i ) ) ) );
  }
  QCOMPARE( faceIndices( read->getGeometry() ), faceIndices( source.getGeometry() ) );
  QCOMPARE( next, QString( "Material" ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsABodyWithoutGeometryAndStopsAfterIt()
{
  SIG_Robot robot;
  QString text( "arm.wrl parts/ n\nMaterial" );
  QTextStream in( &text );

  QString thrown;

  std::unique_ptr< SIG_Body > body = unstreamedBody( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( body->getName(), QString( "arm.wrl" ) );
  QCOMPARE( body->getGeometryFile(), QString( "parts/arm.wrl" ) );
  QCOMPARE( body->getGeometry(), nullptr );
  QCOMPARE( next, QString( "Material" ) );
}

// writeToFileTransfer writes y or n.
void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorThrowsForAFlagOtherThanYOrN()
{
  SIG_Robot robot;
  QString text( "arm.wrl parts/ Y\nMaterial" );
  QTextStream in( &text );

  QString thrown;

  unstreamedBody( robot, in, thrown );

  QEXPECT_FAIL( "", "The SIG_Body unstreaming constructor reads every flag other than y as a body without geometry", Abort );
  QVERIFY2( thrown.contains( "Unstreaming error" ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorThrowsForAYFollowedByAMalformedGeometry()
{
  SIG_Robot robot;
  QString text( "arm.wrl parts/ y\nMaterial" );
  QTextStream in( &text );

  QString thrown;

  unstreamedBody( robot, in, thrown );

  QVERIFY2( thrown.contains( "Unstreaming error" ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorThrowsForAnEndedStream()
{
  SIG_Robot robot;
  QString text;
  QTextStream in( &text );

  QString thrown;

  unstreamedBody( robot, in, thrown );

  QEXPECT_FAIL( "", "The SIG_Body unstreaming constructor reads an ended stream as a body without name, directory and geometry", Abort );
  QVERIFY2( thrown.contains( "Unstreaming error" ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::writeToFileTransferQuotesADirectoryWithWhitespace()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "my parts\t2/" );
  QString text;
  QTextStream stream( &text );

  body.writeToFileTransfer( stream );
  stream.flush();

  QCOMPARE( text, QString( "Body arm.wrl \"my parts\t2/\" n\n" ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsADirectoryWithWhitespaceAndStopsAfterTheBody()
{
  SIG_Robot robot;
  SIG_Geometry *geometry = new SIG_Geometry();
  SIG_Polygon *polygon = new SIG_Polygon( geometry );
  polygon->appendVertex( SIG_Vector( 0, 0, 0 ) );
  polygon->appendVertex( SIG_Vector( 1, 0, 0 ) );
  polygon->appendVertex( SIG_Vector( 0, 1, 0 ) );
  SIG_Body written( &robot, "arm.wrl", "my parts\t2/" );
  written.setGeometry( geometry );
  QString text;
  QTextStream out( &text );
  written.writeToFileTransfer( out );
  out << "Material";
  out.flush();
  QTextStream in( &text );
  QString keyword;
  in >> keyword;

  QString thrown;

  std::unique_ptr< SIG_Body > read = unstreamedBody( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( read->getName(), QString( "arm.wrl" ) );
  QCOMPARE( read->getGeometryFile(), QString( "my parts\t2/arm.wrl" ) );
  QVERIFY( read->getGeometry() );
  QCOMPARE( read->getGeometry()->getNumPolygons(), 1 );
  QCOMPARE( read->getGeometry()->getNumVertices(), 3 );
  QCOMPARE( next, QString( "Material" ) );
}

void SIGEL_Robot::TST_SIG_Body::writeToFileTransferQuotesAGeometryFileNameWithWhitespace()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "left\tarm.wrl", "parts/" );
  QString text;
  QTextStream stream( &text );

  body.writeToFileTransfer( stream );
  stream.flush();

  QCOMPARE( text, QString( "Body \"left\tarm.wrl\" parts/ n\n" ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsAGeometryFileNameWithWhitespaceAndStopsAfterTheBody()
{
  SIG_Robot robot;
  SIG_Geometry *geometry = new SIG_Geometry();
  SIG_Polygon *polygon = new SIG_Polygon( geometry );
  polygon->appendVertex( SIG_Vector( 0, 0, 0 ) );
  polygon->appendVertex( SIG_Vector( 1, 0, 0 ) );
  polygon->appendVertex( SIG_Vector( 0, 1, 0 ) );
  SIG_Body written( &robot, "left\tarm.wrl", "parts/" );
  written.setGeometry( geometry );
  QString text;
  QTextStream out( &text );
  written.writeToFileTransfer( out );
  out << "Material";
  out.flush();
  QTextStream in( &text );
  QString keyword;
  in >> keyword;
  QString thrown;

  std::unique_ptr< SIG_Body > read = unstreamedBody( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( read->getName(), QString( "left\tarm.wrl" ) );
  QCOMPARE( read->getGeometryFile(), QString( "parts/left\tarm.wrl" ) );
  QVERIFY( read->getGeometry() );
  QCOMPARE( read->getGeometry()->getNumPolygons(), 1 );
  QCOMPARE( read->getGeometry()->getNumVertices(), 3 );
  QCOMPARE( next, QString( "Material" ) );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheVerticesAndFacesOfAnIndexedFaceSet()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0.5 -1 2, 3 0 0, 3 4 0, 0 4 0", "0, 1, 2, -1, 0, 2, 3, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  const SIG_Geometry *geometry = body.getGeometry();
  QVERIFY( geometry );
  QCOMPARE( geometry->getNumVertices(), 4 );
  QCOMPARE( facePoints( geometry ), QList< QStringList >( { { "(0.5, -1, 2)", "(3, 0, 0)", "(3, 4, 0)" },
                                                            { "(0.5, -1, 2)", "(3, 4, 0)", "(0, 4, 0)" } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsFacesOfMoreThanThreeVertices()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 1 1 0, 0 1 0, 0 0 1, 1 0 1", "0, 1, 2, 3, -1, 0, 1, 5, 4, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( facePoints( body.getGeometry() ), QList< QStringList >( { { "(0, 0, 0)", "(1, 0, 0)", "(1, 1, 0)", "(0, 1, 0)" },
                                                                     { "(0, 0, 0)", "(1, 0, 0)", "(1, 0, 1)", "(0, 0, 1)" } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadBuildsTheFacesFromTheCoordIndexOfAShapeWithAppearanceNormalsAndTextureCoordinates()
{
  QTemporaryDir folder;

  // The normalIndex and texCoordIndex differ from the coordIndex, so faces built from them show.
  QString fileName = writeVRMLFile( folder,
                                    "Shape { appearance Appearance { material Material { diffuseColor 0.8 0.2 0.2 } }\n"
                                    "        geometry IndexedFaceSet { solid FALSE ccw TRUE creaseAngle 0.5\n"
                                    "                                  coord Coordinate { point [ 0 0 0, 1 0 0, 1 1 0, 0 1 0 ] }\n"
                                    "                                  coordIndex [ 0, 1, 2, -1, 0, 2, 3, -1 ]\n"
                                    "                                  normal Normal { vector [ 0 0 1, 0 0 -1 ] }\n"
                                    "                                  normalIndex [ 1, 1, 1, -1, 0, 0, 0, -1 ]\n"
                                    "                                  texCoord TextureCoordinate { point [ 0 0, 1 0, 1 1, 0 1 ] }\n"
                                    "                                  texCoordIndex [ 3, 2, 1, -1, 3, 1, 0, -1 ] } }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  const SIG_Geometry *geometry = body.getGeometry();
  QVERIFY( geometry );
  QCOMPARE( facePoints( geometry ), QList< QStringList >( { { "(0, 0, 0)", "(1, 0, 0)", "(1, 1, 0)" },
                                                            { "(0, 0, 0)", "(1, 1, 0)", "(0, 1, 0)" } } ) );
}

// SIG_DynaMechsLink makes each vertex of a body a contact point with the ground, so a point that two
// faces share must be one vertex; as two, it would press on the ground twice.
void SIGEL_Robot::TST_SIG_Body::loadMergesEqualPointsIntoOneVertex()
{
  QTemporaryDir folder;

  // Point 4 equals point 1.
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0, 5 5 5, 1 0 0", "2, 0, 1, -1, 4, 2, 0, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  const SIG_Geometry *geometry = body.getGeometry();
  QVERIFY( geometry );
  QCOMPARE( facePoints( geometry ), QList< QStringList >( { { "(0, 1, 0)", "(0, 0, 0)", "(1, 0, 0)" },
                                                            { "(1, 0, 0)", "(0, 1, 0)", "(0, 0, 0)" } } ) );
  const QList< QList< int > > faces = faceIndices( geometry );
  QCOMPARE( faces[ 1 ][ 0 ], faces[ 0 ][ 2 ] );
  QCOMPARE( faces[ 1 ][ 1 ], faces[ 0 ][ 0 ] );
  QCOMPARE( faces[ 1 ][ 2 ], faces[ 0 ][ 1 ] );
}

// VRML 97, IndexedFaceSet: -1 ends a face; every other index names a point of the coord field.
void SIGEL_Robot::TST_SIG_Body::loadRefusesANegativeIndexOtherThanMinusOne()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 1 1 0, 0 1 0", "0, 1, 2, -2, 2, 3, 0, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::readVRMLNode takes every negative coordIndex as the end of a face and loads the file", Abort );
  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\"" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAnIndexBeyondTheCoordinates()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 3, 1, 2, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::readVRMLNode skips a coordIndex beyond the coordinates and loads the file", Abort );
  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\"" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

// With no coord field, every index is beyond the coordinates.
void SIGEL_Robot::TST_SIG_Body::loadRefusesAFaceSetWithIndicesButWithoutCoordinates()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, "Shape { geometry IndexedFaceSet { coordIndex [ 0, 1, 2, -1 ] } }\n" + triangleShape() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::readVRMLNode skips an IndexedFaceSet without coord and loads the file", Abort );
  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\"" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAFaceOfFewerThanThreeVertices_data()
{
  QTest::addColumn< QString >( "indices" );

  // VRML 97, IndexedFaceSet: each face has at least three vertices.
  QTest::newRow( "one vertex" ) << "0, 1, 2, -1, 0, -1";
  QTest::newRow( "two vertices" ) << "0, 1, 2, -1, 0, 1, -1";
  QTest::newRow( "two vertices at the end of the indices" ) << "0, 1, 2, -1, 0, 1";
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAFaceOfFewerThanThreeVertices()
{
  QFETCH( QString, indices );
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", indices ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::readVRMLNode makes a face of every run of indices, also of one or two vertices, and loads the file", Abort );
  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\"" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadEndsAFaceAtTheEndOfItsShape()
{
  QTemporaryDir folder;

  // Neither coordIndex ends with -1.
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 1, 2" )
                                            + faceSetShape( "0 0 1, 1 0 1, 0 1 1", "0, 1, 2" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( facePoints( body.getGeometry() ), QList< QStringList >( { { "(0, 0, 0)", "(1, 0, 0)", "(0, 1, 0)" },
                                                                     { "(0, 0, 1)", "(1, 0, 1)", "(0, 1, 1)" } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAddsTheFacesOfTheShapesInFileOrderEachOverItsOwnCoordinates()
{
  QTemporaryDir folder;

  // The indices of the second shape name its own points, not those of the first.
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 1, 2, -1" )
                                            + faceSetShape( "5 0 0, 6 0 0, 5 1 0, 9 9 9", "2, 1, 0, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  const SIG_Geometry *geometry = body.getGeometry();
  QVERIFY( geometry );
  QCOMPARE( facePoints( geometry ), QList< QStringList >( { { "(0, 0, 0)", "(1, 0, 0)", "(0, 1, 0)" },
                                                            { "(5, 1, 0)", "(6, 0, 0)", "(5, 0, 0)" } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAddsNoFacesForShapesThatHaveNoFaces()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    "Shape { geometry IndexedLineSet { coord Coordinate { point [ 5 5 5, 6 5 5, 5 6 5 ] }\n"
                                    "                                  coordIndex [ 0, 1, 2, -1 ] } }\n"
                                    "Shape { geometry IndexedFaceSet { } }\n"
                                    "Shape { }\n" + triangleShape() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

// VRML 97, Box: a box of the given size, centred on the origin of its coordinate system.
void SIGEL_Robot::TST_SIG_Body::loadMakesTheFacesOfABox()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, cornerShape( 5, 5, 5 ) + "Shape { geometry Box { size 2 4 6 } }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QList< SIG_Vector > vertices = withoutTheBesideShape( verticesOf( body.getGeometry() ) );
  int corners = 0;
  for ( const SIG_Vector &vertex : vertices )
  {
    const bool onX = std::fabs( std::fabs( vertex.x ) - 1 ) < epsilon;
    const bool onY = std::fabs( std::fabs( vertex.y ) - 2 ) < epsilon;
    const bool onZ = std::fabs( std::fabs( vertex.z ) - 3 ) < epsilon;
    const bool inside = std::fabs( vertex.x ) < 1 + epsilon && std::fabs( vertex.y ) < 2 + epsilon && std::fabs( vertex.z ) < 3 + epsilon;
    QVERIFY2( inside && ( onX || onY || onZ ), qPrintable( toString( vertex ) ) );
    if ( onX && onY && onZ )
    {
      corners++;
    }
  }

  // The geometry merges equal points, so each corner is one vertex.
  QCOMPARE( corners, 8 );

  // The faces cover the surface of the box once, 2 * ( 2 * 4 + 4 * 6 + 6 * 2 ), however they divide it.
  QCOMPARE( surfaceArea( body.getGeometry() ) - besideTheShapeArea, 88.0 );
}

void SIGEL_Robot::TST_SIG_Body::loadMakesFacesOnTheSurfaceOfACurvedShape_data()
{
  QTest::addColumn< QString >( "shape" );
  QTest::addColumn< double >( "area" );

  // VRML 97: the shape is centred on the origin, with its axis along y. The area: 2 pi r h + 2 pi r^2.
  QTest::newRow( "Cylinder of radius 1 and height 2" ) << "Cylinder { radius 1 height 2 }" << 6 * M_PI;
}

// How finely a curved surface is divided into faces is not fixed. Faces with their corners on
// the surface cover a little less than its area; a shape without one of its parts, such as a
// Cylinder without its caps, covers less than three quarters.
void SIGEL_Robot::TST_SIG_Body::loadMakesFacesOnTheSurfaceOfACurvedShape()
{
  QFETCH( QString, shape );
  QFETCH( double, area );
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, cornerShape( 5, 5, 5 ) + "Shape { geometry " + shape + " }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  const double coveredArea = surfaceArea( body.getGeometry() ) - besideTheShapeArea;
  QVERIFY2( coveredArea > 0.75 * area && coveredArea < area + epsilon, qPrintable( QString::number( coveredArea ) ) );
  for ( const SIG_Vector &vertex : withoutTheBesideShape( verticesOf( body.getGeometry() ) ) )
  {
    QVERIFY2( isOnTheSurface( vertex ), qPrintable( toString( vertex ) ) );
  }
}

void SIGEL_Robot::TST_SIG_Body::loadWarnsThatASphereOrConeIsNotRead_data()
{
  QTest::addColumn< QString >( "shape" );
  QTest::addColumn< QString >( "geometry" );

  QTest::newRow( "Sphere" ) << "Sphere" << "Sphere { radius 1 }";
  QTest::newRow( "Cone" ) << "Cone" << "Cone { bottomRadius 1 height 2 }";
}

// SIGEL reads no Sphere or Cone; the load says so on SIG_IO::cerr and reads the rest of the file.
void SIGEL_Robot::TST_SIG_Body::loadWarnsThatASphereOrConeIsNotRead()
{
  QFETCH( QString, shape );
  QFETCH( QString, geometry );
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, cornerShape( 5, 5, 5 ) + "Shape { geometry " + geometry + " }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  // Replacing the device deletes the stream's own one on stderr, so the stream gets a new one afterwards.
  QString warned;
  SIGEL_Tools::SIG_IO::cerr.setString( &warned );
  const QString thrown = thrownByLoad( body );
  static QFile standardError;
  const bool opened = standardError.isOpen() || standardError.open( stderr, QIODevice::WriteOnly );
  SIGEL_Tools::SIG_IO::cerr.setDevice( &standardError );

  QVERIFY( opened );
  QCOMPARE( thrown, QString() );
  QCOMPARE( warned, "VRML file \"" + fileName + "\": a " + shape + " is not read, the link gets no faces from it.\n" );
  QVERIFY( body.getGeometry() );
  QCOMPARE( surfaceArea( body.getGeometry() ), besideTheShapeArea );
}

void SIGEL_Robot::TST_SIG_Body::loadSkipsNodesOtherThanShapesAndGroupsAndReadsTheShapesAfterThem()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    "WorldInfo { title \"arm\" }\n"
                                    "NavigationInfo { type \"EXAMINE\" }\n"
                                    "Viewpoint { position 0 0 10 }\n"
                                    "Background { skyColor 0 0 0 }\n"
                                    "DirectionalLight { direction 0 0 -1 }\n"
                                    "Group { children [ PointLight { } " + triangleShape() + " ] }\n"
                                    + cornerShape( 5, 5, 5 ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 2 );
  QCOMPARE( body.getGeometry()->getNumVertices(), 6 );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAFileWithoutFaces_data()
{
  QTest::addColumn< QString >( "contents" );

  QTest::newRow( "no nodes" ) << "";
  QTest::newRow( "shapes without faces" ) << "WorldInfo { title \"arm\" }\nShape { }\nShape { geometry IndexedFaceSet { } }\n";
}

// SIG_Mirtich takes the mass of the link from the faces of its body, so a body without faces gives no usable link.
void SIGEL_Robot::TST_SIG_Body::loadRefusesAFileWithoutFaces()
{
  QFETCH( QString, contents );
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, contents );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::load gives a body an empty geometry when the file has no faces", Abort );
  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\"" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadWalksIntoNestedGroups()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, "Group { children [ Group { children [ " + triangleShape() + cornerShape( 5, 5, 5 ) + " ] }\n"
                                            + cornerShape( 7, 7, 7 ) + " ] }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheFacesInEveryGroupingNode_data()
{
  QTest::addColumn< QString >( "grouping" );

  // VRML 97: Anchor, Billboard and Collision draw their children, Switch draws
  // the choice that whichChoice names, LOD draws one of its levels.
  QTest::newRow( "Anchor" ) << "Anchor { children [ %1 ] }";
  QTest::newRow( "Billboard" ) << "Billboard { children [ %1 ] }";
  QTest::newRow( "Collision" ) << "Collision { children [ %1 ] }";
  QTest::newRow( "Switch" ) << "Switch { whichChoice 0 choice [ %1 ] }";
  QTest::newRow( "LOD" ) << "LOD { level [ %1 ] }";
}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheFacesInEveryGroupingNode()
{
  QFETCH( QString, grouping );
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, grouping.arg( triangleShape() ) + cornerShape( 5, 5, 5 ) );

  // CyberVRML97 gives the grouping node with the shape as its child.
  QByteArray fileNameBytes = fileName.toUtf8();
  SceneGraph scene;
  scene.load( fileNameBytes.data() );
  Node *node = scene.getNodes();
  QVERIFY( node && node->getChildNodes() && node->getChildNodes()->isShapeNode() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 2 );
  QCOMPARE( body.getGeometry()->getNumVertices(), 6 );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformTranslationToEachOfItsChildren()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "translation 1 -2 3", cornerShape( 4, 5, 6 ) + cornerShape( 0, 0, 0 ) ) );

  QVERIFY2( faces.size() == 2 && hasAFaceThatStartsNear( faces, 5, 3, 9 ) && hasAFaceThatStartsNear( faces, 1, -2, 3 ),
            qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesATransformOnlyWithinItsOwnChildren()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( "Group { children [ " + transformed( "translation 1 0 0", cornerShape( 1, 0, 0 ) )
                                                 + cornerShape( 0, 1, 0 ) + " ] }\n"
                                                 + cornerShape( 0, 0, 1 ) );

  QVERIFY2( faces.size() == 3 && hasAFaceThatStartsNear( faces, 2, 0, 0 ) && hasAFaceThatStartsNear( faces, 0, 1, 0 )
            && hasAFaceThatStartsNear( faces, 0, 0, 1 ),
            qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformScaleToTheVertices()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "scale 2 3 0.5", cornerShape( 1, 1, 4 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 2, 3, 2 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformRotationByTheRightHandRule()
{
  // A positive quarter turn about z takes x to y.
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "rotation " ) + quarterTurnAboutZ, cornerShape( 1, 0, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 0, 1, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesBeforeRotating()
{
  // Rotating first gives (0, 1, 0).
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "scale 2 1 1 rotation " ) + quarterTurnAboutZ, cornerShape( 1, 0, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 0, 2, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAboutTheTransformCenter()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "center 1 2 3 scale 2 2 2", cornerShape( 2, 3, 4 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 3, 4, 5 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadRotatesAboutTheTransformCenterBeforeTranslating()
{
  // Rotating about the origin gives (1, 2, 0), translating first (3, 4, 0).
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "translation 1 0 0 center 1 2 0 rotation " ) + quarterTurnAboutZ,
                                                              cornerShape( 2, 0, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 4, 3, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAlongTheTransformScaleOrientation()
{
  // A quarter turn of the scale axes makes the x scale act on y, whichever
  // way the turn goes.
  QString fields = QString( "scale 2 1 1 scaleOrientation " ) + quarterTurnAboutZ;
  QList< QList< SIG_Vector > > onY = loadedFaces( transformed( fields, cornerShape( 0, 1, 0 ) ) );
  QList< QList< SIG_Vector > > onX = loadedFaces( transformed( fields, cornerShape( 1, 0, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( onY, 0, 2, 0 ), qPrintable( toString( onY ) ) );
  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( onX, 1, 0, 0 ), qPrintable( toString( onX ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadRotatesAfterScalingAlongTheScaleOrientation()
{
  // The scale axes turn about x and the rotation about z, so the order shows,
  // whichever way the turns go: rotating before the scaleOrientation gives (-2, 0, 0).
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "scale 1 2 1 scaleOrientation 1 0 0 1.5707963 rotation " )
                                                              + quarterTurnAboutZ,
                                                              cornerShape( 0, 0, 1 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 0, 0, 2 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAlongTheScaleOrientationAboutTheTransformCenter()
{
  // Undoing the scaleOrientation before moving the center to the origin gives (2, 0, 0).
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "center 1 0 0 scale 2 1 1 scaleOrientation " ) + quarterTurnAboutZ,
                                                              cornerShape( 1, 1, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 1, 2, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAlongAnEighthTurnedScaleOrientation()
{
  // The x scale acts along (1, 1, 0). Swapping the scaleOrientation and its
  // inverse gives (1.5, -0.5, 0).
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "scale 2 1 1 scaleOrientation 0 0 1 0.78539816", cornerShape( 1, 0, 0 ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 1.5, 0.5, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesNestedTransformsInnermostFirst()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "translation 1 0 0",
                                                              transformed( "scale 2 2 2", cornerShape( 1, 0, 0 ) ) ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 3, 0, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesATransformToTheShapesInGroupsWithinIt()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( "translation 1 0 0", "Group { children [ " + cornerShape( 0, 1, 0 ) + " ] }\n" ) );

  QVERIFY2( isTheFirstCornerOfTheOnlyFaceNear( faces, 1, 1, 0 ), qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadTransformsEveryVertexOfAShape()
{
  QList< QList< SIG_Vector > > faces = loadedFaces( transformed( QString( "translation 0 0 5 rotation " ) + quarterTurnAboutZ,
                                                              faceSetShape( "1 0 0, 2 0 0, 1 3 0", "0, 1, 2, -1" ) ) );

  QVERIFY2( faces.size() == 1 && faces[ 0 ].size() == 3 && isNear( faces[ 0 ][ 0 ], 0, 1, 5 )
            && isNear( faces[ 0 ][ 1 ], 0, 2, 5 ) && isNear( faces[ 0 ][ 2 ], -3, 1, 5 ),
            qPrintable( toString( faces ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadKeepsTheFacesFacingOutwards_data()
{
  QTest::addColumn< QString >( "body" );
  QTest::addColumn< double >( "normalZ" );

  // SIG_Mirtich takes a face's outside from the order of its corners: counter-clockwise seen from outside.
  // VRML 97: with ccw FALSE the corners go clockwise seen from the front. A Transform with a negative
  // scale mirrors the face, and a mirror turns the order of its corners round.
  const QString triangle = faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 1, 2, -1" );
  QTest::newRow( "ccw TRUE" ) << triangle << 1.0;
  QTest::newRow( "ccw FALSE" ) << QString( "Shape { geometry IndexedFaceSet { ccw FALSE coord Coordinate { point [ 0 0 0, 1 0 0, 0 1 0 ] }\n"
                                           "                                  coordIndex [ 0, 1, 2, -1 ] } }\n" ) << -1.0;
  QTest::newRow( "mirrored along x" ) << transformed( "scale -1 1 1", triangle ) << 1.0;
}

void SIGEL_Robot::TST_SIG_Body::loadKeepsTheFacesFacingOutwards()
{
  QFETCH( QString, body );
  QFETCH( double, normalZ );

  QList< QList< SIG_Vector > > faces = loadedFaces( body );

  QVERIFY2( faces.size() == 1 && faces[ 0 ].size() == 3, qPrintable( toString( faces ) ) );
  const SIG_Vector a = faces[ 0 ][ 0 ];
  const SIG_Vector b = faces[ 0 ][ 1 ];
  const SIG_Vector c = faces[ 0 ][ 2 ];
  const double crossZ = ( b.x - a.x ) * ( c.y - a.y ) - ( b.y - a.y ) * ( c.x - a.x );
  QEXPECT_FAIL( "ccw FALSE", "SIG_Body::readVRMLNode ignores the ccw field and keeps the corners in the order of the coordIndex", Abort );
  QEXPECT_FAIL( "mirrored along x", "SIG_Body::readVRMLNode keeps the order of the corners when a Transform mirrors the face", Abort );
  QCOMPARE( crossZ, normalZ );
}

void SIGEL_Robot::TST_SIG_Body::loadDoesNothingWhenTheBodyHasAGeometry()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, triangleShape() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );
  QCOMPARE( thrownByLoad( body ), QString() );
  const SIG_Geometry *loaded = body.getGeometry();
  QVERIFY( loaded );

  QCOMPARE( thrownByLoad( body ), QString() );

  QCOMPARE( body.getGeometry(), loaded );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheFileRelativeToTheDirectory()
{
  QTemporaryDir folder;
  writeVRMLFile( folder, triangleShape() );
  SIG_Robot robot;
  SIG_Body body( &robot, "body.wrl", folder.path() + "/" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsAFileWithANonAsciiName()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, triangleShape(), QString::fromUtf16( u"k\u00F6rper.wrl" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAMissingFile()
{
  QTemporaryDir folder;
  SIG_Robot robot;
  SIG_Body body( &robot, "missing.wrl", folder.path() + "/" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.startsWith( "VRML file \"" + folder.filePath( "missing.wrl" ) + "\": " ), qPrintable( thrown ) );
  QVERIFY2( thrown.contains( "cannot be read" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAFileThatIsNotVRML()
{
  QTemporaryDir folder;
  QString fileName = writeFile( folder, "This is not a VRML file.\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\", line 1: " ), qPrintable( thrown ) );
  QVERIFY2( thrown.contains( "syntax error" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAFileWithASyntaxErrorAfterValidShapes()
{
  // The shapes before the error must not reach the body as a partial geometry.
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, triangleShape() + "Group { children [ }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.startsWith( "VRML file \"" + fileName + "\", line 4: " ), qPrintable( thrown ) );
  QVERIFY2( thrown.contains( "syntax error" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::readVRMLNodeAppliesTheGivenTransformation()
{
  QTemporaryDir folder;
  QByteArray fileName = writeVRMLFile( folder, cornerShape( 1, 2, 3 ) ).toUtf8();
  SceneGraph scene;
  scene.load( fileName.data() );
  QVERIFY( scene.getNodes() );
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );

  // readVRMLNode adds the faces to the body's geometry and does not create one.
  body.setGeometry( new SIG_Geometry() );
  NEWMAT::Matrix transformation( 4, 4 );
  transformation = 0;
  transformation( 1, 2 ) = 1;
  transformation( 2, 1 ) = 1;
  transformation( 3, 3 ) = 2;
  transformation( 3, 4 ) = 10;
  transformation( 4, 4 ) = 1;

  body.readVRMLNode( scene.getNodes(), transformation );

  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
  SIG_Vector point = body.getGeometry()->getPolygons().first()->getVertex( 0 );
  QVERIFY2( isNear( point, 2, 1, 16 ), qPrintable( toString( point ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheFaceSetsOfAFileWithUse()
{
  QTemporaryDir folder;

  // USE inside a children list, and after a "}" that a comma ends. The comma
  // must put the last copy beside the translated Transform; inside it, the copy
  // would add three shifted vertices.
  QString fileName = writeVRMLFile( folder,
                                    "DEF T Transform { children [ " + triangleShape() + " ] }\n"
                                    "Transform { children [ USE T ] }\n"
                                    "Transform { translation 5 0 0 } ,\n"
                                    "USE T\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 3 );

  // The copies lie where the original does, so they share its vertices: each vertex is one contact point.
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsAShapeWhoseGeometryIsAUse()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    "Shape { geometry DEF G IndexedFaceSet { coord Coordinate { point [ 0 0 0, 1 0 0, 0 1 0 ] }\n"
                                    "                                        coordIndex [ 0, 1, 2, -1 ] } }\n"
                                    "Shape { appearance Appearance { } geometry USE G }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 2 );

  // The second shape uses the same points, and each vertex is one contact point.
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsAUseDirectlyAfterANodeEndAsACopyBesideThatNode()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, useDirectlyAfterTransformEnd() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  const QString thrown = thrownByLoad( body );

  QEXPECT_FAIL( "", "SIG_Body::load refuses a USE directly after a }, because CyberVRML97 would read it into the node that the } closes", Abort );
  QCOMPARE( thrown, QString() );
  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 2 );

  // The copy lies where the original does, so they share its vertices: each vertex is one contact point.
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}
