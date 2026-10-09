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
#include "MT_GPSystem/MT_SearchTest.h"

#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Programline.h"
#include "MT_GPSystem/MT_Randomizer.h"
#include "MT_GPSystem/MT_Search.h"

#include <QtTest>

#include <cstdlib>

void MT_SearchTest::crossoverKeepsTheLinesOfTheParents_data()
{
  addCrossoverPointRows();
}

// With room for each line, the children together have exactly the lines of the parents.
void MT_SearchTest::crossoverKeepsTheLinesOfTheParents()
{
  QFETCH( int, firstThreshold );
  QFETCH( int, secondThreshold );
  QFETCH( int, thirdThreshold );
  QFETCH( int, genesis );
  const int crossoverPoints[3] = { firstThreshold, secondThreshold, thirdThreshold };

  for ( unsigned seed = 1; seed <= seeds; seed++ )
    {
      const QByteArray message = "seed " + QByteArray::number( seed );
      MT_MatingResult mating = mate( seed, crossoverOnly, never, crossoverPoints, startLength, 100 );
      QVERIFY2( mating.complete, message.constData() );

      QStringList fromParents = mating.parentLines[0] + mating.parentLines[1];
      QStringList fromChildren = mating.childLines[0] + mating.childLines[1];
      fromParents.sort();
      fromChildren.sort();
      QVERIFY2( fromChildren == fromParents, message.constData() );
      QVERIFY2( mating.childGenesis[0] == genesis && mating.childGenesis[1] == genesis, message.constData() );
      QVERIFY2( mating.parentLinesAfter[0] == mating.parentLines[0], message.constData() );
      QVERIFY2( mating.parentLinesAfter[1] == mating.parentLines[1], message.constData() );
    }
}

void MT_SearchTest::crossoverWithNoRoomCutsTheChildren_data()
{
  addCrossoverPointRows();
}

// A child is cut at the maximum length and is never empty.
void MT_SearchTest::crossoverWithNoRoomCutsTheChildren()
{
  QFETCH( int, firstThreshold );
  QFETCH( int, secondThreshold );
  QFETCH( int, thirdThreshold );
  const int crossoverPoints[3] = { firstThreshold, secondThreshold, thirdThreshold };

  for ( unsigned seed = 1; seed <= seeds; seed++ )
    {
      const QByteArray message = "seed " + QByteArray::number( seed );
      MT_MatingResult mating = mate( seed, crossoverOnly, never, crossoverPoints, startLength, startLength );
      QVERIFY2( mating.complete, message.constData() );

      for ( const QStringList &child : mating.childLines )
        QVERIFY2( child.size() >= 1 && child.size() <= mating.maxProgramLength, message.constData() );
      QVERIFY2( mating.parentLinesAfter[0] == mating.parentLines[0], message.constData() );
      QVERIFY2( mating.parentLinesAfter[1] == mating.parentLines[1], message.constData() );
    }
}

void MT_SearchTest::mutationKeepsTheLengthOfTheParent()
{
  int mutatedChildren = 0;

  for ( unsigned seed = 1; seed <= seeds; seed++ )
    {
      const QByteArray message = "seed " + QByteArray::number( seed );
      MT_MatingResult mating = mate( seed, mutationOnly, always, onePoint, startLength, startLength );
      QVERIFY2( mating.complete, message.constData() );

      for ( int child = 0; child < 2; child++ )
        {
          const QStringList &parentLines = mating.parentLines[mating.childParent[child]];
          QVERIFY2( mating.childLines[child].size() == parentLines.size(), message.constData() );
          QVERIFY2( mating.childGenesis[child] >= 100, message.constData() );
          if ( mating.childLines[child] != parentLines )
            mutatedChildren++;
          QVERIFY2( mating.parentLinesAfter[child] == mating.parentLines[child], message.constData() );
        }
    }

  // A mutated element can get its old value again, so one child can be equal
  // to its parent. If all are equal, the mutation changes nothing.
  QVERIFY( mutatedChildren > 0 );
}

void MT_SearchTest::mutationWithRateZeroCopiesTheParent()
{
  for ( unsigned seed = 1; seed <= seeds; seed++ )
    {
      const QByteArray message = "seed " + QByteArray::number( seed );
      MT_MatingResult mating = mate( seed, mutationOnly, never, onePoint, startLength, startLength );
      QVERIFY2( mating.complete, message.constData() );

      for ( int child = 0; child < 2; child++ )
        {
          QVERIFY2( mating.childLines[child] == mating.parentLines[mating.childParent[child]], message.constData() );
          QVERIFY2( mating.childGenesis[child] == 100, message.constData() );
        }
    }
}

