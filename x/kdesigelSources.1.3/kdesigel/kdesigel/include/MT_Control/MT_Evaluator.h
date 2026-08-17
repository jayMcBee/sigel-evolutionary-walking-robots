// MT_Evaluator.h: Schnittstelle für die Klasse MT_Evaluator.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_)
#define AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_


#include "MT_Control/MT_Substitute.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
//#include <pvm3.h>

using namespace SIGEL_GP;

class MT_Evaluator : public MT_Substitute, public SIG_GPFitnessTrainer
{
public:

	// administrative method 
	MT_Evaluator(SIGEL_GP::SIG_GPExperiment& exp, QTextStream &File);
	virtual ~MT_Evaluator();
	void loadSetup(QTextStream & File);
   void writeToFile(QTextStream & File);
	void writeToFileSetup(QTextStream & File);

	// see also SIGEL_GP::SIG_GPFitnessTrainer
	virtual int spawnTask(SIGEL_GP::SIG_GPIndividual const& ind);
	// see also SIGEL_GP::SIG_GPFitnessTrainer
	virtual double checkTask(int task);
	
private:
	
	// for internal use 
	QList<MT_TrainingCase> TmpBuffer;
	QArray<double> MT_ResultBuffer; 
	int MT_ResultSize;
	int NextFreePos;

	
	/*create a new TCase from a SIG_Program; 
	* Attention: there isn't a valid Fitness,
	* this mus be setting in checkTask   
	*/
	MT_TrainingCase * createNewTCase(SIGEL_Program::SIG_Program * SIGProg, int PVMTaskID);
	
	/*hier findet sich die Kalibrierungstratgie wieder; falls true wird MetaProg zur Fitnessbewertung
	* des SIGEL Individuums benutzt, sonst Sigel "normal"
	*/
	/* EstimationStrategy: if true = the BestMETAProgram will use for estimate the Fitness of the SigelProgram
	* false = the normal Sigel methode will use to estimate the Fitness
	*/
	bool evaluationTactic();

};

#endif // !defined(AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_)
