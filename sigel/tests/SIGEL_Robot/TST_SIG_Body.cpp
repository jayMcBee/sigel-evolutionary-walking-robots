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
#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

#include <cmath>
#include <memory>

namespace {

  QString faceSetShape( const QString &points, const QString &indices )
  {
    return "Shape { geometry IndexedFaceSet { coord Coordinate { point [ " + points + " ] }\n"
           "                                  coordIndex [ " + indices + " ] } }\n";
  }

  QString triangleShape()
  {
    return faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 1, 2, -1" );
  }

  // A face of a single vertex at point, so the transforms can be read off it.
  QString pointShape( const QString &point )
  {
    return faceSetShape( point, "0" );
  }

  QString transformed( const QString &fields, const QString &children )
  {
    return "Transform { " + fields + " children [ " + children + " ] }\n";
  }

  // Writes contents to name in folder and returns that file's path.
  QString writeFile( const QTemporaryDir &folder, const QString &contents, const QString &name = "body.wrl" )
  {
    QString fileName = folder.filePath( name );
    QFile file( fileName );
    if ( !folder.isValid() || !file.open( QIODevice::WriteOnly ) )
      QTest::qFail( qPrintable( "cannot write " + fileName ), __FILE__, __LINE__ );
    else
      file.write( contents.toUtf8() );
    return fileName;
  }

  QString writeVRMLFile( const QTemporaryDir &folder, const QString &body, const QString &name = "body.wrl" )
  {
    return writeFile( folder, "#VRML V2.0 utf8\n" + body, name );
  }

  // What load throws, empty if it throws nothing. Catching everything here keeps
  // an exception from escaping the test function, which ends the whole test run.
  QString thrownByLoad( SIGEL_Robot::SIG_Body &body )
  {
    try {
      body.load();
    }
    catch ( const SIGEL_Tools::SIG_Exception &e ) {
      return e.getMessage();
    }
    catch ( ... ) {
      return "an exception that is not a SIG_Exception";
    }
    return QString();
  }

  // The body that the unstreaming constructor reads from in; null, with what it
  // throws in thrown, if it throws.
  std::unique_ptr< SIGEL_Robot::SIG_Body > unstreamedBody( SIGEL_Robot::SIG_Robot &robot, QTextStream &in, QString &thrown )
  {
    try {
      return std::make_unique< SIGEL_Robot::SIG_Body >( &robot, in );
    }
    catch ( const SIGEL_Tools::SIG_Exception &e ) {
      thrown = e.getMessage();
    }
    catch ( ... ) {
      thrown = "an exception that is not a SIG_Exception";
    }
    return nullptr;
  }

  // The USE names a node other than the one it follows, so if load's check
  // missed it, CyberVRML97 would misplace the copy instead of aborting the test run.
  QString useDirectlyAfterGroupEnd( const QString &separator = " " )
  {
    return "DEF S " + triangleShape() + "Group { }" + separator + "USE S\n";
  }

  const double epsilon = 1e-5;
  const char *const quarterTurnAboutZ = "0 0 1 1.5707963";

  bool isNear( SIG_Vector v, double x, double y, double z )
  {
    return std::fabs( v.get( 0 ) - x ) < epsilon
      && std::fabs( v.get( 1 ) - y ) < epsilon
      && std::fabs( v.get( 2 ) - z ) < epsilon;
  }

  QString toString( SIG_Vector v )
  {
    return QString( "(%1, %2, %3)" ).arg( v.get( 0 ) ).arg( v.get( 1 ) ).arg( v.get( 2 ) );
  }

  QString toString( const QList< SIG_Vector > &vertices )
  {
    QStringList texts;
    for ( const SIG_Vector &v : vertices )
      texts.append( toString( v ) );
    return "[" + texts.join( ", " ) + "]";
  }

  // Empty for a null geometry, so that a missing one fails the comparison instead of crashing.
  QList< SIG_Vector > verticesOf( const SIGEL_Robot::SIG_Geometry *geometry )
  {
    QList< SIG_Vector > vertices;
    for ( int i = 0; geometry && i < geometry->getNumVertices(); i++ )
      vertices.append( geometry->getVertex( i ) );
    return vertices;
  }

