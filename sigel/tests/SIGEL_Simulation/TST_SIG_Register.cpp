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
#include "SIGEL_Simulation/TST_SIG_Register.h"

#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Simulation/SIG_RegisterWrongSizeException.h"

#include <QList>
#include <QtTest>

void SIGEL_Simulation::TST_SIG_Register::newRegisterHasValueZeroAndItsWidth()
{
  const QList<int> widths = { 1, 3, 8, 16 };

  for ( int width : widths )
    {
      SIG_Register newRegister( width );

      QCOMPARE( newRegister.getValue(), 0 );
      QCOMPARE( newRegister.getSize(), width );
    }
}

void SIGEL_Simulation::TST_SIG_Register::constructorRefusesWidthOutsideOneToSixteen()
{
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register negativeWidth( -1 ) );
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register noWidth( 0 ) );
  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Register tooWide( 17 ) );
}

void SIGEL_Simulation::TST_SIG_Register::getMinValueAndGetMaxValueGiveTheRange()
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

void SIGEL_Simulation::TST_SIG_Register::loadValueWrapsToTheWidth_data()
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

void SIGEL_Simulation::TST_SIG_Register::loadValueWrapsToTheWidth()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, expected );

  SIG_Register loadedRegister( width );
  loadedRegister.loadValue( value );

  QCOMPARE( loadedRegister.getValue(), expected );
}

