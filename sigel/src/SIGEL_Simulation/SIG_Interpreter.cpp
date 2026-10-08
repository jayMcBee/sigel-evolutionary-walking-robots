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

    // save how many registers are available as it will be needed very often...
    uint numberOfRegisters = registers.size();

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
	    for( int loop=0; loop < numberOfRegisters; loop++ )
	      {
		SIGEL_Tools::SIG_IO::cerr << "R" << loop << ":" << registers[loop].getValue() << "  ";
	      }
	    SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

	    // what command was fetched?
	    switch ( programLine.getRobotinstructionType() )
	      {
	      case SIGEL_Program::COPY:
		// is the command allowed?
		if ( languageParameters.hasCommand( "COPY" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].copyReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;
		    
		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "COPY" )->getDuration();

		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time).
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		// the command is not allowed but we have to increase the PC so we don't get the
		// same line again
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::LOAD:
		// is the command allowed?
		if ( languageParameters.hasCommand( "LOAD" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int value = programLine.getInstructionElement(1);
		    registers[reg0].loadValue( value );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "LOAD" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;
		
	      case SIGEL_Program::ADD:
		// is the command allowed?
		if ( languageParameters.hasCommand( "ADD" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].addReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "ADD" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::SUB:
		// is the command allowed?
		if ( languageParameters.hasCommand( "SUB" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].subReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "SUB" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::MUL:
		// is the command allowed?
		if ( languageParameters.hasCommand( "MUL" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].mulReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "MUL" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;
		
	      case SIGEL_Program::DIV:
		// is the command allowed?
		if ( languageParameters.hasCommand( "DIV" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].divReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "DIV" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::MOD:
		// is the command allowed?
		if ( languageParameters.hasCommand( "MOD" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].modReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "MOD" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::MIN:
		// is the command allowed?
		if ( languageParameters.hasCommand( "MIN" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].minReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "MIN" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::MAX:
		// is the command allowed?
		if ( languageParameters.hasCommand( "MAX" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    registers[reg0].maxReg( registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "MAX" )->getDuration();

		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::CMP:
		// is the command allowed?
		if ( languageParameters.hasCommand( "CMP" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int reg1 = programLine.getInstructionElement(1) % numberOfRegisters;
		    if ( registers[reg0].getValue() <= registers[reg1].getValue() )
		      compareFlag = true;
		    else
		      compareFlag = false;
#ifdef SIG_DEBUG
		    SIGEL_Tools::SIG_IO::cerr << "Reg0: " << reg0 << "; Reg1: " << reg1 << Qt::endl;
#endif
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "CMP" )->getDuration();

		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::JMP:
		// is the command allowed?
		if ( languageParameters.hasCommand( "JMP" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0);
		    if( compareFlag )
		      programCounter = ( programCounter + 1 + reg0 ) % programLength;
		    else
		      programCounter = (programCounter + 1) % programLength;
		    if( programCounter < 0 )
		      programCounter = -programCounter;
		    // subtract the needed time for the command

		    timeAccountSize -= languageParameters.getCommand( "JMP" )->getDuration();

		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::SENSE:
		// is the command allowed?
		if ( languageParameters.hasCommand( "SENSE" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;

		    int numberOfSensor = registers[reg0].getValue();
		    simulationQueries.sense( numberOfSensor, registers );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "SENSE" )->getDuration();

		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::MOVE:
		// is the command allowed?
		if ( languageParameters.hasCommand( "MOVE" ) )
		  {
		    // do it!
		    int reg0 = programLine.getInstructionElement(0) % numberOfRegisters;
		    int numberOfJoint = registers[reg0].getValue();
#ifdef SIG_DEBUG
		    SIGEL_Tools::SIG_IO::cerr << "Moving drive " << numberOfJoint << "." << Qt::endl;
#endif
		    commandInterface.moveDrive( numberOfJoint, registers );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "MOVE" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::DELAY:
		// is the command allowed?
		if ( languageParameters.hasCommand( "DELAY" ) )
		  {
		    // do it!
		    // int readOut = programLine.getInstructionElement(0);
		    // get the register
		    int reg = programLine.getInstructionElement(0) % numberOfRegisters;
		    int readOut = registers[reg].getValue();
		    double delayTime = static_cast<double>( readOut ) * 0.001;
		    delayTime = std::abs( delayTime );
		    if( delayTime > maxDelayTime )
		      delayTime = maxDelayTime;
		    timeAccountSize -= delayTime;
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "DELAY" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      case SIGEL_Program::NOP:
		// is the command allowed?
		if ( languageParameters.hasCommand( "NOP" ) )
		  {
		    // do it!
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= languageParameters.getCommand( "NOP" )->getDuration();
		    
		    /*
		     * have we exeeded the allowed time? if so, set remainingLastCommandTime to the amout
		     * by which we have exeeded it and do nothing (will be done the next time). else do it!
		     */
		    if ( timeAccountSize < 0 )
		      remainingLastCommandTime = - timeAccountSize;
		  }
		else
		  {
		    programCounter = (programCounter + 1) % programLength;
		  }
		break;

	      default:
#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Something went horribly wrong in the interpreter! Default was called!!! Didn't recognize command. PC++" << Qt::endl;
#endif
		programCounter = (programCounter + 1) % programLength;
		break;
	      } // close switch
	  } // close while
#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Remaining Last Command Time: " << remainingLastCommandTime << Qt::endl; // debug! delete!
#endif
      } // close else
  };

}




