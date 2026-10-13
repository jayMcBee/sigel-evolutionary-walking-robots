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
#include "SIGEL_Simulation/TST_SIG_Interpreter.h"

#include "SIGEL_Simulation/SIG_Interpreter.h"
#include "SIGEL_Simulation/SIG_RegisterWrongSizeException.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QTextStream>
#include <QtTest>

void SIGEL_Simulation::TST_RecordingCommandInterface::moveDrive( int driveNumber, QList<SIG_Register> const &registers )
{
  TST_MoveRecord moveRecord;
  moveRecord.driveNumber = driveNumber;

  for ( const SIG_Register &oneRegister : registers )
    moveRecord.registerValues.append( oneRegister.getValue() );

  moveRecords.append( moveRecord );
}

const QList<SIGEL_Simulation::TST_MoveRecord> &SIGEL_Simulation::TST_RecordingCommandInterface::getMoveRecords() const
{
  return moveRecords;
}


SIGEL_Simulation::TST_FixedSimulationQueries::TST_FixedSimulationQueries( int sensorValue )
  : sensorValue( sensorValue )
{
}

void SIGEL_Simulation::TST_FixedSimulationQueries::sense( int sensorNumber, QList<SIG_Register> &registers ) const
{
  registers[0].loadValue( sensorValue );
  registers[1].loadValue( sensorNumber );
}

QTime SIGEL_Simulation::TST_FixedSimulationQueries::getCurrentSimulationWholeSeconds() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

double SIGEL_Simulation::TST_FixedSimulationQueries::getCurrentSimulationSeconds() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

SIG_Vector SIGEL_Simulation::TST_FixedSimulationQueries::getLinkPosition( int ) const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

SIG_Matrix SIGEL_Simulation::TST_FixedSimulationQueries::getLinkOrientation( int ) const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

int SIGEL_Simulation::TST_FixedSimulationQueries::getRootNumber() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

std::vector<double*>* SIGEL_Simulation::TST_FixedSimulationQueries::getUsedForces() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

int SIGEL_Simulation::TST_FixedSimulationQueries::getLinkCount() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}

void SIGEL_Simulation::TST_FixedSimulationQueries::checkDynas() const
{
  throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "not used by SIG_Interpreter" );
}


