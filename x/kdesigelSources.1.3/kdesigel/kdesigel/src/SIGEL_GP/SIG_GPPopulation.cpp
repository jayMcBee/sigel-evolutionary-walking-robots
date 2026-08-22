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
#include <QApplication>   // qApp and QProgressDialog need QtWidgets
#include <QProgressDialog>   // widget used in this file only
#include "SIGEL_GP/SIG_GPPopulation.h"

#include<iostream>

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation()
  : pool(),
    randomizer( new SIGEL_Tools::SIG_Randomizer() ),
    nextIdentifier( QString::number( 0 ) ),
    poolGeneration(0)
{  
  pool.setAutoDelete(true);
  history = true;
};

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size)
{
   if( getRandomizerPointer()==0 )
     {
       SIGEL_Tools::SIG_IO::cerr << "\n\nA randomizer is needed to initialize a population ! Program terminated.\n";
       exit(1); 
       // ToDo: Exception
     }

   pool.setAutoDelete( true );
   pool.resize( size );

   history = true;

   setNextIdentifier( QString::number(0) );
   setPoolGeneration( 0 );
   
   for( int x=0; x<getSize(); x++ ) 
     { 
       pool.insert( x, new SIGEL_GP::SIG_GPIndividual( *getRandomizerPointer() ) );
       getIndividualPointer( x )->setPoolPos( x );
     }  
};

    
SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(QString data)
{
   QTextStream                 inputFile(&data, QIODeviceBase::ReadOnly);

   history = true;

#ifdef SIG_DEBUG

     SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION DATA FROM FILE:\n";    
     
#endif

   readFromFile(inputFile);

#ifdef SIG_DEBUG

     SIGEL_Tools::SIG_IO::cerr << "\n\nREADING FINISHED.\n\n";
     
#endif

};


SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size, 
					     SIGEL_Tools::SIG_Randomizer &r, 
					     SIGEL_GP::SIG_GPParameter& param,
					     SIGEL_Robot::SIG_LanguageParameters& languageP)
{
   setRandomizer( &r );

   pool.setAutoDelete( true );
   pool.resize( size );

   history = true;

   setNextIdentifier( QString::number(0) );
   setPoolGeneration( 0 );

   for( int x=0; x<getSize(); x++ )
     { 
       pool.insert( x, new SIGEL_GP::SIG_GPIndividual( r, param, languageP ) );
       getIndividualPointer( x )->setPoolPos( x );
     }
}  

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size, 
					     SIGEL_Tools::SIG_Randomizer &r)
{
   setRandomizer( &r );

   pool.setAutoDelete( true );
   pool.resize( size );

   history = true;
   
   for( int x=0; x<getSize(); x++ )
     { 
       pool.insert( x, new SIGEL_GP::SIG_GPIndividual( r ) );
       getIndividualPointer( x )->setPoolPos( x );
     }
}  
    
SIGEL_GP::SIG_GPPopulation::~SIG_GPPopulation()
{

#ifdef SIG_DEBUG

  //   SIGEL_Tools::SIG_IO::cerr << "\nDestructor of POPULATION called!\n";

#endif

  pool.clear();
  delete randomizer;
};

    
void SIGEL_GP::SIG_GPPopulation::sort()
{


};

    
SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPPopulation::getIndividual(int poolpos)
{
  if( poolpos<getSize() )
    {
      return  *pool[poolpos];
    }
  else
    {
      // Todo: Exception!
      SIGEL_Tools::SIG_IO::cerr << "Wrong Position requested from Population!\n";
      exit( 1 );
    } 
};

    
void SIGEL_GP::SIG_GPPopulation::setIndividual(SIG_GPIndividual& indi, 
					       int poolpos)
{
  // pool.resize(getSize()+1);
  pool.insert( poolpos, &indi );
};

   

void SIGEL_GP::SIG_GPPopulation::addRandomIndividuals(int quantity, 
						      SIGEL_GP::SIG_GPParameter& param, 
						      SIGEL_Robot::SIG_LanguageParameters& languageP)
{
   int maxPos=getSize();

   pool.resize( maxPos + quantity );

   QProgressDialog *progress;
   
   if( qApp ) progress = new QProgressDialog( "Progress:", "Cancel", 0, quantity );
   if( qApp ) progress->setWindowModality( Qt::ApplicationModal );
   if( qApp ) progress->setWindowTitle( "Generating" );
   
   for( int x = maxPos; x<maxPos + quantity; x++ )
     { 
       SIG_GPIndividual *newInd = new SIG_GPIndividual( *getRandomizerPointer(), param, languageP );
       newInd->setName( getNextIdentifier() );
       pool.insert( x, newInd );
       getIndividualPointer( x )->setPoolPos( x );

       if( qApp )
	 {
           progress->setValue( x - maxPos );
           qApp->processEvents(); 

           if( progress->wasCanceled() )
             {
	       // The process has been canceled. Because of process preparations the system may crash if
	       // these preparation are not made undone:

               pool.resize( x + 1 );
  
               break;
	     }
	 }
     }

    if( qApp ) delete progress;
};
    
