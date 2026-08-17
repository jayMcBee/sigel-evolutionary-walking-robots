// MT_Classifier.h: Schnittstelle für die Klasse MT_Classifier.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_CLASSIFIER_H__22632809_B47E_418D_8C86_EDE8B998506D__INCLUDED_)
#define AFX_MT_CLASSIFIER_H__22632809_B47E_418D_8C86_EDE8B998506D__INCLUDED_

#include "MT_Substitute.h"
#include <qvector.h>

#include "SIGEL_GP/SIG_GPTournament.h"
#include "SIGEL_GP/SIG_GPCrossOverTournament.h"
#include "SIGEL_GP/SIG_GPMutationTournament.h"
#include "SIGEL_GP/SIG_GPSimpleTournament.h"

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"
#include "MT_GPSystem/MT_TranslatedIndividual.h"	// Hinzugefügt von der Klassenansicht



class MT_Classifier : public MT_Substitute  
{
public:

	
	// administrative method 
	MT_Classifier(QTextStream &File);
	virtual ~MT_Classifier();
	void writeToFileSetup(QTextStream &File);
	void writeToFile(QTextStream &File);
	void loadSetup(QTextStream &File);

	/* execute the first Tournaments by the MT_Classifier 
	*/
	bool preEvolution(QVector<SIGEL_GP::SIG_GPTournament> *  tours, int PosBest);
	
	/* Methode dient für Tournament.run(Classifer)
	* führt Turnier normal aus, also anhand der Fitness; 
	* erzeugt T-fall 
	*/
	void createNewTCase(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo, double FitDifference);
	
	/* Methode dient für Tournament.classifer(Classifer)
	* führt Turnier  mit Hilfe des Classifier aus !
	* Tournament.classifer(Classifer) wird über preEvolution aufgerufen  
	*/
	double classifer(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo);	
	

private:

	MT_TranslatedIndividual* createDoubleTransIndi(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo);

	// rückgabewert gibt aufschluss wie viele Sigel Turnier klassifiziert anstatt per exakte Fitness
	// ausgeführt werden sollen 
	int evaluationTactic(int ToursSize);


	// zur Bestimmung an welchen Turnieren das beste Sigel Individuum teilnimmt; 
	// Ändert ToursWBestIndi; ToursWBestIndi[i] == 1 genau dann wenn in Turnier i das beste Sigel Individuum
	// vorkommt! Ausnahme: kommt das beste Individuum in keine Turnier vor, so wird 
	// ToursWBestIndi[tours.size-1] == 1 trotzdem gesetzt, so dass das letzte Turnier nicht mit dem Klassi
	// durchgeführt wird, sondern normal durch exakte Fitness; dient der Kalibrierung!
	//
	int evalNeededTours(QVector<SIGEL_GP::SIG_GPTournament> *  tours, QArray<int> * ToursWBestIndi, int PosBest);
};

#endif // !defined(AFX_MT_CLASSIFIER_H__22632809_B47E_418D_8C86_EDE8B998506D__INCLUDED_)
