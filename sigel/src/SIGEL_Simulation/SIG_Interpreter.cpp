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
#include "SIGEL_Tools/SIG_IO.h"
#include <QStringList>
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
		int numberOfRegisters = languageParameters.getMemorySize();
		int registerWidth = languageParameters.getRegisterWidth();
		// Held by value: SIG_Register is two ints with no destructor and no pointers.
		// It has no default constructor, hence append rather than resize.
		registers.reserve( numberOfRegisters );
		for( int count = 0; count < numberOfRegisters; count++ )
		{
			registers.append( SIGEL_Simulation::SIG_Register(registerWidth) );
		}
	};

	void SIG_Interpreter::interprete(double timeAccountSize)
	{
		long programLength = robotProgram.getProgramLength();

		// save the maximalDelayTime
		double maxDelayTime = static_cast<double>( languageParameters.getMaximalDelayTime() ) * 0.001 ;

		if ( timeAccountSize <= remainingLastCommandTime )
		{
			remainingLastCommandTime -= timeAccountSize;
#ifdef SIG_DEBUG
			SIGEL_Tools::SIG_IO::cerr << "Remaining Last Command Time: " << remainingLastCommandTime << Qt::endl;
#endif
		}
		else
		{
			// use the rest of remaining and interprete on! :)
			timeAccountSize -= remainingLastCommandTime;
			remainingLastCommandTime = 0;

			// we still got time to do something
			while ( timeAccountSize > 0)
			{
				// fetch the next command
				SIGEL_Program::SIG_ProgramLine const &programLine = robotProgram.getLine( programCounter );

				/*
				 * ONLY FOR DEBUGGING-PURPOSES!
				 */
#ifdef SIG_DEBUG
				SIGEL_Tools::SIG_IO::cerr << "--------------------------------------------------------" << Qt::endl;
				SIGEL_Tools::SIG_IO::cerr << "Remaining Time:" << timeAccountSize<< Qt::endl;
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
#endif

				// commandNames is in the order of Robotinstruction.
				static const QStringList commandNames = { "COPY", "LOAD", "ADD", "SUB", "MUL", "DIV", "MOD", "MIN", "MAX", "CMP", "JMP", "SENSE", "MOVE", "DELAY", "NOP" };
				// An instruction type that is not in commandNames gets an empty name.
				const QString commandName = commandNames.value( programLine.getRobotinstructionType() );

				long nextProgramCounter = (programCounter + 1) % programLength;

				// A command that is not allowed is skipped: the program goes to the next line and no time is used.
				if ( !languageParameters.hasCommand( commandName ) )
				{
					programCounter = nextProgramCounter;
					continue;
				}

				// what command was fetched?
				switch ( programLine.getRobotinstructionType() )
				{
				case SIGEL_Program::COPY:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].copyReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::LOAD:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int value = programLine.getInstructionElement(1);
					registers[reg0].loadValue( value );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::ADD:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].addReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::SUB:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].subReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::MUL:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].mulReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::DIV:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].divReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::MOD:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].modReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::MIN:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].minReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::MAX:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					registers[reg0].maxReg( registers[reg1] );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::CMP:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int reg1 = getRegisterNumber( programLine.getInstructionElement(1) );
					if ( registers[reg0].getValue() <= registers[reg1].getValue() )
						compareFlag = true;
					else
						compareFlag = false;
#ifdef SIG_DEBUG
					SIGEL_Tools::SIG_IO::cerr << "Reg0: " << reg0 << "; Reg1: " << reg1 << Qt::endl;
#endif

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::JMP:
				{
					// do it!
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

					// subtract the needed time for the command

					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::SENSE:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );

					int numberOfSensor = registers[reg0].getValue();
					simulationQueries.sense( numberOfSensor, registers );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::MOVE:
				{
					// do it!
					int reg0 = getRegisterNumber( programLine.getInstructionElement(0) );
					int numberOfJoint = registers[reg0].getValue();
#ifdef SIG_DEBUG
					SIGEL_Tools::SIG_IO::cerr << "Moving drive " << numberOfJoint << "." << Qt::endl;
#endif
					commandInterface.moveDrive( numberOfJoint, registers );

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::DELAY:
				{
					// do it!
					// get the register
					int reg = getRegisterNumber( programLine.getInstructionElement(0) );
					int readOut = registers[reg].getValue();
					double delayTime = static_cast<double>( readOut ) * 0.001;
					delayTime = std::abs( delayTime );
					if( delayTime > maxDelayTime )
						delayTime = maxDelayTime;
					timeAccountSize -= delayTime;

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
				}
				break;

				case SIGEL_Program::NOP:
					// do it!

					// subtract the needed time for the command
					timeAccountSize -= languageParameters.getCommand( commandName )->getDuration();

					// The command took more time than was left: the excess is taken off the time of the next call.
					if ( timeAccountSize < 0 )
						remainingLastCommandTime = - timeAccountSize;
					break;

				default:
#ifdef SIG_DEBUG
					SIGEL_Tools::SIG_IO::cerr << "Something went horribly wrong in the interpreter! Default was called!!! Didn't recognize command. PC++" << Qt::endl;
#endif
					break;
				} // close switch

				programCounter = nextProgramCounter;
			} // close while
#ifdef SIG_DEBUG
			SIGEL_Tools::SIG_IO::cerr << "Remaining Last Command Time: " << remainingLastCommandTime << Qt::endl; // debug! delete!
#endif
		} // close else
	};

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
