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
#include "SIGEL_Program/TST_SIG_Program.h"

#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QFile>
#include <QSet>
#include <QString>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

void SIGEL_Program::TST_SIG_Program::defaultConstructorGivesAnEmptyProgram()
{
  SIG_Program program;

  QCOMPARE( program.getProgramLength(), 0L );
}

void SIGEL_Program::TST_SIG_Program::appendLineAddsTheLineAtTheEnd()
{
  SIG_Program program;
  appendLines( program, 1, 2 );
  SIG_ProgramLine line;
  line.setRobotInstruction( MOVE, 7, 0 );

  program.appendLine( line );

  QCOMPARE( lineTexts( program ), expectedTexts( 1, 2 ) << "MOVE 7\n" );
}

void SIGEL_Program::TST_SIG_Program::insertLinePutsTheLineAtTheIndex_data()
{
  QTest::addColumn< int >( "index" );

  QTest::newRow( "first" ) << 0;
  QTest::newRow( "middle" ) << 1;
  QTest::newRow( "after the last" ) << 3;
}

void SIGEL_Program::TST_SIG_Program::insertLinePutsTheLineAtTheIndex()
{
  QFETCH( int, index );
  SIG_Program program;
  appendLines( program, 1, 3 );
  SIG_ProgramLine line;
  line.setRobotInstruction( MOVE, 7, 0 );
  QStringList expected = expectedTexts( 1, 3 );
  expected.insert( index, "MOVE 7\n" );

  program.insertLine( index, line );

  QCOMPARE( lineTexts( program ), expected );
}

void SIGEL_Program::TST_SIG_Program::deleteLineRemovesTheLineAndMovesTheNextLinesUp_data()
{
  QTest::addColumn< int >( "index" );

  QTest::newRow( "first" ) << 0;
  QTest::newRow( "middle" ) << 1;
  QTest::newRow( "last" ) << 2;
}

void SIGEL_Program::TST_SIG_Program::deleteLineRemovesTheLineAndMovesTheNextLinesUp()
{
  QFETCH( int, index );
  SIG_Program program;
  appendLines( program, 1, 3 );
  QStringList expected = expectedTexts( 1, 3 );
  expected.removeAt( index );

  program.deleteLine( index );

  QCOMPARE( lineTexts( program ), expected );
}

