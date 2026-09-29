#ifndef MT_GPSYSTEM_MT_STATISTICS_H
#define MT_GPSYSTEM_MT_STATISTICS_H

#include <QList>
#include "MT_GPSystem/MT_StatisticsElement.h"



/* manage the accrued Information of the run. It's include a QList of MT_StatisicsElement 
* and a QList with the Information of crossover events so far.*/
class MT_Statistics  
{
public:

	/*update the Statistics about the whole GP-run*/
	int updateStatistics();

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
