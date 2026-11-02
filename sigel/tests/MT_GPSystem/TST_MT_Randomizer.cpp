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
#include "MT_GPSystem/TST_MT_Randomizer.h"

#include "MT_GPSystem/MT_Randomizer.h"

#include <QtTest>

void TST_MT_Randomizer::createConstantWithTheUpperBoundBelowTheLowerBoundGivesTheLowerBound_data()
{
  QTest::addColumn< bool >( "integer" );

  QTest::newRow( "integer constants" ) << true;
  QTest::newRow( "double constants" ) << false;
}

void TST_MT_Randomizer::createConstantWithTheUpperBoundBelowTheLowerBoundGivesTheLowerBound()
{
  QFETCH( bool, integer );
  MT_Randomizer randomizer;
  int *length = nullptr;
  int *numberOfVariables = nullptr;
  QList<double> *constants = nullptr;
  QList<double> *instructionProbabilities = nullptr;

  randomizer.createConstant( 20, integer, 10, 5 );

  randomizer.returnIndividualsValue( &length, &numberOfVariables, &constants, &instructionProbabilities );
  QCOMPARE( *constants, QList<double>( 20, 10.0 ) );
}

// With the weights 1 and 3, the second player wins three times in four.
void TST_MT_Randomizer::proportionalWinnerWinsInProportionToTheWeight()
{
  MT_Randomizer randomizer;
  randomizer.setNewSeed( 42 );
  QList<int> weights = { 1, 3 };
  const int draws = 40000;
  int secondPlayerWins = 0;

  for ( int i = 0; i < draws; i++ )
    if ( randomizer.getProportionalWinner( &weights ) == 1 )
      secondPlayerWins++;

  const double share = static_cast<double>( secondPlayerWins ) / draws;
  QVERIFY2( share > 0.74 && share < 0.76, qPrintable( QString::number( share ) ) );
}
