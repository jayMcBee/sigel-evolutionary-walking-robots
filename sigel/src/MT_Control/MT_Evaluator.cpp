#include <QList>
#include <QString>
#include <QTextStream>
#include <memory>
#include "MT_Control/MT_Evaluator.h"

// administrative method

MT_Evaluator::MT_Evaluator(SIGEL_GP::SIG_GPExperiment& exp, QTextStream &File) : MT_Substitute(), SIGEL_GP::SIG_GPFitnessTrainer(exp) 
{
	Typ=1;
	AverageSigelFitness = 0.01; 
	CorrectFitness.resize(0);
	AssumedFitness.resize(0);
	MetaProgError =-1.0;
	NumOfMeta =0;

	MT_ResultBuffer.resize(20);
	MT_ResultSize=20;
	for (int i=0; i<MT_ResultSize;i++)
		MT_ResultBuffer[i]=-1;
	NextFreePos =0;

	QString Evaluator( "Evaluator:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Evaluator) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == Evaluator) && !(File.atEnd()))
	{
		EstimationStrategy = (File.readLine()).toInt();
		Tolerance = (File.readLine()).toDouble();
		RefreshInterval = (File.readLine()).toInt();
		GenerationNumber = (File.readLine()).toUInt();

		if(GenerationNumber !=0)
		{
			NumOfCorrectEstimation.resize(GenerationNumber+1);
			NumOfMetaEstimation.resize(GenerationNumber+1);
						
			for(int i=0; i< GenerationNumber; i++)
			{
				NumOfCorrectEstimation[i] = (File.readLine()).toUInt();
				NumOfMetaEstimation[i] = (File.readLine()).toUInt();
			}
			NumOfCorrectEstimation[GenerationNumber] = 0;
			NumOfMetaEstimation[GenerationNumber] = 0;
		}
		else
		{
			NumOfCorrectEstimation.resize(100);
			NumOfMetaEstimation.resize(100);
			for(int i=0; i< 100; i++)
			{
				NumOfCorrectEstimation[i] = 0;
				NumOfMetaEstimation[i] = 0;
			}
		}
	}	
}

void MT_Evaluator::writeToFile(QTextStream & File)
{
	File << ("Evaluator:\n");
	File << EstimationStrategy <<Qt::endl;
	File << Tolerance <<Qt::endl;
	File << RefreshInterval <<Qt::endl;
	File << GenerationNumber <<Qt::endl;

	for(int i=0; i< GenerationNumber; i++)
		{
			File << NumOfCorrectEstimation[i] <<Qt::endl;
			File << NumOfMetaEstimation[i] << Qt::endl;
		}
}

void MT_Evaluator::writeToFileSetup(QTextStream & File)
{
	File << ("Evaluator:\n");
	File << EstimationStrategy <<Qt::endl;
	File << Tolerance <<Qt::endl;
	File << RefreshInterval <<Qt::endl;
	int GenNum =0;
	File << GenNum <<Qt::endl;
}
	
void MT_Evaluator::loadSetup(QTextStream &File)
{
	QString Evaluator( "Evaluator:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Evaluator) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == Evaluator) && !(File.atEnd()))
	{
		EstimationStrategy = (File.readLine()).toInt();
		Tolerance = (File.readLine()).toDouble();
		RefreshInterval = (File.readLine()).toInt();
	}
}

// functionally method

MT_TrainingCase * MT_Evaluator::createNewTCase(SIGEL_Program::SIG_Program const * SIGProg, int PVMTaskID)
{
	MT_TrainingCase * NewTCase;
	MT_TranslatedIndividual *TransIndi = translatedSIGProg(SIGProg); 
	
	NewTCase = new MT_TrainingCase(-1, TransIndi, PVMTaskID, -1);
 
	return NewTCase;
}

bool MT_Evaluator::evaluationTactic()
{
	bool UseMeta = false;
	
	if (BestMETAProgram ==nullptr)
		return UseMeta;

	switch (EstimationStrategy)
	{
	case 1:
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size();i++)
					MetaProgError = MetaProgError + fabs(AssumedFitness[i]-CorrectFitness[i]);
				
				MetaProgError = MetaProgError/ CorrectFitness.size();
			}

			if(MetaProgError< Tolerance)

			{
				if(NumOfMeta<RefreshInterval)
				{
					UseMeta =true;
					NumOfMeta++;
				}
				else
				{
					UseMeta =false;
					NumOfMeta =0;
				}
			}
			else
			{
				UseMeta =false;
				NumOfMeta =0;
			}
		}
		break;

	case 2: 
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				int NumOfWorseError = CorrectFitness.size()/10;
				double PresentError =0.0;
				int PosOfSmallestError =0;
				QList<double> WorseError;
				WorseError.resize(NumOfWorseError);
				// 2003 zero-filled to CorrectFitness.size() into an array that holds
				// only NumOfWorseError entries -- every iteration past the tenth
				// wrote out of range.
				for (int i=0; i<NumOfWorseError; i++)
					WorseError[i] = 0.0;
				
				for (int i=0; i<CorrectFitness.size();i++)
				{
					PresentError = fabs(AssumedFitness[i]-CorrectFitness[i]);
					MetaProgError = MetaProgError + PresentError;
					
					for(int k=0; k<NumOfWorseError; k++)
					{
						// 2003 used i, the OUTER loop variable, to index WorseError.
						// k is the index this loop scans.
						if (WorseError[PosOfSmallestError]>WorseError[k])
							PosOfSmallestError = k;
					}

					if (WorseError[PosOfSmallestError]<PresentError)
						WorseError[PosOfSmallestError]=PresentError;
				}

				PresentError = WorseError[0];
				for(int k=1; k<NumOfWorseError; k++)
					// 2003 wrote WorseError[i]. Pre-standard for-scope left i at
					// CorrectFitness.size() from the loop above, and WorseError holds
					// only size()/10 entries, so this read past the end every time.
					// k is plainly what the loop means.
					PresentError = PresentError + WorseError[k];
			
				MetaProgError = (MetaProgError-PresentError)/ (CorrectFitness.size()-NumOfWorseError);
			}

			if(MetaProgError< Tolerance)
			{
				if(NumOfMeta<RefreshInterval)
				{
					UseMeta =true;
					NumOfMeta++;
				}
				else
				{
					UseMeta =false;
					NumOfMeta =0;
				}
			}
			else
			{
				UseMeta =false;
				NumOfMeta =0;
			}
		}
		break;
		
	case 9:
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size();i++)
					MetaProgError = MetaProgError + fabs(AssumedFitness[i]-CorrectFitness[i]);
				
				MetaProgError = MetaProgError/ CorrectFitness.size();
			}

			double ToleranceNew = Tolerance*10000.0;  // 1 < ToleranceNew < 10000
			double SigelGeneration = GenerationNumber;
			if (SigelGeneration <1.0)
				SigelGeneration =1.0;

			// Tolerance is weighted by generation count
			if (SigelGeneration >5.0 && ToleranceNew<500.0 && SigelGeneration <= 150.0) // 500 = 0.05*10000;
			{
				double Rate =  (500.0 - ToleranceNew) / 145.0;
				if (Rate >0.0)
				{
					ToleranceNew = 500.0 - Rate*SigelGeneration;
				}
			}

			ToleranceNew = ToleranceNew/10000.0; 

			if(MetaProgError< ToleranceNew)
			{
				if(NumOfMeta<RefreshInterval)
				{
					UseMeta =true;
					NumOfMeta++;
				}
				else
				{
					UseMeta =false;
					NumOfMeta =0;
				}
			}
			else
			{
				UseMeta =false;
				NumOfMeta =0;
			}
		}
		break;

	case 10:
		{
		if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size();i++)
					MetaProgError = MetaProgError + fabs(AssumedFitness[i]-CorrectFitness[i]);
				
				MetaProgError = MetaProgError/ CorrectFitness.size();
			}

			double ToleranceNew = Tolerance*10000.0;  // 1 < ToleranceNew < 10000

			if ((AverageSigelFitness>0.1) && (AverageSigelFitness<0.7) && (ToleranceNew<500.0)) // 500 = 0.05*10000;
			{
				double ASigelFit = AverageSigelFitness - 0.1;
				double Rate =  (500.0 - ToleranceNew) /0.6;

					if (Rate >0.0)
					{
						ToleranceNew = 500.0 - Rate*ASigelFit;
					}
			}

			ToleranceNew = ToleranceNew/10000.0; 

			if(MetaProgError< ToleranceNew)
			{
				if(NumOfMeta<RefreshInterval)
				{
					UseMeta =true;
					NumOfMeta++;
				}
				else
				{
					UseMeta =false;
					NumOfMeta =0;
				}
			}
			else
			{
				UseMeta =false;
				NumOfMeta =0;
			}
		}
		break;

	case 13: UseMeta=false; break;
	}

	return UseMeta;
}

