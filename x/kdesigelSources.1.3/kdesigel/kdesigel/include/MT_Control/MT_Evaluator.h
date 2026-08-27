// MT_Evaluator.h: interface for class MT_Evaluator.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_)
#define AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_


#include "compat/q2compat.h"
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
	Q2PtrList<MT_TrainingCase> TmpBuffer;
	QList<double> MT_ResultBuffer; 
	int MT_ResultSize;
	int NextFreePos;

	
	/* Create a training case from a SIG_Program.  Fitness is a -1 placeholder
	* until checkTask() gets the PVM result; it matches this case by getName(),
	* which is why PVMTaskID is stored as the Name.
	*/
	MT_TrainingCase * createNewTCase(SIGEL_Program::SIG_Program * SIGProg, int PVMTaskID);
	
	/*The calibration strategy lives here; if true, MetaProg is used for fitness evaluation
	* of the SIGEL individual, otherwise SIGEL runs "normally"
	*/
	/* EstimationStrategy: if true = the BestMETAProgram will use for estimate the Fitness of the SigelProgram
	* false = the normal SIGEL method will be used to estimate the fitness
	*/
	bool evaluationTactic();

};

#endif // !defined(AFX_MT_EVALUATOR_H__090E0ACB_93A0_4093_95C2_57A10AEBBAE5__INCLUDED_)
