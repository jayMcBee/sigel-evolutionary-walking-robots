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
#include "SIGEL_Simulation/SIG_RegisterTest.h"

#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Simulation/SIG_RegisterWrongSizeException.h"

#include <QList>
#include <QtTest>

void SIGEL_Simulation::SIG_RegisterTest::newRegisterHasValueZeroAndItsWidth()
{
  const QList<int> widths = { 1, 3, 8, 16 };

  for ( int width : widths )
    {
      SIG_Register newRegister( width );

      QCOMPARE( newRegister.getValue(), 0 );
      QCOMPARE( newRegister.getSize(), width );
    }
}

void SIGEL_Simulation::SIG_RegisterTest::constructorRefusesWidthOutsideOneToSixteen()
{
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register negativeWidth( -1 ) );
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register noWidth( 0 ) );
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register tooWide( 17 ) );
}

void SIGEL_Simulation::SIG_RegisterTest::getMinValueAndGetMaxValueGiveTheRange()
{
  SIG_Register oneBit( 1 );
  QCOMPARE( oneBit.getMinValue(), -1 );
  QCOMPARE( oneBit.getMaxValue(), 0 );

  SIG_Register threeBits( 3 );
  QCOMPARE( threeBits.getMinValue(), -4 );
  QCOMPARE( threeBits.getMaxValue(), 3 );

  SIG_Register eightBits( 8 );
  QCOMPARE( eightBits.getMinValue(), -128 );
  QCOMPARE( eightBits.getMaxValue(), 127 );

  SIG_Register sixteenBits( 16 );
  QCOMPARE( sixteenBits.getMinValue(), -32768 );
  QCOMPARE( sixteenBits.getMaxValue(), 32767 );
}

