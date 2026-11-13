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
#ifndef MT_GPSYSTEM_MT_STATISTICS_H
#define MT_GPSYSTEM_MT_STATISTICS_H

#include <QList>
#include "MT_GPSystem/MT_StatisticsElement.h"
#include "MT_GPSystem/MT_ErrorCode.h"


/* manage the accrued Information of the run. It's include a QList of MT_StatisicsElement 
* and a QList with the Information of crossover events so far.*/
class MT_Statistics  
{
public:

	/*update the Statistics about the whole GP-run*/
	MT_ErrorCode updateStatistics();

	/* supply a pointer of the demand MT_StatisticsElement */
	MT_StatisticsElement * getStatisticElement(int ElementOfGeneration);
	
	/*add the given MT_StatisticsElement to the QList */
	void addStatisticElement(MT_StatisticsElement * Element);
	
	MT_Statistics(QTextStream &File);
	MT_Statistics();
	void writeToFileMT_Statistics(QTextStream &File);
	virtual ~MT_Statistics() = default;

	// all Parameter are for the Offspring
	/* This QList contain  Information about Crossover Events so far;  
	* Array[0] indicate the Number of total Crossover with one X Point; 
	* Array[1] indicate the Number of successful Crossover with one X Point; 
	* Array[2] indicate the Number of total Crossover with two X Point; 
	* Array[3] indicate the Number of successful Crossover with two X Point; 
	* Array[4] indicate the Number of total Crossover with three X Point; 
	* Array[5] indicate the Number of successful Crossover with three X Point;
	*/
	QList<unsigned int> TotalCrossoverEvent;
	unsigned int NumOfSimpleCopyParent; // not Parent - Offspring !!!
	unsigned int NumOfMutateIndividuals; 
	unsigned int NumOfMutateImprovingIndividuals; 

	/* This QList is a recording of  MT_StatisicsElement per Generation;
	* the first Element of the List belonging to the first Generation, and so on*/
	QList<MT_StatisticsElement *>  StatisticsOfGeneration;
};

#endif // MT_GPSYSTEM_MT_STATISTICS_H
