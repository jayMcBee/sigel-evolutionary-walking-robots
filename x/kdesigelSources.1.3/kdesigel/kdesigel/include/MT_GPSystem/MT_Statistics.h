// MT_Statistics.h: interface for class MT_Statistics.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_STATISTICS_H__010CEA5A_30F4_406B_93E1_DD5D57975FE5__INCLUDED_)
#define AFX_MT_STATISTICS_H__010CEA5A_30F4_406B_93E1_DD5D57975FE5__INCLUDED_

#include "MT_GPSystem/MT_StatisticsElement.h"
#include <qlist.h>



/* manage the accrued Information of the run. It's include a QList of MT_StatisicsElement 
* and a QArray with the Information of crossover events so far.*/
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
	virtual ~MT_Statistics();

	// all Parameter are for the Offspring
	/* This QArray contain  Information about Crossover Events so far;  
	* Array[0] indicate the Number of total Crossover with one X Point; 
	* Array[1] indicate the Number of successful Crossover with one X Point; 
	* Array[2] indicate the Number of total Crossover with two X Point; 
	* Array[3] indicate the Number of successful Crossover with two X Point; 
	* Array[4] indicate the Number of total Crossover with three X Point; 
	* Array[5] indicate the Number of successful Crossover with three X Point;
	*/
	QArray<unsigned int> TotalCrossoverEvent;
	unsigned int NumOfSimpleCopyParent; // not Parent - Offspring !!!
	unsigned int NumOfMutateIndividuals; 
	unsigned int NumOfMutateImprovingIndividuals; 


	/* This QList is a recording of  MT_StatisicsElement per Generation;
	* the first Element of the List belonging to the first Generation, and so on*/
	QList<MT_StatisticsElement>  StatisticsOfGeneration;


};

#endif // !defined(AFX_MT_STATISTICS_H__010CEA5A_30F4_406B_93E1_DD5D57975FE5__INCLUDED_)
