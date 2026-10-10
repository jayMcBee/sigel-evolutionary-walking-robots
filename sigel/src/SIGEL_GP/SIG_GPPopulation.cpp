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
#include <algorithm>
#include <memory>
#include "SIGEL_Tools/SIG_DialogParent.h"
#include "SIGEL_GP/SIG_GPPopulation.h"

#include<iostream>

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation()
	: pool(),
	  randomizer( new SIGEL_Tools::SIG_Randomizer() ),
	  nextIdentifier( QString::number( 0 ) ),
	  poolGeneration(0)
{
	history = true;
};

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size)
	: randomizer( nullptr )   // the guard below reads it
{
	if( getRandomizerPointer()==nullptr )
	{
		SIGEL_Tools::SIG_IO::cerr << "\n\nA randomizer is needed to initialize a population ! Program terminated." << Qt::endl;
		exit(1);
		// ToDo: Exception
	}

	pool.resize( size );

	history = true;

	setNextIdentifier( QString::number(0) );
	setPoolGeneration( 0 );

	for( int x=0; x<getSize(); x++ )
	{
		delete pool[ x ];
		pool[ x ] = new SIGEL_GP::SIG_GPIndividual( *getRandomizerPointer() );
		getIndividualPointer( x )->setPoolPos( x );
	}
};

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(QString data)
	: randomizer( nullptr )   // the guard below reads it
{
	QTextStream                 inputFile(&data, QIODeviceBase::ReadOnly);

	history = true;

#ifdef SIG_DEBUG

	SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION DATA FROM FILE:" << Qt::endl;

#endif

	readFromFile(inputFile);

#ifdef SIG_DEBUG

	SIGEL_Tools::SIG_IO::cerr << "\n\nREADING FINISHED.\n" << Qt::endl;

#endif
};

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size,
                                             SIGEL_Tools::SIG_Randomizer &newRandomizer,
                                             SIGEL_GP::SIG_GPParameter& gpParameter,
                                             SIGEL_Robot::SIG_LanguageParameters& languageParameters)
{
	setRandomizer( &newRandomizer );

	pool.resize( size );

	history = true;

	setNextIdentifier( QString::number(0) );
	setPoolGeneration( 0 );

	for( int x=0; x<getSize(); x++ )
	{
		delete pool[ x ];
		pool[ x ] = new SIGEL_GP::SIG_GPIndividual( newRandomizer, gpParameter, languageParameters );
		getIndividualPointer( x )->setPoolPos( x );
	}
}

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation(int size,
                                             SIGEL_Tools::SIG_Randomizer &newRandomizer)
{
	setRandomizer( &newRandomizer );

	pool.resize( size );

	history = true;

	for( int x=0; x<getSize(); x++ )
	{
		delete pool[ x ];
		pool[ x ] = new SIGEL_GP::SIG_GPIndividual( newRandomizer );
		getIndividualPointer( x )->setPoolPos( x );
	}
}

namespace
{

	void resizeOwning( QList< SIGEL_GP::SIG_GPIndividual * > &v, qsizetype want )
	{
		if (want < 0)
			want = 0;
		for (qsizetype i = want; i < v.size(); i++)
			delete v[ i ];
		v.resize( want );
	}

}

SIGEL_GP::SIG_GPPopulation::~SIG_GPPopulation()
{
	qDeleteAll( pool );
	pool.clear();
	delete randomizer;
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
		SIGEL_Tools::SIG_IO::cerr << "Wrong Position requested from Population!" << Qt::endl;
		exit( 1 );
	}
};

void SIGEL_GP::SIG_GPPopulation::setIndividual(SIG_GPIndividual& indi,
                                               int poolpos)
{
	// The pool owns its individuals: it frees the one in this slot and
	// takes over the one the caller allocated.
	delete pool[ poolpos ];
	pool[ poolpos ] = &indi;
};

