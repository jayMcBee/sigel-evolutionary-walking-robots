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
#include "SIGEL_Environment/SIG_Terrain.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

// Writes contents to floor.pgm in folder, if fileExists, and returns that file's path.
QString SIGEL_Environment::TST_SIG_Environment::pictureFile( const QTemporaryDir &folder, bool fileExists, const QByteArray &contents )
{
  const QString fileName = folder.filePath( "floor.pgm" );
  if ( !fileExists )
  {
    return fileName;
  }
  QFile picture( fileName );
  if ( !folder.isValid() || !picture.open( QIODevice::WriteOnly ) )
  {
    QTest::qFail( qPrintable( "cannot write " + fileName ), __FILE__, __LINE__ );
    return fileName;
  }
  picture.write( contents );
  return fileName;
}

void SIGEL_Environment::TST_SIG_Environment::addUnreadablePictureRows()
{
  QTest::addColumn< bool >( "fileExists" );
  QTest::addColumn< QByteArray >( "contents" );

  QTest::newRow( "missing file" ) << false << QByteArray();
  QTest::newRow( "empty file" ) << true << QByteArray();
  QTest::newRow( "a picture that is not P2" ) << true << QByteArray( "P3\n1 1\n255\n0 0 0\n" );
}

// SIGEL's y axis points up: the floor heights and the start height are along y.
void SIGEL_Environment::TST_SIG_Environment::newEnvironmentHasAGravityOf9Point81Downwards()
{
  SIG_Environment environment;

  SIG_Vector gravity = environment.getGravity();
  QCOMPARE( gravity.x, 0.0 );
  QCOMPARE( gravity.y, -9.81 );
  QCOMPARE( gravity.z, 0.0 );
}

// No document gives the numbers, so the test states what a usable new environment needs.
void SIGEL_Environment::TST_SIG_Environment::newEnvironmentStartsTheRobotAboveTheFloorOnAGroundThatHoldsIt()
{
  SIG_Environment environment;

  QVERIFY( environment.getStartPosition().y > 0 );
  QVERIFY( environment.getVeloDamping() > 0 && environment.getVeloDamping() <= 1 );
  QVERIFY( environment.getGroundPlanarSpringConstant() > 0 );
  QVERIFY( environment.getGroundNormalSpringConstant() > 0 );
  QVERIFY( environment.getGroundPlanarDamperConstant() > 0 );
  QVERIFY( environment.getGroundNormalDamperConstant() > 0 );

  // Kinetic friction is never above static friction.
  QVERIFY( environment.getFrictionCoeff_u_k() > 0 );
  QVERIFY( environment.getFrictionCoeff_u_k() <= environment.getFrictionCoeff_u_s() );
}

void SIGEL_Environment::TST_SIG_Environment::newEnvironmentHasNoTextureAtFullAlphaAndNoAutosave()
{
  SIG_Environment environment;

  QVERIFY( !environment.getWithTexture() );
  QVERIFY( environment.getTextureFile().isEmpty() );
  QCOMPARE( environment.getTexAlpha(), 255 );
  QCOMPARE( environment.getAutosaveIntervalGenerations(), 0 );
}

// A new experiment can be started without touching the environment page.
void SIGEL_Environment::TST_SIG_Environment::newEnvironmentHasAFlatFloor()
{
  SIG_Environment environment;
  SIG_Terrain terrain;

  QVERIFY( environment.makeTerrain( terrain ) );
  QVERIFY( terrain.getDimensionX() > 0 );
  QVERIFY( terrain.getDimensionZ() > 0 );
  for ( int z = 0; z < terrain.getDimensionZ(); z++ )
  {
    for ( int x = 0; x < terrain.getDimensionX(); x++ )
    {
      QCOMPARE( terrain.getHeight( x, z ), 0.0 );
    }
  }
}

