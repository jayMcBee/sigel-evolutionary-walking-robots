// MT_Search.h: Schnittstelle für die Klasse MT_Search.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_SEARCH_H__B5BB118D_7ED8_4086_8D30_BFC22C562A0F__INCLUDED_)
#define AFX_MT_SEARCH_H__B5BB118D_7ED8_4086_8D30_BFC22C562A0F__INCLUDED_


#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Randomizer.h"	

#include <qqueue.h> 
#include <qarray.h> 

/* This class manage the GP variation;
* another search points in the search space were determinate.
* the offspring will new create from the parent.
*/
class MT_Search  
{
public:

	
	/*start the search process, it's determinate or 
	* create new individuals with regard to the search parameters.
	* @pre: there are a corrcet parent, offspring population
	* @pre: there are a correct randinomizer, which have the necessary 
	* search parameters
	* @param: return value indicate an error; 0 := it was a correct search
	*/ 
	int startMatingProcess();

	/* set/get function form up date the GUI or GP system*/
	void setBrutSize(int  SizeOfBrut);
	int getBrutSize();
	int getLastError();

	MT_Search(MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * _Randi, QTextStream & File);
	MT_Search(MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * _Randi);
	MT_Search();
	virtual ~MT_Search();
	/* für Brut Suche !
	*	MT_Search(MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * Randi, MT_FitnessTranier * Fit Trainer);
	*/

private:
	
	/* generate a exact copy of the individuals*/
	MT_Individual * reproduce(MT_Individual * Progenitor);
	/* generate two new individuals form two parents */
	void crossover(MT_Individual *ParentOne, MT_Individual *ParentTwo);
	/* generate a mutated copy from a parent*/
	MT_Individual * mutate(MT_Individual * Progenitor);

	/*pointer of the offspring population*/
	MT_Population * TargetPop;
	/* pointer of the parent*/
	MT_Population * SourcePop;
	MT_Randomizer * Randi;

	int BrutSize;
	int LastError;

	MT_Individual * ChildOne;
	MT_Individual *ChildTwo;
	MT_Individual * Parent;
	MT_Individual * FirstXOverParent;


};

#endif // !defined(AFX_MT_SEARCH_H__B5BB118D_7ED8_4086_8D30_BFC22C562A0F__INCLUDED_)
