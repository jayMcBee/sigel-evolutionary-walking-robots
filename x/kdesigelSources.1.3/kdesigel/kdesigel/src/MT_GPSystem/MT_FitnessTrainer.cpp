// MT_FitnessTrainer.cpp: implementation of class MT_FitnessTrainer.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_FitnessTrainer.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_FitnessTrainer::MT_FitnessTrainer(QTextStream &File)
{
	LastError=0;

	// NOTE: "FitnessTranier:" keeps the 2003 misspelling deliberately -- it is a
	// section marker in the saved experiment format, not a name. Correcting it
	// would make this build unable to read experiments written by any other.
	QString FitTrainer( "FitnessTranier:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != FitTrainer) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == FitTrainer) && !(File.atEnd()))
	{	
		FitnessFunction=(File.readLine()).toInt();
		PresentTSize =(File.readLine()).toInt();
		TSetSize=(File.readLine()).toInt();
		Result.resize(TSetSize);
		ResultIst.resize(TSetSize);
		for (int i=0; i<TSetSize; i++)
		{
			Result[i] =0.0;
			ResultIst[i] =0.0;
		}

		Interpreter.setDuration((File.readLine()).toInt());
		Interpreter.setVariableNumber((File.readLine()).toInt());
	
		TSet = new MT_Trainingset(File);

	}
	else
		LastError =601;
}



MT_FitnessTrainer::MT_FitnessTrainer()
{
	LastError=0;

	FitnessFunction=1;
	PresentTSize =0;
	TSetSize=10;
	Result.resize(TSetSize);
	ResultIst.resize(TSetSize);
	
	TSet = new MT_Trainingset(TSetSize,1);
}

MT_FitnessTrainer::~MT_FitnessTrainer()
{
	delete TSet;

}


//////////////////////////////////////////////////////////////////////
// further methods
//////////////////////////////////////////////////////////////////////

void MT_FitnessTrainer::loadSetup(QTextStream &File)
{
	
	QString FitTrainer( "FitnessTranier:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != FitTrainer) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == FitTrainer) && !(File.atEnd()))
	{	
		FitnessFunction=(File.readLine()).toInt();
		int NewPresentTSize =(File.readLine()).toInt();
		int NewTSetSize=(File.readLine()).toInt();
		Interpreter.setDuration((File.readLine()).toInt());
		Interpreter.setVariableNumber((File.readLine()).toInt());
	
		// TSetSize was not updated here, so the training set took the
		// file's size while Result and ResultIst kept the old one -- any
		// setup file with a larger set made calculateFitness write past
		// both. Q2Array::at absorbed it by clamping to index 0; plain
		// QList does not. setSelektionValue below always did this right.
		TSetSize = NewTSetSize;
		TSet->changeTSize(TSetSize);
		Result.resize(TSetSize);
		ResultIst.resize(TSetSize);
	}
	else
		LastError =602;

}

void MT_FitnessTrainer::writeToFileSetup(QTextStream &File)
{
	File << ("FitnessTranier:\n");
	File << FitnessFunction << Qt::endl;
	File << PresentTSize << Qt::endl;
	File << TSetSize << Qt::endl;
	File << Interpreter.getDuration() << Qt::endl;
	File << Interpreter.getVariableNumber() << Qt::endl << Qt::endl;
}


void MT_FitnessTrainer::writeToFileTrainer(QTextStream &File)
{
	
	File << ("FitnessTranier:\n");
	File << FitnessFunction << Qt::endl;
	File << PresentTSize << Qt::endl;
	File << TSetSize << Qt::endl;
	File << Interpreter.getDuration() << Qt::endl;
	File << Interpreter.getVariableNumber() << Qt::endl << Qt::endl;

	TSet->writeToFileTSet(File);

}

int MT_FitnessTrainer::getLastError()
{
	return LastError;
}

void MT_FitnessTrainer::getSelektionValue(int *FitFunction, int *TDuration, int *TSize)
{
	*FitFunction = FitnessFunction;
	*TDuration = Interpreter.getDuration();
	*TSize = TSetSize;
	
}
int MT_FitnessTrainer::getTDuration()
{
	return Interpreter.getDuration();

}

void MT_FitnessTrainer::setSelektionValue(int FitFunction, int TDuration, int TSize)
{

	FitnessFunction =FitFunction;
	TSetSize =TSize;
	TSet->changeTSize(TSize);
	Result.resize(TSize);
	ResultIst.resize(TSize);
	Interpreter.setDuration(TDuration);
	
	PresentTSize = TSet->getPresentTSize(); 
	
}

void MT_FitnessTrainer::setNumberOfVariables(int varNumber)
{
	Interpreter.setVariableNumber(varNumber);
}

int MT_FitnessTrainer::insertNewTCases(QQueue<MT_TrainingCase *> *NewTCase)
{
	TSet->updateTSet(NewTCase);
	
	return TSet->getPresentTSize();

}


void MT_FitnessTrainer::calculateFitness(MT_Population *Pop)
{

	MT_Individual *PresentIndi;

	for (int i=0; i<Pop->getSize(); i++)
	{
		PresentIndi = Pop->getIndividual(i);
		calculateFitness(PresentIndi);
	}
}


bool MT_FitnessTrainer::calculateFitness(MT_Individual *Indi)
{
	bool NewEvaluated = false;

	if (false == Indi->toBeEvaluated(TSet->getName()))
	{

//double DebugInfo1=0.0;
//double DebugInfo2=0.0;
		NewEvaluated = true;
		double Fit= -1.5;
		if((Indi->getProgram()->getLength()) > 3)
		{
			PresentTSize = TSet->getPresentTSize();
			if (PresentTSize >0)
			{
				LastError = Interpreter.loadProgram((Indi->getProgram()));
				
				MT_TrainingCase * InstantTCase; 
			
				for (int i=0; i<PresentTSize; i++)
				{
					InstantTCase = TSet->getTCase(i); 
					
	//				DebugInfo1 = 

					Result[i] = InstantTCase->getFitness();


	//				DebugInfo2 = 
			
					ResultIst[i]= Interpreter.interpret(InstantTCase->getIndividual());
				}
				
				switch (FitnessFunction)
				{
				case 1:	Fit = fitSimpleError(); break;
				case 2: Fit = fitSquareError(); break;
				case 3: Fit = simpleYesNo(); break;
				case 4: Fit = weightYesNo(); break;
				default:
					{
						Fit = -999.0;
						LastError = 610;
					}
				}
			}
				Indi->setFitness(Fit);
				Indi->setTrainingsSet(TSet->getName());
		}
		else
		{
			Indi->setFitness(0.0);
			Indi->setTrainingsSet(TSet->getName());
		
		}
		
	}

	return NewEvaluated;
}

double MT_FitnessTrainer::fitSquareError()
{

	double Fitness=0.0;

	for (int i=0; i<PresentTSize; i++)	
		Fitness = Fitness + (ResultIst[i]-Result[i]) * (ResultIst[i]-Result[i]);

	Fitness = 1.0 / (1.0 + Fitness);
/*	
	Fitness *= 10000.0;
	
	if(Fitness <0.0)
		Fitness = 0.0;
	else 
		if(Fitness<0.0001)
			Fitness = 1000;
		else
			Fitness = 0.1/Fitness; 
*/
	return Fitness;
 
}

double MT_FitnessTrainer::fitSimpleError()
{
	double Fitness=0.0;

	// total error
	for (int i=0; i<PresentTSize; i++)
		Fitness = Fitness + fabs(ResultIst[i]-Result[i]);

	// fit error into interval [0, 1]
	Fitness = 1.0 / (1.0 + Fitness);

/*
	if(Fitness <0.0)
		Fitness = 0.0;
	else 
		if(Fitness<0.0001)
			Fitness = 1000;
		else
			Fitness = 0.1/Fitness;  
*/

	return Fitness;
}

int MT_FitnessTrainer::getPresentTSize()
{
	return PresentTSize;
}

QList<double> * MT_FitnessTrainer::getResultIstArray()
{
	return &ResultIst;
}

QList<double> * MT_FitnessTrainer::getResultArray()
{
	return &Result;
}


double MT_FitnessTrainer::simpleYesNo()
{
	double Fitness=0.0;

	for (int i=0; i<PresentTSize; i++)
		if( ((ResultIst[i]<0.0)&&(Result[i]<0.0)) || ((ResultIst[i]>=0.0)&&(Result[i]>=0.0)) )
			Fitness++;


	Fitness = Fitness/PresentTSize;
	Fitness = Fitness * 1000.0;

	return Fitness;
}

double MT_FitnessTrainer::weightYesNo()
{
	double Fitness=0.0;
	double DiffResult =0.0;
	double TotalWeight =0.0;

	for (int i=0; i<PresentTSize; i++)
	{
		DiffResult = fabs(Result[i]);
		// Reminder: Result[i] holds the fitness difference of the two SIGEL individuals
	
		if( ((ResultIst[i]<0.0)&&(Result[i]<0.0)) || ((ResultIst[i]>0.0)&&(Result[i]>0.0)) )
		{	
			if (DiffResult >0.0001)
				DiffResult = 0.1 / DiffResult;
			else
				DiffResult = 1000.0;

			Fitness= Fitness + DiffResult;
			TotalWeight = TotalWeight + DiffResult;
		}
		else
		{
			DiffResult = DiffResult*1000.0;
			Fitness= Fitness - DiffResult;
			TotalWeight = TotalWeight + DiffResult;
		}
	}
	

	if (TotalWeight <=0.0)
		TotalWeight =1.0;

	Fitness = Fitness/TotalWeight; // -1<= Fitness <=1
	Fitness = (Fitness+1.0)*500.0; // 0<=Fitness<=1000

	if (Fitness < 0.0)
		Fitness = 0.0;

	return  Fitness;

}