void SIGEL_Environment::TST_SIG_Environment::makeTerrainGivesTheHeightsOfTheFloorFunction()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 3 );
  environment.setFloorFunction( "0.5*x-10*z" );
  SIG_Terrain terrain;

  QVERIFY( environment.makeTerrain( terrain ) );

  QCOMPARE( terrain.getDimensionX(), 4 );
  QCOMPARE( terrain.getDimensionZ(), 3 );
  QCOMPARE( terrain.getHeight( 3, 2 ), -18.5 );
}

// A grey value of the maximum is a height of 5.
void SIGEL_Environment::TST_SIG_Environment::makeTerrainGivesTheHeightsOfThePictureFile()
{
  QTemporaryDir folder;
  QVERIFY( folder.isValid() );
  const QString fileName = folder.filePath( "floor.pgm" );
  QFile picture( fileName );
  QVERIFY( picture.open( QIODevice::WriteOnly ) );
  picture.write( "P2\n3 2\n255\n0 51 255\n255 0 102\n" );
  picture.close();
  SIG_Environment environment;
  environment.setFloorPictureFile( fileName );
  environment.setFloorFuncSelected( false );
  SIG_Terrain terrain;

  QVERIFY( environment.makeTerrain( terrain ) );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  QCOMPARE( terrain.getHeight( 1, 0 ), 1.0 );
  QCOMPARE( terrain.getHeight( 2, 1 ), 2.0 );
}

// The environment page takes any text as floor function. Start Evolution refuses a floor
// that makeTerrain refuses; a flat floor in place of the one the user gave would go unnoticed.
void SIGEL_Environment::TST_SIG_Environment::makeTerrainRefusesAFloorFunctionThatCannotBeParsed()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "sin(x" );
  SIG_Terrain terrain;

  const bool made = environment.makeTerrain( terrain );

  QVERIFY( !made );
}

void SIGEL_Environment::TST_SIG_Environment::makeTerrainRefusesAPictureFileThatCannotBeRead_data()
{
  addUnreadablePictureRows();
}

void SIGEL_Environment::TST_SIG_Environment::makeTerrainRefusesAPictureFileThatCannotBeRead()
{
  QFETCH( bool, fileExists );
  QFETCH( QByteArray, contents );
  QTemporaryDir folder;
  SIG_Environment environment;
  environment.setFloorPictureFile( pictureFile( folder, fileExists, contents ) );
  environment.setFloorFuncSelected( false );
  SIG_Terrain terrain;

  QVERIFY( !environment.makeTerrain( terrain ) );
}

void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesTheTerrainOfMakeTerrain()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 3 );
  environment.setFloorFunction( "0.5*x-10*z" );

  SIG_Terrain terrain = environment.getTerrain();

  QCOMPARE( terrain.getDimensionX(), 4 );
  QCOMPARE( terrain.getDimensionZ(), 3 );
  QCOMPARE( terrain.getHeight( 3, 2 ), -18.5 );
}

void SIGEL_Environment::TST_SIG_Environment::getTerrainFollowsAChangeOfTheFloorFunction()
{
  SIG_Environment environment;
  QCOMPARE( environment.getTerrain().getHeight( 5, 5 ), 0.0 );

  environment.setFloorFunction( "2" );

  QCOMPARE( environment.getTerrain().getHeight( 5, 5 ), 2.0 );
}

// The 3D view and the simulation take the floor from getTerrain. A grey value of the maximum is a height of 5.
void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesTheTerrainOfThePictureFile()
{
  QTemporaryDir folder;
  SIG_Environment environment;
  environment.setFloorPictureFile( pictureFile( folder, true, "P2\n3 2\n255\n0 51 255\n255 0 102\n" ) );
  environment.setFloorFuncSelected( false );

  SIG_Terrain terrain = environment.getTerrain();

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  QCOMPARE( terrain.getHeight( 1, 0 ), 1.0 );
  QCOMPARE( terrain.getHeight( 2, 1 ), 2.0 );
}

