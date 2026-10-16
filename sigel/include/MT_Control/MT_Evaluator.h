#ifndef MT_CONTROL_MT_EVALUATOR_H
#define MT_CONTROL_MT_EVALUATOR_H

#include <QList>
#include "MT_Control/MT_Substitute.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"

using namespace SIGEL_GP;

class MT_Evaluator : public MT_Substitute, public SIG_GPFitnessTrainer
{
public:

	// administrative method 
	MT_Evaluator(SIGEL_GP::SIG_GPExperiment& exp, QTextStream &File);
	~MT_Evaluator() override = default;
	void loadSetup(QTextStream & File) override;
   void writeToFile(QTextStream & File) override;
	void writeToFileSetup(QTextStream & File) override;

	// see also SIGEL_GP::SIG_GPFitnessTrainer
	int spawnTask(SIGEL_GP::SIG_GPIndividual const& ind) override;
	// see also SIGEL_GP::SIG_GPFitnessTrainer
	double checkTask(int task) override;
	
private:
	
	// for internal use 
	QList<MT_TrainingCase *> TmpBuffer;
	QList<double> MT_ResultBuffer; 
	int MT_ResultSize;
	int NextFreePos;

	/* Create a training case from a SIG_Program.  Fitness is a -1 placeholder
	* until checkTask() gets the PVM result; it matches this case by getName(),
	* which is why PVMTaskID is stored as the Name.
	*/
	MT_TrainingCase * createNewTCase(SIGEL_Program::SIG_Program const * SIGProg, int PVMTaskID);
	
	/*The calibration strategy lives here; if true, MetaProg is used for fitness evaluation
	* of the SIGEL individual, otherwise SIGEL runs "normally"
	*/
	/* EstimationStrategy: if true = the BestMETAProgram will use for estimate the Fitness of the SigelProgram
	* false = the normal SIGEL method will be used to estimate the fitness
	*/
	bool evaluationTactic();
};

#endif // MT_CONTROL_MT_EVALUATOR_H