int MT_Evaluator::spawnTask(SIGEL_GP::SIG_GPIndividual const& ind)
{
	int MetaTaskID =0;
	
	bool useMeta=false;

	pthread_mutex_lock(&fitnessMutex);

	useMeta = evaluationTactic();

	pthread_mutex_unlock(&fitnessMutex);

	if (useMeta)
	{
		// lock the interpreter to prevent exchange of the used program
		pthread_mutex_lock(&interpreterMutex);

		std::unique_ptr< MT_TranslatedIndividual > TransIndi( translatedSIGProg(&ind.getProgram()) );
		double MetaEstimationResult = Interpreter.interpret( TransIndi.get() );

		// unlock the interpreter so that the program can be updated
		pthread_mutex_unlock(&interpreterMutex);

		if (MetaEstimationResult <= 0.0)
			MetaEstimationResult = 0.00000001;

		MetaEstimationResult = (MetaEstimationResult + 2.0) * -1.0;

		if(MT_ResultBuffer[NextFreePos] !=-1)
		{
			qsizetype freeSlot = MT_ResultBuffer.indexOf(-1);
			if(freeSlot >= 0)
				NextFreePos = freeSlot;
			else
			{
				MT_ResultBuffer.resize(MT_ResultSize*2, -1.0);
				NextFreePos = MT_ResultSize;
				MT_ResultSize = MT_ResultBuffer.size();
			}
		}

		MT_ResultBuffer[NextFreePos] = MetaEstimationResult;
		MetaTaskID = (NextFreePos+2)*(-1);
		
		NextFreePos ++;
		if (NextFreePos == MT_ResultSize)
			NextFreePos =0;

		NumOfMetaEstimation[GenerationNumber]=NumOfMetaEstimation[GenerationNumber]+1 ;

		return MetaTaskID;	
	}
	else
	{
		MetaTaskID = SIG_GPFitnessTrainer::spawnTask(ind);
		
		TmpBuffer.append(createNewTCase(&ind.getProgram(), MetaTaskID));
	
		return MetaTaskID;
	}
}