// A slave gets the floor from the master this way; the picture file is on the master only.
void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesTheTerrainOfSetTerrainWhateverTheFloorSettings()
{
  SIG_Terrain given;
  QVERIFY( given.setFromFunction( "x+z", 2, 3 ) );
  SIG_Environment environment;
  environment.setFloorDimensionX( 7 );
  environment.setFloorDimensionZ( 7 );
  environment.setFloorPictureFile( "/this/picture/does/not/exist.pgm" );
  environment.setFloorFuncSelected( false );

  environment.setTerrain( given );
  SIG_Terrain terrain = environment.getTerrain();

  QCOMPARE( terrain.getDimensionX(), 2 );
  QCOMPARE( terrain.getDimensionZ(), 3 );
  QCOMPARE( terrain.getHeight( 1, 2 ), 3.0 );
}

void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesNoQuietFlatFloorForAPictureFileThatCannotBeRead_data()
{
  addUnreadablePictureRows();
}

// The 3D view and the simulation take their floor from getTerrain; a flat floor in place of
// the picture would go unnoticed there.
void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesNoQuietFlatFloorForAPictureFileThatCannotBeRead()
{
  QFETCH( bool, fileExists );
  QFETCH( QByteArray, contents );
  QTemporaryDir folder;
  SIG_Environment environment;
  environment.setFloorPictureFile( pictureFile( folder, fileExists, contents ) );
  environment.setFloorFuncSelected( false );
  bool flatWithoutAWord = false;

  // How the fault is reported is open; a terrain that is quietly flat is the fault. An empty
  // terrain of 0 by 0 counts as flat too.
  try
  {
    flatWithoutAWord = environment.getTerrain().isFlat();
  }
  catch ( ... )
  {
  }

  QVERIFY( !flatWithoutAWord );
}

// The environment page takes any text as floor function.
void SIGEL_Environment::TST_SIG_Environment::getTerrainGivesNoQuietFlatFloorForAFloorFunctionThatCannotBeParsed()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "sin(x" );
  bool flatWithoutAWord = false;

  // How the fault is reported is open; a terrain that is quietly flat is the fault. An empty
  // terrain of 0 by 0 counts as flat too.
  try
  {
    flatWithoutAWord = environment.getTerrain().isFlat();
  }
  catch ( ... )
  {
  }

  QVERIFY( !flatWithoutAWord );
}

void SIGEL_Environment::TST_SIG_Environment::writeToFileThenReadFromFileGivesTheSameEnvironment_data()
{
  QTest::addColumn< bool >( "floorFuncSelected" );
  QTest::addColumn< bool >( "withTexture" );

  QTest::newRow( "floor function, no texture" ) << true << false;
  QTest::newRow( "picture file, with texture" ) << false << true;
}

