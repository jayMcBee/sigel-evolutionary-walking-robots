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
#include "SIGEL_Environment/TST_SIG_Environment.h"

#include "SIGEL_Environment/SIG_Environment.h"

#include <QtTest>

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheHeightsOfTheFloorFunction()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "0.5*x-10*z" );

  // The function's x counts along the second index, its z along the first.
  QCOMPARE( loadedHeight( environment, 0, 0 ), 0.0 );
  QCOMPARE( loadedHeight( environment, 0, 3 ), 1.5 );
  QCOMPARE( loadedHeight( environment, 2, 1 ), -19.5 );
  QCOMPARE( loadedHeight( environment, 3, 3 ), -28.5 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheHeightsOfThePictureFile()
{
  QTemporaryFile pictureFile;
  QVERIFY( pictureFile.open() );
  pictureFile.write( "P2\n# a comment line\n3 2\n255\n0 51 255\n255 0 102\n" );
  pictureFile.close();

  SIG_Environment environment;
  environment.setFloorPictureFile( pictureFile.fileName() );
  environment.setFloorFuncSelected( false );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 3 );
  QCOMPARE( secondDimension, 2 );

  // A grey value of the maximum is a height of 5.
  QCOMPARE( heights[ 0 ][ 0 ], 0.0 );
  QCOMPARE( heights[ 0 ][ 1 ], 1.0 );
  QCOMPARE( heights[ 1 ][ 0 ], 5.0 );
  QCOMPARE( heights[ 2 ][ 1 ], 2.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheDimensionsOfTheFloor()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 7 );
  environment.setFloorDimensionZ( 7 );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 7 );
  QCOMPARE( secondDimension, 7 );
  QCOMPARE( spacing, 1.0 );
}