void SIGEL_Simulation::TST_SIG_Register::copyRegGivesTheExpectedValue_data()
{
  QTest::addColumn< int >( "width" );
  QTest::addColumn< int >( "value" );
  QTest::addColumn< int >( "other" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "width 8: 5, 3" ) << 8 << 5 << 3 << 3;
  QTest::newRow( "width 8: 3, 5" ) << 8 << 3 << 5 << 5;
  QTest::newRow( "width 8: -5, 3" ) << 8 << -5 << 3 << 3;
  QTest::newRow( "width 8: 5, -3" ) << 8 << 5 << -3 << -3;
  QTest::newRow( "width 8: -5, -3" ) << 8 << -5 << -3 << -3;
  QTest::newRow( "width 8: 0, 0" ) << 8 << 0 << 0 << 0;
  QTest::newRow( "width 8: 7, 0" ) << 8 << 7 << 0 << 0;
  QTest::newRow( "width 8: -7, 0" ) << 8 << -7 << 0 << 0;
  QTest::newRow( "width 8: 0, 7" ) << 8 << 0 << 7 << 7;
  QTest::newRow( "width 8: 127, 1" ) << 8 << 127 << 1 << 1;
  QTest::newRow( "width 8: 100, 100" ) << 8 << 100 << 100 << 100;
  QTest::newRow( "width 8: -128, 1" ) << 8 << -128 << 1 << 1;
  QTest::newRow( "width 8: -128, -1" ) << 8 << -128 << -1 << -1;
  QTest::newRow( "width 8: -100, -100" ) << 8 << -100 << -100 << -100;
  QTest::newRow( "width 8: -100, 100" ) << 8 << -100 << 100 << 100;
  QTest::newRow( "width 8: 127, 127" ) << 8 << 127 << 127 << 127;
  QTest::newRow( "width 8: -128, -128" ) << 8 << -128 << -128 << -128;
  QTest::newRow( "width 8: -128, 127" ) << 8 << -128 << 127 << 127;
  QTest::newRow( "width 8: 127, -128" ) << 8 << 127 << -128 << -128;
  QTest::newRow( "width 8: 16, 16" ) << 8 << 16 << 16 << 16;
  QTest::newRow( "width 8: 7, 2" ) << 8 << 7 << 2 << 2;
  QTest::newRow( "width 8: -7, 2" ) << 8 << -7 << 2 << 2;
  QTest::newRow( "width 8: 7, -2" ) << 8 << 7 << -2 << -2;
  QTest::newRow( "width 8: -7, -2" ) << 8 << -7 << -2 << -2;
  QTest::newRow( "width 8: 7, 3" ) << 8 << 7 << 3 << 3;
  QTest::newRow( "width 8: -7, 3" ) << 8 << -7 << 3 << 3;
  QTest::newRow( "width 8: 7, -3" ) << 8 << 7 << -3 << -3;
  QTest::newRow( "width 8: -7, -3" ) << 8 << -7 << -3 << -3;
  QTest::newRow( "width 8: -6, 3" ) << 8 << -6 << 3 << 3;
  QTest::newRow( "width 8: -4, 2" ) << 8 << -4 << 2 << 2;
  QTest::newRow( "width 8: -1, -128" ) << 8 << -1 << -128 << -128;
  QTest::newRow( "width 8: 5, -128" ) << 8 << 5 << -128 << -128;
  QTest::newRow( "width 8: 6, 3" ) << 8 << 6 << 3 << 3;
  QTest::newRow( "width 8: 2, 7" ) << 8 << 2 << 7 << 7;
  QTest::newRow( "width 8: -2, 7" ) << 8 << -2 << 7 << 7;
  QTest::newRow( "width 8: 5, 5" ) << 8 << 5 << 5 << 5;
  QTest::newRow( "width 8: -5, -5" ) << 8 << -5 << -5 << -5;

  QTest::newRow( "width 1: 0, 0" ) << 1 << 0 << 0 << 0;
  QTest::newRow( "width 1: -1, -1" ) << 1 << -1 << -1 << -1;
  QTest::newRow( "width 1: -1, 0" ) << 1 << -1 << 0 << 0;
  QTest::newRow( "width 1: 0, -1" ) << 1 << 0 << -1 << -1;

  QTest::newRow( "width 3: 3, 1" ) << 3 << 3 << 1 << 1;
  QTest::newRow( "width 3: -4, 1" ) << 3 << -4 << 1 << 1;
  QTest::newRow( "width 3: 3, 3" ) << 3 << 3 << 3 << 3;
  QTest::newRow( "width 3: -4, -4" ) << 3 << -4 << -4 << -4;
  QTest::newRow( "width 3: -4, -1" ) << 3 << -4 << -1 << -1;
  QTest::newRow( "width 3: 3, 2" ) << 3 << 3 << 2 << 2;
  QTest::newRow( "width 3: -3, 2" ) << 3 << -3 << 2 << 2;

  QTest::newRow( "width 16: 32767, 1" ) << 16 << 32767 << 1 << 1;
  QTest::newRow( "width 16: -32768, 1" ) << 16 << -32768 << 1 << 1;
  QTest::newRow( "width 16: -32768, -1" ) << 16 << -32768 << -1 << -1;
  QTest::newRow( "width 16: 32767, 32767" ) << 16 << 32767 << 32767 << 32767;
  QTest::newRow( "width 16: -32768, -32768" ) << 16 << -32768 << -32768 << -32768;
  QTest::newRow( "width 16: -32768, 32767" ) << 16 << -32768 << 32767 << 32767;
  QTest::newRow( "width 16: 1000, 7" ) << 16 << 1000 << 7 << 7;
  QTest::newRow( "width 16: -1000, 7" ) << 16 << -1000 << 7 << 7;
}

void SIGEL_Simulation::TST_SIG_Register::copyRegGivesTheExpectedValue()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, other );
  QFETCH( int, expected );

  SIG_Register firstRegister( width );
  firstRegister.loadValue( value );
  SIG_Register otherRegister( width );
  otherRegister.loadValue( other );

  firstRegister.copyReg( otherRegister );

  QCOMPARE( firstRegister.getValue(), expected );
  QCOMPARE( otherRegister.getValue(), other );
}