  // The vertices of the geometry that a body loads from the VRML file body.
  QList< SIG_Vector > loadedVertices( const QString &body )
  {
    QTemporaryDir folder;
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Body loaded( &robot, writeVRMLFile( folder, body ), "" );
    const QString thrown = thrownByLoad( loaded );
    if ( !thrown.isEmpty() ) {
      QTest::qFail( qPrintable( "load threw: " + thrown ), __FILE__, __LINE__ );
      return {};
    }
    if ( !loaded.getGeometry() )
      QTest::qFail( "load left the body without geometry", __FILE__, __LINE__ );
    return verticesOf( loaded.getGeometry() );
  }

  bool isTheOnlyVertexNear( const QList< SIG_Vector > &vertices, double x, double y, double z )
  {
    return vertices.size() == 1 && isNear( vertices.first(), x, y, z );
  }

  // The faces as indices into the geometry's vertices, which are not the VRML
  // coordIndex values (see loadKeepsOnlyTheUsedCoordinatesInFirstUseOrderAndMergesEqualOnes).
  QList< QList< int > > faceIndices( const SIGEL_Robot::SIG_Geometry *geometry )
  {
    QList< QList< int > > faces;
    SIGEL_Robot::SIG_GeometryIterator it( geometry );
    while ( it ) {
      const SIGEL_Robot::SIG_Polygon &polygon = it.iterate();
      QList< int > face;
      for ( int i = 0; i < polygon.getNumVertices(); i++ )
        face.append( polygon.getVertexIndex( i ) );
      faces.append( face );
    }
    return faces;
  }

  // Empty if actual is expected, else where it differs first. Compared at the
  // call site, so a failure names the line of that call and stops the test there.
  QString matrixMismatch( NEWMAT::Matrix actual, const double ( &expected )[ 4 ][ 4 ] )
  {
    if ( actual.Nrows() != 4 || actual.Ncols() != 4 )
      return QString( "is %1x%2, expected 4x4" ).arg( actual.Nrows() ).arg( actual.Ncols() );
    for ( int r = 0; r < 4; r++ )
      for ( int c = 0; c < 4; c++ )
        if ( !( std::fabs( actual( r + 1, c + 1 ) - expected[ r ][ c ] ) < epsilon ) )
          return QString( "(%1,%2) is %3, expected %4" )
            .arg( r + 1 ).arg( c + 1 ).arg( actual( r + 1, c + 1 ) ).arg( expected[ r ][ c ] );
    return QString();
  }

}

void SIGEL_Robot::TST_SIG_Body::constructorStoresTheNameAndLeavesTheBodyWithoutGeometry()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );

  QCOMPARE( body.getName(), QString( "arm.wrl" ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::getGeometryFilePrependsTheDirectoryVerbatimToARelativeName()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  // The robot builder hands over a directory that ends with "/".
  SIG_Body withoutSlash( &robot, "arm.wrl", "parts" );

  QCOMPARE( body.getGeometryFile(), QString( "parts/arm.wrl" ) );
  QCOMPARE( withoutSlash.getGeometryFile(), QString( "partsarm.wrl" ) );
}

void SIGEL_Robot::TST_SIG_Body::getGeometryFileReturnsAnAbsoluteNameWithoutTheDirectory()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "/models/arm.wrl", "parts/" );

  QCOMPARE( body.getGeometryFile(), QString( "/models/arm.wrl" ) );
}

void SIGEL_Robot::TST_SIG_Body::getGeometryFileOfAnEmptyNameIsTheDirectory()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "", "parts/" );

  QCOMPARE( body.getGeometryFile(), QString( "parts/" ) );
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

