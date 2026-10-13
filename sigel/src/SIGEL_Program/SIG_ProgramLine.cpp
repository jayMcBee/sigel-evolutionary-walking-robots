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
#include "SIGEL_Program/SIG_ProgramLine.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_IO.h"

#include "stdlib.h"
#include "iostream.h"
#include <qtextstream.h>

SIGEL_Program::SIG_ProgramLine& SIGEL_Program::SIG_ProgramLine::operator =(SIGEL_Program::SIG_ProgramLine const & prgLine)
{ 
  setRobotInstructionType( prgLine.getRobotInstructionType() );

  element.resize( prgLine.getNumberOfElements() );

  for( int i=0;i<prgLine.getNumberOfElements();i++ )
    setElement( i, prgLine.getElement(i) );
       
  return *this;
}

SIGEL_Program::SIG_ProgramLine::SIG_ProgramLine()
{}

void SIGEL_Program::SIG_ProgramLine::clearLine()
{
  element.resize(0);
}

SIGEL_Program::SIG_ProgramLine::SIG_ProgramLine(SIGEL_Tools::SIG_Randomizer &randomizer, 
					        SIGEL_Robot::SIG_LanguageParameters &languageParameters,
						QList< int > &prob )
{
   generateRandomRobotInstruction(languageParameters, randomizer, prob );
}

void SIGEL_Program::SIG_ProgramLine::setRobotInstructionType( RobotInstruction instr )
{
  instructionType = instr;
}

void SIGEL_Program::SIG_ProgramLine::setRobotInstruction(SIGEL_Program::RobotInstruction instr, 
							 int op1,
							 int op2)
{   
    setRobotInstructionType( instr );     

    switch( instr )
    {
       case SIGEL_Program::COPY:
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::LOAD: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::ADD:
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::SUB: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::MUL: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::DIV: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::MIN: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::MAX: 
	    resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::CMP:
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::JMP: 
            resizeElements( 1 );
            setElement( 0, op1 );
            break;

       case SIGEL_Program::SENSE:
            resizeElements( 1 );
            setElement( 0, op1);
            break;

       case SIGEL_Program::MOVE:
            resizeElements( 1 );
            setElement( 0, op1 );
            break;

       case SIGEL_Program::DELAY:
            resizeElements( 1 );
            setElement( 0, op1 ); 
            break;

       case SIGEL_Program::MOD: 
            resizeElements( 2 );
            setElement( 0, op1 );
            setElement( 1, op2 );
            break;

       case SIGEL_Program::NOP:  
            resizeElements( 0 );
            break;
     }
}

SIGEL_Program::RobotInstruction SIGEL_Program::SIG_ProgramLine::getRobotInstructionType() const
{
     return instructionType;
}

int SIGEL_Program::SIG_ProgramLine::getInstructionElement( int no ) const
{
  if( no >= 0 && no < int(element.size()) )
    {
      return element[no];
    }
  else
    {
#ifdef SIG_DEBUG

      SIGEL_Tools::SIG_IO::cerr << "\n[ProgramLine.cpp<getInstructionElement>]: Invalid access, no="
				<< no << ", elementNo=" 
				<< getNumberOfElements() << Qt::endl;
#endif

      // ToDo: Exception ???
      return 0;
    }  
}

QList< int > SIGEL_Program::SIG_ProgramLine::getElementsArray() const
{
     return element;
}

int SIGEL_Program::SIG_ProgramLine::getElement( int no ) const
{
  if( no >= 0 && no < int(element.size()) )
    {
      return element[no];
    }
  else
    {
#ifdef SIG_DEBUG

      SIGEL_Tools::SIG_IO::cerr << "\n[ProgramLine.cpp<getInstructionElement>]: Invalid access, no="
				<< no << ", elementNo=" 
				<< getNumberOfElements() << Qt::endl;
      
#endif

      // ToDo: Exception ???
      return 0;
    }
}

void SIGEL_Program::SIG_ProgramLine::setElement(int no, int value)
{
  if( no >= 0 && no < int(element.size()) )
    {
     element[no] = value;
    }
 
  else
    {
      // no is out of range, so there is no element to set.
      // ToDo: Exception!

#ifdef SIG_DEBUG

      SIGEL_Tools::SIG_IO::cerr << "\n[ProgramLine.cpp<setElement>]: Invalid access, no="
				<< no << ", elementNo=" 
				<< getNumberOfElements() << Qt::endl;
     
#endif
    }
}

