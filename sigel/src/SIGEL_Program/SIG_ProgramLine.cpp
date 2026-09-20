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
#include "SIGEL_Tools/SIG_IO.h"

#include "stdlib.h"
#include "iostream.h"
#include <qtextstream.h>

SIGEL_Program::SIG_ProgramLine& SIGEL_Program::SIG_ProgramLine::operator =(SIGEL_Program::SIG_ProgramLine const & prgLine)
{ 
  SIG_ProgramLine &tmpPrgLine = const_cast< SIG_ProgramLine& >( prgLine );

  setRobotinstructionType( tmpPrgLine.getRobotinstructionType() );

  element.resize( tmpPrgLine.getNumberOfElements() );

  for( int i=0;i<tmpPrgLine.getNumberOfElements();i++ )
    setElement( i, tmpPrgLine.getElement(i) );
       
  return *this;
}

SIGEL_Program::SIG_ProgramLine::SIG_ProgramLine()
{}

SIGEL_Program::SIG_ProgramLine::~SIG_ProgramLine()
{}

void SIGEL_Program::SIG_ProgramLine::clearLine()
{
  element.resize(0);
}

SIGEL_Program::SIG_ProgramLine::SIG_ProgramLine(SIGEL_Tools::SIG_Randomizer &r, 
					        SIGEL_Robot::SIG_LanguageParameters &languageP,
						QList< int > &prob )
{
   generateRandomRobotInstruction(languageP, r, prob );
}

void SIGEL_Program::SIG_ProgramLine::setRobotinstructionType( Robotinstruction instr )
{
  instructionType = instr;
}

void SIGEL_Program::SIG_ProgramLine::setRobotinstruction(SIGEL_Program::Robotinstruction instr, 
							 int op1,
							 int op2)
{   
    setRobotinstructionType( instr );     

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

SIGEL_Program::Robotinstruction SIGEL_Program::SIG_ProgramLine::getRobotinstructionType()
{
     return instructionType;
}

int SIGEL_Program::SIG_ProgramLine::getInstructionElement( int no )
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
				<< getNumberOfElements() << "\n";
#endif

      // ToDo: Exception ???
      return 0;
    }  
}

QList< int > SIGEL_Program::SIG_ProgramLine::getElementsArray()
{
     return element;
}

int SIGEL_Program::SIG_ProgramLine::getElement( int no )
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
				<< getNumberOfElements() << "\n";
      
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
      // 2003 wrote element[no] here -- in the branch entered BECAUSE no is out
      // of range. Qt 2's QGArray::at clamped the index to 0, so this silently
      // corrupted element 0; the original authors left "ToDo: Exception!" on
      // the next line. The write is dropped: there is no element to set.
      // ToDo: Exception!

#ifdef SIG_DEBUG

      SIGEL_Tools::SIG_IO::cerr << "\n[ProgramLine.cpp<setElement>]: Invalid access, no="
				<< no << ", elementNo=" 
				<< getNumberOfElements() << "\n";
     
#endif

    }
}

int SIGEL_Program::SIG_ProgramLine::getNumberOfElements()
{
  return element.size();
}

void SIGEL_Program::SIG_ProgramLine::print()
{
  QString prgLine = "";

  printToString( prgLine );
  SIGEL_Tools::SIG_IO::cerr << prgLine;

}