// Each value differs from the default, so a value that is not read back shows.
void SIGEL_Environment::TST_SIG_Environment::writeToFileThenReadFromFileGivesTheSameEnvironment()
{
  QFETCH( bool, floorFuncSelected );
  QFETCH( bool, withTexture );
  SIG_Environment written;
  written.setGravity( SIG_Vector( 0.5, -3.5, 1.25 ) );
  written.setVeloDamping( 0.75 );
  written.setStartPosition( SIG_Vector( 2, 3.5, -4 ) );
  written.setFloorDimensionX( 20 );
  written.setFloorDimensionZ( 30 );
  written.setFloorFunction( "sin(x)*cos(z)" );
  written.setFloorPictureFile( "/home/user/my floors/hill.pgm" );
  written.setFloorFuncSelected( floorFuncSelected );
  written.setTextureFile( "/home/user/my textures/grass.ppm" );
  written.setTexAlpha( 128 );
  written.setWithTexture( withTexture );
  written.setAutosaveIntervalGenerations( 25 );
  written.setGroundPlanarSpringConstant( 1234.5 );
  written.setGroundNormalSpringConstant( 2345.5 );
  written.setGroundPlanarDamperConstant( 12.5 );
  written.setGroundNormalDamperConstant( 23.5 );
  written.setFrictionCoeff_u_s( 0.8 );
  written.setFrictionCoeff_u_k( 0.6 );
  QString text;
  QTextStream out( &text );

  written.writeToFile( out );
  out.flush();
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Environment read;
  read.readFromFile( in );

  QCOMPARE( read.getGravity().x, 0.5 );
  QCOMPARE( read.getGravity().y, -3.5 );
  QCOMPARE( read.getGravity().z, 1.25 );
  QCOMPARE( read.getVeloDamping(), 0.75 );
  QCOMPARE( read.getStartPosition().x, 2.0 );
  QCOMPARE( read.getStartPosition().y, 3.5 );
  QCOMPARE( read.getStartPosition().z, -4.0 );
  QCOMPARE( read.getFloorDimensionX(), 20 );
  QCOMPARE( read.getFloorDimensionZ(), 30 );
  QCOMPARE( read.getFloorFunction(), QString( "sin(x)*cos(z)" ) );
  QCOMPARE( read.getFloorPictureFile(), QString( "/home/user/my floors/hill.pgm" ) );
  QCOMPARE( read.getFloorFuncSelected(), floorFuncSelected );
  QCOMPARE( read.getTextureFile(), QString( "/home/user/my textures/grass.ppm" ) );
  QCOMPARE( read.getTexAlpha(), 128 );
  QCOMPARE( read.getWithTexture(), withTexture );
  QCOMPARE( read.getAutosaveIntervalGenerations(), 25 );
  QCOMPARE( read.getGroundPlanarSpringConstant(), 1234.5 );
  QCOMPARE( read.getGroundNormalSpringConstant(), 2345.5 );
  QCOMPARE( read.getGroundPlanarDamperConstant(), 12.5 );
  QCOMPARE( read.getGroundNormalDamperConstant(), 23.5 );
  QCOMPARE( read.getFrictionCoeff_u_s(), 0.8 );
  QCOMPARE( read.getFrictionCoeff_u_k(), 0.6 );
}

// A value that a user types may have more than six significant digits.
void SIGEL_Environment::TST_SIG_Environment::writeToFileThenReadFromFileKeepsEveryDigitOfAValue()
{
  SIG_Environment written;
  written.setGroundPlanarSpringConstant( 12345.678 );
  written.setVeloDamping( 0.9876543 );
  QString text;
  QTextStream out( &text );

  written.writeToFile( out );
  out.flush();
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Environment read;
  read.readFromFile( in );

  QEXPECT_FAIL( "", "SIG_Environment::writeToFile writes the doubles with six significant digits", Continue );
  QCOMPARE( read.getGroundPlanarSpringConstant(), 12345.678 );
  QEXPECT_FAIL( "", "SIG_Environment::writeToFile writes the doubles with six significant digits", Continue );
  QCOMPARE( read.getVeloDamping(), 0.9876543 );
}

// The form of the environment block in experiments/runner.exp, which the saved experiments have.
// FLOORMATERIALNAME is written as floor; nothing reads it.
void SIGEL_Environment::TST_SIG_Environment::writeToFileWritesTheBlockInTheFormOfTheExperimentFiles()
{
  SIG_Environment environment;
  environment.setGravity( SIG_Vector( 0, -9.81, 0 ) );
  environment.setVeloDamping( 0.995 );
  environment.setStartPosition( SIG_Vector( 0, 2, 0 ) );
  environment.setFloorDimensionX( 50 );
  environment.setFloorDimensionZ( 40 );
  environment.setFloorFunction( "0" );
  environment.setFloorPictureFile( "" );
  environment.setFloorFuncSelected( true );
  environment.setTextureFile( "" );
  environment.setTexAlpha( 99 );
  environment.setWithTexture( false );
  environment.setAutosaveIntervalGenerations( 0 );
  environment.setGroundPlanarSpringConstant( 550 );
  environment.setGroundNormalSpringConstant( 750 );
  environment.setGroundPlanarDamperConstant( 5 );
  environment.setGroundNormalDamperConstant( 5 );
  environment.setFrictionCoeff_u_s( 3 );
  environment.setFrictionCoeff_u_k( 2.9 );
  QString text;
  QTextStream out( &text );

  environment.writeToFile( out );
  out.flush();

  QCOMPARE( text, QString( "YPLANELEVEL\n0\nVELODAMPING\n0.995\nGRAVITY\n0\n-9.81\n0\nSTARTPOSITION\n0\n2\n0\n"
                           "FLOORDIMENSION\n50\n40\nFLOORFUNCTION\n0\nFLOORPICTUREFILE\n\nFLOORFUNCSELECTED\n1\n"
                           "TEXTUREFILE\n\nTEXALPHA\n99\nWITHTEXTURE\n0\nFLOORMATERIALNAME\nfloor\nAUTOSAVETIME\n0\n"
                           "GPSconst\n550\nGNSconst\n750\nGPDconst\n5\nGNDconst\n5\nUSFcoeff\n3\nUKFcoeff\n2.9\n" ) );
}

