// MT_StatisticsElement.h: Schnittstelle für die Klasse MT_StatisticsElement.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_)
#define AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_



#include <qstring.h>
#include <qtextstream.h>
#include <qarray.h> 


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

	
 	int NumOfCrossoverEvent; // Eltern durch Crossover  

	int NumOfElementMutationParent; 
	int NumOfMutateIndividuals;   // Eltern druch Mutation 
	
	int NumOfSimpleCopyParent;  // Elern durch einfache Kopie


	/**************** only this parameter of the offspring population ****************/
	
		/* This QArray contain  Information about Crossover Event in this Generation/ Parent;
	* Array[0] indicate the Number of total Crossover with one X Point; 
	* Array[1] indicate the Number of successful Crossover with one X Point; 
	* Array[2] indicate the Number of total Crossover with two X Point; 
	* Array[3] indicate the Number of successful Crossover with two X Point; 
	* Array[4] indicate the Number of total Crossover with three X Point; 
	* Array[5] indicate the Number of successful Crossover with three X Point;
	*/
	QArray<int> CrossoverEventParent;  

	int NumOfTotalElementMutation;
	int NumOfMutateOffspring;
	int NumOfMutateImprovingIndividuals; 
		
	int NumOfSimpleCopyOffspring;


};

#endif // !defined(AFX_MT_STATISTICSELEMENT_H__F9DD215C_9C37_4606_B017_0FD26DB94D92__INCLUDED_)

/*


NumOfSimpleCopyParent := Anzahl der Nachkommen die aus Elternpop in die nächste Generation durch einfaches kopieren übernohmmen wurden
 ...Parameter nur für diese Generation 
NumOfCrossoverEvent := Anzahl der XOver Ereignisse

NumOfMutateIndividuals :=  siehe MT_Statistics.h ...Parameter nur für diese Generation    
NumOfElementMutationParent := Anzahl der elementaren Punktmutationen


  NumOfSimpleCopyParent := NumOfMutateIndividuals := Anzahl der Nachkommen ... durch Mutation ...
NumOfMutateImprovingIndividuals :=  Anzahl der mutierten Nachkommen, die besser sind als der Elter es war!



  
	
	  
Parameter über den Offspring!

CrossoverEventParent  := beschreibt den Erfolg der verschiedenen XOver Arten  ...Parameter nur für diese Generation 
NumOfSimpleCopyOffspring :=  Anzahl der einfachen Kopier-Ereignisse

NumOfTotalElementMutation: = Anzahl der elementatren Mutationen/ Punktmutationen 
NumOfMutateOffspring := Anzahl der mutierten Individuen
-> richtet sich nach der Suchoperatorwahrscheinlichkeit "Mutation" 
NumOfMutateImprovingIndividuals := verbesserte mutierte Indis...Parameter nur für diese Generation 




*/