/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

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
#include "SIGEL_Simulation/SIG_Interpreter.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_IO.h"
#include <algorithm>
#include <cmath>

namespace SIGEL_Simulation
{

	SIG_Interpreter::SIG_Interpreter(SIGEL_Robot::SIG_LanguageParameters const &languageParameters,
	                                 SIGEL_Program::SIG_Program const &robotProgram,
	                                 SIG_CommandInterface &commandInterface,
	                                 SIG_SimulationQueries const &simulationQueries)
		: robotProgram(robotProgram),
		  languageParameters(languageParameters),
		  commandInterface(commandInterface),
		  simulationQueries(simulationQueries),
		  remainingLastCommandTime(0),
		  registers(),
		  programCounter(0),
		  compareFlag(false)
	{
		int numberOfRegisters = std::max( languageParameters.getMemorySize(), 1 );
		int registerWidth = languageParameters.getRegisterWidth();

		// SIG_Register has no default constructor, so each register is appended.
		registers.reserve( numberOfRegisters );
		for( int count = 0; count < numberOfRegisters; count++ )
		{
			registers.append( SIGEL_Simulation::SIG_Register(registerWidth) );
		}
	};

	void SIG_Interpreter::interprete(double timeAccount)
	{
		long programLength = robotProgram.getProgramLength();

		double maxDelayTime = std::max( static_cast<double>( languageParameters.getMaximalDelayTime() ) * 0.001, 0.0 );

		if ( timeAccount <= remainingLastCommandTime )
		{
			remainingLastCommandTime -= timeAccount;
#ifdef SIG_DEBUG
			SIGEL_Tools::SIG_IO::cerr << "Remaining Last Command Time: " << remainingLastCommandTime << Qt::endl;
#endif
			return;
		}

		// use the rest of remaining and interprete on! :)
		timeAccount -= remainingLastCommandTime;
		remainingLastCommandTime = 0;

		// we still got time to do something
		while ( timeAccount > 0)
		{
			// fetch the next command
			SIGEL_Program::SIG_ProgramLine const &programLine = robotProgram.getLine( programCounter );

#ifdef SIG_DEBUG
			debugPrintInterpreterState( timeAccount, programLine );
#endif

			// An instruction type that is not in robotInstructionNames gets an empty name.
			const QString commandName = SIGEL_Program::robotInstructionNames.value( programLine.getRobotInstructionType() );

			// A line whose instruction type is not in RobotInstruction is invalid.
			if ( commandName.isEmpty() )
				throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, QString( "Program line %1 is invalid." ).arg( programCounter ) );

			long nextProgramCounter = (programCounter + 1) % programLength;

			// A command that is not allowed is skipped: the program goes to the next line and no time is used.
			if ( !languageParameters.hasCommand( commandName ) )
			{
				programCounter = nextProgramCounter;
				continue;
			}

			switch ( programLine.getRobotInstructionType() )
			{
			case SIGEL_Program::COPY:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].copyReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::LOAD:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int value = programLine.getInstructionElement(1);
				registers[reg0].loadValue( value );
			}
			break;

			case SIGEL_Program::ADD:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].addReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::SUB:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].subReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::MUL:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].mulReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::DIV:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].divReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::MOD:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].modReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::MIN:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].minReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::MAX:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				registers[reg0].maxReg( registers[reg1] );
			}
			break;

			case SIGEL_Program::CMP:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
				if ( registers[reg0].getValue() <= registers[reg1].getValue() )
					compareFlag = true;
				else
					compareFlag = false;
#ifdef SIG_DEBUG
				SIGEL_Tools::SIG_IO::cerr << "Reg0: " << reg0 << "; Reg1: " << reg1 << Qt::endl;
#endif
			}
			break;

			case SIGEL_Program::JMP:
			{
				int lineOffset = programLine.getInstructionElement(0);

				if ( compareFlag )
				{
					// The remainder of a negative line number is negative in C++.
					nextProgramCounter = (programCounter + 1 + lineOffset) % programLength;

					if ( nextProgramCounter < 0 )
					{
						nextProgramCounter = nextProgramCounter + programLength;
					}
				}
			}
			break;

			case SIGEL_Program::SENSE:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );

				int numberOfSensor = registers[reg0].getValue();
				simulationQueries.sense( numberOfSensor, registers );
			}
			break;

			case SIGEL_Program::MOVE:
			{
				int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
				int numberOfJoint = registers[reg0].getValue();
#ifdef SIG_DEBUG
				SIGEL_Tools::SIG_IO::cerr << "Moving drive " << numberOfJoint << "." << Qt::endl;
#endif
				commandInterface.moveDrive( numberOfJoint, registers );
			}
			break;

			case SIGEL_Program::DELAY:
			{
				int reg = getRegisterNumber( programLine.getInstructionElement(0) );
				int readOut = registers[reg].getValue();
				double delayTime = static_cast<double>( readOut ) * 0.001;
				delayTime = std::abs( delayTime );
				delayTime = std::min( delayTime, maxDelayTime );
				timeAccount -= delayTime;
			}
			break;

			case SIGEL_Program::NOP:
				break;
			} // close switch

			programCounter = nextProgramCounter;
			timeAccount -= languageParameters.getCommand( commandName )->getDuration();
		} // close while

		// The last command took more time than was left: the excess is taken off the time of the next call.
		if ( timeAccount < 0 )
			remainingLastCommandTime = - timeAccount;

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Remaining Last Command Time: " << remainingLastCommandTime << Qt::endl; // debug! delete!
#endif
	};

	void SIG_Interpreter::debugPrintInterpreterState( double timeAccount, SIGEL_Program::SIG_ProgramLine const &programLine ) const
	{
		SIGEL_Tools::SIG_IO::cerr << "--------------------------------------------------------" << Qt::endl;
		SIGEL_Tools::SIG_IO::cerr << "Remaining Time:" << timeAccount<< Qt::endl;
		SIGEL_Tools::SIG_IO::cerr << "PC: " << programCounter << Qt::endl;

		if( compareFlag )
			SIGEL_Tools::SIG_IO::cerr << "CF: 1" << Qt::endl;
		else
			SIGEL_Tools::SIG_IO::cerr << "CF: 0" << Qt::endl;

		QString theQLine;
		programLine.printToString( theQLine );
		SIGEL_Tools::SIG_IO::cerr << theQLine;

		SIGEL_Tools::SIG_IO::cerr << "Registers:" << Qt::endl;
		for( int loop=0; loop < registers.size(); loop++ )
		{
			SIGEL_Tools::SIG_IO::cerr << "R" << loop << ":" << registers[loop].getValue() << "  ";
		}
		SIGEL_Tools::SIG_IO::cerr << Qt::endl;
	}

	int SIG_Interpreter::getRegisterNumber( int operand ) const
	{
		int numberOfRegisters = registers.size();

		// The remainder of a negative operand is negative in C++.
		int registerNumber = operand % numberOfRegisters;
		if ( registerNumber < 0 )
			registerNumber += numberOfRegisters;

		return registerNumber;
	}

}
