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
#ifndef MT_GPSYSTEM_MT_STATISTICSELEMENT_H
#define MT_GPSYSTEM_MT_STATISTICSELEMENT_H

#include <QTextStream>
#include <QList>
#include <qstring.h>
#include <qtextstream.h>

/* This class contain all Information, that could be arise from the GP-System 
* in a generation and which is needed for evaluation a Meta experiment */


class MT_StatisticsElement  
{
public:
	
	MT_StatisticsElement(QTextStream &File);
	MT_StatisticsElement();
	virtual ~MT_StatisticsElement() = default;
	void writeToFileElement(QTextStream &File);
	int Generation; 

	/**************** all parameter of the parent population ****************/	
	double Variance; 
	double AverageFitness;
	double MaxFitness; 

 	int NumOfCrossoverEvent; // Parents by crossover  

	int NumOfElementMutationParent; 
	int NumOfMutateIndividuals;   // Parents by mutation 
	
	int NumOfSimpleCopyParent;  // Parents by simple copy

	/**************** only this parameter of the offspring population ****************/
	
		/* This QList contain  Information about Crossover Event in this Generation/ Parent;
	* Array[0] indicate the Number of total Crossover with one X Point; 
	* Array[1] indicate the Number of successful Crossover with one X Point; 
	* Array[2] indicate the Number of total Crossover with two X Point; 
	* Array[3] indicate the Number of successful Crossover with two X Point; 
	* Array[4] indicate the Number of total Crossover with three X Point; 
	* Array[5] indicate the Number of successful Crossover with three X Point;
	*/
	QList<int> CrossoverEventParent;  

	int NumOfTotalElementMutation;
	int NumOfMutateOffspring;
	int NumOfMutateImprovingIndividuals; 
		
	int NumOfSimpleCopyOffspring;
};

#endif // MT_GPSYSTEM_MT_STATISTICSELEMENT_H

/*


NumOfSimpleCopyParent := number of offspring carried from the parent population into the next generation by simple copying
 ...parameter for this generation only 
NumOfCrossoverEvent := number of crossover events

NumOfMutateIndividuals :=  see MT_Statistics.h ...parameter for this generation only    
NumOfElementMutationParent := number of elementary point mutations


  NumOfSimpleCopyParent := NumOfMutateIndividuals := number of offspring ... by mutation ...
NumOfMutateImprovingIndividuals := number of mutated offspring that are better than the parent was



  
	
	  
Parameters concerning the offspring.

CrossoverEventParent  := describes the success of the different crossover kinds  ...parameter for this generation only 
NumOfSimpleCopyOffspring := number of simple copy events

NumOfTotalElementMutation := number of elementary mutations / point mutations 
NumOfMutateOffspring := number of mutated individuals
-> governed by the search-operator probability "Mutation" 
NumOfMutateImprovingIndividuals := improved mutated individuals...parameter for this generation only 




*/