void SIGEL_Simulation::SIG_RegisterTest::loadValueWrapsToTheWidth_data()
{
  QTest::addColumn< int >( "width" );
  QTest::addColumn< int >( "value" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "width 1: 0" ) << 1 << 0 << 0;
  QTest::newRow( "width 1: 1" ) << 1 << 1 << -1;
  QTest::newRow( "width 1: -1" ) << 1 << -1 << -1;
  QTest::newRow( "width 1: 127" ) << 1 << 127 << -1;
  QTest::newRow( "width 1: 128" ) << 1 << 128 << 0;
  QTest::newRow( "width 1: -128" ) << 1 << -128 << 0;
  QTest::newRow( "width 1: -129" ) << 1 << -129 << -1;
  QTest::newRow( "width 1: 200" ) << 1 << 200 << 0;
  QTest::newRow( "width 1: -200" ) << 1 << -200 << 0;
  QTest::newRow( "width 1: 255" ) << 1 << 255 << -1;
  QTest::newRow( "width 1: 256" ) << 1 << 256 << 0;
  QTest::newRow( "width 1: -256" ) << 1 << -256 << 0;
  QTest::newRow( "width 1: 32000" ) << 1 << 32000 << 0;
  QTest::newRow( "width 1: -31861" ) << 1 << -31861 << -1;
  QTest::newRow( "width 1: 31999" ) << 1 << 31999 << -1;
  QTest::newRow( "width 1: -31999" ) << 1 << -31999 << -1;
  QTest::newRow( "width 1: 3" ) << 1 << 3 << -1;
  QTest::newRow( "width 1: 4" ) << 1 << 4 << 0;
  QTest::newRow( "width 1: -4" ) << 1 << -4 << 0;
  QTest::newRow( "width 1: -5" ) << 1 << -5 << -1;
  QTest::newRow( "width 1: 32767" ) << 1 << 32767 << -1;
  QTest::newRow( "width 1: 32768" ) << 1 << 32768 << 0;
  QTest::newRow( "width 1: -32768" ) << 1 << -32768 << 0;
  QTest::newRow( "width 1: -32769" ) << 1 << -32769 << -1;

  QTest::newRow( "width 3: 0" ) << 3 << 0 << 0;
  QTest::newRow( "width 3: 1" ) << 3 << 1 << 1;
  QTest::newRow( "width 3: -1" ) << 3 << -1 << -1;
  QTest::newRow( "width 3: 127" ) << 3 << 127 << -1;
  QTest::newRow( "width 3: 128" ) << 3 << 128 << 0;
  QTest::newRow( "width 3: -128" ) << 3 << -128 << 0;
  QTest::newRow( "width 3: -129" ) << 3 << -129 << -1;
  QTest::newRow( "width 3: 200" ) << 3 << 200 << 0;
  QTest::newRow( "width 3: -200" ) << 3 << -200 << 0;
  QTest::newRow( "width 3: 255" ) << 3 << 255 << -1;
  QTest::newRow( "width 3: 256" ) << 3 << 256 << 0;
  QTest::newRow( "width 3: -256" ) << 3 << -256 << 0;
  QTest::newRow( "width 3: 32000" ) << 3 << 32000 << 0;
  QTest::newRow( "width 3: -31861" ) << 3 << -31861 << 3;
  QTest::newRow( "width 3: 31999" ) << 3 << 31999 << -1;
  QTest::newRow( "width 3: -31999" ) << 3 << -31999 << 1;
  QTest::newRow( "width 3: 3" ) << 3 << 3 << 3;
  QTest::newRow( "width 3: 4" ) << 3 << 4 << -4;
  QTest::newRow( "width 3: -4" ) << 3 << -4 << -4;
  QTest::newRow( "width 3: -5" ) << 3 << -5 << 3;
  QTest::newRow( "width 3: 32767" ) << 3 << 32767 << -1;
  QTest::newRow( "width 3: 32768" ) << 3 << 32768 << 0;
  QTest::newRow( "width 3: -32768" ) << 3 << -32768 << 0;
  QTest::newRow( "width 3: -32769" ) << 3 << -32769 << -1;

  QTest::newRow( "width 8: 0" ) << 8 << 0 << 0;
  QTest::newRow( "width 8: 1" ) << 8 << 1 << 1;
  QTest::newRow( "width 8: -1" ) << 8 << -1 << -1;
  QTest::newRow( "width 8: 127" ) << 8 << 127 << 127;
  QTest::newRow( "width 8: 128" ) << 8 << 128 << -128;
  QTest::newRow( "width 8: -128" ) << 8 << -128 << -128;
  QTest::newRow( "width 8: -129" ) << 8 << -129 << 127;
  QTest::newRow( "width 8: 200" ) << 8 << 200 << -56;
  QTest::newRow( "width 8: -200" ) << 8 << -200 << 56;
  QTest::newRow( "width 8: 255" ) << 8 << 255 << -1;
  QTest::newRow( "width 8: 256" ) << 8 << 256 << 0;
  QTest::newRow( "width 8: -256" ) << 8 << -256 << 0;
  QTest::newRow( "width 8: 32000" ) << 8 << 32000 << 0;
  QTest::newRow( "width 8: -31861" ) << 8 << -31861 << -117;
  QTest::newRow( "width 8: 31999" ) << 8 << 31999 << -1;
  QTest::newRow( "width 8: -31999" ) << 8 << -31999 << 1;
  QTest::newRow( "width 8: 3" ) << 8 << 3 << 3;
  QTest::newRow( "width 8: 4" ) << 8 << 4 << 4;
  QTest::newRow( "width 8: -4" ) << 8 << -4 << -4;
  QTest::newRow( "width 8: -5" ) << 8 << -5 << -5;
  QTest::newRow( "width 8: 32767" ) << 8 << 32767 << -1;
  QTest::newRow( "width 8: 32768" ) << 8 << 32768 << 0;
  QTest::newRow( "width 8: -32768" ) << 8 << -32768 << 0;
  QTest::newRow( "width 8: -32769" ) << 8 << -32769 << -1;

  QTest::newRow( "width 16: 0" ) << 16 << 0 << 0;
  QTest::newRow( "width 16: 1" ) << 16 << 1 << 1;
  QTest::newRow( "width 16: -1" ) << 16 << -1 << -1;
  QTest::newRow( "width 16: 127" ) << 16 << 127 << 127;
  QTest::newRow( "width 16: 128" ) << 16 << 128 << 128;
  QTest::newRow( "width 16: -128" ) << 16 << -128 << -128;
  QTest::newRow( "width 16: -129" ) << 16 << -129 << -129;
  QTest::newRow( "width 16: 200" ) << 16 << 200 << 200;
  QTest::newRow( "width 16: -200" ) << 16 << -200 << -200;
  QTest::newRow( "width 16: 255" ) << 16 << 255 << 255;
  QTest::newRow( "width 16: 256" ) << 16 << 256 << 256;
  QTest::newRow( "width 16: -256" ) << 16 << -256 << -256;
  QTest::newRow( "width 16: 32000" ) << 16 << 32000 << 32000;
  QTest::newRow( "width 16: -31861" ) << 16 << -31861 << -31861;
  QTest::newRow( "width 16: 31999" ) << 16 << 31999 << 31999;
  QTest::newRow( "width 16: -31999" ) << 16 << -31999 << -31999;
  QTest::newRow( "width 16: 3" ) << 16 << 3 << 3;
  QTest::newRow( "width 16: 4" ) << 16 << 4 << 4;
  QTest::newRow( "width 16: -4" ) << 16 << -4 << -4;
  QTest::newRow( "width 16: -5" ) << 16 << -5 << -5;
  QTest::newRow( "width 16: 32767" ) << 16 << 32767 << 32767;
  QTest::newRow( "width 16: 32768" ) << 16 << 32768 << -32768;
  QTest::newRow( "width 16: -32768" ) << 16 << -32768 << -32768;
  QTest::newRow( "width 16: -32769" ) << 16 << -32769 << 32767;
}

void SIGEL_Simulation::SIG_RegisterTest::loadValueWrapsToTheWidth()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, expected );

  SIG_Register loadedRegister( width );
  loadedRegister.loadValue( value );

  QCOMPARE( loadedRegister.getValue(), expected );
}
