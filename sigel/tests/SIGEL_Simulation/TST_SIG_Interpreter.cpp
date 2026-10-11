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
  QTest::addColumn< SIGEL_Program::Robotinstruction >( "instruction" );
  QTest::addColumn< int >( "first" );
  QTest::addColumn< int >( "second" );
  QTest::addColumn< int >( "result" );

  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << 20 << 6 << 6;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << 20 << 6 << 26;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << 20 << 6 << 14;
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticInstructionPutsTheResultInTheFirstRegister()
{
  QFETCH( SIGEL_Program::Robotinstruction, instruction );
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
  QTest::addColumn< SIGEL_Program::Robotinstruction >( "instruction" );
  QTest::addColumn< QString >( "name" );

  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << "COPY";
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << "ADD";
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << "SUB";
}

void SIGEL_Simulation::TST_SIG_Interpreter::arithmeticCommandThatIsNotAllowedIsSkipped()
{
  QFETCH( SIGEL_Program::Robotinstruction, instruction );
  QFETCH( QString, name );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;
  setAllDurations( languageParameters, 1.0 );
  languageParameters.removeCommand( name );
  SIGEL_Program::SIG_Program program;
  appendLine( program, SIGEL_Program::LOAD, 2, 5 );
  appendLine( program, SIGEL_Program::LOAD, 3, 3 );
  appendLine( program, instruction, 2, 3 );
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

void SIGEL_Simulation::TST_SIG_Interpreter::eachInstructionAsLastLineGoesOnAtLineZero_data()
{
  QTest::addColumn< SIGEL_Program::Robotinstruction >( "instruction" );

  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD;
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP;
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY;
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD;
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB;
}

void SIGEL_Simulation::TST_SIG_Interpreter::eachInstructionAsLastLineGoesOnAtLineZero()
{
  QFETCH( SIGEL_Program::Robotinstruction, instruction );
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
  QTest::addColumn< SIGEL_Program::Robotinstruction >( "instruction" );
  QTest::addColumn< QString >( "name" );

  QTest::newRow( "LOAD" ) << SIGEL_Program::LOAD << "LOAD";
  QTest::newRow( "NOP" ) << SIGEL_Program::NOP << "NOP";
  QTest::newRow( "COPY" ) << SIGEL_Program::COPY << "COPY";
  QTest::newRow( "ADD" ) << SIGEL_Program::ADD << "ADD";
  QTest::newRow( "SUB" ) << SIGEL_Program::SUB << "SUB";
}

void SIGEL_Simulation::TST_SIG_Interpreter::commandTakesItsOwnDuration()
{
  QFETCH( SIGEL_Program::Robotinstruction, instruction );
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

void SIGEL_Simulation::TST_SIG_Interpreter::setAllDurations( SIGEL_Robot::SIG_LanguageParameters &languageParameters, double duration )
{
  for ( const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &command : languageParameters.getCommands() )
    command.value->setDuration( duration );
}

void SIGEL_Simulation::TST_SIG_Interpreter::appendLine( SIGEL_Program::SIG_Program &program, SIGEL_Program::Robotinstruction instruction, int operand1, int operand2 )
{
  SIGEL_Program::SIG_ProgramLine programLine;
  programLine.setRobotinstruction( instruction, operand1, operand2 );
  program.appendLine( programLine );
}
