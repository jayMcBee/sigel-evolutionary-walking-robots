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
#include "SIGEL_Environment/TST_SIG_Terrain.h"

#include "SIGEL_Environment/SIG_Terrain.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

void SIGEL_Environment::TST_SIG_Terrain::everyPresetParses()
{
  for ( SIG_TerrainPreset const &preset : SIG_Terrain::presets() )
  {
    SIG_Terrain terrain;
    QVERIFY2( terrain.setFromFunction( SIG_Terrain::presetFunction( preset, 50, 50 ), 50, 50 ), qPrintable( preset.name ) );
  }
}

void SIGEL_Environment::TST_SIG_Terrain::presetFunctionPutsTheCentreInPlaceOfCxAndCz()
{
  SIG_TerrainPreset const preset = { "Test", "(x-cx)*(z-cz)" };

  QCOMPARE( SIG_Terrain::presetFunction( preset, 50, 30 ), QString( "(x-24.5)*(z-14.5)" ) );
}

void SIGEL_Environment::TST_SIG_Terrain::writeToFileThenReadFromFileGivesTheSameTerrain()
{
  SIG_Terrain written;
  QVERIFY( written.setFromFunction( "sin(x)*cos(z)/3", 5, 3 ) );

  QString text;
  QTextStream out( &text );
  written.writeToFile( out );

  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Terrain read;
  read.readFromFile( in );

  QCOMPARE( read.getDimensionX(), 5 );
  QCOMPARE( read.getDimensionZ(), 3 );

  // Exactly the same doubles, not only close ones.
  for ( int z = 0; z < 3; ++z )
  {
    for ( int x = 0; x < 5; ++x )
    {
      QVERIFY( read.getHeight( x, z ) == written.getHeight( x, z ) );
    }
  }
}

void SIGEL_Environment::TST_SIG_Terrain::readFromFileThrowsWhenAHeightIsMissing()
{
  QString text = "2 2\n1 2\n3\n";
  QTextStream in( &text, QIODevice::ReadOnly );
  SIG_Terrain terrain;

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, terrain.readFromFile( in ) );
}

void SIGEL_Environment::TST_SIG_Terrain::readFromFileThrowsWhenTheTextEndsInsideAHeight()
{
  SIG_Terrain written;
  QVERIFY( written.setFromFunction( "1/3", 1, 1 ) );

  QString text;
  QTextStream out( &text );
  written.writeToFile( out );

  // "0.33333333333333331" cut to "0.333" is still a number.
  QString cut = text.left( text.indexOf( "0.333" ) + 5 );
  QTextStream in( &cut, QIODevice::ReadOnly );
  SIG_Terrain terrain;

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, terrain.readFromFile( in ) );
}

void SIGEL_Environment::TST_SIG_Terrain::setFromPictureFileGivesTheHeightsOfAP2Picture_data()
{
  QTest::addColumn< QByteArray >( "picture" );

  // Netpbm PGM: a comment runs from # to the end of its line, anywhere in the header.
  // The picture is 3 wide and 2 high, and no two of its rows or columns are the same.
  QTest::newRow( "plain" ) << QByteArray( "P2\n3 2\n255\n0 51 255\n255 0 102\n" );
  QTest::newRow( "comments in the header" ) << QByteArray( "# before the magic key\nP2\n3\n# between the sizes\n2 # after the height\n255\n0 51 255\n255 0 102\n" );
  QTest::newRow( "no final line break" ) << QByteArray( "P2\n3 2\n255\n0 51 255\n255 0 102" );
  QTest::newRow( "a maximum grey value other than 255" ) << QByteArray( "P2\n3 2\n50\n0 10 50\n50 0 20\n" );
}

// A grey value of the maximum is a height of 5. The picture's width is dimension X.
void SIGEL_Environment::TST_SIG_Terrain::setFromPictureFileGivesTheHeightsOfAP2Picture()
{
  QFETCH( QByteArray, picture );
  QTemporaryDir folder;
  SIG_Terrain terrain;

  QVERIFY( terrain.setFromPictureFile( writePicture( folder, picture ) ) );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  QCOMPARE( terrain.getHeight( 0, 0 ), 0.0 );
  QCOMPARE( terrain.getHeight( 1, 0 ), 1.0 );
  QCOMPARE( terrain.getHeight( 2, 0 ), 5.0 );
  QCOMPARE( terrain.getHeight( 0, 1 ), 5.0 );
  QCOMPARE( terrain.getHeight( 1, 1 ), 0.0 );
  QCOMPARE( terrain.getHeight( 2, 1 ), 2.0 );
}

