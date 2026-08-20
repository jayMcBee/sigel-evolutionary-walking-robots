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
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_Tools/SIG_IO.h"

#include "stdlib.h"
#include "iostream.h" 

#include <qprogressdialog.h> 

#ifdef _WINDOWS
using namespace std;
#endif

SIGEL_Program::SIG_Program& SIGEL_Program::SIG_Program::operator =(SIGEL_Program::SIG_Program &prg)
{ 
  clear();

#ifdef SIG_DEBUG
  for ( long int i=0; i<lines.size(); i++ )
    if (!lines[i])
      {
	SIGEL_Tools::SIG_IO::cerr << "Ouch [1] !!!! " << i << "\n";
	exit(1);
      };
#endif

  for( long i=0; i<prg.getProgramLength(); i++ )
  {
     SIG_ProgramLine *newProgLine = new SIG_ProgramLine();
     *newProgLine = *prg.getLine( i );
     appendLine( newProgLine );
  }

#ifdef SIG_DEBUG
  if( prg.getProgramLength() != lines.size() )
    {
      SIGEL_Tools::SIG_IO::cerr << "Ouch [2] !!!!\n"; 
      exit(1);
    };
#endif

#ifdef SIG_DEBUG
  for ( long int i=0; i<lines.size(); i++ )
    if (!lines[i])
      {
	SIGEL_Tools::SIG_IO::cerr << "Ouch [3] !!!! " << i << "\n";
	exit(1);
      };
#endif

  return *this;
}

long SIGEL_Program::SIG_Program::getProgramLength()
{
     return lines.size();
}

vector<SIGEL_Program::SIG_ProgramLine*> &SIGEL_Program::SIG_Program::getPrgLines()
{
     return lines;
}

void SIGEL_Program::SIG_Program::printToString( QString &str )
{
  
  for( long i=0; i<lines.size(); i++ )
    {
      QString buffer;
      lines[i]->printToString(buffer);
      str += buffer;
    };

}

void SIGEL_Program::SIG_Program::print()

{

  for (int i=0; i<lines.size(); i++ ) lines[i]->print();

}

void SIGEL_Program::SIG_Program::writeToFile( QTextStream &file )
{
  for( long i=0; i<getProgramLength(); i++ )
    {
      lines[i]->writeToFile(file);
    }

}

void SIGEL_Program::SIG_Program::clear()
{
    for (int i=0; i<lines.size(); i++ ) delete lines[i];
    lines.clear();
}

void SIGEL_Program::SIG_Program::readFromFile( QTextStream &file )
{ 
   QString                         tmpLine,prg;
   int                             pos=-1;
   int                             oldpos=0;
   int                             lineCnt=0;
   SIGEL_Program::SIG_ProgramLine *prgLine;
 
#ifdef SIG_DEBUG

   SIGEL_Tools::SIG_IO::cerr << "Reading Program: ... ";
   

#endif

   if( getProgramLength() > 0 ) clear();
   
   prg=file.read();

#ifdef SIG_DEBUG

   // SIGEL_Tools::SIG_IO::cerr << "The received program:\n";
   // cout<<"["<<prg<<"]\n\n";

#endif
   
   while((pos=prg.indexOf(QChar('\n'), oldpos, Qt::CaseInsensitive))!=-1)
     {
        tmpLine = prg.mid( oldpos, pos - oldpos );
        prgLine = new SIGEL_Program::SIG_ProgramLine();
        if( prgLine->readFromFile( tmpLine, prgLine ) )
	  {
            appendLine( prgLine );
            lineCnt++;
	  }
        oldpos = pos + 1;
     }

#ifdef SIG_DEBUG

   SIGEL_Tools::SIG_IO::cerr << lineCnt << " LINES READ.\n";

#endif
   
}


SIGEL_Program::SIG_ProgramLine *SIGEL_Program::SIG_Program::getLine( long no )
{

#ifdef SIG_DEBUG
  if ((no < 0) || (no >= lines.size()) || (!lines[no]))
  {
    SIGEL_Tools::SIG_IO::cerr << "Ouch [4] ! " << no << " " << lines[no] << "\n";
    exit(1);
  };
#endif

  return lines[no];
}


void SIGEL_Program::SIG_Program::deleteLine( long no )
{
  if( no<lines.size() )
    {
      delete lines[no];
  
      for( long i=no; i<lines.size()-1; i++)
         lines[ i ]=lines[ i + 1 ];  

      lines.resize( lines.size() - 1 );
    }
}


void SIGEL_Program::SIG_Program::appendLine( SIGEL_Program::SIG_ProgramLine *l )
{
  lines.resize( getProgramLength() + 1 );
  lines[ getProgramLength() - 1 ] = l; 

#ifdef SIG_DEBUG
  for ( long int i=0; i<lines.size(); i++ )
    if (!lines[i])
      {
	SIGEL_Tools::SIG_IO::cerr << "Ouch [5] ! " << i << "\n";
	exit(1);
      };
#endif
}