void SIGEL_Simulation::TST_SIG_Register::addRegGivesTheExpectedValue_data()
{
  QTest::addColumn< int >( "width" );
  QTest::addColumn< int >( "value" );
  QTest::addColumn< int >( "other" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "width 8: 5, 3" ) << 8 << 5 << 3 << 8;
  QTest::newRow( "width 8: 3, 5" ) << 8 << 3 << 5 << 8;
  QTest::newRow( "width 8: -5, 3" ) << 8 << -5 << 3 << -2;
  QTest::newRow( "width 8: 5, -3" ) << 8 << 5 << -3 << 2;
  QTest::newRow( "width 8: -5, -3" ) << 8 << -5 << -3 << -8;
  QTest::newRow( "width 8: 0, 0" ) << 8 << 0 << 0 << 0;
  QTest::newRow( "width 8: 7, 0" ) << 8 << 7 << 0 << 7;
  QTest::newRow( "width 8: -7, 0" ) << 8 << -7 << 0 << -7;
  QTest::newRow( "width 8: 0, 7" ) << 8 << 0 << 7 << 7;
  QTest::newRow( "width 8: 127, 1" ) << 8 << 127 << 1 << -128;
  QTest::newRow( "width 8: 100, 100" ) << 8 << 100 << 100 << -56;
  QTest::newRow( "width 8: -128, 1" ) << 8 << -128 << 1 << -127;
  QTest::newRow( "width 8: -128, -1" ) << 8 << -128 << -1 << 127;
  QTest::newRow( "width 8: -100, -100" ) << 8 << -100 << -100 << 56;
  QTest::newRow( "width 8: -100, 100" ) << 8 << -100 << 100 << 0;
  QTest::newRow( "width 8: 127, 127" ) << 8 << 127 << 127 << -2;
  QTest::newRow( "width 8: -128, -128" ) << 8 << -128 << -128 << 0;
  QTest::newRow( "width 8: -128, 127" ) << 8 << -128 << 127 << -1;
  QTest::newRow( "width 8: 127, -128" ) << 8 << 127 << -128 << -1;
  QTest::newRow( "width 8: 16, 16" ) << 8 << 16 << 16 << 32;
  QTest::newRow( "width 8: 7, 2" ) << 8 << 7 << 2 << 9;
  QTest::newRow( "width 8: -7, 2" ) << 8 << -7 << 2 << -5;
  QTest::newRow( "width 8: 7, -2" ) << 8 << 7 << -2 << 5;
  QTest::newRow( "width 8: -7, -2" ) << 8 << -7 << -2 << -9;
  QTest::newRow( "width 8: 7, 3" ) << 8 << 7 << 3 << 10;
  QTest::newRow( "width 8: -7, 3" ) << 8 << -7 << 3 << -4;
  QTest::newRow( "width 8: 7, -3" ) << 8 << 7 << -3 << 4;
  QTest::newRow( "width 8: -7, -3" ) << 8 << -7 << -3 << -10;
  QTest::newRow( "width 8: -6, 3" ) << 8 << -6 << 3 << -3;
  QTest::newRow( "width 8: -4, 2" ) << 8 << -4 << 2 << -2;
  QTest::newRow( "width 8: -1, -128" ) << 8 << -1 << -128 << 127;
  QTest::newRow( "width 8: 5, -128" ) << 8 << 5 << -128 << -123;
  QTest::newRow( "width 8: 6, 3" ) << 8 << 6 << 3 << 9;
  QTest::newRow( "width 8: 2, 7" ) << 8 << 2 << 7 << 9;
  QTest::newRow( "width 8: -2, 7" ) << 8 << -2 << 7 << 5;
  QTest::newRow( "width 8: 5, 5" ) << 8 << 5 << 5 << 10;
  QTest::newRow( "width 8: -5, -5" ) << 8 << -5 << -5 << -10;

  QTest::newRow( "width 1: 0, 0" ) << 1 << 0 << 0 << 0;
  QTest::newRow( "width 1: -1, -1" ) << 1 << -1 << -1 << 0;
  QTest::newRow( "width 1: -1, 0" ) << 1 << -1 << 0 << -1;
  QTest::newRow( "width 1: 0, -1" ) << 1 << 0 << -1 << -1;

  QTest::newRow( "width 3: 3, 1" ) << 3 << 3 << 1 << -4;
  QTest::newRow( "width 3: -4, 1" ) << 3 << -4 << 1 << -3;
  QTest::newRow( "width 3: 3, 3" ) << 3 << 3 << 3 << -2;
  QTest::newRow( "width 3: -4, -4" ) << 3 << -4 << -4 << 0;
  QTest::newRow( "width 3: -4, -1" ) << 3 << -4 << -1 << 3;
  QTest::newRow( "width 3: 3, 2" ) << 3 << 3 << 2 << -3;
  QTest::newRow( "width 3: -3, 2" ) << 3 << -3 << 2 << -1;

  QTest::newRow( "width 16: 32767, 1" ) << 16 << 32767 << 1 << -32768;
  QTest::newRow( "width 16: -32768, 1" ) << 16 << -32768 << 1 << -32767;
  QTest::newRow( "width 16: -32768, -1" ) << 16 << -32768 << -1 << 32767;
  QTest::newRow( "width 16: 32767, 32767" ) << 16 << 32767 << 32767 << -2;
  QTest::newRow( "width 16: -32768, -32768" ) << 16 << -32768 << -32768 << 0;
  QTest::newRow( "width 16: -32768, 32767" ) << 16 << -32768 << 32767 << -1;
  QTest::newRow( "width 16: 1000, 7" ) << 16 << 1000 << 7 << 1007;
  QTest::newRow( "width 16: -1000, 7" ) << 16 << -1000 << 7 << -993;
}

