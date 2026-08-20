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
#include "SIGEL_GP/SIG_GPIndividual.h"
#ifndef _WINDOWS
#include <iostream.h>
#endif


SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPIndividual::operator =(SIG_GPIndividual& ind)
   {     
     setAge(0);
     history=ind.getHistory();
     setName(ind.getName());
     setPoolPos(ind.getPoolPos());
     getProgramVar()=ind.getProgramVar();
     setFitness( ind.getFitness() );
     return *this;
   }
 

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual(SIG_GPParameter& param,
					     SIGEL_Tools::SIG_Randomizer& random,
					     QString name, QDateTime birthtime,
					     int poolpos)
  : prog()
{
  setName(name);
  setPoolPos(poolpos);
  setFitness(-1);
  setAge(0);

  QString nameTmp    = "NAME         : " + getName();
  QString str;
  QString poolposTmp = "Pool Position: " + str.number(getPoolPos(),10);
  QString birthdTmp  = "Date of Birth: " + birthtime.toString();
  QString ageTmp     = "Age          : " + str.number(getAge(),10);
  QString fitTmp     = "Fitness Value: " + str.number(getFitness(),'g',6);

  //history.append( "\n--- History of the Individual in the Generation Number: "+ str.number(getAge(),10) + " ---");
  history.append("CREATED NEW INDIVIDUUM:\n======================="); 
  history.append(nameTmp);
  history.append(birthdTmp);
  history.append(poolposTmp);
  history.append(ageTmp);
  history.append(fitTmp); 
};

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual( SIG_GPParameter const& param,
					      QStringList const& hist,
					      QString name,
					      QDateTime birthtime,

					      int poolpos )
  : prog()
{
  setName( name );
  setPoolPos(poolpos);
  setHistory( hist );
  setFitness( -1 );
  setAge( 0 );
};

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual( SIGEL_Program::SIG_Program const& prog,
					      QString name1,
					      QString name2,
					      QString name3,
					      QDateTime birthtime,
					      int poolpos,
					      QStringList const& hist )
  :prog()
{
   // getProgramVar()=prog;
   setName(name1);
   setPoolPos(poolpos);
   setHistory( hist );
   setFitness( -1 );
   setAge( 0 );
};

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual():prog()
{  
   setName("<no name set>");
   setPoolPos(0);
   setFitness(-1);
   setAge(0);
}

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual(SIGEL_Tools::SIG_Randomizer &random):
  prog()
{ 
   setName("No Name");
   setFitness(-1);
   setAge(0);
   setPoolPos(0);
   QDateTime cdt      = cdt.currentDateTime();
   QString nameTmp    = "NAME         : " + getName();
   QString str;
   QString poolposTmp = "Pool Position: " + str.number(getPoolPos(),10);
   QString birthdTmp  = "Date of Birth: " + cdt.toString();
   QString ageTmp     = "Age          : " + str.number(getAge(),10);
   QString fitTmp     = "Fitness Value: " + str.number(getFitness(),'g',6);     

   history.append("\nINDIVIDUAL WITH RANDOMLY GENERATED PROGRAM:\n===========================================");
   history.append(nameTmp);
   history.append(birthdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);

}

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual(SIGEL_Tools::SIG_Randomizer &random, 
                                             SIGEL_GP::SIG_GPParameter& param, 
                                             SIGEL_Robot::SIG_LanguageParameters& languageP)
  :prog( param, languageP,random )
{ 
   setName( "No name" );
   setFitness( -1 );
   setAge( 0 );
   setPoolPos( 0 );

   QString str;
   QDateTime cdt = cdt.currentDateTime();

   history.append( "INDIVIDUAL IS CREATED:\n====================" );
   history.append( "Date of Birth: " + cdt.toString() );
   //history.append( "Poolpos      : " + str.number(getPoolPos(),10) );
}

SIGEL_GP::SIG_GPIndividual::SIG_GPIndividual(QString data)
{
   loadIndividual(data);
};

SIGEL_GP::SIG_GPIndividual::~SIG_GPIndividual()
{
#ifdef SIG_DEBUG

  //   SIGEL_Tools::SIG_IO::cerr << "\nDestructor of INDIVIDUAL called!\n";

#endif
}