// Six of the seven experiments in experiments/ have an environment block of this form, without
// the floor, texture and autosave keys. Those settings keep the values of a new environment.
void SIGEL_Environment::TST_SIG_Environment::readFromFileOfABlockWithoutTheFloorKeysKeepsTheirDefaults()
{
  QString text( "YPLANELEVEL\n0\nVELODAMPING\n0.9\nGRAVITY\n0\n-9.81\n0\nSTARTPOSITION\n0\n1.2\n0\n"
                "FLOORMATERIALNAME\nplastic\nGPSconst\n5500\nGNSconst\n7500\nGPDconst\n50\nGNDconst\n50\n"
                "USFcoeff\n1.5\nUKFcoeff\n1\n\n" );
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Environment defaults;
  SIG_Environment read;

  read.readFromFile( in );

  QCOMPARE( read.getVeloDamping(), 0.9 );
  QCOMPARE( read.getStartPosition().y, 1.2 );
  QCOMPARE( read.getFloorDimensionX(), defaults.getFloorDimensionX() );
  QCOMPARE( read.getFloorDimensionZ(), defaults.getFloorDimensionZ() );
  QCOMPARE( read.getFloorFunction(), defaults.getFloorFunction() );
  QCOMPARE( read.getFloorPictureFile(), defaults.getFloorPictureFile() );
  QCOMPARE( read.getFloorFuncSelected(), defaults.getFloorFuncSelected() );
  QCOMPARE( read.getTextureFile(), defaults.getTextureFile() );
  QCOMPARE( read.getTexAlpha(), defaults.getTexAlpha() );
  QCOMPARE( read.getWithTexture(), defaults.getWithTexture() );
  QCOMPARE( read.getAutosaveIntervalGenerations(), defaults.getAutosaveIntervalGenerations() );
  SIG_Terrain terrain;
  QVERIFY( read.makeTerrain( terrain ) );
}

