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
    for ( int x = 0; x < 5; ++x )
      QVERIFY( read.getHeight( x, z ) == written.getHeight( x, z ) );
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
