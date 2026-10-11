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