int SIGEL_Program::SIG_ProgramLine::getNumberOfElements() const
{
  return element.size();
}

void SIGEL_Program::SIG_ProgramLine::print() const
{
  QString prgLine = "";

  printToString( prgLine );
  SIGEL_Tools::SIG_IO::cerr << prgLine << Qt::flush;
}

void SIGEL_Program::SIG_ProgramLine::printToString(QString &lineStr) const
{
  bool ok = false;

   lineStr="";

   switch( getRobotInstructionType() )
     {
     case SIGEL_Program::COPY:
          lineStr = "COPY " + lineStr.number( getElement( 0 ) ) + ","
                            + lineStr.number( getElement( 1 ) ) + "\n";
          ok = true;
          break;

     case SIGEL_Program::LOAD:
          lineStr = "LOAD " + lineStr.number( getElement( 0 ) ) + ","
                            + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::ADD:
          lineStr = "ADD " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::SUB:
          lineStr = "SUB " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::MUL:
          lineStr = "MUL " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::DIV:
          lineStr = "DIV " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::MIN:
          lineStr = "MIN " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::MAX:
          lineStr = "MAX " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::CMP:
          lineStr = "CMP " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::JMP:
          lineStr = "JMP " + lineStr.number( getElement( 0 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::SENSE:
          lineStr = "SENSE " + lineStr.number( getElement( 0 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::MOVE:
          lineStr = "MOVE " + lineStr.number( getElement( 0 ) )+ "\n";
	  ok = true;
          break;

     case SIGEL_Program::DELAY:
          lineStr = "DELAY " + lineStr.number( getElement( 0 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::MOD:
          lineStr = "MOD " + lineStr.number( getElement( 0 ) ) + ","
                           + lineStr.number( getElement( 1 ) ) + "\n";
	  ok = true;
          break;

     case SIGEL_Program::NOP:
          lineStr = "NOP\n";
	  ok = true;
          break;
     }

  if( !ok ) lineStr = "#INVALID LINE#\n";  
}

void SIGEL_Program::SIG_ProgramLine::resizeElements(int size)
{
  element.resize(size);
}

int SIGEL_Program::SIG_ProgramLine::operandTextToInteger( const QString &operandText, const QString &lineText ) const
{
  bool isNumber = false;
  const int value = operandText.toInt( &isNumber, 10 );
  if ( !isNumber )
    throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "Program line '" + lineText.trimmed() + "' has a bad operand." );

  return value;
}

void SIGEL_Program::SIG_ProgramLine::readOneRegister(QString &str, int &reg)
{
     int     pos1 = -1,pos2 = -1;
     QString regStr1;

     str=str.simplified() + '\n' ;

     pos1    = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2    = str.indexOf(QChar('\n'), 1, Qt::CaseInsensitive);

     regStr1 = str.mid( pos1 + 1, pos2 - pos1 - 1 );
     reg     = operandTextToInteger( regStr1, str );
}

void SIGEL_Program::SIG_ProgramLine::readOneRegisterAndIntegerConstant(QString &str,int &reg1,int &cnst)
{
     int     pos1=-1, pos2=-1, pos3=-1, pos4=-1;
     QString regStr1;
     QString cnstStr1;

     str=str.simplified() + '\n';

     pos1     = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2     = str.indexOf(QChar(','), pos1, Qt::CaseInsensitive);

     pos3     = pos2;
     pos4     = str.indexOf(QChar('\n'), pos2 + 1, Qt::CaseInsensitive);

     regStr1  = str.mid( pos1 + 1, pos2 - pos1 - 1 );        
     reg1     = operandTextToInteger( regStr1, str );

     cnstStr1 = str.mid( pos3 + 1, pos4 - pos3 - 1 );         
     cnst     = operandTextToInteger( cnstStr1, str );
}

void SIGEL_Program::SIG_ProgramLine::readTwoRegisters(QString &str,int &reg1,int &reg2)
{
     int     pos1 = -1, pos2 = -1, pos3 = -1, pos4 = -1;
     QString regStr1,regStr2;

     str  = str.simplified() + '\n';

     pos1 = str.indexOf(QChar(' '), pos1, Qt::CaseInsensitive);

     pos1    = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2    = str.indexOf(QChar(','), pos1, Qt::CaseInsensitive);

     pos3    = pos2;
     pos4    = str.indexOf(QChar('\n'), pos2 + 1, Qt::CaseInsensitive);

     regStr1 = str.mid( pos1 + 1, pos2 - pos1 - 1 );        
     reg1    = operandTextToInteger( regStr1, str );

     regStr2 = str.mid( pos3 + 1, pos4 - pos3 - 1 );         
     reg2    = operandTextToInteger( regStr2, str );
}

bool SIGEL_Program::SIG_ProgramLine::readFromFile(QString &str, SIGEL_Program::SIG_ProgramLine *line)
{
     int  reg1=0, reg2=0;
     int  cnst1=0;
     bool lineOK = false;

     if(str.contains(QLatin1String("COPY"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::COPY, reg1, reg2 );
           lineOK = true;
	}
     if(str.contains(QLatin1String("LOAD"), Qt::CaseInsensitive))
	{
           readOneRegisterAndIntegerConstant(str,reg1,cnst1);
           line->setRobotInstruction( SIGEL_Program::LOAD, reg1, cnst1 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("ADD"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::ADD, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("SUB"), Qt::CaseInsensitive))
	{ 
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::SUB, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MUL"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::MUL, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("DIV"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::DIV, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MIN"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::MIN, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MAX"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::MAX, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("CMP"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::CMP, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MOVE"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotInstruction( SIGEL_Program::MOVE, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("SENSE"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotInstruction( SIGEL_Program::SENSE, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("DELAY"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotInstruction( SIGEL_Program::DELAY, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("JMP"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotInstruction( SIGEL_Program::JMP, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MOD"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotInstruction( SIGEL_Program::MOD, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("NOP"), Qt::CaseInsensitive))
	{
          line->setRobotInstruction( SIGEL_Program::NOP, reg1, reg2 );
	  lineOK = true;
	}

     return lineOK;
}

void  SIGEL_Program::SIG_ProgramLine::writeToFile(QTextStream &file) const
{
   file<<"       ";

   switch( getRobotInstructionType() )
   {
      case SIGEL_Program::COPY:
           file << "COPY " 
                << getElement( 0 ) << "," 
                << getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::LOAD:
           file << "LOAD " 
                << getElement( 0 ) << "," 
		<< getElement( 1 ) << '\n';
           break;
 
      case SIGEL_Program::ADD:
           file << "ADD " 
		<< getElement( 0 ) << "," 
		<< getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::SUB:
           file << "SUB " 
		<< getElement( 0 ) << "," 
		<< getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::MUL:
           file << "MUL "
		<< getElement( 0 ) << ","
                << getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::DIV:
           file << "DIV "
                << getElement( 0 ) << ","
                << getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::MIN:
           file << "MIN "
                << getElement( 0 ) << ","
                << getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::MAX:
           file << "MAX "
		<< getElement( 0 ) << ","
		<< getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::CMP:
           file << "CMP "
		<< getElement( 0 ) << ","
		<< getElement( 1 ) << '\n';
           break;

      case SIGEL_Program::JMP:
           file << "JMP "
		<< getElement( 0 ) << '\n';
           break;

      case SIGEL_Program::SENSE:
           file << "SENSE "
		<< getElement( 0 ) << '\n';
           break;

      case SIGEL_Program::MOVE:
           file << "MOVE "
                << getElement( 0 ) << '\n';
           break;

      case SIGEL_Program::DELAY:
           file << "DELAY "
		<< getElement( 0 ) << '\n';
           break;

      case SIGEL_Program::MOD:
           file << "MOD "
                << getElement( 0 ) << ","
		<< getElement( 1 ) << '\n';
           break;
 
      case SIGEL_Program::NOP:
           file <<"NOP " <<'\n';
           break;
      }
}

void SIGEL_Program::SIG_ProgramLine::randomRobotInstruction(SIGEL_Robot::SIG_LanguageParameters &languageParameters,
							    SIGEL_Tools::SIG_Randomizer &randomizer,
							    QList< int > &prob   )
{
     int          op1 = 0, 
                  op2 = 0, 
                  selectedInstructionIndex = 0;
     long         totalWeight = 0,
                  randomValue = 0;
     QList<bool> hasCommand;

     hasCommand.resize( instructionCount );
     hasCommand.fill( false );

     op1 = randomizer.getRandomInt( SIGEL_Tools::SIG_Randomizer::fullRandomIntRange );
     op2 = randomizer.getRandomInt( SIGEL_Tools::SIG_Randomizer::fullRandomIntRange );

     if( randomizer.getRandomInt(2) == 1 ) op1 = (-1) * op1;
     if( randomizer.getRandomInt(2) == 1 ) op2 = (-1) * op2;

     totalWeight = totalWeightOfAllowedInstructions( languageParameters, prob, hasCommand );

     if( totalWeight == 0 )
        SIGEL_Tools::SIG_IO::cerr << "\nERROR: If no instructions are allowed, no instructions can be used !" << Qt::endl;

     randomValue = randomizer.getRandomLong( totalWeight );
  
     selectedInstructionIndex = selectInstructionByWeight( randomValue, hasCommand, prob );
       
     switch( selectedInstructionIndex )
       {
        case 0: setRobotInstruction( SIGEL_Program::COPY, op1, op2 );
                break;
        case 1: setRobotInstruction( SIGEL_Program::LOAD, op1, op2 );
                break;
        case 2: setRobotInstruction( SIGEL_Program::ADD, op1, op2 );
                break;
        case 3: setRobotInstruction( SIGEL_Program::SUB, op1, op2 );
                break;
        case 4: setRobotInstruction( SIGEL_Program::MUL, op1, op2 );
                break;
        case 5: setRobotInstruction( SIGEL_Program::DIV, op1, op2 );
                break;
        case 6: setRobotInstruction( SIGEL_Program::MOD, op1, op2 );
                break;
        case 7: setRobotInstruction( SIGEL_Program::MIN, op1, op2 );
                break;
        case 8: setRobotInstruction( SIGEL_Program::MAX, op1, op2 );
                break;
        case 9: setRobotInstruction( SIGEL_Program::CMP, op1, op2 );
                break;
        case 10:setRobotInstruction( SIGEL_Program::JMP, op1, op2 );
                break;
        case 11:setRobotInstruction( SIGEL_Program::SENSE, op1, op2 );
                break;
        case 12:setRobotInstruction( SIGEL_Program::MOVE, op1, op2 );
                break;
        case 13:setRobotInstruction( SIGEL_Program::DELAY, op1, op2 );
                break;
        case 14:setRobotInstruction( SIGEL_Program::NOP, op1, op2 );
                break;
       }
}

long SIGEL_Program::SIG_ProgramLine::totalWeightOfAllowedInstructions( const SIGEL_Robot::SIG_LanguageParameters &languageParameters,
								       const QList<int> &prob,
								       QList<bool> &hasCommand ) const
{
     long totalWeight = 0;

     for( int n = 0; n < instructionCount; n++ )
       {
         if( !languageParameters.hasCommand( robotInstructionNames[n] ) )
           continue;

         totalWeight += prob[n];
         hasCommand[n] = true;
       }

     return totalWeight;
}

int SIGEL_Program::SIG_ProgramLine::selectInstructionByWeight( long randomValue,
							       const QList<bool> &hasCommand,
							       const QList<int> &prob ) const
{
     long cumulativeWeight = 0;

     for( int n = 0; n < instructionCount; n++ )
       {
         if( hasCommand[n] )
           cumulativeWeight += prob[n];

         if( randomValue < cumulativeWeight )
           return n;
       }

     return 0;
}

void SIGEL_Program::SIG_ProgramLine::generateRandomRobotInstruction(SIGEL_Robot::SIG_LanguageParameters &languageParameters,
								    SIGEL_Tools::SIG_Randomizer &randomizer,
								    QList< int > &prob   )
{
     randomRobotInstruction(languageParameters, randomizer, prob);
}

void SIGEL_Program::SIG_ProgramLine::copyLine(SIGEL_Program::SIG_ProgramLine *source,SIGEL_Program::SIG_ProgramLine *destination)
{
   *destination=*source;
};
