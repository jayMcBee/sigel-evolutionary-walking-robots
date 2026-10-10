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
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QtTest>

void SIGEL_GP::SIG_GPPopulationTest::defaultConstructorGivesAnEmptyPopulation()
{
  SIG_GPPopulation population;

  QCOMPARE( population.getSize(), 0 );
  QCOMPARE( population.nextIdentifier, QString( "0" ) );
  QCOMPARE( population.getPoolGeneration(), 0 );
  QVERIFY( population.getHistory() );
}

void SIGEL_GP::SIG_GPPopulationTest::getNextIdentifierCountsUp()
{
  SIG_GPPopulation population;

  QCOMPARE( population.getNextIdentifier(), QString( "0" ) );
  QCOMPARE( population.getNextIdentifier(), QString( "1" ) );
  QCOMPARE( population.nextIdentifier, QString( "2" ) );

  population.setNextIdentifier( "7" );

  QCOMPARE( population.getNextIdentifier(), QString( "7" ) );
  QCOMPARE( population.nextIdentifier, QString( "8" ) );
}

void SIGEL_GP::SIG_GPPopulationTest::addRandomIndividualsGivesNamesAndPoolPositions()
{
  SIG_GPPopulation population;

  QCOMPARE( addIndividuals( population, 4 ), 4 );

  QCOMPARE( population.getSize(), 4 );
  for ( int i = 0; i < 4; i++ )
    {
      QCOMPARE( population.getIndividualPointer( i )->getName(), QString::number( i ) );
      QCOMPARE( population.getIndividualPointer( i )->getPoolPos(), i );
    }
  QCOMPARE( population.nextIdentifier, QString( "4" ) );
}

void SIGEL_GP::SIG_GPPopulationTest::addRandomIndividualsKeepsTheIndividualsThatAreThere()
{
  SIG_GPPopulation population;
  addIndividuals( population, 2 );
  SIG_GPIndividual *first = population.getIndividualPointer( 0 );
  SIG_GPIndividual *second = population.getIndividualPointer( 1 );

  QCOMPARE( addIndividuals( population, 3 ), 3 );

  QCOMPARE( population.getSize(), 5 );
  QCOMPARE( population.getIndividualPointer( 0 ), first );
  QCOMPARE( population.getIndividualPointer( 1 ), second );
  for ( int i = 2; i < 5; i++ )
    {
      QCOMPARE( population.getIndividualPointer( i )->getName(), QString::number( i ) );
      QCOMPARE( population.getIndividualPointer( i )->getPoolPos(), i );
    }
}

void SIGEL_GP::SIG_GPPopulationTest::addRandomIndividualsTakesTheNamesFromTheIdentifier()
{
  SIG_GPPopulation population;
  population.setNextIdentifier( "40" );

  addIndividuals( population, 2 );

  QCOMPARE( population.getIndividualPointer( 0 )->getName(), QString( "40" ) );
  QCOMPARE( population.getIndividualPointer( 1 )->getName(), QString( "41" ) );
  QCOMPARE( population.getIndividualPointer( 0 )->getPoolPos(), 0 );
  QCOMPARE( population.getIndividualPointer( 1 )->getPoolPos(), 1 );
  QCOMPARE( population.nextIdentifier, QString( "42" ) );
}

void SIGEL_GP::SIG_GPPopulationTest::addRandomIndividualsUsesTheLengthLimits()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 7 );
  gpParameter.setMaxIndLength( 7 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIG_GPPopulation population;
  population.getRandomizerPointer()->setNewSeed( 1 );

  population.addRandomIndividuals( 3, gpParameter, languageParameters );

  for ( int i = 0; i < 3; i++ )
    QCOMPARE( population.getIndividualPointer( i )->getProgram().getProgramLength(), 7L );
}