void SIGEL_Program::SIG_ProgramLine::printToString(QString &lineStr)
{
  
  bool ok = false;

#ifdef SIG_DEBUG

  //SIGEL_Tools::SIG_IO::cerr << "\n LINE: ";
  //print(); 
 
#endif

   lineStr="";

   switch( getRobotinstructionType() )
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
          lineStr = "DIV " + lineStr.number( getElement( 0 ) ) + ", "
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

void SIGEL_Program::SIG_ProgramLine::readOneRegister(QString &str, int &reg)
{
     int     pos1 = -1,pos2 = -1;
     QString regStr1;
     bool    ok;

     str=str.simplified() + '\n' ;

#ifdef SIG_DEBUG

     // SIGEL_Tools::SIG_IO::cerr << "\nLINE TO READ: ";    
     // SIGEL_Tools::SIG_IO::cerr << str;
     
#endif     

     pos1    = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2    = str.indexOf(QChar('\n'), 1, Qt::CaseInsensitive);

     regStr1 = str.mid( pos1 + 1, pos2 - pos1 - 1 );
     reg     = regStr1.toInt( &ok, 10 );

#ifdef SIG_DEBUG     

     // SIGEL_Tools::SIG_IO::cerr << " One Register <"
     //                           << regStr1
     //                           << "> found.";

#endif

}

void SIGEL_Program::SIG_ProgramLine::readOneRegisterAndIntegerConstant(QString &str,int &reg1,int &cnst)
{
     int     pos1=-1, pos2=-1, pos3=-1, pos4=-1;
     QString regStr1;
     QString cnstStr1;
     bool    ok;

     str=str.simplified() + '\n';

#ifdef SIG_DEBUG

     // SIGEL_Tools::SIG_IO::cerr << "\nLINE TO READ: ";    
     // SIGEL_Tools::SIG_IO::cerr << str;
     
#endif
  
     pos1     = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2     = str.indexOf(QChar(','), pos1, Qt::CaseInsensitive);

     pos3     = pos2;
     pos4     = str.indexOf(QChar('\n'), pos2 + 1, Qt::CaseInsensitive);

     regStr1  = str.mid( pos1 + 1, pos2 - pos1 - 1 );        
     reg1     = regStr1.toInt( &ok, 10 );

     cnstStr1 = str.mid( pos3 + 1, pos4 - pos3 - 1 );         
     cnst     = cnstStr1.toInt( &ok, 10 );

#ifdef SIG_DEBUG

     // SIGEL_Tools::SIG_IO::cerr << " One Register <"
     //                           << regStr1
     //                           << "> and one constant ";
     // SIGEL_Tools::SIG_IO::cerr << "<"
     //                           << cnstStr1
     //                           << "> found.";
     
#endif

}

void SIGEL_Program::SIG_ProgramLine::readTwoRegisters(QString &str,int &reg1,int &reg2)
{
     int     pos1 = -1, pos2 = -1, pos3 = -1, pos4 = -1;
     bool    ok;
     QString regStr1,regStr2;

     str  = str.simplified() + '\n';

     pos1 = str.indexOf(QChar(' '), pos1, Qt::CaseInsensitive);

#ifdef SIG_DEBUG

     // SIGEL_Tools::SIG_IO::cerr << "\nLINE TO READ: ";    
     // SIGEL_Tools::SIG_IO::cerr << str;
     
#endif

     pos1    = str.indexOf(QChar(' '), 1, Qt::CaseInsensitive);
     pos2    = str.indexOf(QChar(','), pos1, Qt::CaseInsensitive);

     pos3    = pos2;
     pos4    = str.indexOf(QChar('\n'), pos2 + 1, Qt::CaseInsensitive);

     regStr1 = str.mid( pos1 + 1, pos2 - pos1 - 1 );        
     reg1    = regStr1.toInt( &ok, 10);

     regStr2 = str.mid( pos3 + 1, pos4 - pos3 - 1 );         
     reg2    = regStr2.toInt( &ok, 10 );

#ifdef SIG_DEBUG

     // SIGEL_Tools::SIG_IO::cerr << " Two Registers <"
     //                           << regStr1
     //                           << "> and ";
     // SIGEL_Tools::SIG_IO::cerr << "<"
     //                           << regStr2
     //                           << "> found.";
     
#endif

}


bool SIGEL_Program::SIG_ProgramLine::readFromFile(QString &str, SIGEL_Program::SIG_ProgramLine *line)
{
     int  reg1=0, reg2=0;
     int  cnst1=0;
     bool lineOK = false;

#ifdef SIG_DEBUG

     //     SIGEL_Tools::SIG_IO::cerr << ".";
     
#endif

     if(str.contains(QLatin1String("COPY"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::COPY, reg1, reg2 );
           lineOK = true;
	}
     if(str.contains(QLatin1String("LOAD"), Qt::CaseInsensitive))
	{
           readOneRegisterAndIntegerConstant(str,reg1,cnst1);
           line->setRobotinstruction( SIGEL_Program::LOAD, reg1, cnst1 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("ADD"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::ADD, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("SUB"), Qt::CaseInsensitive))
	{ 
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::SUB, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MUL"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::MUL, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("DIV"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::DIV, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MIN"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::MIN, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MAX"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::MAX, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("CMP"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::CMP, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MOVE"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotinstruction( SIGEL_Program::MOVE, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("SENSE"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotinstruction( SIGEL_Program::SENSE, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("DELAY"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotinstruction( SIGEL_Program::DELAY, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("JMP"), Qt::CaseInsensitive))
	{
           readOneRegister(str,reg1);
           line->setRobotinstruction( SIGEL_Program::JMP, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("MOD"), Qt::CaseInsensitive))
	{
           readTwoRegisters(str,reg1,reg2);
           line->setRobotinstruction( SIGEL_Program::MOD, reg1, reg2 );
	   lineOK = true;
	}
     if(str.contains(QLatin1String("NOP"), Qt::CaseInsensitive))
	{
          line->setRobotinstruction( SIGEL_Program::NOP, reg1, reg2 );
	  lineOK = true;
	}

     return lineOK;
}



void  SIGEL_Program::SIG_ProgramLine::writeToFile(QTextStream &file)
{

   file<<"       ";

   switch( getRobotinstructionType() )
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


void SIGEL_Program::SIG_ProgramLine::randomRobotinstruction(SIGEL_Robot::SIG_LanguageParameters &languageP,
							    SIGEL_Tools::SIG_Randomizer &r,
							    QList< int > &prob   )
{
     // QList<int> instr;

     int          maximum = 32000; 
     int          op1 = 0, 
                  op2 = 0, 
                  iType = 0,
                  n = 0;
     long         maximumValue = 0,
                  randomValue = 0,
                  accuValue = 0;
     bool         cont = true;
     QList<bool> hasCommand;

     hasCommand.resize( 15 );
     hasCommand.fill( false );

     // getAllowedRobotinstructionsWithProb( instr, languageP, prob );
     
     op1 = r.getRandomInt( maximum );
     op2 = r.getRandomInt( maximum );

     if( r.getRandomInt(2) == 1 ) op1 = (-1) * op1;
     if( r.getRandomInt(2) == 1 ) op2 = (-1) * op2;

     if( languageP.hasCommand("COPY") )
       {
         maximumValue += prob[0];
         hasCommand[0] = true;
       }
         
     if( languageP.hasCommand("LOAD") )
       { 
         maximumValue += prob[1];
	 hasCommand[1] = true;
       }

     if( languageP.hasCommand("ADD") ) 
       {
	 maximumValue += prob[2];
	 hasCommand[2] = true;
       }

     if( languageP.hasCommand("SUB") ) 
       {
	 maximumValue += prob[3];
	 hasCommand[3] = true;
       }

     if( languageP.hasCommand("MUL") )
       {
	 maximumValue +=  prob[4];
	 hasCommand[4] = true;
       }

     if( languageP.hasCommand("DIV") )
       {
	 maximumValue += prob[5];
	 hasCommand[5] = true;
       }

     if( languageP.hasCommand("MOD") )
       {
	 maximumValue += prob[6];
	 hasCommand[6] = true;
       }

     if( languageP.hasCommand("MIN") ) 
       {
	 maximumValue += prob[7];
	 hasCommand[7] = true;
       }

     if( languageP.hasCommand("MAX") ) 
       {
	 maximumValue +=  prob[8];
         hasCommand[8] = true;
       }

     if( languageP.hasCommand("CMP") )
       {
	 maximumValue +=  prob[9];
         hasCommand[9] = true;
       }

     if( languageP.hasCommand("JMP") ) 
       {
	 maximumValue +=  prob[10];
         hasCommand[10] = true;
       }

     if( languageP.hasCommand("SENSE") ) 
       {
	 maximumValue +=  prob[11];
	 hasCommand[11] = true;
       }

     if( languageP.hasCommand("MOVE") )
       {
	 maximumValue +=  prob[12];
	 hasCommand[12] = true;
       }

     if( languageP.hasCommand("DELAY") )
       {
	 maximumValue +=  prob[13];
	 hasCommand[13] = true;
       }

     if( languageP.hasCommand("NOP") )
       {
	 maximumValue += prob[14];
         hasCommand[14] = true;
       }
        	
     if( maximumValue == 0 )
        SIGEL_Tools::SIG_IO::cerr << "\nERROR: If no instructions are allowed, no instructions can be used !\n";

     randomValue = r.getRandomLong( maximumValue );
  
     while( cont )
       { 
         if( (hasCommand[0]) && (n == 0) ) accuValue += prob[0];
         if( (hasCommand[1]) && (n == 1) ) accuValue += prob[1];
	 if( (hasCommand[2]) && (n == 2) ) accuValue += prob[2];
	 if( (hasCommand[3]) && (n == 3) ) accuValue += prob[3];
	 if( (hasCommand[4]) && (n == 4) ) accuValue += prob[4];
	 if( (hasCommand[5]) && (n == 5) ) accuValue += prob[5];
	 if( (hasCommand[6]) && (n == 6) ) accuValue += prob[6];
	 if( (hasCommand[7]) && (n == 7) ) accuValue += prob[7];
	 if( (hasCommand[8]) && (n == 8) ) accuValue += prob[8];
	 if( (hasCommand[9]) && (n == 9) ) accuValue += prob[9];
	 if( (hasCommand[10]) && (n == 10) ) accuValue += prob[10];
	 if( (hasCommand[11]) && (n == 11) ) accuValue += prob[11];
	 if( (hasCommand[12]) && (n == 12) ) accuValue += prob[12];
	 if( (hasCommand[13]) && (n == 13) ) accuValue += prob[13];
         if( (hasCommand[14]) && (n == 14) ) accuValue += prob[14];

         if( n == 14 ) cont = false;
         
         if( randomValue < accuValue )
           {
             cont  = false;
             iType = n;
           }
         
         n++;
       }
       
     

     // iType = instr[r.getRandomInt( instr.size() )]; 

     switch( iType )
       {
        case 0: setRobotinstruction( SIGEL_Program::COPY, op1, op2 );
                break;
        case 1: setRobotinstruction( SIGEL_Program::LOAD, op1, op2 );
                break;
        case 2: setRobotinstruction( SIGEL_Program::ADD, op1, op2 );
                break;
        case 3: setRobotinstruction( SIGEL_Program::SUB, op1, op2 );
                break;
        case 4: setRobotinstruction( SIGEL_Program::MUL, op1, op2 );
                break;
        case 5: setRobotinstruction( SIGEL_Program::DIV, op1, op2 );
                break;
        case 6: setRobotinstruction( SIGEL_Program::MOD, op1, op2 );
                break;
        case 7: setRobotinstruction( SIGEL_Program::MIN, op1, op2 );
                break;
        case 8: setRobotinstruction( SIGEL_Program::MAX, op1, op2 );
                break;
        case 9: setRobotinstruction( SIGEL_Program::CMP, op1, op2 );
                break;
        case 10:setRobotinstruction( SIGEL_Program::JMP, op1, op2 );
                break;
        case 11:setRobotinstruction( SIGEL_Program::SENSE, op1, op2 );
                break;
        case 12:setRobotinstruction( SIGEL_Program::MOVE, op1, op2 );
                break;
        case 13:setRobotinstruction( SIGEL_Program::DELAY, op1, op2 );
                break;
        case 14:setRobotinstruction( SIGEL_Program::NOP, op1, op2 );
                break;
       }

}

void SIGEL_Program::SIG_ProgramLine::generateRandomRobotInstruction(SIGEL_Robot::SIG_LanguageParameters &languageP,
								    SIGEL_Tools::SIG_Randomizer &r,
								    QList< int > &prob   )
{
     randomRobotinstruction(languageP, r, prob);
}


void SIGEL_Program::SIG_ProgramLine::copyLine(SIGEL_Program::SIG_ProgramLine *source,SIGEL_Program::SIG_ProgramLine *destination)
{
   *destination=*source;
};









