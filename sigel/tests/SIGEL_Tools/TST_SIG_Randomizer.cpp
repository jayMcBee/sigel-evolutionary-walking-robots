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
#include "SIGEL_Tools/SIG_Exception.h"

#include <QtTest>

#include <limits>

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

  QVERIFY( first.getRandomInt( SIG_Randomizer::fullRandomIntRange ) != second.getRandomInt( SIG_Randomizer::fullRandomIntRange ) );
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

void SIGEL_Tools::TST_SIG_Randomizer::setNewSeedAfterTheDefaultConstructorGivesTheNumbersOfTheSeed()
{
  SIG_Randomizer randomizer;
  SIG_Randomizer expected( 1 );

  randomizer.setNewSeed( 1 );

  for ( int i = 0; i < 4; i++ )
    QCOMPARE( randomizer.getRandomInt( SIG_Randomizer::fullRandomIntRange ), expected.getRandomInt( SIG_Randomizer::fullRandomIntRange ) );
}

// Four numbers: two equal first numbers alone can happen by chance.
void SIGEL_Tools::TST_SIG_Randomizer::defaultConstructorGivesOtherNumbersEachTime()
{
  SIG_Randomizer first;
  SIG_Randomizer second;
  bool numbersDiffer = false;

  for ( int i = 0; i < 4; i++ )
    if ( first.getRandomInt( SIG_Randomizer::fullRandomIntRange ) != second.getRandomInt( SIG_Randomizer::fullRandomIntRange ) )
      numbersDiffer = true;

  QVERIFY( numbersDiffer );
}

void SIGEL_Tools::TST_SIG_Randomizer::seedZeroGivesOtherNumbersEachTime()
{
  SIG_Randomizer first( 0 );
  SIG_Randomizer second( 0 );
  bool numbersDiffer = false;

  for ( int i = 0; i < 4; i++ )
    if ( first.getRandomInt( SIG_Randomizer::fullRandomIntRange ) != second.getRandomInt( SIG_Randomizer::fullRandomIntRange ) )
      numbersDiffer = true;

  QVERIFY( numbersDiffer );
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

void SIGEL_Tools::TST_SIG_Randomizer::everyNumberBelowMaximumComes()
{
  SIG_Randomizer randomizer( 42 );
  QList<bool> numberSeen( 10, false );

  for ( int i = 0; i < 1000; i++ )
    numberSeen[ randomizer.getRandomInt( 10 ) ] = true;

  QCOMPARE( numberSeen, QList<bool>( 10, true ) );
}

// With 30000 numbers, the first 2768 have the share 2768 / 30000 = 0.0923.
// A remainder of 32768 divided by 30000 would give them nearly twice that share.
void SIGEL_Tools::TST_SIG_Randomizer::numbersBelowMaximumAreEquallyLikely()
{
  SIG_Randomizer randomizer( 42 );
  const int draws = 300000;
  int lowNumbers = 0;

  for ( int i = 0; i < draws; i++ )
    if ( randomizer.getRandomInt( 30000 ) < 2768 )
      lowNumbers++;

  const double share = static_cast<double>( lowNumbers ) / draws;
  QVERIFY2( share > 0.0873 && share < 0.0973, qPrintable( QString::number( share ) ) );
}

void SIGEL_Tools::TST_SIG_Randomizer::maximumZeroGivesZero()
{
  SIG_Randomizer randomizer( 42 );

  QCOMPARE( randomizer.getRandomInt( 0 ), 0 );
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

void SIGEL_Tools::TST_SIG_Randomizer::negativeMaximumThrows()
{
  SIG_Randomizer randomizer( 42 );

  QVERIFY_THROWS_EXCEPTION( SIG_Exception, randomizer.getRandomInt( -1 ) );
  QVERIFY_THROWS_EXCEPTION( SIG_Exception, randomizer.getRandomLong( -1 ) );
}

void SIGEL_Tools::TST_SIG_Randomizer::getRandomLongGivesNumbersAboveTheIntRange()
{
  SIG_Randomizer randomizer( 42 );
  const long maximum = 1L << 40;
  bool numberAboveTheIntRangeSeen = false;

  for ( int i = 0; i < 100; i++ )
    {
      long number = randomizer.getRandomLong( maximum );
      QVERIFY( number >= 0 );
      QVERIFY( number < maximum );
      if ( number > std::numeric_limits<int>::max() )
        numberAboveTheIntRangeSeen = true;
    }

  QVERIFY( numberAboveTheIntRangeSeen );
}
