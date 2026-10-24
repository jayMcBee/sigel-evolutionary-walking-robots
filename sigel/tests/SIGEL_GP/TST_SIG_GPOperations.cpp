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
#include "SIGEL_GP/TST_SIG_GPOperations.h"

#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_GP/SIG_GPOperations.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QMap>
#include <QtTest>
#include <algorithm>

void SIGEL_GP::TST_SIG_GPOperations::mutationKeepsTheLengthInsideTheLimits()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 5 + seed % 11 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    long const length = child->getProgram().getProgramLength();
    QVERIFY2( length >= 5 && length <= 15, message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::mutationChangesAddsOrDeletesOneLine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 5 + seed % 11 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    QVERIFY2( !mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) ).isEmpty(), message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::mutationChangesAddsAndDeletesALine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QMap<QString, int> kinds;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    kinds[ mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) ) ]++;
  }

  QVERIFY( kinds.value( "changed" ) > 0 );
  QVERIFY( kinds.value( "added" ) > 0 );
  QVERIFY( kinds.value( "deleted" ) > 0 );
}

// Changing a line either keeps the instruction and changes one operand, or takes a new
// instruction and keeps the operands. A new random line also changes the instruction, but
// seldom keeps the first operand.
void SIGEL_GP::TST_SIG_GPOperations::mutationChangesAnOperandOrTheInstruction()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  int operandChanged = 0;
  int instructionChanged = 0;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 8 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    for ( long i = 0; i < 8; i++ )
    {
      SIGEL_Program::SIG_ProgramLine const &before = father->getProgram().getLine( i );
      SIGEL_Program::SIG_ProgramLine const &after = child->getProgram().getLine( i );
      if ( before.getRobotInstructionType() != after.getRobotInstructionType() )
      {
        if ( before.getNumberOfElements() >= 1 && after.getNumberOfElements() >= 1
             && before.getInstructionElement( 0 ) == after.getInstructionElement( 0 ) )
          instructionChanged++;
        continue;
      }
      int operands = 0;
      for ( int j = 0; j < before.getNumberOfElements(); j++ )
      {
        if ( before.getInstructionElement( j ) != after.getInstructionElement( j ) )
          operands++;
      }
      if ( operands == 1 )
        operandChanged++;
    }
  }

  QVERIFY( operandChanged > 0 );
  QVERIFY( instructionChanged > 0 );
}

