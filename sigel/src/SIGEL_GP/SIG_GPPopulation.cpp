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
#include <QProgressDialog>
#include <algorithm>
#include <memory>
#include "SIGEL_CommonGUI/SIG_DialogParent.h"
#include "SIGEL_GP/SIG_GPPopulation.h"

#include<iostream>

#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_Exception.h"

SIGEL_GP::SIG_GPPopulation::SIG_GPPopulation()
	: history( true ),
	  nextIdentifier( QString::number( 0 ) ),
	  poolGeneration(0)
{
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
}

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
}

void SIGEL_GP::SIG_GPPopulation::setIndividual(SIG_GPIndividual& indi,
                                               int poolpos)
{
	// The pool owns its individuals: it frees the one in this slot and
	// takes over the one the caller allocated.
	delete pool[ poolpos ];
	pool[ poolpos ] = &indi;
}

int SIGEL_GP::SIG_GPPopulation::addRandomIndividuals(int quantity,
                                                      SIGEL_GP::SIG_GPParameter& gpParameter,
                                                      SIGEL_Robot::SIG_LanguageParameters& languageParameters)
{
	int maxPos=getSize();

	quantity = qMin( quantity, maximumSize - maxPos );
	if( quantity <= 0 )
		return 0;

	pool.resize( maxPos + quantity );

	std::unique_ptr< QProgressDialog > progress = createProgressDialog( "Generating", quantity );

	for( int x = maxPos; x<maxPos + quantity; x++ )
	{
		SIG_GPIndividual *newInd = new SIG_GPIndividual( randomizer, gpParameter, languageParameters );
		newInd->setName( getNextIdentifier() );
		delete pool[ x ];
		pool[ x ] = newInd;
		getIndividualPointer( x )->setPoolPos( x );

		if( progressCanceled( progress.get(), x - maxPos ) )
		{
			// The slots above x were never filled.
			resizeOwning( pool, x + 1 );

			break;
		}
	}

	return quantity;
}

int SIGEL_GP::SIG_GPPopulation::getSize()
{
	return int( pool.size() );
}

QString SIGEL_GP::SIG_GPPopulation::getNextIdentifier()
{
	QString releasedIdentifier = nextIdentifier;

	int nextIdentifierNumber = nextIdentifier.toInt() + 1;

	nextIdentifier = QString::number( nextIdentifierNumber );

	return releasedIdentifier;
}

void SIGEL_GP::SIG_GPPopulation::setNextIdentifier(QString identifier)
{
	nextIdentifier = identifier;
}

int SIGEL_GP::SIG_GPPopulation::getPoolGeneration()
{
	return poolGeneration;
}

void SIGEL_GP::SIG_GPPopulation::setPoolGeneration(int pGen)
{
	poolGeneration = pGen;
}

void SIGEL_GP::SIG_GPPopulation::incrementPoolGeneration()
{
	poolGeneration++;
}

void SIGEL_GP::SIG_GPPopulation::loadPool(QTextStream & pool)
{
	readFromFile( pool );
}

void SIGEL_GP::SIG_GPPopulation::savePool(QTextStream & pool)
{
	writeToFile( pool );
}

SIGEL_GP::SIG_GPIndividual *SIGEL_GP::SIG_GPPopulation::getIndividualPointer(int poolpos)
{
	return pool[poolpos];
}

void SIGEL_GP::SIG_GPPopulation::deleteIndividual(int poolpos)
{
	delete pool[ poolpos ];
	pool.removeAt( poolpos );

	// Each individual keeps its own pool position; the ones that moved up need their new index.
	for( int x=poolpos; x<getSize(); x++ )
		pool[ x ]->setPoolPos( x );
}

SIGEL_Tools::SIG_Randomizer &SIGEL_GP::SIG_GPPopulation::getRandomizer()
{
	return randomizer;
}

bool SIGEL_GP::SIG_GPPopulation::importNewIndividual( QString& filename )
{
	int lastPos=getSize();

	if( lastPos >= maximumSize )
		return false;

	SIG_GPIndividual *newInd = new SIG_GPIndividual();

	try
	{
		newInd->importIndividual( filename );
	}
	catch( ... )
	{
		delete newInd;
		throw;
	}

	newInd->setName( getNextIdentifier() );
	newInd->setPoolPos( lastPos );

	// we don't know where this individual came from -> void fitness !
	newInd->setFitness(-1.0);

	pool.append( newInd );
	return true;
}