void SIGEL_Program::TST_SIG_Program::deleteLinePastTheEndDeletesNothing()
{
  SIG_Program program;
  appendLines( program, 1, 3 );

  program.deleteLine( 3 );

  QCOMPARE( lineTexts( program ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::getLineOutsideTheProgramThrows_data()
{
  QTest::addColumn< int >( "index" );

  QTest::newRow( "before the first" ) << -1;
  QTest::newRow( "after the last" ) << 3;
}

void SIGEL_Program::TST_SIG_Program::getLineOutsideTheProgramThrows()
{
  QFETCH( int, index );
  SIG_Program program;
  appendLines( program, 1, 3 );

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, program.getLine( index ) );
}

void SIGEL_Program::TST_SIG_Program::setLineOverwritesOneLine()
{
  SIG_Program program;
  appendLines( program, 1, 3 );
  SIG_ProgramLine line;
  line.setRobotInstruction( MOVE, 7, 0 );
  QStringList expected = expectedTexts( 1, 3 );
  expected[1] = "MOVE 7\n";

  program.setLine( 1, line );

  QCOMPARE( lineTexts( program ), expected );
}

void SIGEL_Program::TST_SIG_Program::setLineOutsideTheProgramThrows_data()
{
  QTest::addColumn< int >( "index" );

  QTest::newRow( "before the first" ) << -1;
  QTest::newRow( "after the last" ) << 3;
}

void SIGEL_Program::TST_SIG_Program::setLineOutsideTheProgramThrows()
{
  QFETCH( int, index );
  SIG_Program program;
  appendLines( program, 1, 3 );
  SIG_ProgramLine line;
  line.setRobotInstruction( MOVE, 7, 0 );

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, program.setLine( index, line ) );
  QCOMPARE( lineTexts( program ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::getPrgLinesGivesTheLinesOfTheProgram()
{
  SIG_Program program;
  appendLines( program, 1, 3 );

  program.getPrgLines()[1].setRobotInstruction( MOVE, 7, 0 );
  program.getPrgLines().pop_back();

  QCOMPARE( lineTexts( program ), QStringList( { "ADD 1,0\n", "MOVE 7\n" } ) );
}

void SIGEL_Program::TST_SIG_Program::printToStringGivesTheShownTextOfEveryLine()
{
  SIG_Program program;
  appendLines( program, 1, 3 );
  QString text;

  program.printToString( text );

  QCOMPARE( text, QString( "ADD 1,0\nADD 2,-1\nADD 3,-2\n" ) );
}

void SIGEL_Program::TST_SIG_Program::writeToFileGivesTheFileTextOfEveryLine()
{
  SIG_Program program;
  appendLines( program, 1, 3 );
  QString text;
  QTextStream stream( &text );

  program.writeToFile( stream );
  stream.flush();

  QCOMPARE( text, QString( "       ADD 1,0\n       ADD 2,-1\n       ADD 3,-2\n" ) );
}

// A random program has every instruction, so that each line form goes through the file text.
void SIGEL_Program::TST_SIG_Program::readFromFileReadsWhatWriteToFileWrote()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 200 );
  gpParameter.setMaxIndLength( 200 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIG_Program written( gpParameter, languageParameters, randomizer );
  QSet<RobotInstruction> instructions;
  for ( long i = 0; i < written.getProgramLength(); i++ )
    instructions.insert( written.getLine( i ).getRobotInstructionType() );
  QCOMPARE( instructions.size(), instructionCount );
  QString text;
  QTextStream writeStream( &text );
  written.writeToFile( writeStream );
  writeStream.flush();
  QTextStream readStream( &text );
  SIG_Program read;

  read.readFromFile( readStream );

  QCOMPARE( lineTexts( read ), lineTexts( written ) );
}

void SIGEL_Program::TST_SIG_Program::readFromFileSkipsLinesWithoutAnInstruction()
{
  QString text = "ADD 1,0\n\n   \n# a note\nADD 2,-1\n";
  QTextStream stream( &text );
  SIG_Program program;

  program.readFromFile( stream );

  QCOMPARE( lineTexts( program ), expectedTexts( 1, 2 ) );
}

void SIGEL_Program::TST_SIG_Program::readFromFileReplacesTheOldLines()
{
  QString text = "ADD 1,0\nADD 2,-1\n";
  QTextStream stream( &text );
  SIG_Program program;
  appendLines( program, 100, 5 );

  program.readFromFile( stream );

  QCOMPARE( lineTexts( program ), expectedTexts( 1, 2 ) );
}

void SIGEL_Program::TST_SIG_Program::exportProgramWritesTheFileTextAndImportProgramReadsIt()
{
  QTemporaryDir folder;
  QString fileName = folder.filePath( "program.prg" );
  SIG_Program exported;
  appendLines( exported, 1, 3 );
  SIG_Program imported;
  appendLines( imported, 100, 5 );

  exported.exportProgram( fileName );
  imported.importProgram( fileName );

  QFile file( fileName );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QCOMPARE( QString( file.readAll() ), QString( "       ADD 1,0\n       ADD 2,-1\n       ADD 3,-2\n" ) );
  QCOMPARE( lineTexts( imported ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::exportProgramReplacesAnExistingFile()
{
  QTemporaryDir folder;
  QString fileName = folder.filePath( "program.prg" );
  SIG_Program longer;
  appendLines( longer, 100, 5 );
  longer.exportProgram( fileName );
  SIG_Program exported;
  appendLines( exported, 1, 2 );

  exported.exportProgram( fileName );

  QFile file( fileName );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QCOMPARE( QString( file.readAll() ), QString( "       ADD 1,0\n       ADD 2,-1\n" ) );
}

void SIGEL_Program::TST_SIG_Program::importProgramOfAMissingFileKeepsTheProgram()
{
  QTemporaryDir folder;
  QString fileName = folder.filePath( "missing.prg" );
  SIG_Program program;
  appendLines( program, 1, 3 );

  program.importProgram( fileName );

  QCOMPARE( lineTexts( program ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::importProgramOfABadFileThrowsAndKeepsTheProgram()
{
  QTemporaryDir folder;
  QString fileName = folder.filePath( "bad.prg" );
  QFile file( fileName );
  QVERIFY( file.open( QIODevice::WriteOnly ) );
  file.write( "ADD 100,0\nADD x,y\n" );
  file.close();
  SIG_Program program;
  appendLines( program, 1, 3 );

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, program.importProgram( fileName ) );
  QCOMPARE( lineTexts( program ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::generateRandomProgramKeepsTheLengthInsideTheLimits()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  bool minimumSeen = false;
  bool maximumSeen = false;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    SIG_Program program;

    program.generateRandomProgram( gpParameter, languageParameters, randomizer );

    long const length = program.getProgramLength();
    QVERIFY2( length >= 5 && length <= 15, message.constData() );
    minimumSeen = minimumSeen || length == 5;
    maximumSeen = maximumSeen || length == 15;
  }

  QVERIFY( minimumSeen );
  QVERIFY( maximumSeen );
}

void SIGEL_Program::TST_SIG_Program::generateRandomProgramWithMaximumZeroGivesTheMinimumLength()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 0 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  for ( int seed = 1; seed <= seeds; seed++ )
  {
    QByteArray const message = "seed " + QByteArray::number( seed );
    SIGEL_Tools::SIG_Randomizer randomizer( seed );
    SIG_Program program;

    program.generateRandomProgram( gpParameter, languageParameters, randomizer );

    QVERIFY2( program.getProgramLength() == 5, message.constData() );
  }
}

void SIGEL_Program::TST_SIG_Program::generateRandomProgramReplacesTheOldLines()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 3 );
  gpParameter.setMaxIndLength( 3 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIG_Program program;
  appendLines( program, 100, 5 );

  program.generateRandomProgram( gpParameter, languageParameters, randomizer );

  QCOMPARE( program.getProgramLength(), 3L );
  QVERIFY( !lineTexts( program ).contains( "ADD 100,0\n" ) );
}

void SIGEL_Program::TST_SIG_Program::generateRandomProgramUsesTheInstructionProbabilities()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 50 );
  gpParameter.setMaxIndLength( 50 );
  gpParameter.getInstructionProbabilities() = QList<int>( instructionCount, 0 );
  gpParameter.getInstructionProbabilities()[MUL] = 5;
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIG_Program program;

  program.generateRandomProgram( gpParameter, languageParameters, randomizer );

  QCOMPARE( program.getProgramLength(), 50L );
  for ( long i = 0; i < program.getProgramLength(); i++ )
    QCOMPARE( program.getLine( i ).getRobotInstructionType(), MUL );
}

void SIGEL_Program::TST_SIG_Program::generateRandomProgramUsesOnlyTheAllowedCommands()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 50 );
  gpParameter.setMaxIndLength( 50 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  QStringList otherNames;
  for ( const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &command : languageParameters.getCommands() )
    if ( command.name != "MUL" )
      otherNames.append( command.name );
  for ( const QString &otherName : otherNames )
    languageParameters.removeCommand( otherName );
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIG_Program program;

  program.generateRandomProgram( gpParameter, languageParameters, randomizer );

  QCOMPARE( program.getProgramLength(), 50L );
  for ( long i = 0; i < program.getProgramLength(); i++ )
    QCOMPARE( program.getLine( i ).getRobotInstructionType(), MUL );
}

void SIGEL_Program::TST_SIG_Program::constructorWithParametersGivesTheProgramOfGenerateRandomProgram()
{
  SIGEL_GP::SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 15 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Tools::SIG_Randomizer constructorRandomizer( 1 );
  SIGEL_Tools::SIG_Randomizer generateRandomizer( 1 );
  SIG_Program generated;

  SIG_Program constructed( gpParameter, languageParameters, constructorRandomizer );
  generated.generateRandomProgram( gpParameter, languageParameters, generateRandomizer );

  QVERIFY( constructed.getProgramLength() > 0 );
  QCOMPARE( lineTexts( constructed ), lineTexts( generated ) );
}

void SIGEL_Program::TST_SIG_Program::assignmentCopiesEveryLine()
{
  SIG_Program source;
  appendLines( source, 1, 3 );
  SIG_Program target;

  target = source;

  QCOMPARE( lineTexts( target ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::assignmentLeavesTheSourceAlone()
{
  SIG_Program source;
  appendLines( source, 1, 3 );
  SIG_Program target;
  appendLines( target, 100, 2 );

  target = source;

  QCOMPARE( lineTexts( source ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::assignmentOverALongerProgramLeavesNoOldLines()
{
  SIG_Program source;
  appendLines( source, 1, 2 );
  SIG_Program target;
  appendLines( target, 100, 5 );

  target = source;

  QCOMPARE( lineTexts( target ), expectedTexts( 1, 2 ) );
}

void SIGEL_Program::TST_SIG_Program::selfAssignmentKeepsTheProgram()
{
  SIG_Program target;
  appendLines( target, 1, 3 );

  target = target;

  QCOMPARE( lineTexts( target ), expectedTexts( 1, 3 ) );
}

void SIGEL_Program::TST_SIG_Program::appendLines( SIG_Program &program, int firstOperand, long length )
{
  for ( long i = 0; i < length; i++ )
  {
    SIG_ProgramLine line;
    line.setRobotInstruction( ADD, firstOperand + i, -i );
    program.appendLine( line );
  }
}

QStringList SIGEL_Program::TST_SIG_Program::expectedTexts( int firstOperand, long length )
{
  QStringList texts;
  for ( long i = 0; i < length; i++ )
    texts.append( QString( "ADD %1,%2\n" ).arg( firstOperand + i ).arg( -i ) );
  return texts;
}

QStringList SIGEL_Program::TST_SIG_Program::lineTexts( SIG_Program const &program )
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
