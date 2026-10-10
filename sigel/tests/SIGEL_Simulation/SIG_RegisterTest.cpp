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
