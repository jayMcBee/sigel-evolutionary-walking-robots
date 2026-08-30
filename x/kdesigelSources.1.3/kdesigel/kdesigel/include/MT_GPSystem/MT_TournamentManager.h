// MT_TournamentManager.h: interface for class MT_TournamentManager.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_TOURNAMENTMANAGER_H__8202A42C_476B_4AD2_8E67_442E97B3C8BF__INCLUDED_)
#define AFX_MT_TOURNAMENTMANAGER_H__8202A42C_476B_4AD2_8E67_442E97B3C8BF__INCLUDED_


#include <QList>
#include <QTextStream>
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Tournament.h"




/* This class manage the GP tournament;
* it transfer the winner to the parent population!
*/
class MT_TournamentManager  
{
public:

	/* the individuals of the offspring will separate random into tournaments
	* tournaments will performed 
	*/
	void performTournaments();

	/* get/set method to update the GUI and/ or the GP-system ! */
	void setSelectionMethod(int Method);
	void setNameForParent(int Na);
	int getSelectionMethod ();
	int getTournamentSize();
	int getLastError();
	
	/* change the TournamentSize; 
	* ATTENTION: function always use !! after change of the population size.
	* function change as well the TournamentNumber and the WinnerNumber;
	* it's as well use the function createTournaments() to create adequate tournaments !
	* it more than a simpel set function !!!
	*/
	void setTournamentSize(int SizeOfT);

	/* Typ 1 = means there are Classifier Meta Individuals;
	*  Typ 0 = means there are Evaluator Meta Individuals;
	*  the typ is needed for fitnessproportiona Selection  */
	void setTypOfIndividual (int Typ);

	void writeToFileSetup(QTextStream & File);
	
	MT_TournamentManager(MT_Population * PPop, MT_Population * OPop, MT_Randomizer* _Randi, QTextStream &File);
	MT_TournamentManager(MT_Population * PPop, MT_Population * OPop, MT_Randomizer* _Randi, int Method, int TSize);
	MT_TournamentManager();
	virtual ~MT_TournamentManager();

private:
	

	/* ATTENTION: relation between TournamentSize, TournamentNumber, WinnerNumber, ParentSize and OffspringSize is:
	* @pre: TournamentNumber *WinnerNumber = ParentSize !
	* @pre: TournamentNumber *TournamentSize = OffspringSize !
	*/ 
	MT_Population * OffspringPop;
	MT_Population * ParentPop;
	int WinnerNumber;
	int TournamentNumber;
	int TournamentSize;

	int NameForNewParent;

	int LastError;

	/* denote which players have gain the present tournaments and which are the loser
	* 0 indicate a Loser
	*/
	QList<int>  WinnerLoser;

	/* contain pointer of the single tournaments */
	QList<MT_Tournament*> Tournaments;
	
	/* indicate the selection method 
	* 1 = Ranking Selection
	* 2= fitness proportional Selection
	*/
	int SMethod;
	MT_Randomizer * Randi;
	
	/* create the adequate Tournaments*/
	void createTournaments();

};

#endif // !defined(AFX_MT_TOURNAMENTMANAGER_H__8202A42C_476B_4AD2_8E67_442E97B3C8BF__INCLUDED_)