void SIGEL_GP::SIG_GPPopulationTest::programLengthsAreInsideTheLimits()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIG_GPPopulation population;
  population.getRandomizerPointer()->setNewSeed( 1 );

  population.addRandomIndividuals( 20, gpParameter, languageParameters );

  for ( int i = 0; i < 20; i++ )
    {
      long length = population.getIndividualPointer( i )->getProgram().getProgramLength();
      QVERIFY( length >= 5 );
      QVERIFY( length <= 8 );
    }
}

void SIGEL_GP::SIG_GPPopulationTest::sameSeedGivesTheSameIndividuals()
{
  SIG_GPPopulation first;
  SIG_GPPopulation second;
  // The history of an individual holds the time when it is made.
  first.setHistory( false );
  second.setHistory( false );

  addIndividuals( first, 3 );
  addIndividuals( second, 3 );

  QCOMPARE( writtenText( second ), writtenText( first ) );
}

void SIGEL_GP::SIG_GPPopulationTest::differentSeedsGiveDifferentIndividuals()
{
  SIG_GPPopulation first;
  SIG_GPPopulation second;
  first.setHistory( false );
  second.setHistory( false );

  addIndividuals( first, 3, 1 );
  addIndividuals( second, 3, 2 );

  QVERIFY( writtenText( second ) != writtenText( first ) );
}

void SIGEL_GP::SIG_GPPopulationTest::getIndividualGivesTheIndividualAtThePosition()
{
  SIG_GPPopulation population;
  addIndividuals( population, 3 );

  for ( int i = 0; i < 3; i++ )
    QCOMPARE( &population.getIndividual( i ), population.getIndividualPointer( i ) );
}

void SIGEL_GP::SIG_GPPopulationTest::deleteIndividualMovesTheRestDown()
{
  SIG_GPPopulation population;
  addIndividuals( population, 4 );
  SIG_GPIndividual *first = population.getIndividualPointer( 0 );
  SIG_GPIndividual *third = population.getIndividualPointer( 2 );
  SIG_GPIndividual *last = population.getIndividualPointer( 3 );

  population.deleteIndividual( 1 );

  QCOMPARE( population.getSize(), 3 );
  QCOMPARE( population.getIndividualPointer( 0 ), first );
  QCOMPARE( population.getIndividualPointer( 1 ), third );
  QCOMPARE( population.getIndividualPointer( 2 ), last );
  for ( int i = 0; i < 3; i++ )
    QCOMPARE( population.getIndividualPointer( i )->getPoolPos(), i );
}

void SIGEL_GP::SIG_GPPopulationTest::deleteIndividualOfTheLastLeavesTheOthers()
{
  SIG_GPPopulation population;
  addIndividuals( population, 3 );
  SIG_GPIndividual *first = population.getIndividualPointer( 0 );
  SIG_GPIndividual *second = population.getIndividualPointer( 1 );

  population.deleteIndividual( 2 );

  QCOMPARE( population.getSize(), 2 );
  QCOMPARE( population.getIndividualPointer( 0 ), first );
  QCOMPARE( population.getIndividualPointer( 1 ), second );
}

void SIGEL_GP::SIG_GPPopulationTest::deleteIndividualDownToAnEmptyPopulation()
{
  SIG_GPPopulation population;
  addIndividuals( population, 3 );

  while ( population.getSize() > 0 )
    population.deleteIndividual( population.getSize() - 1 );

  QCOMPARE( population.getSize(), 0 );
}

void SIGEL_GP::SIG_GPPopulationTest::setIndividualReplacesOneIndividual()
{
  SIG_GPPopulation population;
  addIndividuals( population, 3 );
  SIG_GPIndividual *first = population.getIndividualPointer( 0 );
  SIG_GPIndividual *last = population.getIndividualPointer( 2 );

  // The population owns the new individual and frees the old one.
  SIG_GPIndividual *winner = new SIG_GPIndividual();
  population.setIndividual( *winner, 1 );

  QCOMPARE( population.getSize(), 3 );
  QCOMPARE( population.getIndividualPointer( 0 ), first );
  QCOMPARE( population.getIndividualPointer( 1 ), winner );
  QCOMPARE( population.getIndividualPointer( 2 ), last );
}