void SIGEL_Environment::TST_SIG_Environment::anotherEnvironmentDoesNotChangeTheTerrain()
{
  SIG_Environment raised;
  raised.setFloorFunction( "2" );

  // A slave makes a default environment first; its flat floor must not reach the raised one.
  SIG_Environment flat;
  dmEnvironment flatDynaMechsEnvironment;
  flat.loadTerrain( flatDynaMechsEnvironment );

  QCOMPARE( loadedHeight( raised, 5, 5 ), 2.0 );
  QCOMPARE( loadedHeight( flat, 5, 5 ), 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::eachLoadHasATerrainFileOfItsOwnAndLeavesNoneBehind()
{
  SIG_Environment environment;
  dmEnvironment firstDynaMechsEnvironment;
  dmEnvironment secondDynaMechsEnvironment;
  environment.loadTerrain( firstDynaMechsEnvironment );
  environment.loadTerrain( secondDynaMechsEnvironment );

  const QString firstTerrainFile = QFile::decodeName( firstDynaMechsEnvironment.getTerrainFilename() );
  const QString secondTerrainFile = QFile::decodeName( secondDynaMechsEnvironment.getTerrainFilename() );

  QVERIFY( firstTerrainFile != secondTerrainFile );
  QVERIFY( !QFile::exists( firstTerrainFile ) );
  QVERIFY( !QFile::exists( secondTerrainFile ) );
}

void SIGEL_Environment::TST_SIG_Environment::floorFunctionThatCannotBeParsedGivesAFlatFloor()
{
  SIG_Environment raised;
  raised.setFloorFunction( "2" );
  dmEnvironment raisedDynaMechsEnvironment;
  raised.loadTerrain( raisedDynaMechsEnvironment );

  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "sin(x" );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 4 );
  QCOMPARE( secondDimension, 4 );

  for (int first = 0; first < 4; ++first)
    for (int second = 0; second < 4; ++second)
      QCOMPARE( heights[ first ][ second ], 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::pictureFileThatDoesNotExistGivesAFlatFloor()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorPictureFile( "/this/picture/does/not/exist.pgm" );
  environment.setFloorFuncSelected( false );

  QCOMPARE( loadedHeight( environment, 3, 3 ), 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainNeedsNoSigelRoot()
{
  const QByteArray sigelRoot = qgetenv( "SIGEL_ROOT" );
  const bool sigelRootWasSet = qEnvironmentVariableIsSet( "SIGEL_ROOT" );
  qunsetenv( "SIGEL_ROOT" );

  SIG_Environment environment;
  environment.setFloorFunction( "2" );
  const double height = loadedHeight( environment, 5, 5 );

  if (sigelRootWasSet)
    qputenv( "SIGEL_ROOT", sigelRoot );

  QCOMPARE( height, 2.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainUsesTheSettingsAtTheCall()
{
  SIG_Environment environment;
  QCOMPARE( loadedHeight( environment, 5, 5 ), 0.0 );

  environment.setFloorFunction( "2" );
  QCOMPARE( loadedHeight( environment, 5, 5 ), 2.0 );
}

void SIGEL_Environment::TST_SIG_Environment::readFromFileGivesTheTerrainOfTheFile()
{
  QString text = "FLOORDIMENSION\n4\n4\nFLOORFUNCTION\n3\n";
  QTextStream file( &text, QIODevice::ReadOnly );

  SIG_Environment environment;
  environment.readFromFile( file );

  // SIG_EnvironmentRenderer draws the terrain that getTerrain gives.
  const SIG_Terrain terrain = environment.getTerrain();

  QCOMPARE( terrain.getDimensionX(), 4 );
  QCOMPARE( terrain.getDimensionZ(), 4 );
  QCOMPARE( terrain.getHeight( 2, 2 ), 3.0 );
  QCOMPARE( loadedHeight( environment, 2, 2 ), 3.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesHeightsWithSixSignificantDigits()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "(x+z)/3" );

  QCOMPARE( loadedHeight( environment, 1, 0 ), 0.333333 );
  QCOMPARE( loadedHeight( environment, 2, 2 ), 1.33333 );

  // A grey value of 100 of 255 is a height of 1.960784...
  dmEnvironment dynaMechsEnvironment;
  QVERIFY( loadPictureTerrain( "P2\n2 2\n255\n0 100\n100 255\n", dynaMechsEnvironment ) );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( heights[ 0 ][ 1 ], 1.96078 );
  QCOMPARE( heights[ 1 ][ 0 ], 1.96078 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesEachHeightOfASymmetricFloorFunction()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "x*z+x+z" );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 4 );
  QCOMPARE( secondDimension, 4 );

  for (int first = 0; first < 4; ++first)
    for (int second = 0; second < 4; ++second)
      QCOMPARE( heights[ first ][ second ], static_cast< double >( first * second + first + second ) );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheDimensionsOfAFloorThatIsNotSquare()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 3 );
  environment.setFloorDimensionZ( 5 );
  environment.setFloorFunction( "2" );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  verifyFloorOfOneHeight( dynaMechsEnvironment, 3, 5, 2.0 );
}

void SIGEL_Environment::TST_SIG_Environment::floorFunctionThatCannotBeParsedGivesAFlatFloorThatIsNotSquare()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 3 );
  environment.setFloorDimensionZ( 5 );
  environment.setFloorFunction( "sin(x" );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  verifyFloorOfOneHeight( dynaMechsEnvironment, 3, 5, 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::groundElevationIsInterpolatedInTrianglesAndKeepsTheEdgeHeightPastTheFloor()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "x*z+1" );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  CartesianVector normal;

  CartesianVector onAGridPoint = { 2, 1, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( onAGridPoint, normal ), 3.0 );

  // In this triangle the floor rises by 2 along each of the two axes, so its normal is (-2, -2, 1), scaled to length 1.
  QCOMPARE( normal[ 0 ], -2.0 / 3 );
  QCOMPARE( normal[ 1 ], -2.0 / 3 );
  QCOMPARE( normal[ 2 ], 1.0 / 3 );

  CartesianVector betweenTwoGridPoints = { 2.5, 1, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( betweenTwoGridPoints, normal ), 3.5 );

  // The function gives 2.875 here; the height in the triangle of the three grid points around it is 3.
  CartesianVector insideACell = { 1.5, 1.25, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( insideACell, normal ), 3.0 );

  CartesianVector beforeTheFirstCorner = { -4, -4, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( beforeTheFirstCorner, normal ), 1.0 );

  CartesianVector pastTheLastCorner = { 100, 100, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( pastTheLastCorner, normal ), 10.0 );

  CartesianVector pastOneEdge = { 1.5, 100, 0 };
  QCOMPARE( dynaMechsEnvironment.getGroundElevation( pastOneEdge, normal ), 5.5 );
}

void SIGEL_Environment::TST_SIG_Environment::pictureFileWithAnotherMagicKeyGivesAFlatFloor()
{
  dmEnvironment dynaMechsEnvironment;
  QVERIFY( loadPictureTerrain( "P5\n2 2\n255\n0 51\n51 255\n", dynaMechsEnvironment ) );

  verifyFloorOfOneHeight( dynaMechsEnvironment, 3, 5, 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::emptyPictureFileGivesAFlatFloor()
{
  dmEnvironment dynaMechsEnvironment;
  QVERIFY( loadPictureTerrain( "", dynaMechsEnvironment ) );

  verifyFloorOfOneHeight( dynaMechsEnvironment, 3, 5, 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::commentsInThePictureFileAreSkipped()
{
  dmEnvironment dynaMechsEnvironment;
  QVERIFY( loadPictureTerrain( "# before the magic key\nP2\n2\n# between the sizes\n2\n255 #after a value\n0 51\n51 255\n", dynaMechsEnvironment ) );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 2 );
  QCOMPARE( secondDimension, 2 );
  QCOMPARE( heights[ 0 ][ 0 ], 0.0 );
  QCOMPARE( heights[ 0 ][ 1 ], 1.0 );
  QCOMPARE( heights[ 1 ][ 0 ], 1.0 );
  QCOMPARE( heights[ 1 ][ 1 ], 5.0 );
}

void SIGEL_Environment::TST_SIG_Environment::pictureFileNeedsNoFinalNewline()
{
  dmEnvironment dynaMechsEnvironment;
  QVERIFY( loadPictureTerrain( "P2\n2 2\n255\n0 51\n51 255", dynaMechsEnvironment ) );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 2 );
  QCOMPARE( secondDimension, 2 );
  QCOMPARE( heights[ 0 ][ 0 ], 0.0 );
  QCOMPARE( heights[ 0 ][ 1 ], 1.0 );
  QCOMPARE( heights[ 1 ][ 0 ], 1.0 );
  QCOMPARE( heights[ 1 ][ 1 ], 5.0 );
}

double SIGEL_Environment::TST_SIG_Environment::loadedHeight( const SIG_Environment &environment, int first, int second ) const
{
  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  return heights[ first ][ second ];
}

bool SIGEL_Environment::TST_SIG_Environment::loadPictureTerrain( const QByteArray &picture, dmEnvironment &target ) const
{
  QTemporaryFile pictureFile;

  if (!pictureFile.open() || pictureFile.write( picture ) != picture.size())
    return false;

  pictureFile.close();

  SIG_Environment environment;
  environment.setFloorDimensionX( 3 );
  environment.setFloorDimensionZ( 5 );
  environment.setFloorPictureFile( pictureFile.fileName() );
  environment.setFloorFuncSelected( false );
  environment.loadTerrain( target );

  return true;
}

void SIGEL_Environment::TST_SIG_Environment::verifyFloorOfOneHeight( dmEnvironment &dynaMechsEnvironment, int firstDimension, int secondDimension, double height ) const
{
  int loadedFirstDimension = 0;
  int loadedSecondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( loadedFirstDimension, loadedSecondDimension, spacing );

  QCOMPARE( loadedFirstDimension, firstDimension );
  QCOMPARE( loadedSecondDimension, secondDimension );

  for (int first = 0; first < firstDimension; ++first)
    for (int second = 0; second < secondDimension; ++second)
      QCOMPARE( heights[ first ][ second ], height );
}