// A new instruction keeps the old line's operands, also an operand of 0. A new random line
// seldom has 0 in every operand.
void SIGEL_GP::TST_SIG_GPOperations::mutationChangingTheInstructionKeepsAnOperandOfZero()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  int keptZero = 0;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 8 );
    SIGEL_Program::SIG_Program &fatherProgram = father->getProgramVar();

    for ( long i = 0; i < 8; i++ )
    {
      SIGEL_Program::SIG_ProgramLine line = fatherProgram.getLine( i );
      line.setRobotInstruction( line.getRobotInstructionType(), 0, 0 );
      fatherProgram.setLine( i, line );
    }

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    for ( long i = 0; i < 8; i++ )
    {
      SIGEL_Program::SIG_ProgramLine const &before = father->getProgram().getLine( i );
      SIGEL_Program::SIG_ProgramLine const &after = child->getProgram().getLine( i );
      int const shared = std::min( before.getNumberOfElements(), after.getNumberOfElements() );
      if ( before.getRobotInstructionType() == after.getRobotInstructionType() || shared == 0 )
        continue;

      bool allZero = true;
      for ( int j = 0; j < shared; j++ )
      {
        if ( after.getInstructionElement( j ) != 0 )
          allZero = false;
      }
      if ( allZero )
        keptZero++;
    }
  }

  QVERIFY( keptZero > 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::mutationAtTheMaximumDoesNotAddALine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QMap<QString, int> kinds;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 15 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    kinds[ mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) ) ]++;
  }

  QCOMPARE( kinds.value( "added" ), 0 );
  QVERIFY( kinds.value( "deleted" ) > 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::mutationAtTheMinimumDoesNotDeleteALine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QMap<QString, int> kinds;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 5 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    kinds[ mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) ) ]++;
  }

  QCOMPARE( kinds.value( "deleted" ), 0 );
  QVERIFY( kinds.value( "added" ) > 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::mutationWithEqualLimitsOnlyChangesALine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 8 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    QString const kind = mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) );
    QVERIFY2( kind == "changed" || kind == "unchanged", message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::mutationWithMaximumZeroAddsALine()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 0 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QMap<QString, int> kinds;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    kinds[ mutationKind( lineTexts( father->getProgram() ), lineTexts( child->getProgram() ) ) ]++;
  }

  QVERIFY( kinds.value( "added" ) > 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::mutationBringsAParentOutsideTheLimitsInside()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, seed % 2 == 0 ? 2 : 20 );

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    long const length = child->getProgram().getProgramLength();
    QVERIFY2( length >= 5 && length <= 15, message.constData() );
    QStringList const added = child->getHistory().mid( father->getHistory().size() );
    QVERIFY2( added.contains( seed % 2 == 0 ? "Added Length: 3" : "Removed Length: 5" ), message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::mutationLeavesTheParentAlone()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, seed % 2 == 0 ? 10 : 20 );
    QStringList const lines = lineTexts( father->getProgram() );
    QStringList const history = father->getHistory();

    std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

    QVERIFY2( lineTexts( father->getProgram() ) == lines, message.constData() );
    QVERIFY2( father->getHistory() == history, message.constData() );
    QVERIFY2( father->getName() == "father" && father->getPoolPos() == 1 && father->getFitness() == 0.75, message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::mutationGivesTheNameAndPoolPositionAndRecordsTheFather()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
  QStringList const history = father->getHistory();

  std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::mutation( *father, 3, "child", randomizer, gpParameter, languageParameters ) );

  QCOMPARE( child->getName(), QString( "child" ) );
  QCOMPARE( child->getPoolPos(), 3 );
  QCOMPARE( child->getHistory().mid( 0, history.size() ), history );
  QStringList const added = child->getHistory().mid( history.size() );
  QVERIFY( added.contains( "\nMUTATION:\n---------" ) );
  QVERIFY( added.contains( "Father: father" ) );
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverJoinsRunsOfTheParents()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 40 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 15, "mother" );
    QStringList const fatherLines = lineTexts( father->getProgram() );
    QStringList const motherLines = lineTexts( mother->getProgram() );

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    QVERIFY2( joinsRunsOfTheParents( lineTexts( first->getProgram() ), fatherLines, motherLines, false ), message.constData() );
    QVERIFY2( joinsRunsOfTheParents( lineTexts( second->getProgram() ), motherLines, fatherLines, false ), message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverCutsALongChildAtTheEnd()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 11 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  int cut = 0;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 11, "mother" );
    QStringList const fatherLines = lineTexts( father->getProgram() );
    QStringList const motherLines = lineTexts( mother->getProgram() );
    QStringList const fatherHistory = father->getHistory();
    QStringList const motherHistory = mother->getHistory();

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    QStringList const firstLines = lineTexts( first->getProgram() );
    QStringList const secondLines = lineTexts( second->getProgram() );
    QVERIFY2( joinsRunsOfTheParents( firstLines, fatherLines, motherLines, true ), message.constData() );
    QVERIFY2( joinsRunsOfTheParents( secondLines, motherLines, fatherLines, true ), message.constData() );
    QVERIFY2( !first->getHistory().mid( fatherHistory.size() ).contains( "\nPROGRAM LENGTH INCREASED:\n-------------------------" ), message.constData() );
    QVERIFY2( !second->getHistory().mid( motherHistory.size() ).contains( "\nPROGRAM LENGTH INCREASED:\n-------------------------" ), message.constData() );
    // A child of 11 lines may be a full join that needed no cut.
    if ( ( firstLines.size() == 11 && !joinsRunsOfTheParents( firstLines, fatherLines, motherLines, false ) )
         || ( secondLines.size() == 11 && !joinsRunsOfTheParents( secondLines, motherLines, fatherLines, false ) ) )
      cut++;
  }

  QVERIFY( cut > 0 );
}