void SIGEL_Simulation::TST_SIG_Register::addRegGivesTheExpectedValue()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, other );
  QFETCH( int, expected );

  SIG_Register firstRegister( width );
  firstRegister.loadValue( value );
  SIG_Register otherRegister( width );
  otherRegister.loadValue( other );

  firstRegister.addReg( otherRegister );

  QCOMPARE( firstRegister.getValue(), expected );
  QCOMPARE( otherRegister.getValue(), other );
}

void SIGEL_Simulation::TST_SIG_Register::subRegGivesTheExpectedValue_data()
{
  QTest::addColumn< int >( "width" );
  QTest::addColumn< int >( "value" );
  QTest::addColumn< int >( "other" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "width 8: 5, 3" ) << 8 << 5 << 3 << 2;
  QTest::newRow( "width 8: 3, 5" ) << 8 << 3 << 5 << -2;
  QTest::newRow( "width 8: -5, 3" ) << 8 << -5 << 3 << -8;
  QTest::newRow( "width 8: 5, -3" ) << 8 << 5 << -3 << 8;
  QTest::newRow( "width 8: -5, -3" ) << 8 << -5 << -3 << -2;
  QTest::newRow( "width 8: 0, 0" ) << 8 << 0 << 0 << 0;
  QTest::newRow( "width 8: 7, 0" ) << 8 << 7 << 0 << 7;
  QTest::newRow( "width 8: -7, 0" ) << 8 << -7 << 0 << -7;
  QTest::newRow( "width 8: 0, 7" ) << 8 << 0 << 7 << -7;
  QTest::newRow( "width 8: 127, 1" ) << 8 << 127 << 1 << 126;
  QTest::newRow( "width 8: 100, 100" ) << 8 << 100 << 100 << 0;
  QTest::newRow( "width 8: -128, 1" ) << 8 << -128 << 1 << 127;
  QTest::newRow( "width 8: -128, -1" ) << 8 << -128 << -1 << -127;
  QTest::newRow( "width 8: -100, -100" ) << 8 << -100 << -100 << 0;
  QTest::newRow( "width 8: -100, 100" ) << 8 << -100 << 100 << 56;
  QTest::newRow( "width 8: 127, 127" ) << 8 << 127 << 127 << 0;
  QTest::newRow( "width 8: -128, -128" ) << 8 << -128 << -128 << 0;
  QTest::newRow( "width 8: -128, 127" ) << 8 << -128 << 127 << 1;
  QTest::newRow( "width 8: 127, -128" ) << 8 << 127 << -128 << -1;
  QTest::newRow( "width 8: 16, 16" ) << 8 << 16 << 16 << 0;
  QTest::newRow( "width 8: 7, 2" ) << 8 << 7 << 2 << 5;
  QTest::newRow( "width 8: -7, 2" ) << 8 << -7 << 2 << -9;
  QTest::newRow( "width 8: 7, -2" ) << 8 << 7 << -2 << 9;
  QTest::newRow( "width 8: -7, -2" ) << 8 << -7 << -2 << -5;
  QTest::newRow( "width 8: 7, 3" ) << 8 << 7 << 3 << 4;
  QTest::newRow( "width 8: -7, 3" ) << 8 << -7 << 3 << -10;
  QTest::newRow( "width 8: 7, -3" ) << 8 << 7 << -3 << 10;
  QTest::newRow( "width 8: -7, -3" ) << 8 << -7 << -3 << -4;
  QTest::newRow( "width 8: -6, 3" ) << 8 << -6 << 3 << -9;
  QTest::newRow( "width 8: -4, 2" ) << 8 << -4 << 2 << -6;
  QTest::newRow( "width 8: -1, -128" ) << 8 << -1 << -128 << 127;
  QTest::newRow( "width 8: 5, -128" ) << 8 << 5 << -128 << -123;
  QTest::newRow( "width 8: 6, 3" ) << 8 << 6 << 3 << 3;
  QTest::newRow( "width 8: 2, 7" ) << 8 << 2 << 7 << -5;
  QTest::newRow( "width 8: -2, 7" ) << 8 << -2 << 7 << -9;
  QTest::newRow( "width 8: 5, 5" ) << 8 << 5 << 5 << 0;
  QTest::newRow( "width 8: -5, -5" ) << 8 << -5 << -5 << 0;

  QTest::newRow( "width 1: 0, 0" ) << 1 << 0 << 0 << 0;
  QTest::newRow( "width 1: -1, -1" ) << 1 << -1 << -1 << 0;
  QTest::newRow( "width 1: -1, 0" ) << 1 << -1 << 0 << -1;
  QTest::newRow( "width 1: 0, -1" ) << 1 << 0 << -1 << -1;

  QTest::newRow( "width 3: 3, 1" ) << 3 << 3 << 1 << 2;
  QTest::newRow( "width 3: -4, 1" ) << 3 << -4 << 1 << 3;
  QTest::newRow( "width 3: 3, 3" ) << 3 << 3 << 3 << 0;
  QTest::newRow( "width 3: -4, -4" ) << 3 << -4 << -4 << 0;
  QTest::newRow( "width 3: -4, -1" ) << 3 << -4 << -1 << -3;
  QTest::newRow( "width 3: 3, 2" ) << 3 << 3 << 2 << 1;
  QTest::newRow( "width 3: -3, 2" ) << 3 << -3 << 2 << 3;

  QTest::newRow( "width 16: 32767, 1" ) << 16 << 32767 << 1 << 32766;
  QTest::newRow( "width 16: -32768, 1" ) << 16 << -32768 << 1 << 32767;
  QTest::newRow( "width 16: -32768, -1" ) << 16 << -32768 << -1 << -32767;
  QTest::newRow( "width 16: 32767, 32767" ) << 16 << 32767 << 32767 << 0;
  QTest::newRow( "width 16: -32768, -32768" ) << 16 << -32768 << -32768 << 0;
  QTest::newRow( "width 16: -32768, 32767" ) << 16 << -32768 << 32767 << 1;
  QTest::newRow( "width 16: 1000, 7" ) << 16 << 1000 << 7 << 993;
  QTest::newRow( "width 16: -1000, 7" ) << 16 << -1000 << 7 << -1007;
}