void SIGEL_GP::SIG_GPIndividual::generateRandomIndividual(SIGEL_Tools::SIG_Randomizer& random,
                                                          SIGEL_GP::SIG_GPParameter& param, 
                                                          SIGEL_Robot::SIG_LanguageParameters& languageP )
{
   prog.generateRandomProgram(param,languageP,random);
   setName("No Name");
   setPoolPos(0);
   setFitness(-1);
   setAge(0);
   QDateTime cdt    = cdt.currentDateTime();
   QString nameTmp    = "NAME         : " + getName();
   QString str;
   QString poolposTmp = "Pool Position: " + str.number(getPoolPos(),10);
   QString birthdTmp  = "Date of Birth: " + cdt.toString();
   QString ageTmp     = "Age          : " + str.number(getAge(),10);
   QString fitTmp     = "Fitness Value: " + str.number(getFitness(),'g',6);     

   //history.append( "\n--- History of the Individual in the Generation Number: "+ str.number(getAge(),10) + " ---");
   history.append("INDIVIDUUM IS GENERATED RANDOMLY:\n---------------------------------");
   history.append(nameTmp);
   history.append(birthdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);

}

void SIGEL_GP::SIG_GPIndividual::increaseAge()
{
   age++;
};

void SIGEL_GP::SIG_GPIndividual::increaseAgeInfo()
{
   QString str;
   history.append( "\nAGE IS INCREASED:\n-----------------");
   history.append( "New Age: " + str.number( getAge(), 10 ) );
};

long SIGEL_GP::SIG_GPIndividual::getAge() const
{
   return age;
};

void SIGEL_GP::SIG_GPIndividual::importProgram(SIGEL_Program::SIG_Program& prog )
{
   getProgramVar()=prog;
};

void SIGEL_GP::SIG_GPIndividual::importProgram( QString& filename )
{
   getProgramVar().importProgram( filename );
   addImportProgramInfo( QDateTime::currentDateTime() );

   // you'll never know where these programs stem from..
   fitnessValue = -1.0;
}

void SIGEL_GP::SIG_GPIndividual::exportProgram( QString& filename )
{
   getProgramVar().exportProgram( filename );
}


QString SIGEL_GP::SIG_GPIndividual::getName()
{
   return indName;
};

void SIGEL_GP::SIG_GPIndividual::loadIndividual(QString data)
{
   readFromFile(data);
}; 

QString SIGEL_GP::SIG_GPIndividual::saveIndividual(bool history)
{
  QString                     str;
  QTextStream                 outputFile(&str, QIODeviceBase::WriteOnly);

  writeToFile(outputFile,history);

  return str;
};
 
void SIGEL_GP::SIG_GPIndividual::copyIndividual(SIGEL_GP::SIG_GPIndividual& cInd,
                                                int poolPos)
{
   *this=cInd;
   setPoolPos(poolPos);
}; 

int SIGEL_GP::SIG_GPIndividual::getPoolPos() const
{
   return poolPos;
};

void SIGEL_GP::SIG_GPIndividual::setPoolPos(int pp)
{
   poolPos=pp;
};

void SIGEL_GP::SIG_GPIndividual::setPoolPosInfo()
{
   QString str;
   history.append("\nINDIVIDUAL HAS A NEW POOL POSITION:\n-----------------------------------");
   history.append("New Pool Position: " + str.number(getPoolPos(),10));
};

SIGEL_Program::SIG_Program  const & SIGEL_GP::SIG_GPIndividual::getProgram() const
{
   return prog;
};

SIGEL_Program::SIG_Program& SIGEL_GP::SIG_GPIndividual::getProgramVar()
{
   return prog;
};

void SIGEL_GP::SIG_GPIndividual::setFitness(double fit)
{
  fitnessValue=fit;
};

void SIGEL_GP::SIG_GPIndividual::setFitnessInfo()
{
  QString str;
  history.append( "\nNEW FITNESS VALUE SET:\n----------------------");
  history.append( "Evaluated fitness value: " + str.number( getFitness(), 'g', 6 ) );
 
};

double SIGEL_GP::SIG_GPIndividual::getFitness() const
{
   return fitnessValue;
};