// A one-point first child ends with the mother's last line; a two-point one does so only when
// both segments reach the end of their parent, and then counts as one-point.
void SIGEL_GP::TST_SIG_GPOperations::crossoverUsesBothVariants()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 1 );
  gpParameter.setMaxIndLength( 100 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  int onePoint = 0;
  int twoPoint = 0;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 15, "mother" );

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    if ( lineTexts( first->getProgram() ).last() == lineTexts( mother->getProgram() ).last() )
      onePoint++;
    else
      twoPoint++;
  }

  QVERIFY( onePoint > 0 );
  QVERIFY( twoPoint > 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverChildrenHaveAsManyLinesAsTheParents()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 1 );
  gpParameter.setMaxIndLength( 100 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 15, "mother" );

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    long const total = first->getProgram().getProgramLength() + second->getProgram().getProgramLength();
    QVERIFY2( total == 25, message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverKeepsTheChildrenInsideTheLimits()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 12 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 8 + seed % 5 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 8 + ( seed / 5 ) % 5, "mother" );

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    for ( SIG_GPIndividual const *child : { first.get(), second.get() } )
    {
      long const length = child->getProgram().getProgramLength();
      QVERIFY2( length >= 8 && length <= 12, message.constData() );
    }
  }
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverLeavesTheParentsAlone()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 12 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
    std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 12, "mother" );
    QStringList const fatherLines = lineTexts( father->getProgram() );
    QStringList const motherLines = lineTexts( mother->getProgram() );
    QStringList const fatherHistory = father->getHistory();
    QStringList const motherHistory = mother->getHistory();

    QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
    std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
    std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
    QCOMPARE( children.size(), 2 );

    QVERIFY2( lineTexts( father->getProgram() ) == fatherLines, message.constData() );
    QVERIFY2( lineTexts( mother->getProgram() ) == motherLines, message.constData() );
    QVERIFY2( father->getHistory() == fatherHistory, message.constData() );
    QVERIFY2( mother->getHistory() == motherHistory, message.constData() );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::crossoverGivesTheNamesAndPoolPositionsAndRecordsBothParents()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 8 );
  gpParameter.setMaxIndLength( 12 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
  std::unique_ptr<SIG_GPIndividual> mother = parent( randomizer, 12, "mother" );
  QStringList const fatherHistory = father->getHistory();
  QStringList const motherHistory = mother->getHistory();

  QList<SIG_GPIndividual *> const children = SIG_GPOperations::crossOver( *father, 4, *mother, 6, "first", "second", randomizer, gpParameter, languageParameters );
  std::unique_ptr<SIG_GPIndividual> first( children.value( 0 ) );
  std::unique_ptr<SIG_GPIndividual> second( children.value( 1 ) );
  QCOMPARE( children.size(), 2 );

  QCOMPARE( first->getName(), QString( "first" ) );
  QCOMPARE( first->getPoolPos(), 4 );
  QCOMPARE( second->getName(), QString( "second" ) );
  QCOMPARE( second->getPoolPos(), 6 );
  QCOMPARE( first->getHistory().mid( 0, fatherHistory.size() ), fatherHistory );
  QCOMPARE( second->getHistory().mid( 0, motherHistory.size() ), motherHistory );
  for ( QStringList const &added : { first->getHistory().mid( fatherHistory.size() ), second->getHistory().mid( motherHistory.size() ) } )
  {
    QVERIFY( added.contains( "\nCROSSOVER:\n----------" ) );
    QVERIFY( added.contains( "Parents: father + mother" ) );
  }
}