void SIGEL_Environment::TST_SIG_Terrain::setFromPictureFileOfAFileThatCannotBeReadChangesNothing_data()
{
  QTest::addColumn< bool >( "fileExists" );
  QTest::addColumn< QByteArray >( "picture" );

  QTest::newRow( "missing file" ) << false << QByteArray();
  QTest::newRow( "a picture that is not P2" ) << true << QByteArray( "P5\n2 2\n255\n0000" );
}

void SIGEL_Environment::TST_SIG_Terrain::setFromPictureFileOfAFileThatCannotBeReadChangesNothing()
{
  QFETCH( bool, fileExists );
  QFETCH( QByteArray, picture );
  QTemporaryDir folder;
  const QString fileName = fileExists ? writePicture( folder, picture ) : folder.filePath( "missing.pgm" );
  SIG_Terrain terrain;
  QVERIFY( terrain.setFromFunction( "x+2*z", 3, 2 ) );

  QVERIFY( !terrain.setFromPictureFile( fileName ) );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  QCOMPARE( terrain.getHeight( 2, 1 ), 4.0 );
}

// The function's x counts along dimension X, its z along dimension Z.
void SIGEL_Environment::TST_SIG_Terrain::setFromFunctionGivesTheHeightsOfTheFunctionAtEachGridPoint()
{
  SIG_Terrain terrain;

  QVERIFY( terrain.setFromFunction( "x+10*z", 3, 2 ) );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  for ( int z = 0; z < 2; z++ )
  {
    for ( int x = 0; x < 3; x++ )
    {
      QCOMPARE( terrain.getHeight( x, z ), x + 10.0 * z );
    }
  }
}

void SIGEL_Environment::TST_SIG_Terrain::setFromFunctionOfAFunctionThatCannotBeParsedChangesNothing()
{
  SIG_Terrain terrain;
  QVERIFY( terrain.setFromFunction( "x+2*z", 3, 2 ) );

  QVERIFY( !terrain.setFromFunction( "sin(x", 5, 5 ) );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 2 );
  QCOMPARE( terrain.getHeight( 2, 1 ), 4.0 );
}

void SIGEL_Environment::TST_SIG_Terrain::newTerrainIsAnEmptyGrid()
{
  SIG_Terrain terrain;

  QCOMPARE( terrain.getDimensionX(), 0 );
  QCOMPARE( terrain.getDimensionZ(), 0 );
}

void SIGEL_Environment::TST_SIG_Terrain::setFlatGivesHeightZeroAtEveryPointOfTheGrid()
{
  SIG_Terrain terrain;
  QVERIFY( terrain.setFromFunction( "1", 2, 2 ) );

  terrain.setFlat( 3, 4 );

  QCOMPARE( terrain.getDimensionX(), 3 );
  QCOMPARE( terrain.getDimensionZ(), 4 );
  for ( int z = 0; z < 4; z++ )
  {
    for ( int x = 0; x < 3; x++ )
    {
      QCOMPARE( terrain.getHeight( x, z ), 0.0 );
    }
  }
  QVERIFY( terrain.isFlat() );
}

// The one raised point is neither the first nor the last of the grid.
void SIGEL_Environment::TST_SIG_Terrain::isFlatIsFalseWhenOnePointIsNotAtHeightZero()
{
  QTemporaryDir folder;
  SIG_Terrain terrain;
  QVERIFY( terrain.setFromPictureFile( writePicture( folder, "P2\n3 3\n255\n0 0 0\n0 51 0\n0 0 0\n" ) ) );

  QVERIFY( !terrain.isFlat() );
}

// Writes picture to floor.pgm in folder and returns that file's path.
QString SIGEL_Environment::TST_SIG_Terrain::writePicture( const QTemporaryDir &folder, const QByteArray &picture )
{
  const QString fileName = folder.filePath( "floor.pgm" );
  QFile file( fileName );
  if ( !folder.isValid() || !file.open( QIODevice::WriteOnly ) )
  {
    QTest::qFail( qPrintable( "cannot write " + fileName ), __FILE__, __LINE__ );
    return fileName;
  }
  file.write( picture );
  return fileName;
}