void SIGEL_GP::SIG_GPPopulationTest::resetAllFitnessValuesSetsMinusOne()
{
  SIG_GPPopulation population;
  addIndividuals( population, 2 );
  // A new individual has the fitness -1 already, so other values are set first.
  population.getIndividualPointer( 0 )->setFitness( 3.5 );
  population.getIndividualPointer( 1 )->setFitness( 7.5 );

  population.resetAllFitnessValues();

  QCOMPARE( population.getIndividualPointer( 0 )->getFitness(), -1.0 );
  QCOMPARE( population.getIndividualPointer( 1 )->getFitness(), -1.0 );
}

// An individual with no simulation has a fitness below 0.
void SIGEL_GP::SIG_GPPopulationTest::bestWorstAndAverageUseOnlySimulatedFitnessValues()
{
  SIG_GPPopulation population;
  addIndividuals( population, 4 );
  population.getIndividualPointer( 0 )->setFitness( 3.5 );
  population.getIndividualPointer( 1 )->setFitness( 5.0 );
  population.getIndividualPointer( 3 )->setFitness( 2.0 );

  QCOMPARE( population.getBestFitness( true ), 5.0 );
  QCOMPARE( population.getWorstFitness( true ), 2.0 );
  QCOMPARE( population.getAverageFitness(), 3.5 );
}

// 11 individuals: the text then has a position with two digits.
void SIGEL_GP::SIG_GPPopulationTest::writtenTextIsReadBack()
{
  SIG_GPPopulation original;
  addIndividuals( original, 11 );
  original.setHistory( false );
  original.setPoolGeneration( 6 );
  original.getIndividualPointer( 4 )->setFitness( 3.5 );
  QString written = writtenText( original );

  SIG_GPPopulation copy;
  QTextStream stream( &written, QIODevice::ReadOnly );
  copy.readFromFile( stream );

  QCOMPARE( copy.getSize(), 11 );
  QCOMPARE( copy.nextIdentifier, QString( "11" ) );
  QCOMPARE( copy.getPoolGeneration(), 6 );
  QCOMPARE( copy.getIndividualPointer( 4 )->getFitness(), 3.5 );
  QCOMPARE( copy.getIndividualPointer( 10 )->getName(), QString( "10" ) );
  QCOMPARE( copy.getIndividualPointer( 10 )->getPoolPos(), 10 );
  QCOMPARE( writtenText( copy ), written );
}

void SIGEL_GP::SIG_GPPopulationTest::writtenTextWithoutHistoryHasNoHistoryBlock()
{
  SIG_GPPopulation original;
  addIndividuals( original, 2 );
  QVERIFY( writtenText( original ).contains( "HISTORY BEGIN{" ) );

  original.setHistory( false );
  QString written = writtenText( original );

  QVERIFY( !written.contains( "HISTORY BEGIN{" ) );

  SIG_GPPopulation copy;
  QTextStream stream( &written, QIODevice::ReadOnly );
  copy.readFromFile( stream );

  QVERIFY( !copy.getHistory() );
  QCOMPARE( writtenText( copy ), written );
}