void SIGEL_GP::TST_SIG_GPOperations::reproductionCopiesTheParent()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 10 );
  QStringList const lines = lineTexts( father->getProgram() );
  QStringList const history = father->getHistory();

  std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::reproduction( *father, "child", 3, randomizer, gpParameter, languageParameters ) );

  QCOMPARE( lineTexts( child->getProgram() ), lines );
  QCOMPARE( child->getFitness(), 0.75 );
  QCOMPARE( child->getName(), QString( "child" ) );
  QCOMPARE( child->getPoolPos(), 3 );
  QCOMPARE( child->getHistory().mid( 0, history.size() ), history );
  QStringList const added = child->getHistory().mid( history.size() );
  QVERIFY( added.contains( "\nREPRODUCTION:\n-------------" ) );
  QVERIFY( added.contains( "Father: father" ) );
  QCOMPARE( lineTexts( father->getProgram() ), lines );
  QCOMPARE( father->getHistory(), history );
}

void SIGEL_GP::TST_SIG_GPOperations::reproductionPadsAShortParentWithNops()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 10 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 3 );
  QStringList const lines = lineTexts( father->getProgram() );
  QStringList const history = father->getHistory();

  std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::reproduction( *father, "child", 3, randomizer, gpParameter, languageParameters ) );

  QCOMPARE( lineTexts( child->getProgram() ), lines + QStringList( { nopText(), nopText() } ) );
  QStringList const added = child->getHistory().mid( history.size() );
  QVERIFY( added.contains( "\nPROGRAM LENGTH INCREASED:\n-------------------------" ) );
  QVERIFY( added.contains( "Added Length: 2" ) );
}

void SIGEL_GP::TST_SIG_GPOperations::reproductionCutsALongParentAtTheEnd()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 10 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  std::unique_ptr<SIG_GPIndividual> father = parent( randomizer, 12 );
  QStringList const lines = lineTexts( father->getProgram() );
  QStringList const history = father->getHistory();

  std::unique_ptr<SIG_GPIndividual> child( &SIG_GPOperations::reproduction( *father, "child", 3, randomizer, gpParameter, languageParameters ) );

  QCOMPARE( lineTexts( child->getProgram() ), lines.mid( 0, 10 ) );
  QStringList const added = child->getHistory().mid( history.size() );
  QVERIFY( added.contains( "\nPROGRAM LENGTH DECREASED:\n-------------------------" ) );
  QVERIFY( added.contains( "Removed Length: 2" ) );
}

void SIGEL_GP::TST_SIG_GPOperations::clampLengthToLimitsLeavesAProgramInsideTheLimits()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 10 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIGEL_Program::SIG_Program checked = program( randomizer, 7 );
  QStringList const lines = lineTexts( checked );
  int historyInfo = -1;

  checked.clampLengthToLimits( gpParameter, randomizer, languageParameters, historyInfo );

  QCOMPARE( lineTexts( checked ), lines );
  QCOMPARE( historyInfo, 0 );
}

void SIGEL_GP::TST_SIG_GPOperations::clampLengthToLimitsPadsAShortProgramWithNops()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 10 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIGEL_Program::SIG_Program checked = program( randomizer, 3 );
  QStringList const lines = lineTexts( checked );
  int historyInfo = 0;

  checked.clampLengthToLimits( gpParameter, randomizer, languageParameters, historyInfo );

  QCOMPARE( lineTexts( checked ), lines + QStringList( { nopText(), nopText() } ) );
  QCOMPARE( historyInfo, 2 );
}

void SIGEL_GP::TST_SIG_GPOperations::clampLengthToLimitsCutsALongProgramAtTheEnd()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 10 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIGEL_Program::SIG_Program checked = program( randomizer, 12 );
  QStringList const lines = lineTexts( checked );
  int historyInfo = 0;

  checked.clampLengthToLimits( gpParameter, randomizer, languageParameters, historyInfo );

  QCOMPARE( lineTexts( checked ), lines.mid( 0, 10 ) );
  QCOMPARE( historyInfo, -2 );
}

