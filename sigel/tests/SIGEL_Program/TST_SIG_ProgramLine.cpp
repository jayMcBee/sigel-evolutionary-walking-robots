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
#include "SIGEL_Program/TST_SIG_ProgramLine.h"

#include "SIGEL_Program/SIG_ProgramLine.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QtTest>

void SIGEL_Program::TST_SIG_ProgramLine::setRobotInstructionSetsTypeAndElements_data()
{
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfElements" );
  QTest::addColumn< int >( "element0" );
  QTest::addColumn< int >( "element1" );

  QTest::newRow( "MOVE" ) << MOVE << 1 << -12 << 0;
  QTest::newRow( "LOAD" ) << LOAD << 2 << -12 << -34;
  QTest::newRow( "NOP" ) << NOP << 0 << 0 << 0;
  QTest::newRow( "COPY" ) << COPY << 2 << -12 << -34;
  QTest::newRow( "ADD" ) << ADD << 2 << -12 << -34;
  QTest::newRow( "SUB" ) << SUB << 2 << -12 << -34;
  QTest::newRow( "MUL" ) << MUL << 2 << -12 << -34;
  QTest::newRow( "DIV" ) << DIV << 2 << -12 << -34;
  QTest::newRow( "MOD" ) << MOD << 2 << -12 << -34;
  QTest::newRow( "MIN" ) << MIN << 2 << -12 << -34;
  QTest::newRow( "MAX" ) << MAX << 2 << -12 << -34;
  QTest::newRow( "CMP" ) << CMP << 2 << -12 << -34;
  QTest::newRow( "JMP" ) << JMP << 1 << -12 << 0;
  QTest::newRow( "SENSE" ) << SENSE << 1 << -12 << 0;
  QTest::newRow( "DELAY" ) << DELAY << 1 << -12 << 0;
}

