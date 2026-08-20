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

  SIG_Interpreter::SIG_Interpreter(SIGEL_Robot::SIG_LanguageParameters const &langParams,
				   SIGEL_Program::SIG_Program const &robotProgram,
				   SIG_CommandInterface &commandInterface,
				   SIG_SimulationQueries const &simulationQueries)
    : robotProgram(robotProgram),
      langParams(langParams),
      commandInterface(commandInterface),
      simulationQueries(simulationQueries),
      registers(),
      remainingLastCommandTime(0),
      programCounter(0),
      compareFlag(false)
  {
    int numberOfRegisters = langParams.getMemorySize();
    int registerWidth = langParams.getRegisterWidth();
    registers.resize( numberOfRegisters );
    for( int count = 0; count < numberOfRegisters; count++ )
      {
	registers.insert( count, new SIGEL_Simulation::SIG_Register(registerWidth) );
      }
  };

  void SIG_Interpreter::interprete(double timeAccountSize)
  {
    long programLength = const_cast<SIGEL_Program::SIG_Program &>(robotProgram).getProgramLength();

    // save how many registers are available as it will be needed very often...
    uint numberOfRegisters = registers.size();

    // save the maximalDelayTime
    double maxDelayTime = static_cast<double>( langParams.getMaximalDelayTime() ) * 0.001 ;

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
	    SIGEL_Program::SIG_ProgramLine *theLine = const_cast< SIGEL_Program::SIG_Program &>( robotProgram ).getLine( programCounter );
	    
	    /*
	     * ONLY FOR DEBUGGING-PURPOSES!
	     */
#ifdef SIG_DEBUG
	    SIGEL_Tools::SIG_IO::cerr << "--------------------------------------------------------\n";
	    SIGEL_Tools::SIG_IO::cerr << "Remaining Time:" << timeAccountSize<< Qt::endl;
	    SIGEL_Tools::SIG_IO::cerr << "PC: " << programCounter << Qt::endl;
	    if( compareFlag )
	      SIGEL_Tools::SIG_IO::cerr << "CF: 1\n";
	    else
	      SIGEL_Tools::SIG_IO::cerr << "CF: 0\n";
	    QString theQLine;
	    theLine->printToString( theQLine );
	    SIGEL_Tools::SIG_IO::cerr << theQLine;
	    SIGEL_Tools::SIG_IO::cerr << "Registers:\n";
	    for( int loop=0; loop < numberOfRegisters; loop++ )
	      {
		SIGEL_Tools::SIG_IO::cerr << "R" << loop << ":" << registers[loop]->getValue() << "  ";
	      }
	    SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

	    // what command was fetched?
	    switch ( theLine->getRobotinstructionType() )
	      {
	      case SIGEL_Program::COPY:
		// is the command allowed?
		if ( langParams.hasCommand( "COPY" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->copyReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;
		    
		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "COPY" )->getDuration();

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
		if ( langParams.hasCommand( "LOAD" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int value = theLine->getInstructionElement(1);
		    registers[reg0]->loadValue( value );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "LOAD" )->getDuration();
		    
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
		if ( langParams.hasCommand( "ADD" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->addReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "ADD" )->getDuration();
		    
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
		if ( langParams.hasCommand( "SUB" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->subReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "SUB" )->getDuration();
		    
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
		if ( langParams.hasCommand( "MUL" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->mulReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "MUL" )->getDuration();
		    
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
		if ( langParams.hasCommand( "DIV" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->divReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "DIV" )->getDuration();
		    
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
		if ( langParams.hasCommand( "MOD" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->modReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "MOD" )->getDuration();
		    
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
		if ( langParams.hasCommand( "MIN" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->minReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "MIN" )->getDuration();
		    
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
		if ( langParams.hasCommand( "MAX" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    registers[reg0]->maxReg( *registers[reg1] );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "MAX" )->getDuration();

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
		if ( langParams.hasCommand( "CMP" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int reg1 = theLine->getInstructionElement(1) % numberOfRegisters;
		    if ( registers[reg0]->getValue() <= registers[reg1]->getValue() )
		      compareFlag = true;
		    else
		      compareFlag = false;
#ifdef SIG_DEBUG
		    SIGEL_Tools::SIG_IO::cerr << "Reg0: " << reg0 << "; Reg1: " << reg1 << endl;
#endif
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "CMP" )->getDuration();

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
		if ( langParams.hasCommand( "JMP" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0);
		    if( compareFlag )
		      programCounter = ( programCounter + 1 + reg0 ) % programLength;
		    else
		      programCounter = (programCounter + 1) % programLength;
		    if( programCounter < 0 )
		      programCounter = -programCounter;
		    // subtract the needed time for the command

		    timeAccountSize -= langParams.getCommand( "JMP" )->getDuration();

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
		if ( langParams.hasCommand( "SENSE" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;

		    int numberOfSensor = registers[reg0]->getValue();
		    simulationQueries.sense( numberOfSensor, registers );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "SENSE" )->getDuration();

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
		if ( langParams.hasCommand( "MOVE" ) )
		  {
		    // do it!
		    int reg0 = theLine->getInstructionElement(0) % numberOfRegisters;
		    int numberOfJoint = registers[reg0]->getValue();
#ifdef SIG_DEBUG
		    SIGEL_Tools::SIG_IO::cerr << "Moving drive " << numberOfJoint << ".\n";
#endif
		    commandInterface.moveDrive( numberOfJoint, registers );
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "MOVE" )->getDuration();
		    
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
		if ( langParams.hasCommand( "DELAY" ) )
		  {
		    // do it!
		    // int readOut = theLine->getInstructionElement(0);
		    // get the register
		    int reg = theLine->getInstructionElement(0) % numberOfRegisters;
		    int readOut = registers[reg]->getValue();
		    double delayTime = static_cast<double>( readOut ) * 0.001;
#ifdef _WINDOWS
		    delayTime = ::abs( delayTime );
#else
		    delayTime = std::abs( delayTime );
#endif		
		    if( delayTime > maxDelayTime )
		      delayTime = maxDelayTime;
		    timeAccountSize -= delayTime;
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "DELAY" )->getDuration();
		    
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
		if ( langParams.hasCommand( "NOP" ) )
		  {
		    // do it!
		    programCounter = (programCounter + 1) % programLength;

		    // subtract the needed time for the command
		    timeAccountSize -= langParams.getCommand( "NOP" )->getDuration();
		    
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
		SIGEL_Tools::SIG_IO::cerr << "Something went horribly wrong in the interpreter! Default was called!!! Didn't recognize command. PC++\n";
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