void SIGEL_Simulation::TST_SIG_Register::subRegGivesTheExpectedValue()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, other );
  QFETCH( int, expected );

  SIG_Register firstRegister( width );
  firstRegister.loadValue( value );
  SIG_Register otherRegister( width );
  otherRegister.loadValue( other );

  firstRegister.subReg( otherRegister );

  QCOMPARE( firstRegister.getValue(), expected );
  QCOMPARE( otherRegister.getValue(), other );
}

void SIGEL_Simulation::TST_SIG_Register::mulRegGivesTheExpectedValue_data()
{
  QTest::addColumn< int >( "width" );
  QTest::addColumn< int >( "value" );
  QTest::addColumn< int >( "other" );
  QTest::addColumn< int >( "expected" );

  QTest::newRow( "width 8: 5, 3" ) << 8 << 5 << 3 << 15;
  QTest::newRow( "width 8: 3, 5" ) << 8 << 3 << 5 << 15;
  QTest::newRow( "width 8: -5, 3" ) << 8 << -5 << 3 << -15;
  QTest::newRow( "width 8: 5, -3" ) << 8 << 5 << -3 << -15;
  QTest::newRow( "width 8: -5, -3" ) << 8 << -5 << -3 << 15;
  QTest::newRow( "width 8: 0, 0" ) << 8 << 0 << 0 << 0;
  QTest::newRow( "width 8: 7, 0" ) << 8 << 7 << 0 << 0;
  QTest::newRow( "width 8: -7, 0" ) << 8 << -7 << 0 << 0;
  QTest::newRow( "width 8: 0, 7" ) << 8 << 0 << 7 << 0;
  QTest::newRow( "width 8: 127, 1" ) << 8 << 127 << 1 << 127;
  QTest::newRow( "width 8: 100, 100" ) << 8 << 100 << 100 << 16;
  QTest::newRow( "width 8: -128, 1" ) << 8 << -128 << 1 << -128;
  QTest::newRow( "width 8: -128, -1" ) << 8 << -128 << -1 << -128;
  QTest::newRow( "width 8: -100, -100" ) << 8 << -100 << -100 << 16;
  QTest::newRow( "width 8: -100, 100" ) << 8 << -100 << 100 << -16;
  QTest::newRow( "width 8: 127, 127" ) << 8 << 127 << 127 << 1;
  QTest::newRow( "width 8: -128, -128" ) << 8 << -128 << -128 << 0;
  QTest::newRow( "width 8: -128, 127" ) << 8 << -128 << 127 << -128;
  QTest::newRow( "width 8: 127, -128" ) << 8 << 127 << -128 << -128;
  QTest::newRow( "width 8: 16, 16" ) << 8 << 16 << 16 << 0;
  QTest::newRow( "width 8: 7, 2" ) << 8 << 7 << 2 << 14;
  QTest::newRow( "width 8: -7, 2" ) << 8 << -7 << 2 << -14;
  QTest::newRow( "width 8: 7, -2" ) << 8 << 7 << -2 << -14;
  QTest::newRow( "width 8: -7, -2" ) << 8 << -7 << -2 << 14;
  QTest::newRow( "width 8: 7, 3" ) << 8 << 7 << 3 << 21;
  QTest::newRow( "width 8: -7, 3" ) << 8 << -7 << 3 << -21;
  QTest::newRow( "width 8: 7, -3" ) << 8 << 7 << -3 << -21;
  QTest::newRow( "width 8: -7, -3" ) << 8 << -7 << -3 << 21;
  QTest::newRow( "width 8: -6, 3" ) << 8 << -6 << 3 << -18;
  QTest::newRow( "width 8: -4, 2" ) << 8 << -4 << 2 << -8;
  QTest::newRow( "width 8: -1, -128" ) << 8 << -1 << -128 << -128;
  QTest::newRow( "width 8: 5, -128" ) << 8 << 5 << -128 << -128;
  QTest::newRow( "width 8: 6, 3" ) << 8 << 6 << 3 << 18;
  QTest::newRow( "width 8: 2, 7" ) << 8 << 2 << 7 << 14;
  QTest::newRow( "width 8: -2, 7" ) << 8 << -2 << 7 << -14;
  QTest::newRow( "width 8: 5, 5" ) << 8 << 5 << 5 << 25;
  QTest::newRow( "width 8: -5, -5" ) << 8 << -5 << -5 << 25;

  QTest::newRow( "width 1: 0, 0" ) << 1 << 0 << 0 << 0;
  QTest::newRow( "width 1: -1, -1" ) << 1 << -1 << -1 << -1;
  QTest::newRow( "width 1: -1, 0" ) << 1 << -1 << 0 << 0;
  QTest::newRow( "width 1: 0, -1" ) << 1 << 0 << -1 << 0;

  QTest::newRow( "width 3: 3, 1" ) << 3 << 3 << 1 << 3;
  QTest::newRow( "width 3: -4, 1" ) << 3 << -4 << 1 << -4;
  QTest::newRow( "width 3: 3, 3" ) << 3 << 3 << 3 << 1;
  QTest::newRow( "width 3: -4, -4" ) << 3 << -4 << -4 << 0;
  QTest::newRow( "width 3: -4, -1" ) << 3 << -4 << -1 << -4;
  QTest::newRow( "width 3: 3, 2" ) << 3 << 3 << 2 << -2;
  QTest::newRow( "width 3: -3, 2" ) << 3 << -3 << 2 << 2;

  QTest::newRow( "width 16: 32767, 1" ) << 16 << 32767 << 1 << 32767;
  QTest::newRow( "width 16: -32768, 1" ) << 16 << -32768 << 1 << -32768;
  QTest::newRow( "width 16: -32768, -1" ) << 16 << -32768 << -1 << -32768;
  QTest::newRow( "width 16: 32767, 32767" ) << 16 << 32767 << 32767 << 1;
  QTest::newRow( "width 16: -32768, -32768" ) << 16 << -32768 << -32768 << 0;
  QTest::newRow( "width 16: -32768, 32767" ) << 16 << -32768 << 32767 << -32768;
  QTest::newRow( "width 16: 1000, 7" ) << 16 << 1000 << 7 << 7000;
  QTest::newRow( "width 16: -1000, 7" ) << 16 << -1000 << 7 << -7000;
}

void SIGEL_Simulation::TST_SIG_Register::mulRegGivesTheExpectedValue()
{
  QFETCH( int, width );
  QFETCH( int, value );
  QFETCH( int, other );
  QFETCH( int, expected );

  SIG_Register firstRegister( width );
  firstRegister.loadValue( value );
  SIG_Register otherRegister( width );
  otherRegister.loadValue( other );

  firstRegister.mulReg( otherRegister );

  QCOMPARE( firstRegister.getValue(), expected );
  QCOMPARE( otherRegister.getValue(), other );
}

void SIGEL_Simulation::TST_SIG_Register::operationWithTheSameRegisterOnBothSides()
{
  SIG_Register oneRegister( 8 );

  oneRegister.loadValue( 5 );
  oneRegister.addReg( oneRegister );
  QCOMPARE( oneRegister.getValue(), 10 );

  oneRegister.loadValue( 5 );
  oneRegister.subReg( oneRegister );
  QCOMPARE( oneRegister.getValue(), 0 );

  oneRegister.loadValue( 5 );
  oneRegister.mulReg( oneRegister );
  QCOMPARE( oneRegister.getValue(), 25 );
}
