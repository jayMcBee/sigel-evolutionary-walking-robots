// MT_Tournament.cpp: Implementierung der Klasse MT_Tournament.
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

void MT_Tournament::ranking(QArray<int> * WinnerLoser)
{
	int WinnerLoseSize = WinnerLoser->size();

	for (int i=0; i<WinnerLoseSize; i++)
		(*WinnerLoser)[i]=0;


	double PresentFit =0.0;
	double SmallestFit =0.0;
	int PosSmallestFit =0;
	
	QArray <double> FitValue;	// gibt die Fitness des Individuum an der Position Pos[i] an...
	FitValue.resize(NumberOfWinner);
	for (i=0; i<NumberOfWinner; i++)
		FitValue[i]=0.0;

	QArray <int> Position;	// gibt die Position des Individuums in der Population an
	Position.resize(NumberOfWinner);
	for (i=0; i<NumberOfWinner; i++)
		Position[i]=0;

	// Suche die NumberOfWinner fittesten Individuen 
	for (i=0; i<PopSize; i++)
	{
		PresentFit = Individuals[i]->getFitness();
		
		// Suche nach der kleinsten Fitness im FitValue und die Position!

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



void MT_Tournament::fitnessProp(MT_Randomizer* Randi, QArray<int> * WinnerLoser)
{

	int WinnerLoseSize = WinnerLoser->size();
	for (int i=0; i<WinnerLoseSize; i++)
		(*WinnerLoser)[i]=0;

	int Winner=0;
	double FitTemp;
	
double DebugInfo = 0.0;
int DebugIn = 1;

	QArray <int> ProporFit;
	ProporFit.resize(PopSize);
	
	if (TypOfIndividual == 0)		// Evaluator Meta System;
	{
		for(i=0; i<PopSize; i++)
		{
			FitTemp = 100.0*Individuals[i]->getFitness();	
			ProporFit[i]= FitTemp;
			if (ProporFit[i]<=0)
				ProporFit[i]=1;
		}
	}
	else						// Classifier Meta System;
	{
		for(i=0; i<PopSize; i++)
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

	
	// Ermittlung der Gewinner mit Hilfe des Randis
int DebugNumber = 0;
QArray<int> DebugArray;

	for(i=0; i<NumberOfWinner; i++)
	{
		Winner = Randi->getProportionalWinner(&ProporFit);
		if ((*WinnerLoser)[Winner] !=0)
		{
			DebugNumber =DebugNumber;
		}
		
		(*WinnerLoser)[Winner]=1;
		// Sieger darf nicht mehr am weiterem Turnier teilnehmen;
		ProporFit[Winner] =0;
		
		DebugNumber ++;
	}

	if (DebugNumber != NumberOfWinner )
	{
			DebugNumber =DebugNumber;
	}
		


}
