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
#include "SIGEL_Tools/TST_SIG_Randomizer.h"

#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QTime>
#include <QtTest>

void SIGEL_Tools::TST_SIG_Randomizer::sameSeedGivesSameNumbers()
{
  SIG_Randomizer first( 42 );
  SIG_Randomizer second( 42 );

  for ( int i = 0; i < 100; i++ )
    QCOMPARE( first.getRandomInt( 1000 ), second.getRandomInt( 1000 ) );
}

void SIGEL_Tools::TST_SIG_Randomizer::differentSeedsGiveDifferentNumbers()
{
  SIG_Randomizer first( 1 );
  SIG_Randomizer second( 2 );

  QVERIFY( first.getRandomInt( 32768 ) != second.getRandomInt( 32768 ) );
}

void SIGEL_Tools::TST_SIG_Randomizer::setNewSeedStartsTheNumbersAgain()
{
  SIG_Randomizer randomizer( 42 );
  int firstNumber = randomizer.getRandomInt( 1000 );
  int secondNumber = randomizer.getRandomInt( 1000 );

  randomizer.setNewSeed( 42 );

  QCOMPARE( randomizer.getRandomInt( 1000 ), firstNumber );
  QCOMPARE( randomizer.getRandomInt( 1000 ), secondNumber );
}

// The default constructor takes its seed from the time of day.
void SIGEL_Tools::TST_SIG_Randomizer::setNewSeedAfterTheDefaultConstructorGivesTheNumbersOfTheSeed()
{
  SIG_Randomizer randomizer;

  randomizer.setNewSeed( 1 );

  QCOMPARE( randomizer.getRandomInt( 32768 ), 16838 );
}

void SIGEL_Tools::TST_SIG_Randomizer::defaultConstructorTakesTheSecondsSinceMidnightAsSeed()
{
  for ( int attempt = 0; attempt < 20; attempt++ )
    {
      int secondsBefore = QTime( 0, 0 ).secsTo( QTime::currentTime() );
      SIG_Randomizer randomizer;
      int secondsAfter = QTime( 0, 0 ).secsTo( QTime::currentTime() );

      // The second changes between the two readings, or it is midnight,
      // where the seed 0 means the time of day again.
      if ( secondsBefore != secondsAfter || secondsBefore == 0 )
        {
          QTest::qSleep( 100 );
          continue;
        }

      SIG_Randomizer expected( secondsBefore );
      QCOMPARE( randomizer.getRandomInt( 32768 ), expected.getRandomInt( 32768 ) );
      QCOMPARE( randomizer.getRandomInt( 32768 ), expected.getRandomInt( 32768 ) );
      return;
    }

  QFAIL( "The clock gives no two equal readings." );
}

void SIGEL_Tools::TST_SIG_Randomizer::numberIsBelowMaximum()
{
  SIG_Randomizer randomizer( 42 );

  for ( int i = 0; i < 1000; i++ )
    {
      int number = randomizer.getRandomInt( 10 );
      QVERIFY( number >= 0 );
      QVERIFY( number < 10 );
    }
}

void SIGEL_Tools::TST_SIG_Randomizer::maximumZeroGivesZero()
{
  SIG_Randomizer randomizer( 42 );

  QCOMPARE( randomizer.getRandomInt( 0 ), 0 );
}

// A change here gives each seed another run.
void SIGEL_Tools::TST_SIG_Randomizer::maximumZeroMovesTheGeneratorOn()
{
  SIG_Randomizer reference( 42 );
  reference.getRandomInt( 1000 );
  int secondNumber = reference.getRandomInt( 1000 );
  SIG_Randomizer randomizer( 42 );

  randomizer.getRandomInt( 0 );

  QCOMPARE( randomizer.getRandomInt( 1000 ), secondNumber );
}

// A change here gives each seed another run.
void SIGEL_Tools::TST_SIG_Randomizer::getRandomLongGivesTheNumberOfGetRandomInt()
{
  SIG_Randomizer first( 42 );
  SIG_Randomizer second( 42 );

  QCOMPARE( first.getRandomLong( 1000 ), static_cast<long>( second.getRandomInt( 1000 ) ) );
}

void SIGEL_Tools::TST_SIG_Randomizer::getRandomLongWithMaximumZeroGivesZero()
{
  SIG_Randomizer randomizer( 42 );

  QCOMPARE( randomizer.getRandomLong( 0 ), 0L );
}

// Four numbers: the first one alone does not show a small change of the
// multiplier or the increment. A change here gives each seed another run.
void SIGEL_Tools::TST_SIG_Randomizer::seedOneGivesKnownNumbers()
{
  SIG_Randomizer randomizer( 1 );

  QCOMPARE( randomizer.getRandomInt( 32768 ), 16838 );
  QCOMPARE( randomizer.getRandomInt( 32768 ), 5758 );
  QCOMPARE( randomizer.getRandomInt( 32768 ), 10113 );
  QCOMPARE( randomizer.getRandomInt( 32768 ), 17515 );
}

// A maximum that is not a power of two: the numbers also depend on the mask
// and on the remainder of the division by the maximum. A change here gives
// each seed another run.
void SIGEL_Tools::TST_SIG_Randomizer::seedFortyTwoGivesKnownNumbersBelowOneThousand()
{
  SIG_Randomizer randomizer( 42 );

  QCOMPARE( randomizer.getRandomInt( 1000 ), 81 );
  QCOMPARE( randomizer.getRandomInt( 1000 ), 33 );
  QCOMPARE( randomizer.getRandomInt( 1000 ), 269 );
  QCOMPARE( randomizer.getRandomInt( 1000 ), 461 );
}