QStringList SIGEL_GP::SIG_GPIndividual::getHistory() const
{
   return history;
};


void SIGEL_GP::SIG_GPIndividual::addMutationInfo(QString name, 
                                                 QDateTime time, 
                                                 int mutpoint)
{
   QString str;
   QString mutdTmp    = "Date of Mutation: " + time.toString();
   QString ageTmp     = "Current Age: " + str.number(getAge(),10);
   QString mpointTmp  = "Mutation Point: " + str.number(mutpoint);
   QString fitTmp     = "Fitness Value (before Mutation): " + str.number(getFitness(),'g',6);
   QString poolposTmp = "Current Pool Position: " + str.number(getPoolPos(),10);

   history.append("\nMUTATION:\n---------");
   history.append("Father: " + name);
   history.append(mpointTmp);
   history.append(mutdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);   
};

void SIGEL_GP::SIG_GPIndividual::addMutationInfo(QString name, 
                                                 QDateTime time, 
                                                 int mutpoint,
						 double fit )
{
   QString str;
   QString mutdTmp    = "Date of Mutation: " + time.toString();
   QString ageTmp     = "Current Age: " + str.number(getAge(),10);
   QString mpointTmp  = "Mutation Point: " + str.number(mutpoint);
   QString fitTmp     = "Fitness Value (before Mutation): " + str.number(fit,'g',6);
   QString poolposTmp = "Current Pool Position: " + str.number(getPoolPos(),10);

   history.append("\nMUTATION:\n---------");
   history.append("Father: " + name);
   history.append(mpointTmp);
   history.append(mutdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);   
};

void SIGEL_GP::SIG_GPIndividual::addCrossOverInfo(QString name1, 
                                                  QString name2, 
                                                  QDateTime time, 
                                                  int xoverpnt1, 
                                                  int xoverpnt2)
{
   QString str;
   QString mutdTmp    = "Date of CrossOver: " + time.toString();
   QString nameTmp    = "Parents: " + name1 +" + "+name2;
   QString ageTmp     = "Current Age: " + str.number(getAge(),10);
   QString cpointTmp  = "Crossover Points: " + str.number(xoverpnt1) + " + " + str.number(xoverpnt2);
   QString fitTmp     = "Fitness Value (before Crossover): " + str.number(getFitness(),'g',6);
   QString poolposTmp = "Current Pool Position: " + str.number(getPoolPos(),10);

   history.append("\nCROSSOVER:\n----------");
   history.append(nameTmp);
   history.append(cpointTmp);
   history.append(mutdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);
};


void SIGEL_GP::SIG_GPIndividual::addCrossOverInfo(QString name1, 
                                                  QString name2, 
                                                  QDateTime time, 
                                                  int xoverpnt1, 
                                                  int xoverpnt2,
						  double fitness1,
						  double fitness2 )
{
   QString str;
   QString mutdTmp    = "Date of CrossOver: " + time.toString();
   QString nameTmp    = "Parents: " + name1 +" + "+name2;
   QString cpointTmp  = "Crossover Points: " + str.number(xoverpnt1) + " + " + str.number(xoverpnt2);
   QString fitTmp1    = "Fitness (Elter 1): " + str.number(fitness1,'g',6);
   QString fitTmp2    = "Fitness (Elter 2): " + str.number(fitness2,'g',6);
   QString poolposTmp = "Current Pool Position: " + str.number(getPoolPos(),10);

   history.append("\nCROSSOVER:\n----------");
   history.append(nameTmp);
   history.append(cpointTmp);
   history.append(mutdTmp);
   history.append(poolposTmp);
   history.append(fitTmp1);
   history.append(fitTmp2);
};

void SIGEL_GP::SIG_GPIndividual::addReproductionInfo(QString name, 
                                                     QDateTime time)
{
   QString str;
   QString reprdTmp   = "Date of Reproduction: " + time.toString();
   QString ageTmp     = "Current Age: " + str.number(getAge(),10);
   QString fitTmp     = "Fitness Value: " + str.number(getFitness(),'g',6);
   QString poolposTmp = "Current Pool Position: " + str.number(getPoolPos(),10);

   history.append("\nREPRODUCTION:\n-------------");
   history.append("Father: " + name);
   history.append(reprdTmp);
   history.append(poolposTmp);
   history.append(ageTmp);
   history.append(fitTmp);
};