int SIGEL_GP::SIG_GPPopulation::getSize()
{
    return pool.size();
};

    
QString SIGEL_GP::SIG_GPPopulation::getNextIdentifier()
{
  QString releasedIdentifier = nextIdentifier;

  int nextIdentifierNumber = nextIdentifier.toInt() + 1;

  nextIdentifier = QString::number( nextIdentifierNumber );

  return releasedIdentifier;
};

void SIGEL_GP::SIG_GPPopulation::setNextIdentifier(QString identifier)
{
    nextIdentifier = identifier;
}; 

int SIGEL_GP::SIG_GPPopulation::getPoolGeneration()
{
    return poolGeneration;
};

void SIGEL_GP::SIG_GPPopulation::setPoolGeneration(int pGen)
{
    poolGeneration = pGen;
}

void SIGEL_GP::SIG_GPPopulation::loadPool(QTextStream & pool)
{
    readFromFile( pool );
};



void SIGEL_GP::SIG_GPPopulation::savePool(QTextStream & pool)
{
    writeToFile( pool );
};


SIGEL_GP::SIG_GPIndividual *SIGEL_GP::SIG_GPPopulation::getIndividualPointer(int poolpos)
{
    return pool[poolpos];
}

void SIGEL_GP::SIG_GPPopulation::deleteIndividual(int poolpos)
{
  // pool.remove(poolpos);

   SIGEL_GP::SIG_GPIndividual *tmpInd;

   for( int x=poolpos; x<getSize()-1; x++ )
     { 
       //pool.remove(poolpos);
       tmpInd = getIndividualPointer( x + 1 );
       tmpInd->setPoolPos( x );
       pool.insert( x, pool.take( x+1 ) );
     }

   pool.resize( getSize() - 1 );
}


  
void SIGEL_GP::SIG_GPPopulation::setRandomizer(SIGEL_Tools::SIG_Randomizer *r)
{
  randomizer=r;
}

  
SIGEL_Tools::SIG_Randomizer SIGEL_GP::SIG_GPPopulation::getRandomizer()
{
  return *randomizer;
}

SIGEL_Tools::SIG_Randomizer *SIGEL_GP::SIG_GPPopulation::getRandomizerPointer()
{
  return randomizer;
}

void SIGEL_GP::SIG_GPPopulation::importNewIndividual( QString& filename )
{
   int lastPos=getSize();

   pool.resize( lastPos + 1 );

   SIG_GPIndividual *newInd = new SIG_GPIndividual();

   newInd->importIndividual( filename );
   newInd->setName( getNextIdentifier() );
   newInd->setPoolPos( lastPos );

   // we don't know where this individual came from -> void fitness !
   newInd->setFitness(-1.0);

   pool.insert( lastPos, newInd );
}

void SIGEL_GP::SIG_GPPopulation::readFromFile(QTextStream &file)
{
	// first read the global options for the population
	QString s=file.readLine(); //WITHHISTORY
	if (s == "WITHHISTORY") {
		s=file.readLine();
		int tmp = s.toInt();
		if (tmp == 0) {
			history = false;
		}
		else history = true;
	}

  QString          populationStr=file.readAll();
  QString          tmpStr1, indStr;
  long             pos, pos2;
  QProgressDialog *progress;

#ifdef SIG_DEBUG

     SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION DATA FROM FILE:\n";    
     
#endif
  
  //cout<<"TEST Population : \n"<<populationStr<<"\n\n";  
	     
  if( (pos=populationStr.indexOf("POPULATIONSIZE=", 0, Qt::CaseInsensitive) )!=-1 ) 
    { 
        pos2=populationStr.indexOf(";", pos + 16, Qt::CaseInsensitive); 
        pool.resize((populationStr.mid(pos+15,pos2-pos-15)).toLong());

#ifdef SIG_DEBUG

	SIGEL_Tools::SIG_IO::cerr << "\n<Poolsize loaded:"
				  << populationStr.mid(pos+15,pos2-pos-15).toLong()
	                          << ">";

#endif

        if( qApp ) 
           progress = new QProgressDialog( "Progress:", "Cancel", 0, getSize() );
        if( qApp ) progress->setWindowModality( Qt::ApplicationModal );
        if( qApp ) progress->setWindowTitle( "Loading" );

        pos2=populationStr.indexOf(";", pos2+1, Qt::CaseInsensitive);
        if( (pos=populationStr.indexOf("NEXTIDENTIFIER=", 0, Qt::CaseInsensitive))!=-1 )
          {

#ifdef SIG_DEBUG

	     SIGEL_Tools::SIG_IO::cerr << "\n<Identifier loaded:"
				       << populationStr.mid(pos+15,pos2-pos-15).toLong()
	                               << ">";

#endif
	     setNextIdentifier(populationStr.mid(pos+15,pos2-pos-15));
	  }
        else 
	     setNextIdentifier(QString::number(0));
        

        pos2=populationStr.indexOf(";", pos2+1, Qt::CaseInsensitive);
        if( (pos=populationStr.indexOf("POOLGENERATION=", 0, Qt::CaseInsensitive))!=-1 ) 
	  {
#ifdef SIG_DEBUG

	     SIGEL_Tools::SIG_IO::cerr << "\n<Poolgeneration loaded:"
				       << populationStr.mid(pos+15,pos2-pos-15)
	                               << ">\n\n";
#endif
             setPoolGeneration((populationStr.mid(pos+15,pos2-pos-15)).toLong());
          }
        else
             setPoolGeneration(0); 

        for(long x=0;x<getSize();x++) 
          {  
             tmpStr1.setNum(x); 
	     
             pos  = populationStr.indexOf("INDIVIDUAL("+tmpStr1+") BEGIN{", pos2, Qt::CaseInsensitive); 
	     pos2 = populationStr.indexOf("}INDIVIDUAL("+tmpStr1+") END", pos2, Qt::CaseInsensitive); 
             
	     
             pool.insert(x,new SIGEL_GP::SIG_GPIndividual());  

             indStr = populationStr.mid(pos+19+tmpStr1.length(),pos2-pos-20-tmpStr1.length()); 
             
             getIndividualPointer(x)->readFromFile(indStr); 

             if( qApp )
	      {
                progress->setValue( x );
                qApp->processEvents(); 

                if ( progress->wasCanceled() )
		  {

		    // The process has been canceled. Because of process preparations the system may crash if
		    // these preparation are not made undone:

		    pool.resize( x + 1 );
                    
		    break;
		  }
	      }

          } 
#ifdef SIG_DEBUG

        SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION FINISHED.\n";    
     
#endif

        if( qApp ) delete progress;  

    } 
  else 
    { 
      
    }
 
}

