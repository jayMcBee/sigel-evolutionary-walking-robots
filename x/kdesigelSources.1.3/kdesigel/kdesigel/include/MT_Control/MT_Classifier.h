// MT_Classifier.h: interface for class MT_Classifier.
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
#include "MT_GPSystem/MT_TranslatedIndividual.h"	// Added from the class view



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
	
	/* Method serves Tournament.run(Classifier)
	* Runs the tournament normally, i.e. on exact fitness; 
	* creates a training case 
	*/
	void createNewTCase(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo, double FitDifference);
	
	/* Method serves Tournament.classifier(Classifier)
	* Runs the tournament using the classifier.
	* Tournament.classifier(Classifier) is called via preEvolution  
	*/
	double classifier(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo);	
	

private:

	MT_TranslatedIndividual* createDoubleTransIndi(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo);

	// Return value indicates how many SIGEL tournaments should be classified rather than run by exact fitness
	//  
	int evaluationTactic(int ToursSize);


	// Determines which tournaments the best SIGEL individual takes part in 
	// Modifies ToursWBestIndi; ToursWBestIndi[i] == 1 exactly when the best SIGEL individual appears
	// in tournament i. Exception: if the best individual appears in no tournament, 
	// ToursWBestIndi[tours.size-1] == 1 is set anyway, so that the last tournament is not run with the
	// classifier but normally by exact fitness. This serves as calibration.
	//
	int evalNeededTours(QVector<SIGEL_GP::SIG_GPTournament> *  tours, QArray<int> * ToursWBestIndi, int PosBest);
};

#endif // !defined(AFX_MT_CLASSIFIER_H__22632809_B47E_418D_8C86_EDE8B998506D__INCLUDED_)