void SIGEL_Simulation::TST_SIG_Interpreter::registersStartAtZero()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::memorySizeGivesTheNumberOfRegisters()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setMemorySize( 6 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 6, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::constructorRefusesRegisterWidthZero()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.setRegisterWidth( 0 );
  SIGEL_Program::SIG_Program program;
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );

  QVERIFY_THROWS_EXCEPTION( SIG_RegisterWrongSizeException, SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::emptyProgramThrows()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  SIGEL_Program::SIG_Program program;
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  QVERIFY_THROWS_EXCEPTION( SIGEL_Tools::SIG_Exception, interpreter.interprete( 1 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::invalidProgramLineThrows()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  SIGEL_Program::SIG_ProgramLine programLine;
  // 15 is the first value after the last instruction of RobotInstruction.
  programLine.setRobotInstructionType( static_cast<SIGEL_Program::RobotInstruction>( 15 ) );
  program.appendLine( programLine );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QString thrown;

  try
    {
      interpreter.interprete( 1 );
    }
  catch ( const SIGEL_Tools::SIG_Exception &e )
    {
      thrown = e.getMessage();
    }

  QVERIFY2( thrown.contains( "Program line 0 is invalid." ), qPrintable( thrown ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::invalidProgramLineThrowsOnlyWhenItIsReached()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "MOVE" )->setDuration( 2.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  SIGEL_Program::SIG_ProgramLine programLine;
  // 15 is the first value after the last instruction of RobotInstruction.
  programLine.setRobotInstructionType( static_cast<SIGEL_Program::RobotInstruction>( 15 ) );
  program.appendLine( programLine );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QString thrown;

  try
    {
      // The MOVE exceeds the time account by 1 s.
      interpreter.interprete( 1 );
      // This call pays the exceeded time exactly, so line 1 is not reached.
      interpreter.interprete( 1 );
    }
  catch ( const SIGEL_Tools::SIG_Exception &e )
    {
      thrown = e.getMessage();
    }

  QVERIFY2( thrown.isEmpty(), qPrintable( thrown ) );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  try
    {
      interpreter.interprete( 0.25 );
    }
  catch ( const SIGEL_Tools::SIG_Exception &e )
    {
      thrown = e.getMessage();
    }

  QVERIFY2( thrown.contains( "Program line 1 is invalid." ), qPrintable( thrown ) );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::programTextLineWithoutAnInstructionIsNotAProgramLine_data()
{
  QTest::addColumn< QString >( "programText" );

  QTest::newRow( "comment line" ) << "LOAD 2,5\n# a comment\nMOVE 0\n";
  QTest::newRow( "empty line" ) << "LOAD 2,5\n\nMOVE 0\n";
}

void SIGEL_Simulation::TST_SIG_Interpreter::programTextLineWithoutAnInstructionIsNotAProgramLine()
{
  QFETCH( QString, programText );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  QTextStream programStream( &programText, QIODeviceBase::ReadOnly );
  program.readFromFile( programStream );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 5;

  // The time is enough for the LOAD and the MOVE.
  interpreter.interprete( 2 );

  QCOMPARE( program.getProgramLength(), 2L );
  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveTakesItsOwnDuration()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.getCommand( "MOVE" )->setDuration( 2.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 3 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "MOVE" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  // A skipped line takes no time; the NOP uses up the time so that interprete returns.
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 1 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::loadPutsTheValueInTheRegister()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 77 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 77;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::loadPutsANegativeValueInTheRegister()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, -77 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = -77;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::loadWrapsTheValueToTheRegisterWidth()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 200 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = -56;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::registerWidthComesFromTheLanguageParameters()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 200 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 200;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::loadThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "LOAD" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 0, 5 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the MOVE only, so its record also shows that the skipped LOAD took no time.
  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveGivesTheRegisterValueAsDriveNumber()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 42 );
  appendLine( program, SIGEL_Program::MOVE, 3, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[3] = 42;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 42 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveRegisterOperandWraps()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 42 );
  appendLine( program, SIGEL_Program::MOVE, 11, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[3] = 42;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 42 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveTakesItsOwnDurationWithALargeDriveNumber()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1000 );
  appendLine( program, SIGEL_Program::MOVE, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 2, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the LOAD and the two MOVEs only. A MOVE that takes more time with a larger drive number gives one record.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 2 );
  QCOMPARE( moveRecords[0].driveNumber, 1000 );
  QCOMPARE( moveRecords[1].driveNumber, 1000 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::negativeRegisterOperandWithEightRegisters()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, -1, 5 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[7] = 5;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::negativeRegisterOperandWithSixRegisters_data()
{
  QTest::addColumn< int >( "operand" );
  QTest::addColumn< int >( "registerNumber" );

  QTest::newRow( "-1" ) << -1 << 5;
  QTest::newRow( "-3" ) << -3 << 3;
  QTest::newRow( "-6" ) << -6 << 0;
  QTest::newRow( "-7" ) << -7 << 5;
}

void SIGEL_Simulation::TST_SIG_Interpreter::negativeRegisterOperandWithSixRegisters()
{
  QFETCH( int, operand );
  QFETCH( int, registerNumber );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setMemorySize( 6 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, operand, 5 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 6, 0 );
  expectedValues[registerNumber] = 5;

  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  // MOVE 0 gives the value of register 0 as drive number.
  QCOMPARE( moveRecords[0].driveNumber, expectedValues[0] );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::memorySizeOfZeroGivesOneRegister()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setMemorySize( 0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 5 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // With one register, operand 3 of the LOAD and operand 0 of the MOVE both give register 0.
  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 5 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 1, 5 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::senseGivesTheRegisterValueAsSensorNumber()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 42 );
  appendLine( program, SIGEL_Program::SENSE, 3, 0 );
  appendLine( program, SIGEL_Program::MOVE, 7, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[0] = 55;
  expectedValues[1] = 42;
  expectedValues[3] = 42;

  // TST_FixedSimulationQueries::sense puts its sensor value in register 0 and the sensor number in register 1.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::senseRegisterOperandWraps()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 42 );
  appendLine( program, SIGEL_Program::SENSE, 11, 0 );
  appendLine( program, SIGEL_Program::MOVE, 7, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[0] = 55;
  expectedValues[1] = 42;
  expectedValues[3] = 42;

  // TST_FixedSimulationQueries::sense puts its sensor value in register 0 and the sensor number in register 1.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::senseThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "SENSE" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::SENSE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 7, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the MOVE only, so its record also shows that the skipped SENSE took no time.
  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheRegisterValueInMilliseconds()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 2000 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 2000;

  // The LOAD takes 1 s and the DELAY 2 s + 1 s, so the time account is exceeded by 0.5 s.
  interpreter.interprete( 3.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheAbsoluteValue()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, -2000 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD takes 1 s and the DELAY 2 s + 1 s, so the time account is exceeded by 0.5 s.
  interpreter.interprete( 3.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 1 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayOfZeroTakesOnlyTheCommandTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 3.5 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayIsLimitedToTheMaximalDelayTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  languageParameters.setMaximalDelayTime( 1000 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 2000 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD takes 1 s and the DELAY 1 s + 1 s, so 0.5 s is left for one MOVE.
  interpreter.interprete( 3.5 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::negativeMaximalDelayTimeGivesNoDelayTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  languageParameters.setMaximalDelayTime( -1000 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 2000 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD takes 1 s and the DELAY 0 s + 1 s, so 1 s is left for one MOVE.
  interpreter.interprete( 3 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayRegisterOperandWraps()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 2000 );
  appendLine( program, SIGEL_Program::DELAY, 11, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD takes 1 s and the DELAY 2 s + 1 s, so no time is left for a MOVE.
  interpreter.interprete( 3.5 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.setRegisterWidth( 16 );
  languageParameters.removeCommand( "DELAY" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 2000 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 2000;

  // The time is enough for the LOAD and the MOVE only, so the record also shows that the skipped DELAY took no time.
  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::nopChangesNothingAndTakesItsTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.getCommand( "NOP" )->setDuration( 2.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 5 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 5;

  interpreter.interprete( 3 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::nopThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "NOP" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the MOVE only, so its record shows that the skipped NOP took no time.
  interpreter.interprete( 1 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticInstructionPutsTheResultInTheFirstRegister_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "first" );
  QTest::addColumn< int >( "second" );
  QTest::addColumn< int >( "result" );

  QTest::newRow( "COPY first is greater" ) << SIGEL_Program::COPY << 20 << 6 << 6;
  QTest::newRow( "COPY first is less" ) << SIGEL_Program::COPY << 6 << 20 << 20;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 20 << 6 << 26;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 20 << 6 << 14;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << 20 << 6 << 120;
  // 23 / 3 is 7 and 7 % 3 is not 7, so the row fails if the DIV case of the switch runs on into the MOD case.
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << 23 << 3 << 7;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << 20 << 6 << 2;
  // The lesser of 2 and -6 is not 2, so the row fails if the MOD case of the switch runs on into the MIN case.
  QTest::newRow( "MOD second is negative" ) << SIGEL_Program::MOD << 20 << -6 << 2;
  QTest::newRow( "MIN first is greater" ) << SIGEL_Program::MIN << 20 << 6 << 6;
  QTest::newRow( "MIN first is less" ) << SIGEL_Program::MIN << 6 << 20 << 6;
  QTest::newRow( "MAX first is greater" ) << SIGEL_Program::MAX << 20 << 6 << 20;
  QTest::newRow( "MAX first is less" ) << SIGEL_Program::MAX << 6 << 20 << 20;
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticInstructionPutsTheResultInTheFirstRegister()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( int, first );
  QFETCH( int, second );
  QFETCH( int, result );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, first );
  appendLine( program, SIGEL_Program::LOAD, 5, second );
  // With 8 registers, the operands 10 and 13 are the registers 2 and 5.
  appendLine( program, instruction, 10, 13 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = result;
  expectedValues[5] = second;

  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::addWithTheSameRegisterTwice()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 20 );
  appendLine( program, SIGEL_Program::ADD, 2, 2 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 40;

  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticCommandThatIsNotAllowedIsSkipped_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< QString >( "name" );
  QTest::addColumn< int >( "firstRegister" );
  QTest::addColumn< int >( "secondRegister" );

  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << "COPY" << 2 << 3;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << "ADD" << 2 << 3;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << "SUB" << 2 << 3;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << "MUL" << 2 << 3;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << "DIV" << 2 << 3;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << "MOD" << 2 << 3;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << "MIN" << 2 << 3;

  // Register 2 has 5 and register 3 has 3, so MAX 2,3 would change nothing. MAX 3,2 would change register 3.
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << "MAX" << 3 << 2;
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticCommandThatIsNotAllowedIsSkipped()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( QString, name );
  QFETCH( int, firstRegister );
  QFETCH( int, secondRegister );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( name );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 5 );
  appendLine( program, SIGEL_Program::LOAD, 3, 3 );
  appendLine( program, instruction, firstRegister, secondRegister );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 5;
  expectedValues[3] = 3;

  // The time is enough for the two LOADs and the MOVE only, so the record also shows that the skipped line took no time.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpJumpsWhenFirstRegisterIsLess()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1 );
  appendLine( program, SIGEL_Program::LOAD, 3, 2 );
  appendLine( program, SIGEL_Program::CMP, 10, 11 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 1;
  expectedValues[3] = 2;

  // The operands 10 and 11 of the CMP are the registers 2 and 3.
  interpreter.interprete( 5 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpJumpsWhenRegistersAreEqual()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 2 );
  appendLine( program, SIGEL_Program::LOAD, 3, 2 );
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 2;
  expectedValues[3] = 2;

  interpreter.interprete( 5 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpJumpsWhenFirstRegisterIsNegative()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, -5 );
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = -5;

  // Register 2 is -5 and register 3 is 0, so the CMP sets the compare flag.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpDoesNotJumpWhenFirstRegisterIsGreater()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 3 );
  appendLine( program, SIGEL_Program::LOAD, 3, 2 );
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 3;
  expectedValues[3] = 2;
  expectedValues[4] = 99;

  interpreter.interprete( 6 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpDoesNotJumpBeforeAnyCmp()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 99;

  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpZeroGoesToTheNextLine()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 99;

  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpDoesNotClearTheFlag()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 5, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The second JMP jumps only if the first JMP has left the compare flag set.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpThatDoesNotJumpDoesNotSetTheCompareFlag()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 99;

  // The second JMP jumps over the LOAD only if the first JMP has set the compare flag.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::cmpClearsTheFlag()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1 );
  appendLine( program, SIGEL_Program::CMP, 3, 2 );
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 1;
  expectedValues[4] = 99;

  // The first CMP sets the compare flag, because 0 <= 1. The second CMP clears it.
  interpreter.interprete( 6 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::compareFlagIsKeptBetweenCalls()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The CMP uses up the time account exactly, so the JMP runs in the next call.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // The time is enough for the JMP and the MOVE only. Without a jump the LOAD runs and there is no record.
  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::compareFlagIsKeptByACallThatOnlyPaysExceededTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.getCommand( "CMP" )->setDuration( 2.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The CMP exceeds the time account by 1 s.
  interpreter.interprete( 1 );

  // This call pays the exceeded time exactly, so no command runs.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // The time is enough for the JMP and the MOVE only. Without a jump the LOAD runs and there is no record.
  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandBetweenCmpAndJmpDoesNotClearTheCompareFlag_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfMoveRecords" );

  // Register 2 is 0, so the DELAY has no delay time.
  // The MOVE between the CMP and the JMP gives a record of its own.
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << 1;
  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << 1;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 1;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 1;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << 1;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << 1;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << 1;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << 1;
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << 1;
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE << 1;
  QTest::newRow( "MOVE" ) << SIGEL_Program::MOVE << 2;
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY << 1;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << 1;
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandBetweenCmpAndJmpDoesNotClearTheCompareFlag()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( int, numberOfMoveRecords );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, instruction, 2, 3 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the CMP, the command, the JMP and the MOVE only. Without a jump the LOAD runs in place of the MOVE.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), numberOfMoveRecords );
  QCOMPARE( moveRecords[numberOfMoveRecords - 1].registerValues[4], 0 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandBetweenCmpAndJmpDoesNotSetTheCompareFlag_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< int >( "numberOfMoveRecords" );

  // Register 3 is 0, so the DELAY has no delay time.
  // The MOVE between the CMP and the JMP gives a record of its own.
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << 1;
  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << 1;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 1;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 1;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << 1;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << 1;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << 1;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << 1;
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << 1;
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE << 1;
  QTest::newRow( "MOVE" ) << SIGEL_Program::MOVE << 2;
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY << 1;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << 1;
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandBetweenCmpAndJmpDoesNotSetTheCompareFlag()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( int, numberOfMoveRecords );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1 );
  // Register 2 is 1 and register 3 is 0, so the CMP does not set the compare flag.
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, instruction, 3, 2 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for all 6 lines. With a jump the second LOAD does not run before the MOVE.
  interpreter.interprete( 6 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), numberOfMoveRecords );
  QCOMPARE( moveRecords[numberOfMoveRecords - 1].registerValues[2], 1 );
  QCOMPARE( moveRecords[numberOfMoveRecords - 1].registerValues[4], 99 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpBackwardsMakesALoop()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 1 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, -4, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> firstExpectedValues( 8, 0 );
  firstExpectedValues[2] = 1;
  firstExpectedValues[3] = 1;
  QList<int> secondExpectedValues( 8, 0 );
  secondExpectedValues[2] = 2;
  secondExpectedValues[3] = 1;

  // The JMP on line 4 goes to the ADD on line 1. The time ends with the second MOVE, so a jump to line 0 gives one record only.
  interpreter.interprete( 7 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 2 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, firstExpectedValues );
  QCOMPARE( moveRecords[1].driveNumber, 0 );
  QCOMPARE( moveRecords[1].registerValues, secondExpectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpPastTheLastLineWrapsToTheStart()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 4, 11 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 3, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 22 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 11;

  interpreter.interprete( 4 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  // The JMP on line 3 goes to line 3 + 1 + 3 = 7, which is line 1 in a program of 6 lines: the MOVE.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  // After the MOVE on line 1 come the CMP and the JMP. After the MOVE on line 5 come the LOAD and the MOVE, with one more record.
  interpreter.interprete( 2 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 2 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
  QCOMPARE( moveRecords[1].driveNumber, 0 );
  QCOMPARE( moveRecords[1].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpBeforeLineZeroWrapsFromTheEnd()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, -6, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 11 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 22 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 11;

  // The JMP on line 1 goes to line 1 + 1 - 6 = -4, which is line 3 in a program of 7 lines: the first LOAD.
  // Without a jump the NOP on line 2 runs, and the time ends before the MOVE.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpWrapsOverSeveralProgramLengths_data()
{
  QTest::addColumn< int >( "lineOffset" );

  // The JMP is on line 1 of a program of 7 lines, so the line after it is line 2.
  // Each line offset gives line 3: the offsets differ from 1 by a multiple of 7.
  QTest::newRow( "15" ) << 15;
  QTest::newRow( "-13" ) << -13;
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpWrapsOverSeveralProgramLengths()
{
  QFETCH( int, lineOffset );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, lineOffset, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 11 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 22 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 11;

  // The time is enough for the CMP, the JMP, the LOAD on line 3 and the MOVE on line 4.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpToLineZero_data()
{
  QTest::addColumn< int >( "lineOffset" );

  // The JMP is on line 2 of a program of 5 lines, so the line after it is line 3.
  // Each line offset gives line 0: the offsets differ from -3 by a multiple of 5.
  QTest::newRow( "-3" ) << -3;
  QTest::newRow( "2" ) << 2;
  QTest::newRow( "-8" ) << -8;
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpToLineZero()
{
  QFETCH( int, lineOffset );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, lineOffset, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the MOVE, the CMP, the JMP and one more command. Only a jump to line 0 gives a second record.
  interpreter.interprete( 4 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 2 );
  QCOMPARE( moveRecords[1].driveNumber, 0 );
  QCOMPARE( moveRecords[1].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::cmpThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "CMP" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 99;

  // The time is enough for the JMP, the LOAD and the MOVE only, so the record also shows that the skipped CMP took no time.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpThatIsNotAllowedIsSkipped()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "JMP" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[4] = 99;

  // The time is enough for the CMP, the LOAD and the MOVE only, so the record also shows that the skipped JMP took no time.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandThatIsNotAllowedDoesNotClearTheCompareFlag()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( "NOP" );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::LOAD, 4, 99 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time is enough for the CMP, the JMP and the MOVE only. Without a jump the LOAD runs and there is no record.
  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
}

void SIGEL_Simulation::TST_SIG_Interpreter::eachInstructionAsLastLineGoesOnAtLineZero_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );

  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP;
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN;
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX;
  QTest::newRow( "CMP" ) << SIGEL_Program::CMP;
  QTest::newRow( "JMP" ) << SIGEL_Program::JMP;
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE;
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY;
}

void SIGEL_Simulation::TST_SIG_Interpreter::eachInstructionAsLastLineGoesOnAtLineZero()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, instruction, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 3 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandTakesItsOwnDuration_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< QString >( "name" );

  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << "LOAD";
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << "NOP";
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << "COPY";
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << "ADD";
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << "SUB";
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << "MUL";
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << "DIV";
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << "MOD";
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << "MIN";
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << "MAX";
  QTest::newRow( "CMP" ) << SIGEL_Program::CMP << "CMP";
  QTest::newRow( "JMP" ) << SIGEL_Program::JMP << "JMP";
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE << "SENSE";
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY << "DELAY";
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandTakesItsOwnDuration()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( QString, name );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.getCommand( name )->setDuration( 2.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, instruction, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 3 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::programStartsAgainAfterTheLastLine()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  // The MOVE on line 0 gives a record of its own in each pass, so going on at another line after the last line gives other records.
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 3, 3 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValuesAfterTheFirstPass( 8, 0 );
  expectedValuesAfterTheFirstPass[2] = 3;
  expectedValuesAfterTheFirstPass[3] = 3;
  QList<int> expectedValuesAfterTheSecondPass( 8, 0 );
  expectedValuesAfterTheSecondPass[2] = 6;
  expectedValuesAfterTheSecondPass[3] = 3;

  interpreter.interprete( 8 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 4 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, QList<int>( 8, 0 ) );
  QCOMPARE( moveRecords[1].driveNumber, 0 );
  QCOMPARE( moveRecords[1].registerValues, expectedValuesAfterTheFirstPass );
  QCOMPARE( moveRecords[2].driveNumber, 0 );
  QCOMPARE( moveRecords[2].registerValues, expectedValuesAfterTheFirstPass );
  QCOMPARE( moveRecords[3].driveNumber, 0 );
  QCOMPARE( moveRecords[3].registerValues, expectedValuesAfterTheSecondPass );
}

void SIGEL_Simulation::TST_SIG_Interpreter::stateIsKeptBetweenCalls()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 3, 3 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> firstExpectedValues( 8, 0 );
  firstExpectedValues[2] = 3;
  firstExpectedValues[3] = 3;
  QList<int> secondExpectedValues( 8, 0 );
  secondExpectedValues[2] = 6;
  secondExpectedValues[3] = 3;

  interpreter.interprete( 2 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 3 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 2 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, firstExpectedValues );
  QCOMPARE( moveRecords[1].driveNumber, 0 );
  QCOMPARE( moveRecords[1].registerValues, secondExpectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::timeAccountOfThreeRunsThreeCommands()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 3 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::timeAccountOfZeroRunsNothing()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 0 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeIsSubtractedFromTheNextCall()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 2.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 5 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeSmallerThanTheTimeAccountIsSubtractedFromThatCall()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 2.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 1.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeLargerThanTheTimeAccountIsSubtractedOverSeveralCalls()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::remainingLastCommandTimeIsSubtractedOnlyOnce()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  interpreter.interprete( 2.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  // This call pays the exceeded 0.5 and ends at exactly 0.
  interpreter.interprete( 1.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );

  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 5 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 6 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandTakesItsOwnDurationAmongDifferentDurations_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< double >( "duration" );

  // The durations are those of setDifferentDurations.
  // Register 0 is 0, so the DELAY has no delay time.
  // The JMP goes to the next line: no CMP runs before it, and its operand is 0.
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << 0.5;
  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << 0.75;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 1.0;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 1.25;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << 1.5;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << 1.75;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << 2.0;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << 2.25;
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << 2.5;
  QTest::newRow( "CMP" ) << SIGEL_Program::CMP << 2.75;
  QTest::newRow( "JMP" ) << SIGEL_Program::JMP << 3.0;
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE << 3.25;
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY << 3.75;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << 4.0;
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandTakesItsOwnDurationAmongDifferentDurations()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( double, duration );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, instruction, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The command uses up the time account exactly. With a smaller duration the MOVE runs in this call.
  interpreter.interprete( duration );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // With a larger duration this call only pays exceeded time, because each duration is a multiple of 0.25.
  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveTakesItsOwnDurationAmongDifferentDurations()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The MOVE takes 3.5 s and uses up the time account exactly.
  interpreter.interprete( 3.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  // The second MOVE exceeds the time account by 3.5 s - 0.25 s = 3.25 s.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  interpreter.interprete( 3.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::cmpThatDoesNotSetTheCompareFlagTakesItsOwnDuration()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1 );
  // Register 2 is 1 and register 3 is 0, so the CMP does not set the compare flag.
  appendLine( program, SIGEL_Program::CMP, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 1;

  // The LOAD takes 0.75 s and the CMP 2.75 s, so the time account is used up exactly. With a smaller duration the MOVE runs in this call.
  interpreter.interprete( 3.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // With a larger duration this call only pays exceeded time, because each duration is a multiple of 0.25.
  interpreter.interprete( 0.25 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpThatJumpsTakesItsOwnDuration()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 2, 7 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 7;

  // The CMP takes 2.75 s and the JMP 3 s, so the time account is used up exactly.
  interpreter.interprete( 5.75 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // The JMP goes to the LOAD on line 3, which takes 0.75 s. Without the jump the MOVE on line 2 gives a record here.
  interpreter.interprete( 0.75 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.25 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::jmpThatExceedsTheTimeAccountJumps()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  appendLine( program, SIGEL_Program::JMP, 1, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 2, 7 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  QList<int> expectedValues( 8, 0 );
  expectedValues[2] = 7;

  // The CMP takes 2.75 s, so 0.25 s is left for the JMP. The JMP takes 3 s and exceeds the time account by 2.75 s.
  interpreter.interprete( 3 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // This call pays the exceeded time exactly.
  interpreter.interprete( 2.75 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // The JMP goes to the LOAD on line 3, which takes 0.75 s. Without the jump the MOVE on line 2 gives a record here.
  // With less exceeded time the MOVE on line 4 gives a record here or in the call before.
  interpreter.interprete( 0.75 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.25 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 1 );
  QCOMPARE( moveRecords[0].driveNumber, 0 );
  QCOMPARE( moveRecords[0].registerValues, expectedValues );
}

void SIGEL_Simulation::TST_SIG_Interpreter::programWithAllCommandsTakesTheSumOfTheirDurations()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::COPY, 0, 0 );
  appendLine( program, SIGEL_Program::LOAD, 0, 0 );
  appendLine( program, SIGEL_Program::ADD, 0, 0 );
  appendLine( program, SIGEL_Program::SUB, 0, 0 );
  appendLine( program, SIGEL_Program::MUL, 0, 0 );
  appendLine( program, SIGEL_Program::DIV, 0, 0 );
  appendLine( program, SIGEL_Program::MOD, 0, 0 );
  appendLine( program, SIGEL_Program::MIN, 0, 0 );
  appendLine( program, SIGEL_Program::MAX, 0, 0 );
  appendLine( program, SIGEL_Program::CMP, 0, 0 );
  // The compare flag is set, and JMP 0 goes to the next line.
  appendLine( program, SIGEL_Program::JMP, 0, 0 );
  appendLine( program, SIGEL_Program::SENSE, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 2, 0 );
  // Register 2 is 0, so the DELAY has no delay time.
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The 12 commands before the MOVE take 0.5 s + 0.75 s + ... + 3.25 s = 12 * ( 0.5 s + 3.25 s ) / 2 = 22.5 s.
  interpreter.interprete( 22.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // The MOVE exceeds the time account by 3.5 s - 0.25 s = 3.25 s.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  // 3.25 s of exceeded time, 3.75 s for the DELAY and 4 s for the NOP are 11 s. The 12 commands of the second pass take 22.5 s.
  interpreter.interprete( 33.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::differentDurationsAddUpOverSeveralProgramPasses()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "LOAD" )->setDuration( 0.5 );
  languageParameters.getCommand( "ADD" )->setDuration( 0.75 );
  languageParameters.getCommand( "MOVE" )->setDuration( 1.5 );
  languageParameters.getCommand( "NOP" )->setDuration( 0.25 );
  SIGEL_Program::SIG_Program program;
  // Register 2 counts the program passes.
  appendLine( program, SIGEL_Program::LOAD, 3, 1 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // One program pass takes 3 s, and the MOVE starts 1.25 s after the start of the pass.
  // The third MOVE starts at 7.25 s and ends at 8.75 s, so the time account is exceeded by 1.25 s.
  interpreter.interprete( 7.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 1.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  // The NOP uses up the time account exactly.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  // The LOAD and the ADD use up the time account exactly.
  interpreter.interprete( 1.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 0.25 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 4 );
  QCOMPARE( moveRecords[0].registerValues[2], 1 );
  QCOMPARE( moveRecords[1].registerValues[2], 2 );
  QCOMPARE( moveRecords[2].registerValues[2], 3 );
  QCOMPARE( moveRecords[3].registerValues[2], 4 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandWithADurationOfZeroRunsAndTakesNoTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 0.0 );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.5 );
  SIGEL_Program::SIG_Program program;
  // Register 2 counts the program passes.
  appendLine( program, SIGEL_Program::LOAD, 3, 1 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // Only the MOVE takes time. The third MOVE uses up the time account exactly, so the commands after it do not run.
  interpreter.interprete( 1.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  // The fourth MOVE exceeds the time account by 0.25 s.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );

  interpreter.interprete( 0.5 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 5 );
  QCOMPARE( moveRecords[0].registerValues[2], 1 );
  QCOMPARE( moveRecords[1].registerValues[2], 2 );
  QCOMPARE( moveRecords[2].registerValues[2], 3 );
  QCOMPARE( moveRecords[3].registerValues[2], 4 );
  QCOMPARE( moveRecords[4].registerValues[2], 5 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::moveWithADurationOfZeroTakesNoTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "MOVE" )->setDuration( 0.0 );
  languageParameters.getCommand( "NOP" )->setDuration( 0.5 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The second NOP uses up the time account exactly, so a third MOVE does not run.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  // The NOP after the third MOVE exceeds the time account by 0.25 s.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 4 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandThatIsNotAllowedTakesNoTimeAmongDifferentDurations()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "LOAD" )->setDuration( 0.25 );
  languageParameters.getCommand( "ADD" )->setDuration( 0.5 );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.75 );
  languageParameters.removeCommand( "SUB" );
  languageParameters.removeCommand( "COPY" );
  SIGEL_Program::SIG_Program program;
  // Register 2 counts the program passes. The SUB would take 1 off again, and the COPY would put 0 in register 2.
  appendLine( program, SIGEL_Program::LOAD, 3, 1 );
  appendLine( program, SIGEL_Program::SUB, 2, 3 );
  appendLine( program, SIGEL_Program::ADD, 2, 3 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  appendLine( program, SIGEL_Program::COPY, 2, 5 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // One program pass takes 0.25 s + 0.5 s + 0.75 s = 1.5 s, so two passes use up the time account exactly.
  interpreter.interprete( 3 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  // The LOAD uses up the time account exactly.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  // The ADD uses up the time account exactly.
  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );

  // The MOVE exceeds the time account by 0.5 s.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 3 );

  // The LOAD and the ADD take 0.75 s, so 0.25 s is left for the MOVE.
  interpreter.interprete( 1 );

  const QList<TST_MoveRecord> &moveRecords = commandInterface.getMoveRecords();
  QCOMPARE( moveRecords.size(), 4 );
  QCOMPARE( moveRecords[0].registerValues[2], 1 );
  QCOMPARE( moveRecords[1].registerValues[2], 2 );
  QCOMPARE( moveRecords[2].registerValues[2], 3 );
  QCOMPARE( moveRecords[3].registerValues[2], 4 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeOfACommandIsSubtractedFromTheNextCall_data()
{
  QTest::addColumn< SIGEL_Program::RobotInstruction >( "instruction" );
  QTest::addColumn< double >( "exceededTime" );

  // The durations are those of setDifferentDurations. The exceeded time is the duration - 0.25 s.
  // Register 0 is 0, so the DELAY has no delay time.
  // The JMP goes to the next line: no CMP runs before it, and its operand is 0.
  // The MOVE is in moveTakesItsOwnDurationAmongDifferentDurations.
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << 0.25;
  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << 0.5;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 0.75;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 1.0;
  QTest::newRow( "MUL" ) << SIGEL_Program::MUL << 1.25;
  QTest::newRow( "DIV" ) << SIGEL_Program::DIV << 1.5;
  QTest::newRow( "MOD" ) << SIGEL_Program::MOD << 1.75;
  QTest::newRow( "MIN" ) << SIGEL_Program::MIN << 2.0;
  QTest::newRow( "MAX" ) << SIGEL_Program::MAX << 2.25;
  QTest::newRow( "CMP" ) << SIGEL_Program::CMP << 2.5;
  QTest::newRow( "JMP" ) << SIGEL_Program::JMP << 2.75;
  QTest::newRow( "SENSE" ) << SIGEL_Program::SENSE << 3.0;
  QTest::newRow( "DELAY" ) << SIGEL_Program::DELAY << 3.5;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << 3.75;
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeOfACommandIsSubtractedFromTheNextCall()
{
  QFETCH( SIGEL_Program::RobotInstruction, instruction );
  QFETCH( double, exceededTime );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setDifferentDurations( languageParameters );
  SIGEL_Program::SIG_Program program;
  appendLine( program, instruction, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The time account is smaller than the duration of the command.
  interpreter.interprete( 0.25 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // This call pays the exceeded time exactly. With less exceeded time the MOVE runs in this call.
  interpreter.interprete( exceededTime );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeIsKeptOverSeveralCallsAndProgramPasses()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "NOP" )->setDuration( 2.0 );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.5 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::NOP, 0, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );
  // Each of the 10 calls has a time account of 0.75 s. The NOP takes 2 s and the MOVE 0.5 s.
  // A call first pays the exceeded time of the call before. The time that is left is for commands.
  // A command that takes more than the time that is left gives the exceeded time for the next call.
  // All times in the table are in seconds.
  //
  // call | exceeded time | time left    | commands  | exceeded time     | number of move
  //      | before        | for commands | that run  | after             | records after
  //    1 | 0             | 0.75         | NOP       | 2 - 0.75 = 1.25   | 0
  //    2 | 1.25          | 0            | none      | 1.25 - 0.75 = 0.5 | 0
  //    3 | 0.5           | 0.25         | MOVE      | 0.5 - 0.25 = 0.25 | 1
  //    4 | 0.25          | 0.5          | NOP       | 2 - 0.5 = 1.5     | 1
  //    5 | 1.5           | 0            | none      | 1.5 - 0.75 = 0.75 | 1
  //    6 | 0.75          | 0            | none      | 0.75 - 0.75 = 0   | 1
  //    7 | 0             | 0.75         | MOVE, NOP | 2 - 0.25 = 1.75   | 2
  //    8 | 1.75          | 0            | none      | 1.75 - 0.75 = 1   | 2
  //    9 | 1             | 0            | none      | 1 - 0.75 = 0.25   | 2
  //   10 | 0.25          | 0.5          | MOVE      | 0.5 - 0.5 = 0     | 3
  const QList<int> expectedNumbersOfMoveRecords = { 0, 0, 1, 1, 1, 1, 2, 2, 2, 3 };
  QList<int> numbersOfMoveRecords;

  for ( int call = 1; call <= 10; call++ )
    {
      interpreter.interprete( 0.75 );
      numbersOfMoveRecords.append( commandInterface.getMoveRecords().size() );
    }

  // On a difference, Qt Test gives the index of the first call that differs. Index 0 is call 1.
  QCOMPARE( numbersOfMoveRecords, expectedNumbersOfMoveRecords );
}

void SIGEL_Simulation::TST_SIG_Interpreter::timeAccountOfZeroKeepsTheExceededTime()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "MOVE" )->setDuration( 1.5 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The MOVE exceeds the time account by 0.5 s.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );

  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 2 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheLimitedDelayTimeAndItsOwnDuration_data()
{
  QTest::addColumn< int >( "registerValue" );
  QTest::addColumn< int >( "maximalDelayTime" );
  QTest::addColumn< double >( "durationOfTheDelayCommand" );
  QTest::addColumn< double >( "timeOfLoadAndDelay" );

  // The LOAD takes 0.25 s. Then come the delay time and the duration of the DELAY.
  // The register value and the maximal delay time are in milliseconds. 0.001 is not exact in binary,
  // but 1250 * 0.001 and 1500 * 0.001 give exactly the double values 1.25 and 1.5.
  QTest::newRow( "less than the maximal delay time" ) << 1250 << 1500 << 0.5 << 2.0;
  QTest::newRow( "equal to the maximal delay time" ) << 1500 << 1500 << 0.5 << 2.25;
  QTest::newRow( "more than the maximal delay time" ) << 1750 << 1500 << 0.5 << 2.25;
  QTest::newRow( "negative, less than the maximal delay time" ) << -1250 << 1500 << 0.5 << 2.0;
  QTest::newRow( "negative, more than the maximal delay time" ) << -1750 << 1500 << 0.5 << 2.25;
  QTest::newRow( "maximal delay time of zero" ) << 1250 << 0 << 0.5 << 0.75;
  QTest::newRow( "duration of zero" ) << 1250 << 1500 << 0.0 << 1.5;
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheLimitedDelayTimeAndItsOwnDuration()
{
  QFETCH( int, registerValue );
  QFETCH( int, maximalDelayTime );
  QFETCH( double, durationOfTheDelayCommand );
  QFETCH( double, timeOfLoadAndDelay );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "LOAD" )->setDuration( 0.25 );
  languageParameters.getCommand( "DELAY" )->setDuration( durationOfTheDelayCommand );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.75 );
  languageParameters.setRegisterWidth( 16 );
  languageParameters.setMaximalDelayTime( maximalDelayTime );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, registerValue );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD and the DELAY use up the time account exactly. With a smaller delay time the MOVE runs in this call.
  interpreter.interprete( timeOfLoadAndDelay );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // With a larger delay time this call only pays exceeded time.
  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheWholeDelayTimeAtTheLimitsOfTheRegister_data()
{
  QTest::addColumn< int >( "registerValue" );

  // With a register width of 16 the register values go from -32768 to 32767.
  QTest::newRow( "largest register value" ) << 32767;
  QTest::newRow( "smallest register value" ) << -32768;
}

void SIGEL_Simulation::TST_SIG_Interpreter::delayTakesTheWholeDelayTimeAtTheLimitsOfTheRegister()
{
  QFETCH( int, registerValue );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "LOAD" )->setDuration( 0.25 );
  languageParameters.getCommand( "DELAY" )->setDuration( 0.5 );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.75 );
  languageParameters.setRegisterWidth( 16 );
  // The maximal delay time of 40 s is more than each delay time that a register can give.
  languageParameters.setMaximalDelayTime( 40000 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, registerValue );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The delay time is 32.767 s or 32.768 s. The LOAD takes 0.25 s and the DELAY 0.5 s more: 33.517 s or 33.518 s.
  // These values are not exact in binary, so the test does not use up the time account exactly.
  // The first call ends before the DELAY ends, and the two calls together end after it.
  // So the test accepts a delay time from 32.75 s to below 33.0 s.
  interpreter.interprete( 33.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // 0.017 s or 0.018 s of exceeded time is paid, so more than 0.23 s is left for the MOVE.
  interpreter.interprete( 0.25 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::exceededTimeOfADelayIsSubtractedOverSeveralCalls()
{
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  languageParameters.getCommand( "LOAD" )->setDuration( 0.25 );
  languageParameters.getCommand( "DELAY" )->setDuration( 0.5 );
  languageParameters.getCommand( "MOVE" )->setDuration( 0.75 );
  languageParameters.setRegisterWidth( 16 );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 1500 );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD takes 0.25 s and the DELAY 1.5 s + 0.5 s, so the time account is exceeded by 1.75 s.
  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( 0.5 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  // 0.25 s of exceeded time is left, so 0.25 s is left for the MOVE.
  interpreter.interprete( 0.5 );

  QCOMPARE( commandInterface.getMoveRecords().size(), 1 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::delaySubtractsTheDelayTimeFirstAndItsDurationSecond_data()
{
  QTest::addColumn< int >( "registerValue" );
  QTest::addColumn< double >( "durationOfTheDelayCommand" );
  QTest::addColumn< double >( "timeAccount" );
  QTest::addColumn< int >( "numberOfMoveRecords" );

  // The register value is the delay time in milliseconds: 80 is 0.08 s and 100 is 0.1 s.
  // 80 * 0.001 and 100 * 0.001 give exactly the double values 0.08 and 0.1.
  // The DELAY subtracts the delay time from the time account first and its duration second.

  // In double values, 0.1 - 0.08 is 2^-58 more than 0.02, so time is left for the MOVE.
  // 0.08 + 0.02 is 0.1, and ( 0.1 - 0.02 ) - 0.08 is 0: in another order no time is left.
  QTest::newRow( "time is left" ) << 80 << 0.02 << 0.1 << 1;

  // In double values, 0.45 - 0.1 is 0.35, so no time is left for the MOVE.
  // 0.1 + 0.35 is 2^-54 less than 0.45, and 0.45 - 0.35 is 2^-55 more than 0.1: in another order time is left.
  QTest::newRow( "no time is left" ) << 100 << 0.35 << 0.45 << 0;
}

void SIGEL_Simulation::TST_SIG_Interpreter::delaySubtractsTheDelayTimeFirstAndItsDurationSecond()
{
  QFETCH( int, registerValue );
  QFETCH( double, durationOfTheDelayCommand );
  QFETCH( double, timeAccount );
  QFETCH( int, numberOfMoveRecords );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.getCommand( "DELAY" )->setDuration( durationOfTheDelayCommand );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, registerValue );
  appendLine( program, SIGEL_Program::DELAY, 2, 0 );
  appendLine( program, SIGEL_Program::MOVE, 0, 0 );
  TST_RecordingCommandInterface commandInterface;
  TST_FixedSimulationQueries simulationQueries( 55 );
  SIG_Interpreter interpreter( languageParameters, program, commandInterface, simulationQueries );

  // The LOAD uses up the time account exactly, so the DELAY starts with the whole time account of the next call.
  interpreter.interprete( 1 );
  QCOMPARE( commandInterface.getMoveRecords().size(), 0 );

  interpreter.interprete( timeAccount );

  QCOMPARE( commandInterface.getMoveRecords().size(), numberOfMoveRecords );
}

void SIGEL_Simulation::TST_SIG_Interpreter::setAllDurations( SIGEL_Robot::SIG_LanguageParameters &languageParameters, double duration )
{
  for ( const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &command : languageParameters.getCommands() )
    command.value->setDuration( duration );
}

void SIGEL_Simulation::TST_SIG_Interpreter::setDifferentDurations( SIGEL_Robot::SIG_LanguageParameters &languageParameters )
{
  languageParameters.getCommand( "COPY" )->setDuration( 0.5 );
  languageParameters.getCommand( "LOAD" )->setDuration( 0.75 );
  languageParameters.getCommand( "ADD" )->setDuration( 1.0 );
  languageParameters.getCommand( "SUB" )->setDuration( 1.25 );
  languageParameters.getCommand( "MUL" )->setDuration( 1.5 );
  languageParameters.getCommand( "DIV" )->setDuration( 1.75 );
  languageParameters.getCommand( "MOD" )->setDuration( 2.0 );
  languageParameters.getCommand( "MIN" )->setDuration( 2.25 );
  languageParameters.getCommand( "MAX" )->setDuration( 2.5 );
  languageParameters.getCommand( "CMP" )->setDuration( 2.75 );
  languageParameters.getCommand( "JMP" )->setDuration( 3.0 );
  languageParameters.getCommand( "SENSE" )->setDuration( 3.25 );
  languageParameters.getCommand( "MOVE" )->setDuration( 3.5 );
  languageParameters.getCommand( "DELAY" )->setDuration( 3.75 );
  languageParameters.getCommand( "NOP" )->setDuration( 4.0 );
}

void SIGEL_Simulation::TST_SIG_Interpreter::appendLine( SIGEL_Program::SIG_Program &program, SIGEL_Program::RobotInstruction instruction, int operand1, int operand2 )
{
  SIGEL_Program::SIG_ProgramLine programLine;
  programLine.setRobotInstruction( instruction, operand1, operand2 );
  program.appendLine( programLine );
}
