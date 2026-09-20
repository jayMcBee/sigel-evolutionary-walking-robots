// MT_StatisticsElement.h: interface for class MT_StatisticsElement.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_)
#define AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_



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
	virtual ~MT_StatisticsElement();
	void writeToFileElement(QTextStream &File);
	int Generation; 

	/**************** all parameter of the parent population ****************/	
	double Varianz; 
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

#endif // !defined(AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_)

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