void MT_SearchTest::reproductionCopiesTheParent()
{
  for ( unsigned seed = 1; seed <= seeds; seed++ )
    {
      const QByteArray message = "seed " + QByteArray::number( seed );
      MT_MatingResult mating = mate( seed, reproductionOnly, never, onePoint, startLength, startLength );
      QVERIFY2( mating.complete, message.constData() );

      for ( int child = 0; child < 2; child++ )
        {
          QVERIFY2( mating.childLines[child] == mating.parentLines[mating.childParent[child]], message.constData() );
          QVERIFY2( mating.childGenesis[child] == 4, message.constData() );
          QVERIFY2( mating.parentLinesAfter[child] == mating.parentLines[child], message.constData() );
        }
    }
}

void MT_SearchTest::addCrossoverPointRows()
{
  QTest::addColumn<int>( "firstThreshold" );
  QTest::addColumn<int>( "secondThreshold" );
  QTest::addColumn<int>( "thirdThreshold" );
  QTest::addColumn<int>( "genesis" );

  QTest::newRow( "1 point" ) << 1000 << 1000 << 1000 << 1;
  QTest::newRow( "2 points" ) << 0 << 1000 << 1000 << 2;
  QTest::newRow( "3 points" ) << 0 << 0 << 1000 << 3;
}

QStringList MT_SearchTest::programLines( MT_Program *program )
{
  QStringList lines;

  for ( int i = 0; i < program->getLength(); i++ )
    {
      QString text;
      QTextStream stream( &text );
      program->getProgramLine( i )->writeToFileProgramLine( stream );
      lines << text.trimmed();
    }

  return lines;
}

QString MT_SearchTest::randomizerText( const int searchOperator[3], const int mutationPower[2],
                                       const int crossoverPoints[3], int maxProgramLength )
{
  QString text;
  QTextStream stream( &text );

  // Parents, offspring, registers, maximum program length.
  stream << "Randomizer:\n2\n4\n10\n" << maxProgramLength << "\n";
  for ( int i = 0; i < 3; i++ )
    stream << searchOperator[i] << "\n";
  for ( int i = 0; i < 2; i++ )
    stream << mutationPower[i] << "\n";
  for ( int i = 0; i < 3; i++ )
    stream << crossoverPoints[i] << "\n";
  // The 18 instruction thresholds of stdConf.mt.
  for ( int i = 0; i < 18; i++ )
    stream << i * 1000 << "\n";
  stream << "\nConstant:\n3\n1\n2\n3\n\n";

  return text;
}

MT_MatingResult MT_SearchTest::mate( unsigned seed, const int searchOperator[3], const int mutationPower[2],
                                     const int crossoverPoints[3], int startLength, int maxProgramLength )
{
  MT_MatingResult result;
  result.maxProgramLength = maxProgramLength;

  QString text = randomizerText( searchOperator, mutationPower, crossoverPoints, startLength );
  QTextStream stream( &text );
  MT_Randomizer randomizer( stream );
  // MT_Randomizer seeds rand() from the clock in its constructor.
  srand( seed );

  MT_Population parents( &randomizer, 2 );
  parents.setMaxProgLen( maxProgramLength );
  MT_Individual *parent[2] = { parents.getIndividual( 0 ), parents.getIndividual( 1 ) };
  for ( int i = 0; i < 2; i++ )
    {
      parent[i]->setFitness( i + 1.0 );
      result.parentLines[i] = programLines( parent[i]->getProgram() );
    }

  MT_Population offspring;
  offspring.changePopSize( 4 );
  MT_Search search( &parents, &offspring, &randomizer );
  const int error = search.startMatingProcess();

  int parentsFound = 0;
  bool allFilled = true;
  for ( int i = 0; i < offspring.getSize(); i++ )
    {
      MT_Individual *individual = offspring.getIndividual( i );
      if ( !individual )
        {
          allFilled = false;
          continue;
        }
      if ( individual == parent[0] || individual == parent[1] )
        {
          parentsFound++;
          continue;
        }
      result.childLines << programLines( individual->getProgram() );
      result.childGenesis << individual->getTypOfGenesis();
      result.childParent << ( individual->getFitnessOfParent() == 1.0 ? 0 : 1 );
    }

  for ( int i = 0; i < 2; i++ )
    result.parentLinesAfter[i] = programLines( parent[i]->getProgram() );
  result.complete = ( error == 0 ) && allFilled && ( parentsFound == 2 ) && ( result.childLines.size() == 2 );

  return result;
}
