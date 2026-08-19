// MT_Tournament.h: interface for class MT_Tournament.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_TOURNAMENT_H__0A623C55_DEBF_45AE_8F39_4F5AC54F0E85__INCLUDED_)
#define AFX_MT_TOURNAMENT_H__0A623C55_DEBF_45AE_8F39_4F5AC54F0E85__INCLUDED_



#include "MT_GPSystem/MT_Population.h"


/* This class represent a single GP tournament;
* it inherited from theMT_Population
* it determine the tournament winner (s)
*/
class MT_Tournament : public MT_Population  
{
public:
	/* realization of a fitness proportional selection
	* @pre: the tournament member must be inside the tournament
	* @pre: the fitness of the tournament member must be estimated beforehand
	* @post: in the WinnerLoser array indicat a one a Winner and a zero a Loser 
	*/
	void fitnessProp(MT_Randomizer* Randi, Q2Array<int> * WinnerLoser);
	
	/* realization of a ranking selection
	* @pre: the tournament member must be inside the tournament
	* @pre: the fitness of the tournament member must be estimated beforehand.   
	* @post: in the WinnerLoser array indicat a one a Winner and a zero a Loser 
	*/
	void ranking(Q2Array<int> * WinnerLoser);

	void setTypOfIndividual(int Typ);

	/* create a empty tournament of Size "Size" and a "Winner Array" */
	MT_Tournament(int Size, int num );
	MT_Tournament();
	virtual ~MT_Tournament();

private:

	/* the number of winner in this tournament
	* @pre: NumberOfWinner<= tournament.Size
	*/
	int NumberOfWinner;
		
	int TypOfIndividual;

	/*  denote which players have gain the present tournaments and which are the loser*/
	Q2Array<int> * WinnerLoser;
};

#endif // !defined(AFX_MT_TOURNAMENT_H__0A623C55_DEBF_45AE_8F39_4F5AC54F0E85__INCLUDED_)
