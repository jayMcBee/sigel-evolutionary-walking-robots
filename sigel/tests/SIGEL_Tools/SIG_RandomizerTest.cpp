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
#include "SIGEL_Tools/SIG_RandomizerTest.h"

#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QtTest>

void SIGEL_Tools::SIG_RandomizerTest::sameSeedGivesSameNumbers()
{
  SIG_Randomizer first( 42 );
  SIG_Randomizer second( 42 );

  for ( int i = 0; i < 100; i++ )
    QCOMPARE( first.getRandomInt( 1000 ), second.getRandomInt( 1000 ) );
}

void SIGEL_Tools::SIG_RandomizerTest::setNewSeedStartsTheNumbersAgain()
{
  SIG_Randomizer randomizer( 42 );
  int firstNumber = randomizer.getRandomInt( 1000 );
  int secondNumber = randomizer.getRandomInt( 1000 );

  randomizer.setNewSeed( 42 );

  QCOMPARE( randomizer.getRandomInt( 1000 ), firstNumber );
  QCOMPARE( randomizer.getRandomInt( 1000 ), secondNumber );
}

void SIGEL_Tools::SIG_RandomizerTest::numberIsBelowMaximum()
{
  SIG_Randomizer randomizer( 42 );

  for ( int i = 0; i < 1000; i++ )
    {
      int number = randomizer.getRandomInt( 10 );
      QVERIFY( number >= 0 );
      QVERIFY( number < 10 );
    }
}

void SIGEL_Tools::SIG_RandomizerTest::maximumZeroGivesZero()
{
  SIG_Randomizer randomizer( 42 );

  QCOMPARE( randomizer.getRandomInt( 0 ), 0 );
}

void SIGEL_Tools::SIG_RandomizerTest::getRandomLongGivesTheNumberOfGetRandomInt()
{
  SIG_Randomizer first( 42 );
  SIG_Randomizer second( 42 );

  QCOMPARE( first.getRandomLong( 1000 ), static_cast<long>( second.getRandomInt( 1000 ) ) );
}

// Pins the formula: any change to it changes this number.
void SIGEL_Tools::SIG_RandomizerTest::seedOneGivesKnownFirstNumber()
{
  SIG_Randomizer randomizer( 1 );

  QCOMPARE( randomizer.getRandomInt( 32768 ), 16838 );
}