void SIGEL_Program::TST_SIG_ProgramLine::setRobotInstructionSetsTypeAndElements()
{
  QFETCH( RobotInstruction, instruction );
  QFETCH( int, numberOfElements );
  QFETCH( int, element0 );
  QFETCH( int, element1 );
  QList<int> expectedElements = { element0, element1 };
  expectedElements.resize( numberOfElements );

  // The line starts as a different instruction with three elements.
  SIG_ProgramLine programLine;
  if ( instruction == ADD )
    programLine.setRobotInstruction( SUB, 7, 8 );
  else
    programLine.setRobotInstruction( ADD, 7, 8 );
  programLine.resizeElements( 3 );
  programLine.setElement( 2, 9 );

  programLine.setRobotInstruction( instruction, -12, -34 );

  QCOMPARE( programLine.getRobotInstructionType(), instruction );
  QCOMPARE( programLine.getNumberOfElements(), numberOfElements );
  QCOMPARE( programLine.getElement( 0 ), element0 );
  QCOMPARE( programLine.getElement( 1 ), element1 );
  QCOMPARE( programLine.getInstructionElement( 0 ), element0 );
  QCOMPARE( programLine.getInstructionElement( 1 ), element1 );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::printToStringGivesTheShownText_data()
{
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< QString >( "text" );

  QTest::newRow( "MOVE" ) << MOVE << "MOVE -12\n";
  QTest::newRow( "LOAD" ) << LOAD << "LOAD -12,-34\n";
  QTest::newRow( "NOP" ) << NOP << "NOP\n";
  QTest::newRow( "COPY" ) << COPY << "COPY -12,-34\n";
  QTest::newRow( "ADD" ) << ADD << "ADD -12,-34\n";
  QTest::newRow( "SUB" ) << SUB << "SUB -12,-34\n";
  QTest::newRow( "MUL" ) << MUL << "MUL -12,-34\n";
  QTest::newRow( "DIV" ) << DIV << "DIV -12,-34\n";
  QTest::newRow( "MOD" ) << MOD << "MOD -12,-34\n";
  QTest::newRow( "MIN" ) << MIN << "MIN -12,-34\n";
  QTest::newRow( "MAX" ) << MAX << "MAX -12,-34\n";
  QTest::newRow( "CMP" ) << CMP << "CMP -12,-34\n";
  QTest::newRow( "JMP" ) << JMP << "JMP -12\n";
  QTest::newRow( "SENSE" ) << SENSE << "SENSE -12\n";
  QTest::newRow( "DELAY" ) << DELAY << "DELAY -12\n";
}

void SIGEL_Program::TST_SIG_ProgramLine::printToStringGivesTheShownText()
{
  QFETCH( RobotInstruction, instruction );
  QFETCH( QString, text );
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( instruction, -12, -34 );
  QString shownText;

  programLine.printToString( shownText );

  QCOMPARE( shownText, text );
}

void SIGEL_Program::TST_SIG_ProgramLine::writeToFileGivesTheFileText_data()
{
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< QString >( "text" );

  QTest::newRow( "MOVE" ) << MOVE << "       MOVE -12\n";
  QTest::newRow( "LOAD" ) << LOAD << "       LOAD -12,-34\n";
  QTest::newRow( "NOP" ) << NOP << "       NOP \n";
  QTest::newRow( "COPY" ) << COPY << "       COPY -12,-34\n";
  QTest::newRow( "ADD" ) << ADD << "       ADD -12,-34\n";
  QTest::newRow( "SUB" ) << SUB << "       SUB -12,-34\n";
  QTest::newRow( "MUL" ) << MUL << "       MUL -12,-34\n";
  QTest::newRow( "DIV" ) << DIV << "       DIV -12,-34\n";
  QTest::newRow( "MOD" ) << MOD << "       MOD -12,-34\n";
  QTest::newRow( "MIN" ) << MIN << "       MIN -12,-34\n";
  QTest::newRow( "MAX" ) << MAX << "       MAX -12,-34\n";
  QTest::newRow( "CMP" ) << CMP << "       CMP -12,-34\n";
  QTest::newRow( "JMP" ) << JMP << "       JMP -12\n";
  QTest::newRow( "SENSE" ) << SENSE << "       SENSE -12\n";
  QTest::newRow( "DELAY" ) << DELAY << "       DELAY -12\n";
}

void SIGEL_Program::TST_SIG_ProgramLine::writeToFileGivesTheFileText()
{
  QFETCH( RobotInstruction, instruction );
  QFETCH( QString, text );
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( instruction, -12, -34 );
  QString fileText;
  QTextStream file( &fileText );

  programLine.writeToFile( file );
  file.flush();

  QCOMPARE( fileText, text );
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileReadsEachTextForm_data()
{
  QTest::addColumn< QString >( "text" );
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfElements" );
  QTest::addColumn< int >( "element0" );
  QTest::addColumn< int >( "element1" );

  QTest::newRow( "MOVE file" ) << "       MOVE -12" << MOVE << 1 << -12 << 0;
  QTest::newRow( "MOVE shown" ) << "MOVE -12" << MOVE << 1 << -12 << 0;
  QTest::newRow( "MOVE lower case" ) << "       move -12" << MOVE << 1 << -12 << 0;
  QTest::newRow( "LOAD file" ) << "       LOAD -12,-34" << LOAD << 2 << -12 << -34;
  QTest::newRow( "LOAD shown" ) << "LOAD -12,-34" << LOAD << 2 << -12 << -34;
  QTest::newRow( "LOAD lower case" ) << "       load -12,-34" << LOAD << 2 << -12 << -34;
  QTest::newRow( "NOP file" ) << "       NOP " << NOP << 0 << 0 << 0;
  QTest::newRow( "NOP shown" ) << "NOP" << NOP << 0 << 0 << 0;
  QTest::newRow( "NOP lower case" ) << "       nop " << NOP << 0 << 0 << 0;
  QTest::newRow( "COPY file" ) << "       COPY -12,-34" << COPY << 2 << -12 << -34;
  QTest::newRow( "COPY shown" ) << "COPY -12,-34" << COPY << 2 << -12 << -34;
  QTest::newRow( "COPY lower case" ) << "       copy -12,-34" << COPY << 2 << -12 << -34;
  QTest::newRow( "ADD file" ) << "       ADD -12,-34" << ADD << 2 << -12 << -34;
  QTest::newRow( "ADD shown" ) << "ADD -12,-34" << ADD << 2 << -12 << -34;
  QTest::newRow( "ADD lower case" ) << "       add -12,-34" << ADD << 2 << -12 << -34;
  QTest::newRow( "SUB file" ) << "       SUB -12,-34" << SUB << 2 << -12 << -34;
  QTest::newRow( "SUB shown" ) << "SUB -12,-34" << SUB << 2 << -12 << -34;
  QTest::newRow( "SUB lower case" ) << "       sub -12,-34" << SUB << 2 << -12 << -34;
  QTest::newRow( "MUL file" ) << "       MUL -12,-34" << MUL << 2 << -12 << -34;
  QTest::newRow( "MUL shown" ) << "MUL -12,-34" << MUL << 2 << -12 << -34;
  QTest::newRow( "MUL lower case" ) << "       mul -12,-34" << MUL << 2 << -12 << -34;
  QTest::newRow( "DIV file" ) << "       DIV -12,-34" << DIV << 2 << -12 << -34;
  QTest::newRow( "DIV space after the comma" ) << "DIV -12, -34" << DIV << 2 << -12 << -34;
  QTest::newRow( "DIV lower case" ) << "       div -12,-34" << DIV << 2 << -12 << -34;
  QTest::newRow( "MOD file" ) << "       MOD -12,-34" << MOD << 2 << -12 << -34;
  QTest::newRow( "MOD shown" ) << "MOD -12,-34" << MOD << 2 << -12 << -34;
  QTest::newRow( "MOD lower case" ) << "       mod -12,-34" << MOD << 2 << -12 << -34;
  QTest::newRow( "MIN file" ) << "       MIN -12,-34" << MIN << 2 << -12 << -34;
  QTest::newRow( "MIN shown" ) << "MIN -12,-34" << MIN << 2 << -12 << -34;
  QTest::newRow( "MIN lower case" ) << "       min -12,-34" << MIN << 2 << -12 << -34;
  QTest::newRow( "MAX file" ) << "       MAX -12,-34" << MAX << 2 << -12 << -34;
  QTest::newRow( "MAX shown" ) << "MAX -12,-34" << MAX << 2 << -12 << -34;
  QTest::newRow( "MAX lower case" ) << "       max -12,-34" << MAX << 2 << -12 << -34;
  QTest::newRow( "CMP file" ) << "       CMP -12,-34" << CMP << 2 << -12 << -34;
  QTest::newRow( "CMP shown" ) << "CMP -12,-34" << CMP << 2 << -12 << -34;
  QTest::newRow( "CMP lower case" ) << "       cmp -12,-34" << CMP << 2 << -12 << -34;
  QTest::newRow( "JMP file" ) << "       JMP -12" << JMP << 1 << -12 << 0;
  QTest::newRow( "JMP shown" ) << "JMP -12" << JMP << 1 << -12 << 0;
  QTest::newRow( "JMP lower case" ) << "       jmp -12" << JMP << 1 << -12 << 0;
  QTest::newRow( "SENSE file" ) << "       SENSE -12" << SENSE << 1 << -12 << 0;
  QTest::newRow( "SENSE shown" ) << "SENSE -12" << SENSE << 1 << -12 << 0;
  QTest::newRow( "SENSE lower case" ) << "       sense -12" << SENSE << 1 << -12 << 0;
  QTest::newRow( "DELAY file" ) << "       DELAY -12" << DELAY << 1 << -12 << 0;
  QTest::newRow( "DELAY shown" ) << "DELAY -12" << DELAY << 1 << -12 << 0;
  QTest::newRow( "DELAY lower case" ) << "       delay -12" << DELAY << 1 << -12 << 0;
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileReadsEachTextForm()
{
  QFETCH( QString, text );
  QFETCH( RobotInstruction, instruction );
  QFETCH( int, numberOfElements );
  QFETCH( int, element0 );
  QFETCH( int, element1 );
  QList<int> expectedElements = { element0, element1 };
  expectedElements.resize( numberOfElements );

  // The line starts as a different instruction.
  SIG_ProgramLine programLine;
  if ( instruction == NOP )
    programLine.setRobotInstruction( COPY, 1, 1 );
  else
    programLine.setRobotInstruction( NOP, 0, 0 );

  const bool lineRead = programLine.readFromFile( text, &programLine );

  QVERIFY( lineRead );
  QCOMPARE( programLine.getRobotInstructionType(), instruction );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileAllowsExtraWhiteSpace_data()
{
  QTest::addColumn< QString >( "text" );
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfElements" );
  QTest::addColumn< int >( "element0" );
  QTest::addColumn< int >( "element1" );

  QTest::newRow( "ADD spaces around the comma" ) << "ADD  1 , 2" << ADD << 2 << 1 << 2;
  QTest::newRow( "ADD space at the end" ) << "ADD 1,2 " << ADD << 2 << 1 << 2;
  QTest::newRow( "ADD tabs" ) << "\tADD\t1,2" << ADD << 2 << 1 << 2;
  QTest::newRow( "MOVE tab before the operand" ) << "MOVE\t5" << MOVE << 1 << 5 << 0;
  QTest::newRow( "LOAD tab before the operands" ) << "LOAD\t1,2" << LOAD << 2 << 1 << 2;
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileAllowsExtraWhiteSpace()
{
  QFETCH( QString, text );
  QFETCH( RobotInstruction, instruction );
  QFETCH( int, numberOfElements );
  QFETCH( int, element0 );
  QFETCH( int, element1 );
  QList<int> expectedElements = { element0, element1 };
  expectedElements.resize( numberOfElements );
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( NOP, 0, 0 );

  const bool lineRead = programLine.readFromFile( text, &programLine );

  QVERIFY( lineRead );
  QCOMPARE( programLine.getRobotInstructionType(), instruction );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileRefusesTextWithoutAnInstruction_data()
{
  QTest::addColumn< QString >( "text" );

  QTest::newRow( "empty" ) << "";
  QTest::newRow( "spaces" ) << "   ";
  QTest::newRow( "comment without an instruction name" ) << "# a comment";
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileRefusesTextWithoutAnInstruction()
{
  QFETCH( QString, text );
  const QList<int> expectedElements = { 3, 4 };
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( LOAD, 3, 4 );

  const bool lineRead = programLine.readFromFile( text, &programLine );

  QVERIFY( !lineRead );
  QCOMPARE( programLine.getRobotInstructionType(), LOAD );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileGivesAnErrorForABadOperand_data()
{
  QTest::addColumn< QString >( "text" );
  QTest::addColumn< QString >( "message" );

  QTest::newRow( "second operand missing" ) << "ADD 1" << "Program line 'ADD 1' has a bad operand.";
  QTest::newRow( "operands are not numbers" ) << "ADD x,y" << "Program line 'ADD x,y' has a bad operand.";
  QTest::newRow( "operand missing" ) << "MOVE" << "Program line 'MOVE' has a bad operand.";
  QTest::newRow( "text after the operands" ) << "ADD 1,2 # note" << "Program line 'ADD 1,2 # note' has a bad operand.";
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileGivesAnErrorForABadOperand()
{
  QFETCH( QString, text );
  QFETCH( QString, message );
  const QList<int> expectedElements = { 3, 4 };
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( LOAD, 3, 4 );
  QString thrown;

  try
    {
      programLine.readFromFile( text, &programLine );
    }
  catch ( const SIGEL_Tools::SIG_Exception &e )
    {
      thrown = e.getMessage();
    }

  QVERIFY2( thrown.contains( message ), qPrintable( thrown ) );
  QCOMPARE( programLine.getRobotInstructionType(), LOAD );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::getElementOutsideTheElementsGivesZero()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );

  QCOMPARE( programLine.getElement( 2 ), 0 );
  QCOMPARE( programLine.getElement( -1 ), 0 );
  QCOMPARE( programLine.getInstructionElement( 2 ), 0 );
  QCOMPARE( programLine.getInstructionElement( -1 ), 0 );
}

void SIGEL_Program::TST_SIG_ProgramLine::setElementChangesOneElement()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );
  const QList<int> expectedElements = { 1, 9 };

  programLine.setElement( 1, 9 );

  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::setElementOutsideTheElementsChangesNothing()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );
  const QList<int> expectedElements = { 1, 2 };

  programLine.setElement( 2, 9 );
  programLine.setElement( -1, 9 );

  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::resizeElementsKeepsTheFirstElementsAndAddsZeros()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );
  const QList<int> expectedElementsAfterTheFirstResize = { 1 };
  const QList<int> expectedElementsAfterTheSecondResize = { 1, 0, 0 };

  programLine.resizeElements( 1 );
  QCOMPARE( programLine.getElementsArray(), expectedElementsAfterTheFirstResize );

  programLine.resizeElements( 3 );
  QCOMPARE( programLine.getElementsArray(), expectedElementsAfterTheSecondResize );
}

void SIGEL_Program::TST_SIG_ProgramLine::clearLineRemovesTheElementsAndKeepsTheType()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );

  programLine.clearLine();

  QCOMPARE( programLine.getNumberOfElements(), 0 );
  QCOMPARE( programLine.getRobotInstructionType(), ADD );
}

void SIGEL_Program::TST_SIG_ProgramLine::setRobotInstructionTypeKeepsTheElements()
{
  SIG_ProgramLine programLine;
  programLine.setRobotInstruction( ADD, 1, 2 );
  const QList<int> expectedElements = { 1, 2 };

  programLine.setRobotInstructionType( JMP );

  QCOMPARE( programLine.getRobotInstructionType(), JMP );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::assignmentCopiesTypeAndElements()
{
  SIG_ProgramLine source;
  source.setRobotInstruction( LOAD, 3, 4 );
  SIG_ProgramLine destination;
  destination.setRobotInstruction( NOP, 0, 0 );
  const QList<int> expectedElements = { 3, 4 };

  destination = source;
  source.setElement( 0, 8 );

  QCOMPARE( destination.getRobotInstructionType(), LOAD );
  QCOMPARE( destination.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::copyLineCopiesTypeAndElements()
{
  SIG_ProgramLine source;
  source.setRobotInstruction( LOAD, 3, 4 );
  SIG_ProgramLine destination;
  destination.setRobotInstruction( NOP, 0, 0 );
  const QList<int> expectedElements = { 3, 4 };

  destination.copyLine( &source, &destination );
  source.setElement( 0, 8 );

  QCOMPARE( destination.getRobotInstructionType(), LOAD );
  QCOMPARE( destination.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::assignmentOfAShorterLineRemovesTheElements()
{
  SIG_ProgramLine source;
  source.setRobotInstruction( NOP, 0, 0 );
  SIG_ProgramLine destination;
  destination.setRobotInstruction( LOAD, 3, 4 );

  destination = source;

  QCOMPARE( destination.getRobotInstructionType(), NOP );
  QCOMPARE( destination.getNumberOfElements(), 0 );
}

void SIGEL_Program::TST_SIG_ProgramLine::copyLineOfAShorterLineRemovesTheElements()
{
  SIG_ProgramLine source;
  source.setRobotInstruction( NOP, 0, 0 );
  SIG_ProgramLine destination;
  destination.setRobotInstruction( LOAD, 3, 4 );

  destination.copyLine( &source, &destination );

  QCOMPARE( destination.getRobotInstructionType(), NOP );
  QCOMPARE( destination.getNumberOfElements(), 0 );
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyCommandWithAWeight_data()
{
  QTest::addColumn< RobotInstruction >( "instruction" );

  QTest::newRow( "MOVE" ) << MOVE;
  QTest::newRow( "LOAD" ) << LOAD;
  QTest::newRow( "NOP" ) << NOP;
  QTest::newRow( "COPY" ) << COPY;
  QTest::newRow( "ADD" ) << ADD;
  QTest::newRow( "SUB" ) << SUB;
  QTest::newRow( "MUL" ) << MUL;
  QTest::newRow( "DIV" ) << DIV;
  QTest::newRow( "MOD" ) << MOD;
  QTest::newRow( "MIN" ) << MIN;
  QTest::newRow( "MAX" ) << MAX;
  QTest::newRow( "CMP" ) << CMP;
  QTest::newRow( "JMP" ) << JMP;
  QTest::newRow( "SENSE" ) << SENSE;
  QTest::newRow( "DELAY" ) << DELAY;
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyCommandWithAWeight()
{
  QFETCH( RobotInstruction, instruction );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  // probabilities has one entry for each instruction, in the order of RobotInstruction.
  QList<int> probabilities( 15, 0 );
  probabilities[instruction] = 5;
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );

  for ( int count = 0; count < 100; count++ )
    {
      SIG_ProgramLine programLine;
      programLine.generateRandomRobotInstruction( languageParameters, randomizer, probabilities );

      QCOMPARE( programLine.getRobotInstructionType(), instruction );
    }
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsNeverTheCommandWithoutAWeight()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  // probabilities has one entry for each instruction, in the order of RobotInstruction.
  QList<int> probabilities( 15, 1 );
  probabilities[COPY] = 0;
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );
  QList<bool> typeSeen( 15, false );
  QList<bool> expectedTypeSeen( 15, true );
  expectedTypeSeen[COPY] = false;

  for ( int count = 0; count < 10000; count++ )
    {
      SIG_ProgramLine programLine( randomizer, languageParameters, probabilities );

      typeSeen[programLine.getRobotInstructionType()] = true;
    }

  QCOMPARE( typeSeen, expectedTypeSeen );
}

void SIGEL_Program::TST_SIG_ProgramLine::randomOperandsAreInsideTheRange()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QList<int> probabilities( 15, 1 );
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );
  bool negativeElementSeen = false;
  bool positiveElementSeen = false;

  // typeSeen has one entry for each instruction, in the order of RobotInstruction.
  QList<bool> typeSeen( 15, false );

  for ( int count = 0; count < 10000; count++ )
    {
      SIG_ProgramLine programLine( randomizer, languageParameters, probabilities );

      typeSeen[programLine.getRobotInstructionType()] = true;
      for ( int element : programLine.getElementsArray() )
        {
          QVERIFY( element >= -31999 );
          QVERIFY( element <= 31999 );
          if ( element < 0 )
            negativeElementSeen = true;
          if ( element > 0 )
            positiveElementSeen = true;
        }
    }

  QVERIFY( negativeElementSeen );
  QVERIFY( positiveElementSeen );
  QCOMPARE( typeSeen, QList<bool>( 15, true ) );
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyAllowedCommandWithOperandsFromTheRandomizer_data()
{
  QTest::addColumn< QString >( "name" );
  QTest::addColumn< RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfElements" );

  QTest::newRow( "MOVE" ) << "MOVE" << MOVE << 1;
  QTest::newRow( "LOAD" ) << "LOAD" << LOAD << 2;
  QTest::newRow( "NOP" ) << "NOP" << NOP << 0;
  QTest::newRow( "COPY" ) << "COPY" << COPY << 2;
  QTest::newRow( "ADD" ) << "ADD" << ADD << 2;
  QTest::newRow( "SUB" ) << "SUB" << SUB << 2;
  QTest::newRow( "MUL" ) << "MUL" << MUL << 2;
  QTest::newRow( "DIV" ) << "DIV" << DIV << 2;
  QTest::newRow( "MOD" ) << "MOD" << MOD << 2;
  QTest::newRow( "MIN" ) << "MIN" << MIN << 2;
  QTest::newRow( "MAX" ) << "MAX" << MAX << 2;
  QTest::newRow( "CMP" ) << "CMP" << CMP << 2;
  QTest::newRow( "JMP" ) << "JMP" << JMP << 1;
  QTest::newRow( "SENSE" ) << "SENSE" << SENSE << 1;
  QTest::newRow( "DELAY" ) << "DELAY" << DELAY << 1;
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyAllowedCommandWithOperandsFromTheRandomizer()
{
  QFETCH( QString, name );
  QFETCH( RobotInstruction, instruction );
  QFETCH( int, numberOfElements );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QStringList otherNames;
  for ( const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &command : languageParameters.getCommands() )
    if ( command.name != name )
      otherNames.append( command.name );
  for ( const QString &otherName : otherNames )
    languageParameters.removeCommand( otherName );
  QList<int> probabilities( 15, 1 );
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );
  SIGEL_Tools::SIG_Randomizer expectedRandomizer( 42 );

  for ( int count = 0; count < 1000; count++ )
    {
      // A random instruction takes five numbers from the randomizer: two operands, two signs and the choice of the instruction.
      int firstOperand = expectedRandomizer.getRandomInt( 32000 );
      int secondOperand = expectedRandomizer.getRandomInt( 32000 );
      if ( expectedRandomizer.getRandomInt( 2 ) == 1 )
        firstOperand = -firstOperand;
      if ( expectedRandomizer.getRandomInt( 2 ) == 1 )
        secondOperand = -secondOperand;
      expectedRandomizer.getRandomLong( 1 );
      QList<int> expectedElements = { firstOperand, secondOperand };
      expectedElements.resize( numberOfElements );
      SIG_ProgramLine programLine;

      programLine.randomRobotInstruction( languageParameters, randomizer, probabilities );

      QCOMPARE( programLine.getRobotInstructionType(), instruction );
      QCOMPARE( programLine.getElementsArray(), expectedElements );
    }
}

void SIGEL_Program::TST_SIG_ProgramLine::sameSeedGivesTheSameInstructions()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QList<int> probabilities( 15, 1 );
  SIGEL_Tools::SIG_Randomizer firstRandomizer( 7 );
  SIGEL_Tools::SIG_Randomizer secondRandomizer( 7 );

  for ( int count = 0; count < 100; count++ )
    {
      SIG_ProgramLine firstProgramLine( firstRandomizer, languageParameters, probabilities );
      SIG_ProgramLine secondProgramLine( secondRandomizer, languageParameters, probabilities );
      QString firstShownText;
      QString secondShownText;

      firstProgramLine.printToString( firstShownText );
      secondProgramLine.printToString( secondShownText );

      QCOMPARE( firstShownText, secondShownText );
    }
}

void SIGEL_Program::TST_SIG_ProgramLine::seedOneGivesKnownInstructions()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QList<int> probabilities( 15, 1 );
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  const QStringList expectedShownTexts = { "LOAD -16838,-5758\n", "MOD -5627,23010\n", "DIV 2749,12767\n" };

  for ( const QString &expectedShownText : expectedShownTexts )
    {
      SIG_ProgramLine programLine( randomizer, languageParameters, probabilities );
      QString shownText;

      programLine.printToString( shownText );

      QCOMPARE( shownText, expectedShownText );
    }
}