double MT_Evaluator::checkTask(int taskId)
{
	double MetaFitness =0.0;

	if(taskId <-1)
	{
		// Fitness was determined using the meta program
		int Index = (taskId *(-1))-2;
		MetaFitness = MT_ResultBuffer[Index];
		MT_ResultBuffer[Index] = -1.0;
		
		return MetaFitness;
	}
	else
	{
		MetaFitness =  SIG_GPFitnessTrainer::checkTask(taskId);
		if(MetaFitness != -1)
		{
			NumOfCorrectEstimation[GenerationNumber]=NumOfCorrectEstimation[GenerationNumber]+1;
		
			MT_TrainingCase * TCases;
			int TmpBufferSize = int( TmpBuffer.size() );

			for (int i=0; i< TmpBufferSize; i++) 
			{
				TCases=TmpBuffer.takeAt(i);

				if(TCases->getName() == taskId)
				{
					TmpBufferSize--;
					TCases->setFitness(MetaFitness);

// lock the TCaseBuffer so that no TCases could be taken from it
					pthread_mutex_lock(&tCaseBufferMutex);
	
					TCaseBuffer.enqueue(TCases);

// unlock the TCaseBuffer
					pthread_mutex_unlock(&tCaseBufferMutex);
					break;
				}
				else
					TmpBuffer.insert(i,TCases);		
			}
		}
	
	return MetaFitness;
	}
}
