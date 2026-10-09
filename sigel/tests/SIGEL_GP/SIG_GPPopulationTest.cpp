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
#include "SIGEL_GP/SIG_GPPopulationTest.h"

#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

#include <QtTest>

void SIGEL_GP::SIG_GPPopulationTest::individualsAreDeletedReplacedAndReset()
{
  SIG_GPParameter gpParameter;
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIG_GPPopulation population;
  population.addRandomIndividuals( 4, gpParameter, languageParameters );
  QCOMPARE( population.getSize(), 4 );

  // deleteIndividual frees one individual and moves the rest down.
  SIG_GPIndividual *third = population.getIndividualPointer( 2 );
  SIG_GPIndividual *last = population.getIndividualPointer( 3 );
  population.deleteIndividual( 1 );
  QCOMPARE( population.getSize(), 3 );
  QCOMPARE( population.getIndividualPointer( 1 ), third );
  QCOMPARE( population.getIndividualPointer( 2 ), last );
  QCOMPARE( population.getIndividualPointer( 1 )->getPoolPos(), 1 );
  QCOMPARE( population.getIndividualPointer( 2 )->getPoolPos(), 2 );

  // The last individual: nothing moves down.
  population.deleteIndividual( population.getSize() - 1 );
  QCOMPARE( population.getSize(), 2 );

  // setIndividual frees the individual at the position and owns the new one.
  SIG_GPIndividual *winner = new SIG_GPIndividual();
  population.setIndividual( *winner, 0 );
  QCOMPARE( population.getIndividualPointer( 0 ), winner );
  QCOMPARE( population.getSize(), 2 );

  // A new individual has the fitness -1 already, so other values are set first.
  population.getIndividualPointer( 0 )->setFitness( 3.5 );
  population.getIndividualPointer( 1 )->setFitness( 7.5 );
  population.resetAllFitnessValues();
  QCOMPARE( population.getIndividualPointer( 0 )->getFitness(), -1.0 );
  QCOMPARE( population.getIndividualPointer( 1 )->getFitness(), -1.0 );

  // One individual stays, so that the destructor has one to free.
  population.deleteIndividual( 0 );
  QCOMPARE( population.getSize(), 1 );
}
