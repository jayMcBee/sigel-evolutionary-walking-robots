// MT_GPManager.h: interface for class MT_GPManager.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_GPMANAGER_H__0590D02D_760F_43CC_B902_EA5EE0196061__INCLUDED_)
#define AFX_MT_GPMANAGER_H__0590D02D_760F_43CC_B902_EA5EE0196061__INCLUDED_

#include "compat/q2compat.h"
class MT_Substitute;

#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_StatisticsElement.h"
#include "MT_GPSystem/MT_Statistics.h"
#include "MT_GPSystem/MT_Randomizer.h"
#include "MT_GPSystem/MT_Search.h"
#include "MT_GPSystem/MT_FitnessTrainer.h"
#include "MT_GPSystem/MT_TournamentManager.h"	// Added from the class view

#ifdef _WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <qstring.h>
#include <qtextstream.h>


/* GP-Manager manage the Evolution !*/
class MT_GPManager : public QObject
{
	Q_OBJECT

public:

	
	/******************** administrative method ********************/

	MT_GPManager (QTextStream & File ); // the only correct Construktor 

	MT_GPManager(); // not used 
	MT_GPManager (MT_Substitute * _Substitue); // not used
	
	virtual ~MT_GPManager();	

	void loadSetup(QTextStream &File);
	
	void writeToFilePop(QTextStream &File); 
	void writeToFileSetup(QTextStream &File);
	void writeToFileGPSystem(QTextStream &File);


	/******************** set/ get method: ********************/
	/* primary to update the GUI after Start/load or to update the GP-System after change by the user*/

	/*supply a pointer of the "Generation" S-Element or null if "Generation" out of range (<max.Generation) */
	MT_Statistics * getGPStatistics();
	/*supply a pointer of the Parentpopulation*/
	MT_Population * getParent();
	/*supply a pointer of the Randomizer*/
	MT_Randomizer * getRandomizer();

	/* update the Gp-System with the given SelektionValue
	* there are involved the class 
	* @pre: the Offspring must be empty! Flush it first.*/
	void setSelektionValue( int _OffspringSize, int _TournamentSize, int _SMethod, int _FitnessFunction, int _TrainingSetSize, int _TrainingDuration);
	
	/* for update the Selektion-Window;
	* the GP-System will change direct the value in the GUI-class 
	*/
	void getSelektionValue (int *_OffspringSize, int  * _TournamentSize, int * _SMethod, int * _FitnessFunction, int *_TrainingSetSize, int * _TrainingDuration);
	
	/*change the BrutSize*/
	void setBrutSize(int NewSize);
	int getBrutSize();

	/* ATTENTION: function always use !! after change of the population size!
	* it's set the ParentSize into the randi, the tournamentmanager and it calculate
	* correct number of tournament with a right size. 
	*/
	void setPopAndTournamentSize(int NewPopSize, int NewTournamentSize);

	void setInterpreterNumVar(int NewSize);		

	/* Returns a result != 0 if an error occurred anywhere in the meta GP system;
	* if != 0, the value indicates which error it was
	*/
	int getLastError();

	/******************** special method ********************/

	/* set EvolStopped =true, so that the GP-Evolution will stop*/
	void stopEvolution();
	/* starte the evolution loop */ 
	void startEvolution(MT_Substitute * Substitute);	

	/* returns true if the meta gp-system is able to run without sigel
	 * that's the case if there are enough training cases
	 */
	bool separateEvolutionAllowed();

	/* evolution locks */
#ifdef _WINDOWS
	HANDLE evolutionMutex;
#else
	pthread_mutex_t evolutionMutex;
#endif

signals:
	void metaEvolutionRunning(bool);

private:

	/*read out parameter about the creating of the individual and set this into the Statistics*/
	void collectParentParameter(MT_StatisticsElement * SElement);
	/*read out parameter about the creating of the individual and set this into the Statistics*/
	void collectOffspringParameter(MT_StatisticsElement * SElement);

	/* serve the updating of the  Trainingsset  */
	int checkForNewTCase();
	/* exchange the momentary BestIndividual with the individual in the
	Substituter*/
	void exchangeBest();
	/*add important information to the Statistics  */

	/* if true, so will the evolutionloop stop*/
	bool EvolStopped;
	bool sepEvolPossible;
	
	/* if an error occurs anywhere in the GP system, LastError is set != 0 
	*/
	int LastError;

	int GenerationNumber;

	/* the Parent and Offspring population*/
	MT_Population * Offspring;
	MT_Population * Parent;
	
	/* the random provider */
	MT_Randomizer * Randi;

	/* a pointer of the META Substituter; for updating the Fitnesscases
	* and the momentary  best Meta Individual
	*/
	MT_Substitute * Substituter;

	/* this individuals is equal to the individual into the Substituter !
	* after a evolutionsloop it will updating */
	MT_Individual * BestIndividual;
	
	
	/* appraise the offspring*/
	MT_FitnessTrainer * FitnessTrainer;

	/* manage and collected  information for evaluation of a SIGEL&META run*/
	MT_Statistics * Statistics;
	MT_StatisticsElement * SElement;

	/* responsibly for the movement in the search space, create a new offspring */
	MT_Search * Seeker;

	/* responsibly for the selection of the offspring*/
	MT_TournamentManager * Selector;

protected:


};

#endif // !defined(AFX_MT_GPMANAGER_H__0590D02D_760F_43CC_B902_EA5EE0196061__INCLUDED_)