void SIGEL_GP::SIG_GPPopulationTest::individualsWithHistoryAreReadBack()
{
  SIG_GPPopulation original;
  addIndividuals( original, 3 );
  original.getIndividualPointer( 1 )->setFitness( 3.5 );
  original.getIndividualPointer( 1 )->setAge( 7 );
  QString written = writtenText( original );

  SIG_GPPopulation copy;
  copy.setHistory( false );
  QTextStream stream( &written, QIODevice::ReadOnly );
  copy.readFromFile( stream );

  QVERIFY( copy.getHistory() );
  QCOMPARE( copy.getSize(), 3 );
  for ( int i = 0; i < 3; i++ )
    {
      SIG_GPIndividual *read = copy.getIndividualPointer( i );
      SIG_GPIndividual *expected = original.getIndividualPointer( i );
      QCOMPARE( read->getName(), expected->getName() );
      QCOMPARE( read->getPoolPos(), expected->getPoolPos() );
      QCOMPARE( read->getFitness(), expected->getFitness() );
      QCOMPARE( read->getAge(), expected->getAge() );
      QCOMPARE( read->getProgram().getProgramLength(), expected->getProgram().getProgramLength() );
      // readFromFile keeps the white space in front of the end of the history block.
      QCOMPARE( read->getHistory().join( "\n" ).trimmed(), expected->getHistory().join( "\n" ).trimmed() );
    }
}

// Some experiment files have no WITHHISTORY line.
void SIGEL_GP::SIG_GPPopulationTest::textWithoutHeaderIsRead()
{
  SIG_GPPopulation original;
  addIndividuals( original, 2 );
  original.setHistory( false );
  QString written = writtenText( original );
  QVERIFY( written.startsWith( "WITHHISTORY\n0\n" ) );
  QString withoutHeader = written.mid( QString( "WITHHISTORY\n0\n" ).length() );

  SIG_GPPopulation copy;
  QTextStream stream( &withoutHeader, QIODevice::ReadOnly );
  copy.readFromFile( stream );

  QVERIFY( copy.getHistory() );
  QCOMPARE( copy.getSize(), 2 );
  QCOMPARE( copy.getIndividualPointer( 1 )->getName(), QString( "1" ) );
}

void SIGEL_GP::SIG_GPPopulationTest::emptyPopulationIsReadBack()
{
  SIG_GPPopulation original;
  QString written = writtenText( original );

  SIG_GPPopulation copy;
  addIndividuals( copy, 2 );
  QTextStream stream( &written, QIODevice::ReadOnly );
  copy.readFromFile( stream );

  QCOMPARE( copy.getSize(), 0 );
  QCOMPARE( copy.nextIdentifier, QString( "0" ) );
}

void SIGEL_GP::SIG_GPPopulationTest::savePoolWritesTheTextOfWriteToFile()
{
  SIG_GPPopulation population;
  addIndividuals( population, 2 );
  population.setHistory( false );
  QString saved;
  QTextStream stream( &saved );

  population.savePool( stream );

  QCOMPARE( saved, writtenText( population ) );
}

void SIGEL_GP::SIG_GPPopulationTest::readFromFileReplacesTheIndividualsThatAreThere()
{
  SIG_GPPopulation original;
  addIndividuals( original, 2 );
  original.setHistory( false );
  original.getIndividualPointer( 0 )->setFitness( 3.5 );
  QString written = writtenText( original );

  SIG_GPPopulation larger;
  addIndividuals( larger, 5 );
  QTextStream largerStream( &written, QIODevice::ReadOnly );
  larger.readFromFile( largerStream );

  QCOMPARE( larger.getSize(), 2 );
  QCOMPARE( writtenText( larger ), written );

  SIG_GPPopulation smaller;
  addIndividuals( smaller, 1 );
  QTextStream smallerStream( &written, QIODevice::ReadOnly );
  smaller.readFromFile( smallerStream );

  QCOMPARE( smaller.getSize(), 2 );
  QCOMPARE( writtenText( smaller ), written );
}

int SIGEL_GP::SIG_GPPopulationTest::addIndividuals( SIG_GPPopulation &population, int quantity, int seed )
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  population.getRandomizerPointer()->setNewSeed( seed );

  return population.addRandomIndividuals( quantity, gpParameter, languageParameters );
}

QString SIGEL_GP::SIG_GPPopulationTest::writtenText( SIG_GPPopulation &population )
{
  QString text;
  QTextStream stream( &text );
  population.writeToFile( stream );

  return text;
}