void SIGEL_Robot::TST_SIG_Body::setGeometryPinsThatItReplacesTheGeometryWithoutFreeingTheOldOne()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  // The test frees first, which the body no longer holds; the body frees second.
  std::unique_ptr< SIG_Geometry > first( new SIG_Geometry() );
  SIG_Geometry *second = new SIG_Geometry();
  body.setGeometry( first.get() );

  body.setGeometry( second );

  QCOMPARE( body.getGeometry(), second );
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
  // Not the temporary folder, whose path may contain whitespace, on which the transfer
  // format splits (see unstreamingConstructorPinsThatADirectoryWithASpaceDoesNotRoundTrip).
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
    QVERIFY2( isNear( read->getGeometry()->getVertex( i ),
                      source.getGeometry()->getVertex( i ).get( 0 ),
                      source.getGeometry()->getVertex( i ).get( 1 ),
                      source.getGeometry()->getVertex( i ).get( 2 ) ),
              qPrintable( toString( read->getGeometry()->getVertex( i ) ) ) );
  QCOMPARE( faceIndices( read->getGeometry() ), QList< QList< int > >( { { 0, 1, 2 }, { 2, 3, 0 } } ) );
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

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsAFlagOtherThanYAsNoGeometry()
{
  SIG_Robot robot;
  QString text( "arm.wrl parts/ Y\nMaterial" );
  QTextStream in( &text );

  QString thrown;

  std::unique_ptr< SIG_Body > body = unstreamedBody( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( body->getGeometry(), nullptr );
  QCOMPARE( next, QString( "Material" ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorThrowsForAYFollowedByAMalformedGeometry()
{
  SIG_Robot robot;
  QString text( "arm.wrl parts/ y\nMaterial" );
  QTextStream in( &text );

  QString thrown;

  unstreamedBody( robot, in, thrown );

  QVERIFY2( thrown.contains( "Unstreaming error: Geometry expected, found 'Material'." ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorReadsAnEndedStreamAsABodyWithoutNameOrGeometry()
{
  SIG_Robot robot;
  QString text;
  QTextStream in( &text );

  QString thrown;

  std::unique_ptr< SIG_Body > body = unstreamedBody( robot, in, thrown );

  QCOMPARE( thrown, QString() );
  QCOMPARE( body->getName(), QString() );
  QCOMPARE( body->getGeometryFile(), QString() );
  QCOMPARE( body->getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::unstreamingConstructorPinsThatADirectoryWithASpaceDoesNotRoundTrip()
{
  // Pins the current behaviour: writeToFileTransfer writes the directory
  // unquoted and the unstreaming constructor reads it up to the space, so the
  // rest of it is taken for the geometry flag.
  SIG_Robot robot;
  SIG_Body written( &robot, "arm.wrl", "my parts/" );
  QString text;
  QTextStream out( &text );
  written.writeToFileTransfer( out );
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
  QCOMPARE( read->getGeometryFile(), QString( "myarm.wrl" ) );
  QCOMPARE( read->getGeometry(), nullptr );
  QCOMPARE( next, QString( "n" ) );
}

void SIGEL_Robot::TST_SIG_Body::createRotationMatrixPinsTheCurrentSignOfTheSineTerms()
{
  // Pins the current behaviour: the sine terms have the opposite sign from
  // rotationMatrix in IFunctions.cpp, and whether that is right is open (item 144).
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  const double quarter = M_PI / 2;
  const double aboutX[ 4 ][ 4 ] = { { 1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, -1, 0, 0 }, { 0, 0, 0, 1 } };
  const double aboutY[ 4 ][ 4 ] = { { 0, 0, -1, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 }, { 0, 0, 0, 1 } };
  const double aboutZ[ 4 ][ 4 ] = { { 0, 1, 0, 0 }, { -1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

  QCOMPARE( matrixMismatch( body.createRotationMatrix( { 1, 0, 0, float( quarter ) } ), aboutX ), QString() );
  QCOMPARE( matrixMismatch( body.createRotationMatrix( { 0, 1, 0, float( quarter ) } ), aboutY ), QString() );
  QCOMPARE( matrixMismatch( body.createRotationMatrix( { 0, 0, 1, float( quarter ) } ), aboutZ ), QString() );
}

void SIGEL_Robot::TST_SIG_Body::createRotationMatrixTurnsHalfwayAboutAnOffAxisDirection()
{
  // A half turn has no sine terms, so this holds for either sign: 2 a a^T - I.
  // No axis component is zero, so every product of two of them counts.
  SIG_Robot robot;
  SIG_Body body( &robot, "arm.wrl", "parts/" );
  const double expected[ 4 ][ 4 ] = { { -0.5392, 0.576, 0.6144, 0 },
                                      { 0.576, -0.28, 0.768, 0 },
                                      { 0.6144, 0.768, -0.1808, 0 },
                                      { 0, 0, 0, 1 } };

  QCOMPARE( matrixMismatch( body.createRotationMatrix( { 0.48f, 0.6f, 0.64f, float( M_PI ) } ), expected ), QString() );
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
  QCOMPARE( toString( verticesOf( geometry ) ), QString( "[(0.5, -1, 2), (3, 0, 0), (3, 4, 0), (0, 4, 0)]" ) );
  QCOMPARE( faceIndices( geometry ), QList< QList< int > >( { { 0, 1, 2 }, { 0, 2, 3 } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadKeepsOnlyTheUsedCoordinatesInFirstUseOrderAndMergesEqualOnes()
{
  QTemporaryDir folder;
  // Point 3 is unused and point 4 equals point 1.
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0, 5 5 5, 1 0 0", "2, 0, 1, -1, 4, 2, 0, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  const SIG_Geometry *geometry = body.getGeometry();
  QVERIFY( geometry );
  QCOMPARE( toString( verticesOf( geometry ) ), QString( "[(0, 1, 0), (0, 0, 0), (1, 0, 0)]" ) );
  QCOMPARE( faceIndices( geometry ), QList< QList< int > >( { { 0, 1, 2 }, { 2, 0, 1 } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadEndsAFaceAtANegativeIndexAndAtTheEndOfTheIndices()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 1 1 0, 0 1 0", "0, 1, 2, -2, 2, 3, 0" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( faceIndices( body.getGeometry() ), QList< QList< int > >( { { 0, 1, 2 }, { 2, 3, 0 } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadSkipsIndicesBeyondTheCoordinates()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", "0, 3, 1, 9, 2, -1" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( faceIndices( body.getGeometry() ), QList< QList< int > >( { { 0, 1, 2 } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadMakesNoFaceOfAnIndexRunWithoutVertices()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, faceSetShape( "0 0 0, 1 0 0, 0 1 0", "-1, 7, -1, -1, 0, 1, 2, -1, 5" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( faceIndices( body.getGeometry() ), QList< QList< int > >( { { 0, 1, 2 } } ) );
}

void SIGEL_Robot::TST_SIG_Body::loadIgnoresShapesWithoutAnIndexedFaceSetWithCoordinates()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    "Shape { geometry Box { } }\n"
                                    "Shape { geometry IndexedLineSet { coord Coordinate { point [ 5 5 5, 6 5 5, 5 6 5 ] }\n"
                                    "                                  coordIndex [ 0, 1, 2, -1 ] } }\n"
                                    "Shape { geometry IndexedFaceSet { coordIndex [ 0, 1, 2, -1 ] } }\n"
                                    "Shape { }\n" + triangleShape() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadWalksIntoNestedGroups()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, "Group { children [ Group { children [ " + triangleShape() + pointShape( "5 5 5" ) + " ] }\n"
                                            + pointShape( "7 7 7" ) + " ] }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadPinsThatItDropsTheFacesInGroupingNodesOtherThanGroupAndTransform()
{
  // Pins the current behaviour: readVRMLNode walks only into Group and
  // Transform, though CyberVRML97 parses these into nodes with the shape as child.
  const QStringList groupings = { "Anchor { children [ %1 ] }",
                                  "Billboard { children [ %1 ] }",
                                  "Collision { children [ %1 ] }",
                                  "Switch { whichChoice 0 choice [ %1 ] }",
                                  "LOD { level [ %1 ] }" };
  for ( const QString &grouping : groupings ) {
    QTemporaryDir folder;
    QString fileName = writeVRMLFile( folder, grouping.arg( triangleShape() ) );
    QByteArray fileNameBytes = fileName.toUtf8();
    SceneGraph scene;
    scene.load( fileNameBytes.data() );
    Node *node = scene.getNodes();
    QVERIFY2( node && node->getChildNodes() && node->getChildNodes()->isShapeNode(), qPrintable( grouping ) );
    SIG_Robot robot;
    SIG_Body body( &robot, fileName, "" );

    QCOMPARE( thrownByLoad( body ), QString() );

    QVERIFY2( body.getGeometry() && body.getGeometry()->getNumPolygons() == 0, qPrintable( grouping ) );
  }
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformTranslationToEachOfItsChildren()
{
  QList< SIG_Vector > vertices = loadedVertices( transformed( "translation 1 -2 3", pointShape( "4 5 6" ) + pointShape( "0 0 0" ) ) );

  QVERIFY2( vertices.size() == 2 && isNear( vertices[ 0 ], 5, 3, 9 ) && isNear( vertices[ 1 ], 1, -2, 3 ),
            qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesATransformOnlyWithinItsOwnChildren()
{
  QList< SIG_Vector > vertices = loadedVertices( "Group { children [ " + transformed( "translation 1 0 0", pointShape( "1 0 0" ) )
                                                 + pointShape( "0 1 0" ) + " ] }\n"
                                                 + pointShape( "0 0 1" ) );

  QVERIFY2( vertices.size() == 3 && isNear( vertices[ 0 ], 2, 0, 0 ) && isNear( vertices[ 1 ], 0, 1, 0 )
            && isNear( vertices[ 2 ], 0, 0, 1 ),
            qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformScaleToTheVertices()
{
  QList< SIG_Vector > vertices = loadedVertices( transformed( "scale 2 3 0.5", pointShape( "1 1 4" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 2, 3, 2 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesTheTransformRotationAsCreateRotationMatrixDefinesIt()
{
  // Pins the current behaviour (item 144): a positive quarter turn about z
  // takes x to -y, where VRML 97 turns it counterclockwise to +y.
  QList< SIG_Vector > vertices = loadedVertices( transformed( QString( "rotation " ) + quarterTurnAboutZ, pointShape( "1 0 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 0, -1, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesBeforeRotating()
{
  // The rotation takes x to -y as the rotation test pins (item 144). Rotating
  // first gives (0, -1, 0); with the other sign the orders give (0, 2, 0) and (0, 1, 0).
  QList< SIG_Vector > vertices = loadedVertices( transformed( QString( "scale 2 1 1 rotation " ) + quarterTurnAboutZ, pointShape( "1 0 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 0, -2, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAboutTheTransformCenter()
{
  QList< SIG_Vector > vertices = loadedVertices( transformed( "center 1 2 3 scale 2 2 2", pointShape( "2 3 4" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 3, 4, 5 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadRotatesAboutTheTransformCenterBeforeTranslating()
{
  // The rotation takes x to -y as the rotation test pins (item 144). Rotating
  // about the origin gives (1, -2, 0), translating first (-1, 0, 0).
  QList< SIG_Vector > vertices = loadedVertices( transformed( QString( "translation 1 0 0 center 1 2 0 rotation " ) + quarterTurnAboutZ,
                                                              pointShape( "2 0 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 0, 1, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAlongTheTransformScaleOrientation()
{
  // A quarter turn of the scale axes makes the x scale act on y, whichever
  // way the turn goes.
  QString fields = QString( "scale 2 1 1 scaleOrientation " ) + quarterTurnAboutZ;
  QList< SIG_Vector > onY = loadedVertices( transformed( fields, pointShape( "0 1 0" ) ) );
  QList< SIG_Vector > onX = loadedVertices( transformed( fields, pointShape( "1 0 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( onY, 0, 2, 0 ), qPrintable( toString( onY ) ) );
  QVERIFY2( isTheOnlyVertexNear( onX, 1, 0, 0 ), qPrintable( toString( onX ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadRotatesAfterScalingAlongTheScaleOrientation()
{
  // The scale axes turn about x and the rotation about z, so the order shows,
  // whichever way the turns go: rotating before the scaleOrientation gives (-2, 0, 0).
  QList< SIG_Vector > vertices = loadedVertices( transformed( QString( "scale 1 2 1 scaleOrientation 1 0 0 1.5707963 rotation " )
                                                              + quarterTurnAboutZ,
                                                              pointShape( "0 0 1" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 0, 0, 2 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadScalesAlongTheScaleOrientationAboutTheTransformCenter()
{
  // The expected point holds for either sign of createRotationMatrix (item 144).
  // With the current sign, undoing the scaleOrientation before moving the center
  // to the origin gives (2, 4, 0).
  QList< SIG_Vector > vertices = loadedVertices( transformed( QString( "center 1 0 0 scale 2 1 1 scaleOrientation " ) + quarterTurnAboutZ,
                                                              pointShape( "1 1 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 1, 2, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadPinsTheCurrentNumbersOfAScaleAlongAnEighthTurnedScaleOrientation()
{
  // Pins the current behaviour: the x scale acts along (1, -1, 0), the
  // direction createRotationMatrix turns x to (item 144). Swapping the
  // scaleOrientation and its inverse, or the other sign, gives (1.5, 0.5, 0).
  QList< SIG_Vector > vertices = loadedVertices( transformed( "scale 2 1 1 scaleOrientation 0 0 1 0.78539816", pointShape( "1 0 0" ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 1.5, -0.5, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadAppliesNestedTransformsInnermostFirst()
{
  QList< SIG_Vector > vertices = loadedVertices( transformed( "translation 1 0 0",
                                                              transformed( "scale 2 2 2", pointShape( "1 0 0" ) ) ) );

  QVERIFY2( isTheOnlyVertexNear( vertices, 3, 0, 0 ), qPrintable( toString( vertices ) ) );
}

void SIGEL_Robot::TST_SIG_Body::loadDoesNothingWhenTheBodyHasAGeometry()
{
  // Pins the current behaviour: load's to-do says this should throw.
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

void SIGEL_Robot::TST_SIG_Body::loadOfAMissingFileGivesAnEmptyGeometry()
{
  // Pins the current behaviour: a missing file is not reported.
  QTemporaryDir folder;
  SIG_Robot robot;
  SIG_Body body( &robot, "missing.wrl", folder.path() + "/" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 0 );
  QCOMPARE( body.getGeometry()->getNumVertices(), 0 );
}

void SIGEL_Robot::TST_SIG_Body::loadOfAFileThatIsNotVRMLGivesAnEmptyGeometry()
{
  // Pins the current behaviour: CyberVRML97's parse error is not reported.
  QTemporaryDir folder;
  QString fileName = writeFile( folder, "This is not a VRML file.\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 0 );
}

void SIGEL_Robot::TST_SIG_Body::readVRMLNodeAppliesTheGivenTransformation()
{
  QTemporaryDir folder;
  QByteArray fileName = writeVRMLFile( folder, pointShape( "1 2 3" ) ).toUtf8();
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

  QCOMPARE( body.getGeometry()->getNumVertices(), 1 );
  SIG_Vector point = body.getGeometry()->getVertex( 0 );
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
  // The copies lie where the original does, so they share its vertices.
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAUseDirectlyAfterANodeEnd()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, useDirectlyAfterGroupEnd() );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  // Caught here rather than by thrownByLoad, which accepts any SIG_Exception.
  QString thrown;
  try {
    body.load();
    QFAIL( "load threw nothing" );
  }
  catch ( const SIG_GeometryFileError &e ) {
    thrown = e.getMessage();
  }
  catch ( const SIGEL_Tools::SIG_Exception &e ) {
    QFAIL( qPrintable( "not a SIG_GeometryFileError: " + e.getMessage() ) );
  }
  catch ( ... ) {
    QFAIL( "an exception that is not a SIG_Exception" );
  }

  QVERIFY2( thrown.startsWith( "Geometry file \"" + fileName + "\", line 4: " ), qPrintable( thrown ) );
  QVERIFY2( thrown.contains( "Put a comma after" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAUseDirectlyAfterANodeEndInTheFileRelativeToTheDirectory()
{
  QTemporaryDir folder;
  writeVRMLFile( folder, useDirectlyAfterGroupEnd() );
  SIG_Robot robot;
  SIG_Body body( &robot, "body.wrl", folder.path() + "/" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.contains( "line 4:" ), qPrintable( thrown ) );
  QVERIFY2( thrown.contains( folder.filePath( "body.wrl" ) ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAUseThatWhitespaceOtherThanASpaceSeparatesFromANodeEnd()
{
  // vrml.l has no rule for a form feed, vertical tab or carriage return, so
  // flex's default rule passes them over without a token and without counting
  // a line; DOS line endings do not separate the USE either and count as one
  // line each.
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, useDirectlyAfterGroupEnd( "\t\f\v\r\n\r\n" ) );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.contains( "line 6:" ), qPrintable( thrown ) );
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadReportsTheLineOfTheUseCountingLinesInStringsAndComments()
{
  QTemporaryDir folder;
  // A comment between the "}" and the USE does not separate them for the parser.
  // As in useDirectlyAfterGroupEnd, the USE names a node other than the one it follows.
  QString fileName = writeVRMLFile( folder,
                                    "# a } comment\n"
                                    "DEF S Group { } ,\n"
                                    "WorldInfo { info [ \"a\nb\" ] } # c\n"
                                    "\n"
                                    "USE S\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.contains( "line 7:" ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::loadReportsTheLineOfTheFirstRefusedUse()
{
  QTemporaryDir folder;
  // Each USE names a node other than the one it follows.
  QString fileName = writeVRMLFile( folder,
                                    "DEF S Group { }\n"
                                    "DEF R Group { }\n"
                                    "Group { } USE S\n"
                                    "Group { } USE R\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QString thrown = thrownByLoad( body );

  QVERIFY2( thrown.contains( "line 4:" ), qPrintable( thrown ) );
}

void SIGEL_Robot::TST_SIG_Body::loadIgnoresUseInCommentsAndStrings()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    triangleShape() +
                                    "# } USE S\n"
                                    "WorldInfo { title \"} USE S\" }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::loadReadsAFileThatEndsInACommentWithoutANewline()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, triangleShape() + "Group { } # end" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}

void SIGEL_Robot::TST_SIG_Body::loadAcceptsAUseThatAFieldNameSeparatesFromANodeEnd()
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
  QCOMPARE( body.getGeometry()->getNumVertices(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadAcceptsAUseThatAStringSeparatesFromANodeEnd()
{
  // Not valid VRML, which has no string directly before a USE: it only shows
  // that a string ends what follows a "}". Nothing is DEFed, so CyberVRML97
  // finds no node to copy.
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder, "Group { } \"x\" USE S\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  QCOMPARE( thrownByLoad( body ), QString() );

  QVERIFY( body.getGeometry() );
}

void SIGEL_Robot::TST_SIG_Body::loadAcceptsAWordOtherThanUseDirectlyAfterANodeEnd()
{
  // Not valid VRML, as these are no node types: CyberVRML97 lexes each as one
  // name, the USE keyword being case-sensitive (vrml.l), stops at a parse error
  // and nothing reaches a USE.
  const QStringList names = { "USEFUL", "USE1", "USE_FUL", "USE-FUL", "USE+FUL", "use", "Use" };
  for ( const QString &name : names ) {
    QTemporaryDir folder;
    QString fileName = writeVRMLFile( folder, "Group { } " + name + " { }\n" );
    SIG_Robot robot;
    SIG_Body body( &robot, fileName, "" );

    const QString thrown = thrownByLoad( body );

    QVERIFY2( thrown.isEmpty(), qPrintable( name + ": " + thrown ) );
    QVERIFY2( body.getGeometry(), qPrintable( name ) );
  }
}