void SIGEL_GP::SIG_GPIndividual::addPreparationOfHistoryInfo()
{
   history.append( "\n-----------------------------" );
   history.append( " OLD INDIVIDUAL DATA DELETED" );
   history.append( " DATA OF IMPORTED INDIVIDUAL:" );
   history.append( "-----------------------------" );
}

void SIGEL_GP::SIG_GPIndividual::addImportIndividualInfo(QDateTime time)
{
   history.append( "\nINDIVIDUAL IMPORTED:\n--------------------" );
   history.append( "Date of Import: " + time.toString() );
}

void SIGEL_GP::SIG_GPIndividual::addImportProgramInfo(QDateTime time)
{
   history.append( "\nOLD PROGRAM DELETED\n" );
   history.append( "\nPROGRAM IMPORTED:\n-----------------" );
   history.append( "Date of Import: " + time.toString() );
}

void SIGEL_GP::SIG_GPIndividual::addLengthIncreasedInfo( QDateTime time, long lgth )
{
    history.append( "\nPROGRAM LENGTH INCREASED:\n-------------------------");
    history.append( "Date of action: " + time.toString() );
    QString str;
    history.append( "Added Length: " + str.number( lgth ) );
}

void SIGEL_GP::SIG_GPIndividual::addLengthDecreasedInfo( QDateTime time, long lgth )
{
    history.append( "\nPROGRAM LENGTH DECREASED:\n-------------------------");
    history.append( "Date of action: " + time.toString() );
    QString str;
    history.append( "Removed Length: " + str.number( lgth ) );
}


bool SIGEL_GP::SIG_GPIndividual::upToDate()const
{
  return ( getFitness() != -1 ); 
};


void SIGEL_GP::SIG_GPIndividual::importIndividual( QString& filename )
{   
   QFile   indFile( filename );
   QString indString;

   if( indFile.open( QIODeviceBase::ReadOnly ) )
     {
       addPreparationOfHistoryInfo();
       QTextStream buffer( &indFile );
       indString = buffer.readAll();
       readFromFile( indString );
       indFile.close();
       addImportIndividualInfo( QDateTime::currentDateTime() );
       fitnessValue = -1.0;
     }
   else
     SIGEL_Tools::SIG_IO::cerr << "Could not import Individual from "
					<< filename
					<< "!\n";   
}


void SIGEL_GP::SIG_GPIndividual::exportIndividual( QString& filename )
{
   QFile indFile( filename );

   if( indFile.open( QIODeviceBase::WriteOnly ) )
     {
       QTextStream buffer( &indFile );
       writeToFile( buffer,true );
       indFile.close();
     }
   else
     SIGEL_Tools::SIG_IO::cerr << "Could not export Individual to "
			       << filename
			       << "!\n";   
}


SIGEL_Program::SIG_Program *SIGEL_GP::SIG_GPIndividual::getProgramPointer()
{
   return &prog;
};

void SIGEL_GP::SIG_GPIndividual::setAge(long a)
{
   age=a;
};

void SIGEL_GP::SIG_GPIndividual::setAgeInfo()
{
   QString str;
   history.append( "*INDIVIDUAL HAS A NEW AGE*");
   history.append( "New Age: " + str.number( getAge(), 10 ) );
};

void SIGEL_GP::SIG_GPIndividual::setName(QString n)
{
   indName=n; 
};

void SIGEL_GP::SIG_GPIndividual::setNameInfo()
{
   QString str;
   history.append( "\nINDIVIDUAL HAS A NEW NAME:\n--------------------------");
   history.append( "New Name: " + str.number( getAge(), 10 ) );
};


void SIGEL_GP::SIG_GPIndividual::print()
{
  SIGEL_Tools::SIG_IO::cerr << "\nCurrent individual's data:\n--------------------------"
                            << "\n- Name   : "
                            << getName()
                            << "\n- Poolpos: "
                            << getPoolPos()
                            << "\n- Fitness: "
                            << getFitness()
                            << "\n- Age    : "
                            << getAge()
                            << "\n- Program:\n\n";

  getProgramVar().print();
  
}