int SIGEL_GP::SIG_GPPopulation::addRandomIndividuals(int quantity,
                                                      SIGEL_GP::SIG_GPParameter& gpParameter,
                                                      SIGEL_Robot::SIG_LanguageParameters& languageParameters)
{
	int maxPos=getSize();

	quantity = qMin( quantity, maximumSize - maxPos );
	if( quantity <= 0 )
		return 0;

	pool.resize( maxPos + quantity );

	std::unique_ptr< QProgressDialog > progress;

	if( qApp )
	{
		progress = std::make_unique< QProgressDialog >( "Progress:", "Cancel", 0, quantity, SIGEL_Tools::dialogParent() );
		progress->setWindowModality( Qt::ApplicationModal );
		progress->setWindowTitle( "Generating" );
	}

	for( int x = maxPos; x<maxPos + quantity; x++ )
	{
		SIG_GPIndividual *newInd = new SIG_GPIndividual( *getRandomizerPointer(), gpParameter, languageParameters );
		newInd->setName( getNextIdentifier() );
		delete pool[ x ];
		pool[ x ] = newInd;
		getIndividualPointer( x )->setPoolPos( x );

		if( qApp )
		{
			progress->setValue( x - maxPos );
			qApp->processEvents();

			if( progress->wasCanceled() )
			{
				// The process has been canceled. Because of process preparations the system may crash if
				// these preparation are not made undone:

				// The slots above x were never filled.
				resizeOwning( pool, x + 1 );

				break;
			}
		}
	}

	return quantity;
};

int SIGEL_GP::SIG_GPPopulation::getSize()
{
	return int( pool.size() );
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
	SIGEL_GP::SIG_GPIndividual *tmpInd;

	delete pool[ poolpos ];

	for( int x=poolpos; x<getSize()-1; x++ )
	{
		tmpInd = getIndividualPointer( x + 1 );
		tmpInd->setPoolPos( x );
		pool[ x ] = pool[ x+1 ];
	}

	pool.removeLast();
}

void SIGEL_GP::SIG_GPPopulation::setRandomizer(SIGEL_Tools::SIG_Randomizer *newRandomizer)
{
	randomizer=newRandomizer;
}

SIGEL_Tools::SIG_Randomizer SIGEL_GP::SIG_GPPopulation::getRandomizer()
{
	return *randomizer;
}

SIGEL_Tools::SIG_Randomizer *SIGEL_GP::SIG_GPPopulation::getRandomizerPointer()
{
	return randomizer;
}

bool SIGEL_GP::SIG_GPPopulation::importNewIndividual( QString& filename )
{
	int lastPos=getSize();

	if( lastPos >= maximumSize )
		return false;

	pool.resize( lastPos + 1 );

	SIG_GPIndividual *newInd = new SIG_GPIndividual();

	newInd->importIndividual( filename );
	newInd->setName( getNextIdentifier() );
	newInd->setPoolPos( lastPos );

	// we don't know where this individual came from -> void fitness !
	newInd->setFitness(-1.0);

	delete pool[ lastPos ];
	pool[ lastPos ] = newInd;
	return true;
}