// The form of experiments/runner.exp, with a value other than the default for each key.
void SIGEL_Environment::TST_SIG_Environment::readFromFileReadsEachKeyOfABlockOfTheCurrentForm()
{
  QString text( "YPLANELEVEL\n0\nVELODAMPING\n0.75\nGRAVITY\n0.5\n-3.5\n1.25\nSTARTPOSITION\n2\n3.5\n-4\n"
                "FLOORDIMENSION\n20\n30\nFLOORFUNCTION\nsin(x)*cos(z)\nFLOORPICTUREFILE\n/home/user/my floors/hill.pgm\n"
                "FLOORFUNCSELECTED\n0\nTEXTUREFILE\n/home/user/my textures/grass.ppm\nTEXALPHA\n99\nWITHTEXTURE\n1\n"
                "FLOORMATERIALNAME\ngreenPlastic\nAUTOSAVETIME\n25\nGPSconst\n550\nGNSconst\n750\nGPDconst\n5\nGNDconst\n6\n"
                "USFcoeff\n3\nUKFcoeff\n2.9\n\n" );
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Environment read;

  read.readFromFile( in );

  QCOMPARE( read.getVeloDamping(), 0.75 );
  QCOMPARE( read.getGravity().x, 0.5 );
  QCOMPARE( read.getGravity().y, -3.5 );
  QCOMPARE( read.getGravity().z, 1.25 );
  QCOMPARE( read.getStartPosition().x, 2.0 );
  QCOMPARE( read.getStartPosition().y, 3.5 );
  QCOMPARE( read.getStartPosition().z, -4.0 );
  QCOMPARE( read.getFloorDimensionX(), 20 );
  QCOMPARE( read.getFloorDimensionZ(), 30 );
  QCOMPARE( read.getFloorFunction(), QString( "sin(x)*cos(z)" ) );
  QCOMPARE( read.getFloorPictureFile(), QString( "/home/user/my floors/hill.pgm" ) );
  QCOMPARE( read.getFloorFuncSelected(), false );
  QCOMPARE( read.getTextureFile(), QString( "/home/user/my textures/grass.ppm" ) );
  QCOMPARE( read.getTexAlpha(), 99 );
  QCOMPARE( read.getWithTexture(), true );
  QCOMPARE( read.getAutosaveIntervalGenerations(), 25 );
  QCOMPARE( read.getGroundPlanarSpringConstant(), 550.0 );
  QCOMPARE( read.getGroundNormalSpringConstant(), 750.0 );
  QCOMPARE( read.getGroundPlanarDamperConstant(), 5.0 );
  QCOMPARE( read.getGroundNormalDamperConstant(), 6.0 );
  QCOMPARE( read.getFrictionCoeff_u_s(), 3.0 );
  QCOMPARE( read.getFrictionCoeff_u_k(), 2.9 );
}

// Environment > Import and the loading of an experiment read into an environment that is in use.
// An experiment file without these keys was made before they existed and ran with their defaults.
void SIGEL_Environment::TST_SIG_Environment::readFromFileOfABlockWithoutTheFloorKeysGivesTheirDefaultsToAnEnvironmentInUse()
{
  QString text( "YPLANELEVEL\n0\nVELODAMPING\n0.9\nGRAVITY\n0\n-9.81\n0\nSTARTPOSITION\n0\n1.2\n0\n"
                "FLOORMATERIALNAME\nplastic\nGPSconst\n5500\nGNSconst\n7500\nGPDconst\n50\nGNDconst\n50\n"
                "USFcoeff\n1.5\nUKFcoeff\n1\n\n" );
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Environment defaults;
  SIG_Environment inUse;
  inUse.setFloorDimensionX( 20 );
  inUse.setFloorDimensionZ( 30 );
  inUse.setFloorFunction( "sin(x)" );
  inUse.setFloorPictureFile( "/home/user/hill.pgm" );
  inUse.setFloorFuncSelected( false );
  inUse.setTextureFile( "/home/user/grass.ppm" );
  inUse.setTexAlpha( 99 );
  inUse.setWithTexture( true );
  inUse.setAutosaveIntervalGenerations( 25 );

  inUse.readFromFile( in );

  QCOMPARE( inUse.getVeloDamping(), 0.9 );
  QEXPECT_FAIL( "", "SIG_Environment::readFromFile leaves a setting whose key the text does not have as it was", Abort );
  QCOMPARE( inUse.getFloorDimensionX(), defaults.getFloorDimensionX() );
  QCOMPARE( inUse.getFloorDimensionZ(), defaults.getFloorDimensionZ() );
  QCOMPARE( inUse.getFloorFunction(), defaults.getFloorFunction() );
  QCOMPARE( inUse.getFloorPictureFile(), defaults.getFloorPictureFile() );
  QCOMPARE( inUse.getFloorFuncSelected(), defaults.getFloorFuncSelected() );
  QCOMPARE( inUse.getTextureFile(), defaults.getTextureFile() );
  QCOMPARE( inUse.getTexAlpha(), defaults.getTexAlpha() );
  QCOMPARE( inUse.getWithTexture(), defaults.getWithTexture() );
  QCOMPARE( inUse.getAutosaveIntervalGenerations(), defaults.getAutosaveIntervalGenerations() );
}