void SIGEL_GP::TST_SIG_GPOperations::clampLengthToLimitsWithMaximumZeroHasNoMaximum()
{
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 0 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIGEL_Program::SIG_Program checked = program( randomizer, 50 );
  QStringList const lines = lineTexts( checked );
  int historyInfo = -1;

  checked.clampLengthToLimits( gpParameter, randomizer, languageParameters, historyInfo );

  QCOMPARE( lineTexts( checked ), lines );
  QCOMPARE( historyInfo, 0 );
}

std::unique_ptr<SIGEL_GP::SIG_GPIndividual> SIGEL_GP::TST_SIG_GPOperations::parent( SIGEL_Tools::SIG_Randomizer &randomizer, long length, QString const &name )
{
  SIG_GPParameter lengthParameter;
  lengthParameter.setMinIndLength( length );
  lengthParameter.setMaxIndLength( length );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  std::unique_ptr<SIG_GPIndividual> individual = std::make_unique<SIG_GPIndividual>( randomizer, lengthParameter, languageParameters );
  individual->setName( name );
  individual->setFitness( 0.75 );
  individual->setPoolPos( 1 );
  return individual;
}

SIGEL_Program::SIG_Program SIGEL_GP::TST_SIG_GPOperations::program( SIGEL_Tools::SIG_Randomizer &randomizer, long length )
{
  SIG_GPParameter lengthParameter;
  lengthParameter.setMinIndLength( length );
  lengthParameter.setMaxIndLength( length );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  return SIGEL_Program::SIG_Program( lengthParameter, languageParameters, randomizer );
}

QStringList SIGEL_GP::TST_SIG_GPOperations::lineTexts( SIGEL_Program::SIG_Program const &program )
{
  QStringList texts;
  for ( long i = 0; i < program.getProgramLength(); i++ )
  {
    QString text;
    program.getLine( i ).printToString( text );
    texts.append( text );
  }
  return texts;
}

bool SIGEL_GP::TST_SIG_GPOperations::joinsRunsOfTheParents( QStringList const &child, QStringList const &first, QStringList const &second, bool mayBeCut )
{
  // child = first[0, a) + second[b, c) + first[d, end), with 1 <= a <= d: every crossover
  // point is at least 1.
  for ( qsizetype a = 1; a <= first.size() && first.mid( 0, a ) == child.mid( 0, a ); a++ )
  {
    if ( mayBeCut && a == child.size() )
      return true;
    for ( qsizetype b = 0; b <= second.size(); b++ )
    {
      for ( qsizetype c = b; c <= second.size() && second.mid( b, c - b ) == child.mid( a, c - b ); c++ )
      {
        QStringList const rest = child.mid( a + c - b );
        for ( qsizetype d = a; d <= first.size(); d++ )
        {
          QStringList const tail = first.mid( d );
          if ( tail == rest || ( mayBeCut && tail.mid( 0, rest.size() ) == rest ) )
            return true;
        }
      }
    }
  }
  return false;
}

QString SIGEL_GP::TST_SIG_GPOperations::nopText()
{
  SIGEL_Program::SIG_ProgramLine nop;
  nop.setRobotInstruction( SIGEL_Program::NOP, 0, 0 );
  QString text;
  nop.printToString( text );
  return text;
}

QString SIGEL_GP::TST_SIG_GPOperations::mutationKind( QStringList const &parent, QStringList const &child )
{
  if ( child.size() == parent.size() )
  {
    int differences = 0;
    for ( qsizetype i = 0; i < parent.size(); i++ )
    {
      if ( parent[ i ] != child[ i ] )
        differences++;
    }
    if ( differences == 0 )
      return "unchanged";
    return differences == 1 ? "changed" : QString();
  }

  // The longer list without one of its lines must be the shorter one.
  QStringList const &longer = child.size() > parent.size() ? child : parent;
  QStringList const &shorter = child.size() > parent.size() ? parent : child;
  if ( longer.size() != shorter.size() + 1 )
    return QString();

  for ( qsizetype i = 0; i < longer.size(); i++ )
  {
    QStringList rest = longer;
    rest.removeAt( i );
    if ( rest == shorter )
      return child.size() > parent.size() ? "added" : "deleted";
  }
  return QString();
}
