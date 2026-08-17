// MT_FitnessTranier.cpp: Implementierung der Klasse MT_FitnessTranier.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_FitnessTranier.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_FitnessTranier::MT_FitnessTranier(QTextStream &File)
{
	LastError=0;

	QString FitTranier( "FitnessTranier:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != FitTranier) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == FitTranier) && !(File.atEnd()))
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



MT_FitnessTranier::MT_FitnessTranier()
{
	LastError=0;

	FitnessFunction=1;
	PresentTSize =0;
	TSetSize=10;
	Result.resize(TSetSize);
	ResultIst.resize(TSetSize);
	
	TSet = new MT_Trainingset(TSetSize,1);
}

MT_FitnessTranier::~MT_FitnessTranier()
{
	delete TSet;

}


//////////////////////////////////////////////////////////////////////
// weitere Methoden
//////////////////////////////////////////////////////////////////////

void MT_FitnessTranier::loadSetup(QTextStream &File)
{
	
	QString FitTranier( "FitnessTranier:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != FitTranier) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == FitTranier) && !(File.atEnd()))
	{	
		FitnessFunction=(File.readLine()).toInt();
		int NewPresentTSize =(File.readLine()).toInt();
		int NewTSetSize=(File.readLine()).toInt();
		Interpreter.setDuration((File.readLine()).toInt());
		Interpreter.setVariableNumber((File.readLine()).toInt());
	
		TSet->changeTSize(NewTSetSize);
		Result.resize(TSetSize);
		ResultIst.resize(TSetSize);
	}
	else
		LastError =602;

}

void MT_FitnessTranier::writeToFileSetup(QTextStream &File)
{
	File << ("FitnessTranier:\n");
	File << FitnessFunction << endl;
	File << PresentTSize << endl;
	File << TSetSize << endl;
	File << Interpreter.getDuration() << endl;
	File << Interpreter.getVariableNumber() << endl << endl;
}


void MT_FitnessTranier::writeToFileTrainer(QTextStream &File)
{
	
	File << ("FitnessTranier:\n");
	File << FitnessFunction << endl;
	File << PresentTSize << endl;
	File << TSetSize << endl;
	File << Interpreter.getDuration() << endl;
	File << Interpreter.getVariableNumber() << endl << endl;

	TSet->writeToFileTSet(File);

}

int MT_FitnessTranier::getLastError()
{
	return LastError;
}

void MT_FitnessTranier::getSelektionValue(int *FitFunction, int *TDuration, int *TSize)
{
	*FitFunction = FitnessFunction;
	*TDuration = Interpreter.getDuration();
	*TSize = TSetSize;
	
}
int MT_FitnessTranier::getTDuration()
{
	return Interpreter.getDuration();

}

void MT_FitnessTranier::setSelektionValue(int FitFunction, int TDuration, int TSize)
{

	FitnessFunction =FitFunction;
	TSetSize =TSize;
	TSet->changeTSize(TSize);
	Result.resize(TSize);
	ResultIst.resize(TSize);
	Interpreter.setDuration(TDuration);
	
	PresentTSize = TSet->getPresentTSize(); 
	
}

void MT_FitnessTranier::setNumberOfVariables(int varNumber)
{
	Interpreter.setVariableNumber(varNumber);
}

int MT_FitnessTranier::insertNewTCases(QQueue<MT_TrainingCase> *NewTCase)
{
	TSet->updateTSet(NewTCase);
	
	return TSet->getPresentTSize();

}


void MT_FitnessTranier::calculateFitness(MT_Population *Pop)
{

	MT_Individual *PresentIndi;

	for (int i=0; i<Pop->getSize(); i++)
	{
		PresentIndi = Pop->getIndividual(i);
		calculateFitness(PresentIndi);
	}
}


bool MT_FitnessTranier::calculateFitness(MT_Individual *Indi)
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

double MT_FitnessTranier::fitSquareError()
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

double MT_FitnessTranier::fitSimpleError()
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

int MT_FitnessTranier::getPresentTSize()
{
	return PresentTSize;
}

QArray<double> * MT_FitnessTranier::getResultIstArray()
{
	return &ResultIst;
}

QArray<double> * MT_FitnessTranier::getResultArray()
{
	return &Result;
}


double MT_FitnessTranier::simpleYesNo()
{
	double Fitness=0.0;

	for (int i=0; i<PresentTSize; i++)
		if( ((ResultIst[i]<0.0)&&(Result[i]<0.0)) || ((ResultIst[i]>=0.0)&&(Result[i]>=0.0)) )
			Fitness++;


	Fitness = Fitness/PresentTSize;
	Fitness = Fitness * 1000.0;

	return Fitness;
}

double MT_FitnessTranier::weightYesNo()
{
	double Fitness=0.0;
	double DiffResult =0.0;
	double TotalWeight =0.0;

	for (int i=0; i<PresentTSize; i++)
	{
		DiffResult = fabs(Result[i]);
		// Erinnerung Result[i] enthält Fitnessdifferenz der beiden Sigel Individuen
	
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