void SIGEL_Program::SIG_Program::insertLine( long no, SIGEL_Program::SIG_ProgramLine *l )
{
  lines.resize( getProgramLength() + 1 );

  for( long i=lines.size()-1; i>no; i--)
         lines[ i ]=lines[ i - 1 ];  

  lines[no] = l;
}

   
void SIGEL_Program::SIG_Program::importProgram( QString& filename )
{
   QFile prgFile( filename );

   if( prgFile.open( IO_ReadOnly ) )
     {
       clear();
       QTextStream buffer( &prgFile );
       readFromFile( buffer );
       prgFile.close();
     }
   else
     SIGEL_Tools::SIG_IO::cerr << "Could not import program from "
			       << filename
			       << "!\n";  

}

void SIGEL_Program::SIG_Program::exportProgram( QString& filename )
{
   QFile prgFile( filename );

   if( prgFile.open( IO_WriteOnly ) )
     {
       QTextStream buffer( &prgFile );
       writeToFile( buffer );
       prgFile.close();
     }
   else
     SIGEL_Tools::SIG_IO::cerr << "Could not export program to "
			       << filename
			       << "!\n";  
}


SIGEL_Program::SIG_Program::SIG_Program()
  : lines()
{

#ifdef SIG_DEBUG
  for ( long int i=0; i<lines.size(); i++ )
    if (!lines[i])
      {
	SIGEL_Tools::SIG_IO::cerr << "Ouch [4] ! " << i << "\n";
	exit(1);
      };
#endif
}


void SIGEL_Program::SIG_Program::checkLength( long minimumLength, 
					      long maximumLength, 
					      SIGEL_Tools::SIG_Randomizer &r, 
					      SIGEL_Robot::SIG_LanguageParameters &languageP,
					      Q2Array< int > &prob,
					      int &historyInfo )
{
  long prgLength = getProgramLength();
  historyInfo    = 0;

  if( minimumLength > 0 )
    if( prgLength < minimumLength )
      {

#ifdef SIG_DEBUG

       SIGEL_Tools::SIG_IO::cerr << "\n\n--> PROGRAM LENGTH ("
                                 << prgLength
                                 << ") TOO SHORT (minimum="
                                 << minimumLength
                                 << "): ";

#endif   
       historyInfo = minimumLength - prgLength;

       for( long i = 0; i < minimumLength - prgLength; i++ )
	 {
          SIG_ProgramLine *newLine = new SIGEL_Program::SIG_ProgramLine( r, 
								         languageP,
		 							 prob );

          // If there is a problem during the evolution, try to delete the following
          // instruction:

          newLine->setRobotinstruction( SIGEL_Program::NOP, 0, 0 );

          // If the problem is still existing, please try to delete the line 135 (resizeElements( 0 );)
          // within the function SIGEL_Program::SIG_ProgramLine::setRobotinstruction.
  
          appendLine( newLine );
         } 

#ifdef SIG_DEBUG

       SIGEL_Tools::SIG_IO::cerr << "Program increased to " 
				 << getProgramLength()
	                         << " lines.\n";
#endif
       
      }

  prgLength = getProgramLength();

  if( maximumLength > 0 )
    if( prgLength > maximumLength )
      {

#ifdef SIG_DEBUG

       SIGEL_Tools::SIG_IO::cerr << "\n\n--> PROGRAM LENGTH ("
                                 << prgLength
                                 << ") TOO LONG (maximum="
	                         << maximumLength
                                 << "): ";

#endif 
       historyInfo = - prgLength - maximumLength; 

       for( long i = prgLength; i >= maximumLength; i-- )
          deleteLine( i );

#ifdef SIG_DEBUG

       SIGEL_Tools::SIG_IO::cerr << "Program decreased to " 
				 << getProgramLength()
	                         << " lines.\n";
#endif
       historyInfo = 2;
      }
}

SIGEL_Program::SIG_Program::SIG_Program( SIGEL_GP::SIG_GPParameter &param, 
                                         SIGEL_Robot::SIG_LanguageParameters &languageP, 
                                         SIGEL_Tools::SIG_Randomizer& random )
  : lines()
{
  generateRandomProgram( param, languageP, random );
}

void SIGEL_Program::SIG_Program::generateRandomProgram( SIGEL_GP::SIG_GPParameter &param, 
                                                        SIGEL_Robot::SIG_LanguageParameters &languageP, 
						        SIGEL_Tools::SIG_Randomizer& random )
{
    clear();

    long n = param.getMinIndLength() + 
             random.getRandomInt( param.getMaxIndLength() - param.getMinIndLength() );


    for( long x=0; x<n; x++ )
      {
        SIG_ProgramLine *newLine = new SIGEL_Program::SIG_ProgramLine( random, 
								       languageP,
								       param.getInstructionProbabilities() );

        appendLine( newLine );

      }
}
   
SIGEL_Program::SIG_Program::~SIG_Program()
{
#ifdef SIG_DEBUG

  //  SIGEL_Tools::SIG_IO::cerr << "\nDestructor of PROGRAMM called!\n";

#endif
  
   clear();
}