void SIGEL_GP::SIG_GPPopulation::readFromFile(QTextStream &file)
{
	// The text is read into these and the population takes them only at the end, so a refused text leaves it as it was.
	bool readHistory = history;
	QString readNextIdentifier = nextIdentifier;
	int readPoolGeneration = poolGeneration;
	QList<SIGEL_GP::SIG_GPIndividual *> oldPool;

	// first read the global options for the population
	QString s=file.readLine();
	if( s == "WITHHISTORY" )
		readHistory = file.readLine().toInt() != 0;

	QString          populationStr=file.readAll();
	long             pos, endPosition;

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "\n\nREADING POPULATION DATA FROM FILE:" << Qt::endl;
#endif

	if( populationStr.indexOf("POPULATIONSIZE=", 0, Qt::CaseInsensitive)==-1 )
		throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The population has no POPULATIONSIZE field." );

	if( populationStr.indexOf("NEXTIDENTIFIER=", 0, Qt::CaseInsensitive)==-1 )
		throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The population has no NEXTIDENTIFIER field." );

	if( populationStr.indexOf("POOLGENERATION=", 0, Qt::CaseInsensitive)==-1 )
		throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The population has no POOLGENERATION field." );

	pos=populationStr.indexOf("POPULATIONSIZE=", 0, Qt::CaseInsensitive);
	endPosition=populationStr.indexOf(";", pos + 16, Qt::CaseInsensitive);
	oldPool.swap( pool );
	resizeOwning( pool, (populationStr.mid(pos+15,endPosition-pos-15)).toLong() );

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "<Poolsize loaded:" << populationStr.mid(pos+15,endPosition-pos-15).toLong() << ">" << Qt::endl;
#endif

	std::unique_ptr< QProgressDialog > progress = createProgressDialog( "Loading", getSize() );

	endPosition=populationStr.indexOf(";", endPosition+1, Qt::CaseInsensitive);
	pos=populationStr.indexOf("NEXTIDENTIFIER=", 0, Qt::CaseInsensitive);

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "<Identifier loaded:" << populationStr.mid(pos+15,endPosition-pos-15).toLong() << ">" << Qt::endl;
#endif

	readNextIdentifier = populationStr.mid(pos+15,endPosition-pos-15);

	endPosition=populationStr.indexOf(";", endPosition+1, Qt::CaseInsensitive);
	pos=populationStr.indexOf("POOLGENERATION=", 0, Qt::CaseInsensitive);

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "<Poolgeneration loaded:" << populationStr.mid(pos+15,endPosition-pos-15) << ">\n" << Qt::endl;
#endif

	readPoolGeneration = (populationStr.mid(pos+15,endPosition-pos-15)).toLong();

	for(long index=0;index<getSize();index++)
	{
		try
		{
			pool[ index ] = readIndividual( populationStr, index, endPosition );
		}
		catch( ... )
		{
			qDeleteAll( pool );
			pool = oldPool;
			throw;
		}

		if( progressCanceled( progress.get(), index ) )
		{
			// Cancel keeps the individuals read so far; the slots above index were never filled.
			resizeOwning( pool, index + 1 );

			break;
		}
	}

	qDeleteAll( oldPool );
	history = readHistory;
	nextIdentifier = readNextIdentifier;
	poolGeneration = readPoolGeneration;

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "\n\nFINISHED READING POPULATION." << Qt::endl;
#endif

	if( getSize() > maximumSize )
	{
		QString message = QString( "Warning: the pool has %1 individuals; only the first %2 take part in tournaments reliably." ).arg( getSize() ).arg( maximumSize );
		SIGEL_Tools::SIG_IO::cerr << message << Qt::endl;
	}
}

SIGEL_GP::SIG_GPIndividual *SIGEL_GP::SIG_GPPopulation::readIndividual( const QString &populationText, long index, long &endPosition )
{
	QString indexText = QString::number( index );
	QString beginMarker = "INDIVIDUAL(" + indexText + ") BEGIN{";

	long beginPosition = populationText.indexOf( beginMarker, endPosition, Qt::CaseInsensitive ) + beginMarker.length();
	endPosition = populationText.indexOf( "}INDIVIDUAL(" + indexText + ") END", endPosition, Qt::CaseInsensitive );

	return new SIGEL_GP::SIG_GPIndividual( populationText.mid( beginPosition, endPosition - beginPosition ) );
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
}

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
}

bool SIGEL_GP::SIG_GPPopulation::getHistory()
{
	return history;
}

std::unique_ptr< QProgressDialog > SIGEL_GP::SIG_GPPopulation::createProgressDialog( const QString &title, long maximum )
{
	if( !qApp )
		return nullptr;

	std::unique_ptr< QProgressDialog > progress = std::make_unique< QProgressDialog >( "Progress:", "Cancel", 0, maximum, SIGEL_CommonGUI::dialogParent() );
	progress->setWindowModality( Qt::ApplicationModal );
	progress->setWindowTitle( title );
	return progress;
}

bool SIGEL_GP::SIG_GPPopulation::progressCanceled( QProgressDialog *progress, long value )
{
	if( !progress )
		return false;

	progress->setValue( value );
	qApp->processEvents();
	return progress->wasCanceled();
}