void SIGEL_GP::SIG_GPPopulation::writeToFile(QTextStream &file)
{
	// save the global options for the history
	file << "WITHHISTORY\n";
 	file << history << "\n";

  file<<"\n// ---------------------------------------";
  file<<"\n// (C)2001, PG 368, UNIVERSITY OF DORTMUND";
  file<<"\n// ---------------------------------------";

  file<<"\n\nPOPULATION BEGIN{ \n"<<"\n  POPULATIONSIZE="<<getSize()<<";";
  file<<"\n  NEXTIDENTIFIER="<<nextIdentifier<<";";
  file<<"\n  POOLGENERATION="<<getPoolGeneration()<<";";

  for( long x=0; x<getSize(); x++ )
    { 
	file<<"\n\n  INDIVIDUAL("<<x<<") BEGIN{";
  
        getIndividualPointer(x)->writeToFile(file,history);

        file<<"\n  }INDIVIDUAL("<<x<<") END;";

        if( qApp )
          {
            qApp->processEvents();
          }  
    }

  file<<"\n\n}POPULATION END";
}

double SIGEL_GP::SIG_GPPopulation::getBestFitness(bool high)
{
  // take only the simulated values into account
  // the estimated fitness values are < -2
  if (getSize()>0)
    {
      double bestFitness = getIndividualPointer( 0 )->getFitness();
	  if(bestFitness < 0.0) bestFitness = 0.0;

      for (int i=1; i<getSize(); i++)
	{
	  double actFitness = getIndividualPointer( i )->getFitness();

	  if(actFitness >= 0.0){
		if (high)
			bestFitness = ( actFitness > bestFitness ) ? actFitness : bestFitness;
		else
			bestFitness = ( actFitness < bestFitness ) ? actFitness : bestFitness;
	  }
	};

      return bestFitness;
    }
  else
    return 0;
}

void SIGEL_GP::SIG_GPPopulation::resetPool()
{
  for(unsigned int counter = 0; counter < pool.size(); counter++ )
    pool[ counter ]->setFitness( -1 );
};

double SIGEL_GP::SIG_GPPopulation::getWorstFitness(bool high)
{
  // take only the simulated values into account
  // the estimated fitness values are < -2
  if (getSize()>0)
    {
      double worstFitness = getIndividualPointer( 0 )->getFitness();
	  if(worstFitness < 0.0) worstFitness = 0.0;

      for (int i=1; i<getSize(); i++)
	{
	  double actFitness = getIndividualPointer( i )->getFitness();

	  if(actFitness >= 0.0){
		if (high)
			worstFitness = ( actFitness < worstFitness ) ? actFitness : worstFitness;
		else
			worstFitness = ( actFitness > worstFitness ) ? actFitness : worstFitness;
	  }
	};

      return worstFitness;
    }
  else
    return 0;
};

double SIGEL_GP::SIG_GPPopulation::getAverageFitness()
{
  double fitnessSum = 0;
  double actFitness = 0;
  int size = 0;

  // take only the simulated values into account
  // the estimated fitness values are < -2
  for (int i=0; i<getSize(); i++){
	  actFitness = getIndividualPointer( i )->getFitness();
	  if(actFitness >= 0.0){
		fitnessSum += actFitness;
		size++;
	  }
  }

  double averageFitness = 0.0;
  if(size)
	  averageFitness = fitnessSum / size;

  return averageFitness;
};

void SIGEL_GP::SIG_GPPopulation::setHistory(bool _history) {
  history = _history;
};

bool SIGEL_GP::SIG_GPPopulation::getHistory() {
  return history;
};
