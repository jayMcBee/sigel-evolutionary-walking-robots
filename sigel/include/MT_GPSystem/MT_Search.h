#ifndef MT_GPSYSTEM_MT_SEARCH_H
#define MT_GPSYSTEM_MT_SEARCH_H

#include <QTextStream>
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Randomizer.h"	


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

	/* set/get functions used to update the GUI or the GP system*/
	void setBrutSize(int  SizeOfBrut);
	int getBrutSize();
	int getLastError();

	MT_Search(MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * _Randi, QTextStream & File);
	MT_Search(MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * _Randi);
	MT_Search();
	virtual ~MT_Search() = default;

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

	// The brood size. It is loaded and saved; no search reads it.
	int BrutSize;
	int LastError;

	MT_Individual * ChildOne;
	MT_Individual *ChildTwo;
	MT_Individual * Parent;
	MT_Individual * FirstXOverParent;
};

#endif // MT_GPSYSTEM_MT_SEARCH_H
