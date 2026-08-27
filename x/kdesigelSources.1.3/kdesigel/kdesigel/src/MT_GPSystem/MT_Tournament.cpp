// MT_Tournament.cpp: implementation of class MT_Tournament.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Tournament.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktionfurther method 
//////////////////////////////////////////////////////////////////////

MT_Tournament::MT_Tournament()
{

}

MT_Tournament::~MT_Tournament()
{
}

MT_Tournament::MT_Tournament(int Size, int num)
{
	Individuals.resize(Size);
	for(int i=0; i<Size; i++)
		Individuals[i]=0;

	NumberOfWinner=num;
	LastError =0;
	FirstFreePos =num;
	PopSize =Size;
	TypOfIndividual = 0;
}


//////////////////////////////////////////////////////////////////////
//further method 
//////////////////////////////////////////////////////////////////////

void MT_Tournament::setTypOfIndividual(int Typ)
{
	TypOfIndividual = Typ;
}

void MT_Tournament::ranking(QList<int> * WinnerLoser)
{
	int WinnerLoseSize = WinnerLoser->size();

	for (int i=0; i<WinnerLoseSize; i++)
		(*WinnerLoser)[i]=0;


	double PresentFit =0.0;
	double SmallestFit =0.0;
	int PosSmallestFit =0;
	
	QList<double> FitValue;	// Gives the fitness of the individual at position Pos[i]
	FitValue.resize(NumberOfWinner);
	for (int i=0; i<NumberOfWinner; i++)
		FitValue[i]=0.0;

	QList<int> Position;	// Gives the individual's position in the population
	Position.resize(NumberOfWinner);
	for (int i=0; i<NumberOfWinner; i++)
		Position[i]=0;

	// Find the NumberOfWinner fittest individuals 
	for (int i=0; i<PopSize; i++)
	{
		PresentFit = Individuals[i]->getFitness();
		
		// Find the smallest fitness in FitValue and its position

		PosSmallestFit = 0;
		SmallestFit = FitValue[0];
		for(int k=1; k<NumberOfWinner; k++)
		{
			if (SmallestFit>FitValue[k])
			{
				PosSmallestFit = k;
				SmallestFit = FitValue[k];
				if (SmallestFit==0.0)
					break;
			}
		}

		if (PresentFit>SmallestFit)
		{
			FitValue[PosSmallestFit]=PresentFit;
			Position[PosSmallestFit]=i;
		}
	}

	for(int k=0; k<NumberOfWinner; k++)
		(*WinnerLoser)[Position[k]]=1;
}



void MT_Tournament::fitnessProp(MT_Randomizer* Randi, QList<int> * WinnerLoser)
{

	int WinnerLoseSize = WinnerLoser->size();
	for (int i=0; i<WinnerLoseSize; i++)
		(*WinnerLoser)[i]=0;

	int Winner=0;
	double FitTemp;
	
double DebugInfo = 0.0;
int DebugIn = 1;

	QList<int> ProporFit;
	ProporFit.resize(PopSize);
	
	if (TypOfIndividual == 0)		// Evaluator Meta System;
	{
		for(int i=0; i<PopSize; i++)
		{
			FitTemp = 100.0*Individuals[i]->getFitness();	
			ProporFit[i]= FitTemp;
			if (ProporFit[i]<=0)
				ProporFit[i]=1;
		}
	}
	else						// Classifier Meta System;
	{
		for(int i=0; i<PopSize; i++)
		{
	DebugInfo = Individuals[i]->getFitness();
		
			ProporFit[i]= Individuals[i]->getFitness();

	DebugIn = ProporFit[i];
	
			ProporFit[i]= 	ProporFit[i]-500; 
			
		
	DebugIn = ProporFit[i];

			if (ProporFit[i]<=0)
				ProporFit[i]=1;
		}
				
	}

	
	// Determine the winners using the randomizer
int DebugNumber = 0;
QList<int> DebugArray;

	for(int i=0; i<NumberOfWinner; i++)
	{
		Winner = Randi->getProportionalWinner(&ProporFit);
		if ((*WinnerLoser)[Winner] !=0)
		{
			DebugNumber =DebugNumber;
		}
		
		(*WinnerLoser)[Winner]=1;
		// A winner may not take part in any further tournament
		ProporFit[Winner] =0;
		
		DebugNumber ++;
	}

	if (DebugNumber != NumberOfWinner )
	{
			DebugNumber =DebugNumber;
	}
		


}
