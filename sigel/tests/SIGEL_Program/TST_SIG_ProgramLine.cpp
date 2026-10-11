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
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QtTest>

void SIGEL_Program::TST_SIG_ProgramLine::setRobotinstructionSetsTypeAndElements_data()
{
  QTest::addColumn< Robotinstruction >( "instruction" );
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
}

void SIGEL_Program::TST_SIG_ProgramLine::setRobotinstructionSetsTypeAndElements()
{
  QFETCH( Robotinstruction, instruction );
  QFETCH( int, numberOfElements );
  QFETCH( int, element0 );
  QFETCH( int, element1 );
  QList<int> expectedElements = { element0, element1 };
  expectedElements.resize( numberOfElements );
  SIG_ProgramLine programLine;

  programLine.setRobotinstruction( instruction, -12, -34 );

  QCOMPARE( programLine.getRobotinstructionType(), instruction );
  QCOMPARE( programLine.getNumberOfElements(), numberOfElements );
  QCOMPARE( programLine.getElement( 0 ), element0 );
  QCOMPARE( programLine.getElement( 1 ), element1 );
  QCOMPARE( programLine.getInstructionElement( 0 ), element0 );
  QCOMPARE( programLine.getInstructionElement( 1 ), element1 );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::printToStringGivesTheShownText_data()
{
  QTest::addColumn< Robotinstruction >( "instruction" );
  QTest::addColumn< QString >( "text" );

  QTest::newRow( "MOVE" ) << MOVE << "MOVE -12\n";
  QTest::newRow( "LOAD" ) << LOAD << "LOAD -12,-34\n";
  QTest::newRow( "NOP" ) << NOP << "NOP\n";
  QTest::newRow( "COPY" ) << COPY << "COPY -12,-34\n";
  QTest::newRow( "ADD" ) << ADD << "ADD -12,-34\n";
  QTest::newRow( "SUB" ) << SUB << "SUB -12,-34\n";
  QTest::newRow( "MUL" ) << MUL << "MUL -12,-34\n";
  QTest::newRow( "DIV" ) << DIV << "DIV -12, -34\n";
  QTest::newRow( "MOD" ) << MOD << "MOD -12,-34\n";
  QTest::newRow( "MIN" ) << MIN << "MIN -12,-34\n";
  QTest::newRow( "MAX" ) << MAX << "MAX -12,-34\n";
  QTest::newRow( "CMP" ) << CMP << "CMP -12,-34\n";
  QTest::newRow( "JMP" ) << JMP << "JMP -12\n";
}

void SIGEL_Program::TST_SIG_ProgramLine::printToStringGivesTheShownText()
{
  QFETCH( Robotinstruction, instruction );
  QFETCH( QString, text );
  SIG_ProgramLine programLine;
  programLine.setRobotinstruction( instruction, -12, -34 );
  QString shownText;

  programLine.printToString( shownText );

  QCOMPARE( shownText, text );
}

void SIGEL_Program::TST_SIG_ProgramLine::writeToFileGivesTheFileText_data()
{
  QTest::addColumn< Robotinstruction >( "instruction" );
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
}

void SIGEL_Program::TST_SIG_ProgramLine::writeToFileGivesTheFileText()
{
  QFETCH( Robotinstruction, instruction );
  QFETCH( QString, text );
  SIG_ProgramLine programLine;
  programLine.setRobotinstruction( instruction, -12, -34 );
  QString fileText;
  QTextStream file( &fileText );

  programLine.writeToFile( file );
  file.flush();

  QCOMPARE( fileText, text );
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileReadsEachTextForm_data()
{
  QTest::addColumn< QString >( "text" );
  QTest::addColumn< Robotinstruction >( "instruction" );
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
  QTest::newRow( "DIV shown" ) << "DIV -12, -34" << DIV << 2 << -12 << -34;
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
}

void SIGEL_Program::TST_SIG_ProgramLine::readFromFileReadsEachTextForm()
{
  QFETCH( QString, text );
  QFETCH( Robotinstruction, instruction );
  QFETCH( int, numberOfElements );
  QFETCH( int, element0 );
  QFETCH( int, element1 );
  QList<int> expectedElements = { element0, element1 };
  expectedElements.resize( numberOfElements );
  // The line starts as a different instruction, so that the read must change it.
  SIG_ProgramLine programLine;
  if ( instruction == NOP )
    programLine.setRobotinstruction( COPY, 1, 1 );
  else
    programLine.setRobotinstruction( NOP, 0, 0 );

  const bool lineRead = programLine.readFromFile( text, &programLine );

  QVERIFY( lineRead );
  QCOMPARE( programLine.getRobotinstructionType(), instruction );
  QCOMPARE( programLine.getElementsArray(), expectedElements );
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyAllowedCommand_data()
{
  QTest::addColumn< QString >( "name" );
  QTest::addColumn< Robotinstruction >( "instruction" );

  QTest::newRow( "MOVE" ) << "MOVE" << MOVE;
  QTest::newRow( "LOAD" ) << "LOAD" << LOAD;
  QTest::newRow( "NOP" ) << "NOP" << NOP;
  QTest::newRow( "COPY" ) << "COPY" << COPY;
  QTest::newRow( "ADD" ) << "ADD" << ADD;
  QTest::newRow( "SUB" ) << "SUB" << SUB;
  QTest::newRow( "MUL" ) << "MUL" << MUL;
  QTest::newRow( "DIV" ) << "DIV" << DIV;
  QTest::newRow( "MOD" ) << "MOD" << MOD;
  QTest::newRow( "MIN" ) << "MIN" << MIN;
  QTest::newRow( "MAX" ) << "MAX" << MAX;
  QTest::newRow( "CMP" ) << "CMP" << CMP;
  QTest::newRow( "JMP" ) << "JMP" << JMP;
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyAllowedCommand()
{
  QFETCH( QString, name );
  QFETCH( Robotinstruction, instruction );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QStringList otherNames;
  for ( const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &command : languageParameters.getCommands() )
    if ( command.name != name )
      otherNames.append( command.name );
  for ( const QString &otherName : otherNames )
    languageParameters.removeCommand( otherName );
  QList<int> probabilities( 15, 1 );
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );

  for ( int count = 0; count < 100; count++ )
    {
      SIG_ProgramLine programLine( randomizer, languageParameters, probabilities );

      QCOMPARE( programLine.getRobotinstructionType(), instruction );
    }
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyCommandWithAWeight_data()
{
  QTest::addColumn< Robotinstruction >( "instruction" );

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
}

void SIGEL_Program::TST_SIG_ProgramLine::randomInstructionIsTheOnlyCommandWithAWeight()
{
  QFETCH( Robotinstruction, instruction );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  // probabilities has one entry for each instruction, in the order of Robotinstruction.
  QList<int> probabilities( 15, 0 );
  probabilities[instruction] = 5;
  SIGEL_Tools::SIG_Randomizer randomizer( 42 );

  for ( int count = 0; count < 100; count++ )
    {
      SIG_ProgramLine programLine;
      programLine.generateRandomRobotInstruction( languageParameters, randomizer, probabilities );

      QCOMPARE( programLine.getRobotinstructionType(), instruction );
    }
}