void SIGEL_GP::SIG_GPPopulation::readFromFile(QTextStream &file)
{
	// first read the global options for the population
	QString s=file.readLine(); //WITHHISTORY
	if (s == "WITHHISTORY")
	{
		s=file.readLine();
		int tmp = s.toInt();
		if (tmp == 0)
		{
			history = false;
		}
		else
			history = true;
	}

	QString          populationStr=file.readAll();
	QString          tmpStr1, indStr;
	long             pos, pos2;

#ifdef SIG_DEBUG

	SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION DATA FROM FILE:" << Qt::endl;

#endif

	if( (pos=populationStr.indexOf("POPULATIONSIZE=", 0, Qt::CaseInsensitive) )!=-1 )
	{
		pos2=populationStr.indexOf(";", pos + 16, Qt::CaseInsensitive);
		resizeOwning( pool, (populationStr.mid(pos+15,pos2-pos-15)).toLong() );

#ifdef SIG_DEBUG

		SIGEL_Tools::SIG_IO::cerr << "<Poolsize loaded:"
		                          << populationStr.mid(pos+15,pos2-pos-15).toLong()
		                          << ">" << Qt::endl;

#endif

		std::unique_ptr< QProgressDialog > progress;
		if( qApp )
		{
			progress = std::make_unique< QProgressDialog >( "Progress:", "Cancel", 0, getSize(), SIGEL_Tools::dialogParent() );
			progress->setWindowModality( Qt::ApplicationModal );
			progress->setWindowTitle( "Loading" );
		}

		pos2=populationStr.indexOf(";", pos2+1, Qt::CaseInsensitive);
		if( (pos=populationStr.indexOf("NEXTIDENTIFIER=", 0, Qt::CaseInsensitive))!=-1 )
		{
#ifdef SIG_DEBUG

			SIGEL_Tools::SIG_IO::cerr << "<Identifier loaded:"
			                          << populationStr.mid(pos+15,pos2-pos-15).toLong()
			                          << ">" << Qt::endl;

#endif
			setNextIdentifier(populationStr.mid(pos+15,pos2-pos-15));
		}
		else
			setNextIdentifier(QString::number(0));

		pos2=populationStr.indexOf(";", pos2+1, Qt::CaseInsensitive);
		if( (pos=populationStr.indexOf("POOLGENERATION=", 0, Qt::CaseInsensitive))!=-1 )
		{
#ifdef SIG_DEBUG

			SIGEL_Tools::SIG_IO::cerr << "<Poolgeneration loaded:"
			                          << populationStr.mid(pos+15,pos2-pos-15)
			                          << ">\n" << Qt::endl;
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

			delete pool[ x ];
			pool[ x ] = new SIGEL_GP::SIG_GPIndividual();

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

					// Slots above x still hold the individuals from before this
					// load -- the replace loop only reached x -- so this shrink
					// frees them.
					resizeOwning( pool, x + 1 );

					break;
				}
			}
		}
#ifdef SIG_DEBUG

		SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION FINISHED." << Qt::endl;

#endif
	}

	if( getSize() > maximumSize )
	{
		QString message = QString( "Warning: the pool has %1 individuals; only the first %2 take part in tournaments reliably." ).arg( getSize() ).arg( maximumSize );
		SIGEL_Tools::SIG_IO::cerr << message << Qt::endl;
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

double SIGEL_GP::SIG_GPPopulation::getBestFitness()
{
	QList<double> simulated = getFitnessValuesOfSimulatedIndividuals();
	if( simulated.isEmpty() )
		return 0;

	return *std::max_element( simulated.begin(), simulated.end() );
}

void SIGEL_GP::SIG_GPPopulation::resetAllFitnessValues()
{
	for(unsigned int counter = 0; counter < pool.size(); counter++ )
		pool[ counter ]->setFitness( -1 );
};

double SIGEL_GP::SIG_GPPopulation::getWorstFitness()
{
	QList<double> simulated = getFitnessValuesOfSimulatedIndividuals();
	if( simulated.isEmpty() )
		return 0;

	return *std::min_element( simulated.begin(), simulated.end() );
}

double SIGEL_GP::SIG_GPPopulation::getAverageFitness()
{
	QList<double> simulated = getFitnessValuesOfSimulatedIndividuals();
	if( simulated.isEmpty() )
		return 0;

	double fitnessSum = 0;
	for( double fitness : simulated )
		fitnessSum += fitness;

	return fitnessSum / simulated.size();
}

QList<double> SIGEL_GP::SIG_GPPopulation::getFitnessValuesOfSimulatedIndividuals() const
{
	QList<double> simulated;

	for( const SIG_GPIndividual *individual : pool )
		if( individual->getFitness() >= 0.0 )
			simulated.append( individual->getFitness() );

	return simulated;
}

void SIGEL_GP::SIG_GPPopulation::setHistory(bool newHistory)
{
	history = newHistory;
};

bool SIGEL_GP::SIG_GPPopulation::getHistory()
{
	return history;
};