void SIGEL_GP::SIG_GPIndividual::writeToFile(QTextStream &file, bool _history)
{
  // HISTORY IS MISSING !!!!

  file<<"\n    INDIVIDUAL BEGIN{ "<<"\n      NAME='"<<getName()<<"';";
  file<<"\n      POOLPOS="<<getPoolPos()<<";";
  file<<"\n      FITNESS="<<getFitness()<<";";
  file<<"\n      AGE="<<getAge()<<";";

  // you select whether the history should be saved or not.
  if (_history) {
  	file<<"\n      HISTORY BEGIN{";
  	file<<history.join("\n");
  	file<<"\n      }HISTORY END;";
  }

  file<<"\n      PROGRAM BEGIN{"<<'\n';
  
  getProgramPointer()->writeToFile(file);

  file<<"        }PROGRAM END;";

  file<<"\n    }INDIVIDUAL END";
}

void SIGEL_GP::SIG_GPIndividual::setHistory( QStringList hist )
{
  history = hist;
}

void SIGEL_GP::SIG_GPIndividual::readFromFile(QString indStr)
{

  long                        pos;

  QString                     prgStr;
  QString                     histStr;
  QTextStream                 outputFile(&prgStr, QIODeviceBase::WriteOnly);  
  QTextStream                 inputFile(&prgStr, QIODeviceBase::ReadOnly);


  // cout<<"Received string:\n"<<indStr<<"\n";  
  

  if((pos=indStr.indexOf("NAME='", 0, Qt::CaseInsensitive))!=-1)
    {
      // cout<<indStr.mid(pos+6,indStr.indexOf("'", pos+7, Qt::CaseInsensitive)-pos-6);
      setName(indStr.mid(pos+6,indStr.indexOf("'", pos+7, Qt::CaseInsensitive)-pos-6));
    }
  else
    {
      
    }

  if((pos=indStr.indexOf("POOLPOS=", 0, Qt::CaseInsensitive))!=-1)
    {
      //cout<<indStr.mid(pos+8,indStr.indexOf(";", pos+9, Qt::CaseInsensitive)-pos-8);
      setPoolPos((indStr.mid(pos+8,indStr.indexOf(";", pos+9, Qt::CaseInsensitive)-pos-8)).toLong());
    }
  else
    {
      
    }  

  if((pos=indStr.indexOf("FITNESS=", 0, Qt::CaseInsensitive))!=-1)
    {
      //cout<<indStr.mid(pos+8,indStr.indexOf(";", pos+9, Qt::CaseInsensitive)-pos-8);
      setFitness((indStr.mid(pos+8,indStr.indexOf(";", pos+9, Qt::CaseInsensitive)-pos-8)).toDouble());
    }
  else
    {
      
    }

  if((pos=indStr.indexOf("AGE=", 0, Qt::CaseInsensitive))!=-1)
    {
      //cout<<indStr.mid(pos+4,indStr.indexOf(";", pos+5, Qt::CaseInsensitive)-pos-4);
      setAge((indStr.mid(pos+4,indStr.indexOf(";", pos+5, Qt::CaseInsensitive)-pos-4)).toLong());
    }
  else
    {
      
    }  
  
  if((pos=indStr.indexOf("PROGRAM BEGIN{", 0, Qt::CaseInsensitive))!=-1)
    {
      //cout<<indStr.mid(pos+14,indStr.indexOf("}PROGRAM END", pos+15, Qt::CaseInsensitive)-pos-14);
      prgStr=indStr.mid(pos+15,indStr.indexOf("}PROGRAM END", pos+16, Qt::CaseInsensitive)-pos-14);
      getProgramPointer()->clear();
      getProgramPointer()->readFromFile(inputFile);
 
      
    }
  else
    {
      
    } 
  
  if((pos=indStr.indexOf("HISTORY BEGIN{", 0, Qt::CaseInsensitive))!=-1)
    {
       // history.clear();
       histStr=indStr.mid(pos+14,indStr.indexOf("}HISTORY END", pos+15, Qt::CaseInsensitive)-pos-14);
       history.append(histStr);
    }
  else
    {


    } 
   
}
