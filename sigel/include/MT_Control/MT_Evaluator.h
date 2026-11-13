/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef MT_CONTROL_MT_EVALUATOR_H
#define MT_CONTROL_MT_EVALUATOR_H

#include <QList>
#include "MT_Control/MT_Substitute.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"

class MT_Evaluator : public MT_Substitute, public SIGEL_GP::SIG_GPFitnessTrainer
{
public:

	// administrative method 
	MT_Evaluator(SIGEL_GP::SIG_GPMetaGPExperiment& exp, QTextStream &File);
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
